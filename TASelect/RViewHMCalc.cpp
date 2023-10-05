#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "Utilities.h"
#include "SheetHMCalc.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "DlgCreateModule.h"
#include "DlgCreateModuleTiny.h"
#include "DlgLeftTabProject.h"
#include "DlgEditString.h"
#include "DlgHMDirSel.h"
#include "DlgAccessory.h"
#include "DlgWizCircuit.h"
#include "DlgInjectionError.h"

// These includes if to avoid to have them in 'RViewProj.h'
#include "SSheetLDSumm.h"
#include "SSheetLogData.h"
#include "SSheetPanelCirc2.h"
#include "SSheetQuickMeas.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewProj.h"

#include "RViewHMCalc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRViewHMCalc *pRViewHMCalc = NULL;

IMPLEMENT_DYNCREATE( CRViewHMCalc, CFormViewEx )

CRViewHMCalc::CRViewHMCalc(): CFormViewEx( CRViewHMCalc::IDD )
{
	m_pSheetHMCalc = NULL;
	m_bEnableOnDraw = true;
	m_bCopyDistribAccess = true;
	m_lRow = 0;
	m_lColumn = 0;
	m_iSpreadYPos = 0;
	m_pTAPForFullCatalog = NULL;
	m_bActivateContextMenu = true;
	m_eLastOperation = LastOperation::Undefined;
	pRViewHMCalc = this;
}

CRViewHMCalc::~CRViewHMCalc()
{
	if( NULL != m_pSheetHMCalc )
	{
		delete m_pSheetHMCalc;
	}

	if( NULL != TASApp.GetpTADS() && TASApp.GetpTADS()->GetCountHMClipboard() > 0 )
	{
		TASApp.GetpTADS()->CleanClipboard();
	}

	pRViewHMCalc = NULL;
}

BOOL CRViewHMCalc::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID,
						   CCreateContext *pContext )
{
	// Create the RViewHMCalc.
	if( FALSE == CFormViewEx::Create( lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, CRViewHMCalc::IDD, pContext ) )
	{
		return FALSE;
	}

	m_FltMenuTabProj.LoadMenu( IDR_FLTMENU_TABPROJ );
	m_FltMenuRVHMCalc.LoadMenu( IDR_FLTMENU_RVHMCALC );

	// Create the Spread Sheet
	m_pSheetHMCalc = new CSheetHMCalc();
	ASSERT( m_pSheetHMCalc );

	if( NULL != m_pSheetHMCalc )
	{
		if( FALSE == m_pSheetHMCalc->Create( ( GetStyle() | WS_CHILD ) | WS_VISIBLE ^ WS_BORDER, rect, this, IDC_FPSPREAD ) )
		{
			return FALSE;
		}
	}

	return TRUE;
}

#ifdef _DEBUG
void CRViewHMCalc::AssertValid() const
{
	CFormViewEx::AssertValid();
}

void CRViewHMCalc::Dump( CDumpContext &dc ) const
{
	CFormViewEx::Dump( dc );
}
#endif //_DEBUG

void CRViewHMCalc::Reset( void )
{
	if( NULL != m_pSheetHMCalc && NULL != m_pSheetHMCalc->GetSafeHwnd() )
	{
		m_pSheetHMCalc->Reset( true );
	}
}

void CRViewHMCalc::RedrawHM( CDS_HydroMod *pHM )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );
	m_pSheetHMCalc->SaveState();
	m_pSheetHMCalc->Reset();
	m_pSheetHMCalc->Init( pHM );
	m_pSheetHMCalc->RestoreState();
	m_pSheetHMCalc->SetFocus();
	CWnd::SetRedraw( TRUE );
	Invalidate();
	EndWaitCursor();
}

void CRViewHMCalc::EditHM( CDS_HydroMod *pHM )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	if( NULL != pHM )
	{
		m_pSheetHMCalc->UnSelectMultipleRows();
		m_pSheetHMCalc->SelectRow( pHM );
		m_pSheetHMCalc->EditHMRow();
	}
}

void CRViewHMCalc::SetLastCopyCutOperation( LastOperation eLastOperation )
{
	m_eLastOperation = eLastOperation;
}

CRViewHMCalc::LastOperation CRViewHMCalc::GetLastCopyCutOperation( void )
{
	return m_eLastOperation;
}

void CRViewHMCalc::SetFlagCut( std::vector<CDS_HydroMod *> *pvecHydromod )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() || NULL == pDlgLeftTabProject )
	{
		return;
	}

	pDlgLeftTabProject->SetFlagCut( pvecHydromod );

	if( NULL != m_pSheetHMCalc )
	{
		m_pSheetHMCalc->SetFlagCut( pvecHydromod );
	}
}

bool CRViewHMCalc::ResetFlagCut( void )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() || NULL == pDlgLeftTabProject )
	{
		return false;
	}

	// We set the flag only for the 'SheetHMCalc'. Because changes in the tree are directly repaint but it's not the
	// case with 'SheetHMCalc'. We need to force a paint to apply changes.
	bool fAtLeastOneItemChanged = false;

	pDlgLeftTabProject->ResetFlagCut();

	if( NULL != m_pSheetHMCalc )
	{
		fAtLeastOneItemChanged = m_pSheetHMCalc->ResetFlagCut();
	}

	return fAtLeastOneItemChanged;
}

bool CRViewHMCalc::IsFlagCutSet( void )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() || NULL == pDlgLeftTabProject )
	{
		return false;
	}

	bool fTabCDialogFlagCut = pDlgLeftTabProject->IsFlagCutSet();
	bool fSheetHMCalcFlagCut = ( NULL != m_pSheetHMCalc ) ? m_pSheetHMCalc->IsFlagCutSet() : false;
	return ( fTabCDialogFlagCut || fSheetHMCalcFlagCut );
}

bool CRViewHMCalc::IsObjectInCutMode( CData *pObject )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return false;
	}

	bool fIsInCutMode = false;

	if( NULL != m_pSheetHMCalc )
	{
		fIsInCutMode = m_pSheetHMCalc->IsObjectInCutMode( pObject );
	}

	return fIsInCutMode;
}

BEGIN_MESSAGE_MAP( CRViewHMCalc, CFormViewEx )
	ON_COMMAND( ID_TABPROJ_CREATEIN, OnFltMenuTabProjCreateIn )
	ON_COMMAND( ID_TABPROJ_CREATEBEFORE, OnFltMenuTabProjCreateBefore )
	ON_COMMAND( ID_TABPROJ_ADDVALVE, OnFltMenuTabProjAddValve )
	ON_COMMAND( ID_TABPROJ_EDIT, OnFltMenuTabProjEdit )
	ON_COMMAND( ID_TABPROJ_RENAME, OnFltMenuTabProjRename )
	ON_COMMAND( ID_TABPROJ_DELETE, OnFltMenuTabProjDelete )
	ON_COMMAND( ID_TABPROJ_CUT, OnFltMenuTabProjCut )
	ON_COMMAND( ID_TABPROJ_COPY, OnFltMenuTabProjCopy )
	ON_COMMAND( ID_TABPROJ_PASTEIN, OnFltMenuTabProjPasteIn )
	ON_COMMAND( ID_TABPROJ_PASTEBEFORE, OnFltMenuTabProjPasteBefore )
	ON_COMMAND( ID_TABPROJ_PRINT, OnFltMenuTabProjPrint )
	ON_COMMAND( ID_TABPROJ_PRINTRECURSIVE, OnFltMenuTabProjPrintRecursive )
	ON_COMMAND( ID_TABPROJ_EXPORT, OnFltMenuTabProjExport )
	ON_COMMAND( ID_TABPROJ_EXPORTRECURSIVE, OnFltMenuTabProjExportRecursive )
	ON_COMMAND( ID_TABPROJ_GETFULLINFO, OnFltMenuRVHMCalcGetFullinfo )

	ON_COMMAND( ID_FLTMRVHMCALC_ADDCIRCUIT, OnFltMenuRVHMCalcAddCircuit )
	ON_COMMAND( ID_FLTMRVHMCALC_EDIT, OnFltMenuRVHMCalcEdit )
	ON_COMMAND( ID_FLTMRVHMCALC_RENAME, OnFltMenuRVHMCalcRename )
	ON_COMMAND( ID_FLTMRVHMCALC_DELETE, OnFltMenuRVHMCalcDelete )
	ON_COMMAND( ID_FLTMRVHMCALC_CUT, OnFltMenuRVHMCalcCut )
	ON_COMMAND( ID_FLTMRVHMCALC_COPY, OnFltMenuRVHMCalcCopy )
	ON_COMMAND( ID_FLTMRVHMCALC_PASTEIN, OnFltMenuRVHMCalcPasteIn )
	ON_COMMAND( ID_FLTMRVHMCALC_PASTEBEFORE, OnFltMenuRVHMCalcPasteBefore )
	ON_COMMAND( ID_FLTMRVHMCALC_EDITACCESSORIES, OnFltMenuRVHMCalcEditAccessories )
	ON_COMMAND( ID_FLTMRVHMCALC_COPYACCESSORIES, OnFltMenuRVHMCalcCopyAccessories )
	ON_COMMAND( ID_FLTMRVHMCALC_PASTEACCESSORIES, OnFltMenuRVHMCalcPasteAccessories )
	ON_COMMAND( ID_FLTMRVHMCALC_LOCK, OnFltMenuRVHMCalcLock )
	ON_COMMAND( ID_FLTMRVHMCALC_UNLOCK, OnFltMenuRVHMCalcUnLock )
	ON_COMMAND( ID_FLTMRVHMCALC_LOCKCOLUMN, OnFltMenuRVHMCalcLockColumn )
	ON_COMMAND( ID_FLTMRVHMCALC_UNLOCKCOLUMN, OnFltMenuRVHMCalcUnlockColumn )
	ON_COMMAND( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT, OnFltMenuRVHMCalcLockAll )
	ON_COMMAND( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT, OnFltMenuRVHMCalcUnlockAll )
	ON_COMMAND( ID_FLTMRVHMCALC_PRINT, OnFltMenuRVHMCalcPrint )
	ON_COMMAND( ID_FLTMRVHMCALC_PRINTRECURSIVE, OnFltMenuRVHMCalcPrintRecursive )
	ON_COMMAND( ID_FLTMRVHMCALC_EXPORT, OnFltMenuRVHMCalcExport )
	ON_COMMAND( ID_FLTMRVHMCALC_EXPORTRECURSIVE, OnFltMenuRVHMCalcExportRecursive )
	ON_COMMAND( ID_FLTMRVHMCALC_GETFULLINFO, OnFltMenuRVHMCalcGetFullinfo )

	ON_UPDATE_COMMAND_UI_RANGE( ID_TABPROJ_CREATEIN, ID_TABPROJ_GETFULLINFO, OnUpdateMenuText )
	ON_UPDATE_COMMAND_UI_RANGE( ID_FLTMRVHMCALC_ADDCIRCUIT, ID_FLTMRVHMCALC_GETFULLINFO, OnUpdateMenuText )

	ON_WM_SIZE()
	ON_WM_KILLFOCUS()
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROYCLIPBOARD()
	ON_WM_DESTROY()

	// Spread DLL messages
	ON_MESSAGE( SSM_COMBODROPDOWN, ComboDropDown )
	ON_MESSAGE( SSM_DRAGDROP, DragDropBlock )
	ON_MESSAGE( SSM_EDITCHANGE, EditChange )
	ON_MESSAGE( SSM_LEAVECELL, LeaveCell )
	ON_MESSAGE( SSM_SHEETCHANGED, SheetChanged )
	ON_MESSAGE( SSM_TEXTTIPFETCH, TextTipFetch )
END_MESSAGE_MAP()

BOOL CRViewHMCalc::PreTranslateMessage( MSG *pMsg )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return FALSE;
	}

	// By default message should be dispatched.
	BOOL fReturn = FALSE;

	if( WM_KEYDOWN == pMsg->message )
	{
		if( GetKeyState( VK_LCONTROL ) < 0 && GetKeyState( VK_TAB ) < 0 )
		{
			m_pSheetHMCalc->ChangeSheet( ( GetKeyState( VK_SHIFT ) < 0 ) ? false : true );
			fReturn = TRUE;
		}
		else if( GetKeyState( VK_ESCAPE ) < 0 && LastOperation::Cut == GetLastCopyCutOperation() && true == IsFlagCutSet() )
		{
			ResetFlagCut();
			m_pTADS->CleanClipboard();
			SetLastCopyCutOperation( LastOperation::Undefined );

			if( NULL != m_pSheetHMCalc->GetpParentHM() )
			{
				RedrawHM( m_pSheetHMCalc->GetpParentHM() );
			}
		}
	}
	else if( WM_RBUTTONDOWN == pMsg->message )
	{
		// Spread shows a context menu (copy, paste, cut) when user right click in a cell while this one is in edit mode.
		// In the code that manage that, spread will send the message to the default procedure.
		// When user has finished his choice in the context menu, the WM_CONTEXTMENU message arrives in 'CRViewHMCalc::OnContextMenu'.
		// This method will show our internal context menu. And it's a problem.
		// To avoid that, when we receive a right click message, we verify if we are in edit mode. If it's the case, we set
		// 'm_fActivateContextMenu' to false to avoid to execute our 'OnContextMenu'. If we are not in edit mode, we reset all values of the
		// 'SS_RBUTTON' to avoid that spread manage this message and we set 'm_fActivateContextMenu' to true to allow to execute our
		// 'OnContextMenu'.

		if( NULL != m_pSheetHMCalc && true == m_pSheetHMCalc->IsEditModeActive() )
		{
			m_bActivateContextMenu = false;
		}
		else
		{
			m_bActivateContextMenu = true;
		}
	}

	if( FALSE == fReturn )
	{
		fReturn = CFormViewEx::PreTranslateMessage( pMsg );
	}

	return fReturn;

}

BOOL CRViewHMCalc::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pSheetHMCalc && NULL != m_pSheetHMCalc->GetSafeHwnd() && IDC_FPSPREAD == LOWORD( wParam ) &&
		( SSN_TOPLEFTCHANGE == HIWORD( wParam ) || SSN_TOPROWCHANGE == HIWORD( wParam ) || SSN_LEFTCOLCHANGE == HIWORD( wParam ) ) )
	{
		long lNewLeftColumn, lNewTopRow;
		m_pSheetHMCalc->GetTopLeftCell( &lNewLeftColumn, &lNewTopRow );
		m_pSheetHMCalc->TopLeftChangeFpspread( lNewLeftColumn, lNewTopRow );
	}

	return CFormViewEx::OnCommand( wParam, lParam );
}

void CRViewHMCalc::OnDraw( CDC *pDC )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() || TRUE == pDC->IsPrinting() )
	{
		return;
	}

	pDC->SetMapMode( MM_TEXT );
	int iYPos = m_iTopMargin;

	// FormView size.
	CRect PageRect;
	GetClientRect( ( LPRECT )PageRect );
	m_PrintRect = PageRect;

	if( PageRect.Width() < RIGHTVIEWWIDTH )
	{
		m_PrintRect.right = RIGHTVIEWWIDTH;
	}

	m_PrintRect.top = m_iSpreadYPos;
	m_PrintRect.bottom = m_PrintRect.top + m_pSheetHMCalc->GetSheetSizeInPixels().Height();

	// Fill background with white brush.
	CBrush BGBrush;
	BGBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );
	pDC->FillRect( CRect( 0, 0, m_PrintRect.right, m_PrintRect.bottom ), &BGBrush );

	// Draw page title.
	CDS_HydroMod *pHM = NULL;

	if( true == m_bEnableOnDraw )
	{
		pHM = m_pSheetHMCalc->GetpParentHM();
	}

	CString str = ( false == m_bEnableOnDraw || NULL == pHM ) ? _T(" ") : pHM->GetHMName();
	CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pPrjParam->IsFreezed() && true == pMainFrame->IsHMCalcChecked() )
	{
		CString	str1;
		str1 = TASApp.LoadLocalizedString( IDS_RVGENPARAM_FREEZED );
		str += _T(" ") + str1;
	}

	iYPos = DrawTitle( pDC, m_iLeftMargin, iYPos, str, m_TitleBGColor );
	iYPos += m_iLineSpace * m_Xfactor / 100;

	if( m_pSheetHMCalc->SMeasurement == m_pSheetHMCalc->GetActiveSheet() 
			|| m_pSheetHMCalc->STADiagnostic == m_pSheetHMCalc->GetActiveSheet() )
	{
		iYPos = DrawHMInfo( pDC, pHM, m_iLeftMargin, iYPos, m_PrintRect.Width(), true, true );
	}
	else
	{
		iYPos = DrawHMInfo( pDC, pHM, m_iLeftMargin, iYPos, m_PrintRect.Width(), true, false );
	}

	iYPos += m_iLineSpace * m_Xfactor / 100;

	// Reset the sheet position (because of Measurement tab sheet).
	if( INVALID_HANDLE_VALUE != m_pSheetHMCalc->GetSafeHwnd() )
	{
		if( m_pSheetHMCalc->SMeasurement == m_pSheetHMCalc->GetActiveSheet() 
				|| m_pSheetHMCalc->STADiagnostic == m_pSheetHMCalc->GetActiveSheet() )
		{
			m_iSpreadYPos = 152;
		}
		else
		{
			m_iSpreadYPos = 180;
		}

		m_iSpreadYPos = iYPos;
		::MoveWindow( m_pSheetHMCalc->GetSafeHwnd(), 0, m_iSpreadYPos, PageRect.Width(), PageRect.Height() - m_iSpreadYPos, TRUE );
	}

	m_pSheetHMCalc->Invalidate();
}

void CRViewHMCalc::OnSize( UINT nType, int cx, int cy )
{
	CFormViewEx::OnSize( nType, cx, cy );

	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CRect PageRect;
	GetClientRect( &PageRect );

	m_iLeftMargin = 3;

	if( m_pSheetHMCalc->GetSafeHwnd() )
	{
		if( m_pSheetHMCalc->SMeasurement == m_pSheetHMCalc->GetActiveSheet() )
		{
			m_iSpreadYPos = 152;
		}
		else
		{
			m_iSpreadYPos = 180;
		}

		::MoveWindow( m_pSheetHMCalc->GetSafeHwnd(), 0, m_iSpreadYPos, PageRect.Width(), PageRect.Height() - m_iSpreadYPos, TRUE );
	}
}

void CRViewHMCalc::OnKillFocus( CWnd *pNewWnd )
{
	CFormViewEx::OnKillFocus( pNewWnd );

	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	m_pSheetHMCalc->SetREDRAW( true );
}

void CRViewHMCalc::OnDestroyClipboard()
{
	OpenClipboard();
	HGLOBAL hData = ::GetClipboardData( CF_PRIVATEFIRST );

	if( hData != NULL )
	{
		GlobalFree( hData );
	}

	CloseClipboard();
}

void CRViewHMCalc::OnDestroy()
{
	OnDestroyClipboard();
}

void CRViewHMCalc::OnContextMenu( CWnd *pWnd, CPoint point )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() || false == m_bActivateContextMenu )
	{
		return;
	}

	// If there is no product where user has clicked, we don't display the context menu.
	long lColumn, lRow;

	if( NULL == m_pSheetHMCalc->GetHMUnderMouse( point, &lColumn, &lRow ) )
	{
		return;
	}

	// Remark: normally it's the 'RightClick' method that manages the saving of these two variables. But in a certain circumstance
	//         (in fact if user clicks just below the context menu) _TSpread doesn't call the 'RightClick' method. And then we are
	//         with the previous 'm_lColumn' and 'm_lRow' saved that are no more correct.
	m_lColumn = lColumn;
	m_lRow = lRow;

	CArray<long> arlProductSelected;
	int iNbrProductSelected = m_pSheetHMCalc->GetSelectedRows( &arlProductSelected );

	if( 0 == iNbrProductSelected )
	{
		return;
	}

	m_mapUpdateMenuTooltip.clear();

	int iNbrProductInClipboard = m_pTADS->GetCountHMClipboard();

	CDS_ProjectParams *pProjectParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pProjectParam );

	CDS_HydroMod *pHMRightClicked = m_pSheetHMCalc->GetSelectedHM();
	bool fIsRibbonHMCalcChecked = pMainFrame->IsHMCalcChecked();
	eBool3 eIsProjectHMCalc = m_pTADS->IsHMCalcMode( NULL, true );
	bool bIsProjectFrozen = pProjectParam->IsFreezed();

	// Check if there is at least one module child under the selected lines (allow to activate or not 'Print recursive' or
	// 'Export recursive' menu items).
	// Also check if all selected lines contains at least one module and contains at least one child.
	bool bChildModuleExist = false;
	bool bModuleExistInSelection = false;
	bool bCircuitExistInSelection = false;
	bool bParentInSelection = false;
	bool bIsTerminalUnit = false;
	CDS_HydroMod *pclFirstHMInSelection = NULL;

	for( int iLoopArray = 0; iLoopArray < iNbrProductSelected; iLoopArray++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( arlProductSelected.GetAt( iLoopArray ) );

		if( NULL == pHM )
		{
			continue;
		}

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->EnsureVisible( pHM );
		}

		if( false == pHM->IsaModule() )
		{
			bIsTerminalUnit = true;
		}

		if( NULL == pclFirstHMInSelection || pHM->GetLevel() < pclFirstHMInSelection->GetLevel() || pHM->GetPos() < pclFirstHMInSelection->GetPos() )
		{
			pclFirstHMInSelection = pHM;
		}

		if( pHM == m_pSheetHMCalc->GetpParentHM() )
		{
			bParentInSelection = true;
		}

		if( true == pHM->IsaModule() )
		{
			bModuleExistInSelection = true;

			for( IDPTR ChildIDPtr = pHM->GetFirst(); NULL != ChildIDPtr.MP && false == bChildModuleExist; ChildIDPtr = pHM->GetNext() )
			{
				CDS_HydroMod *pChildHM = dynamic_cast<CDS_HydroMod *>( ( CData * )ChildIDPtr.MP );

				if( NULL != pChildHM )
				{
					if( true == pChildHM->IsaModule() )
					{
						bChildModuleExist = true;
					}
				}
			}
		}
		else
		{
			bCircuitExistInSelection = true;
		}
	}

	// Two types of context menu can be displayed !!!!
	// It depends on the HMCalc Mode !!!
	if( false == fIsRibbonHMCalcChecked )
	{
		// We are in CBI mode.

		CDS_HydroMod *pParentHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpParentHM();

		if( NULL == pParentHM )
		{
			return;
		}

		if( NULL != pHMRightClicked && true == pHMRightClicked->IsForHub() )
		{
			return;
		}

		m_FltMenuTabProj.DestroyMenu();

		if( FALSE == m_FltMenuTabProj.LoadMenu( IDR_FLTMENU_TABPROJ ) )
		{
			return;
		}

		CMenu *pContextMenu = m_FltMenuTabProj.GetSubMenu( 0 );

		if( NULL == pContextMenu )
		{
			return;
		}

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
		str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EXPORTRECURSIVE );
		pContextMenu->ModifyMenu( ID_TABPROJ_EXPORTRECURSIVE, MF_BYCOMMAND, ID_TABPROJ_EXPORTRECURSIVE, str );
		str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EXPORT_CHART );
		pContextMenu->ModifyMenu( ID_TABPROJ_EXPORT_CHART, MF_BYCOMMAND, ID_TABPROJ_EXPORT_CHART, str );
		str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EXPORT_LDLIST );
		pContextMenu->ModifyMenu( ID_TABPROJ_EXPORT_LDLIST, MF_BYCOMMAND, ID_TABPROJ_EXPORT_LDLIST, str );
		str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_GETFULLINFO );
		pContextMenu->ModifyMenu( ID_TABPROJ_GETFULLINFO, MF_BYCOMMAND, ID_TABPROJ_GETFULLINFO, str );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Create module' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 		// For a next release
		// 		bool fAddCreateIn = false;
		// 		bool fAddCreateBefore = false;
		// 		CString strCreateIn( _T("") );
		// 		CString strCreateBefore( _T("") );
		// 		if( 1 == iNbrProductSelected )
		// 		{
		// 			if( true == pHMRightClicked->IsaModule() )
		// 			{
		// 				// If there is only one line selected and it's a module, user can create in.
		// 				FormatString( strCreateIn, IDS_TABPROJ_CREATEIN, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
		// 				fAddCreateIn = true;
		//
		// 				// By cons, if the module is the parent, user can't insert.
		// 				if( false == bParentInSelection )
		// 				{
		// 					FormatString( strCreateBefore, IDS_TABPROJ_CREATEBEFORE, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
		// 					fAddCreateBefore = true;
		// 				}
		// 			}
		// 			else
		// 			{
		// 				// If there is only one line selected and it's a circuit, user can only insert.
		// 				FormatString( strCreateBefore, IDS_TABPROJ_CREATEBEFORE, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
		// 				fAddCreateBefore = true;
		// 			}
		// 		}
		// 		else
		// 		{
		// 			if( true == bCircuitExistInSelection && false == fModuleExistInSelection )
		// 			{
		// 				// There are only circuit selected, user can only insert before the first one.
		// 				FormatString( strCreateBefore, IDS_TABPROJ_CREATEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
		// 				fAddCreateBefore = true;
		// 			}
		// 			else if( false == bCircuitExistInSelection && true == fModuleExistInSelection )
		// 			{
		// 				// There are only modules selected, user can create in each modules.
		// 				strCreateIn = TASApp.LoadLocalizedString( IDS_TABPROJ_CREATEINALLMODULES );
		// 				fAddCreateIn = true;
		//
		// 				// By cons, if at least one module is the parent, user can't insert.
		// 				if( false == bParentInSelection )
		// 				{
		// 					FormatString( strCreateBefore, IDS_TABPROJ_CREATEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
		// 					fAddCreateBefore = true;
		// 				}
		// 			}
		// 			else
		// 			{
		// 				// At least one module and one circuit are selected. User can't create.
		//
		// 				// But user can insert before the first object ONLY if the module selected is not the parent.
		// 				if( false == bParentInSelection )
		// 				{
		// 					FormatString( strCreateBefore, IDS_TABPROJ_CREATEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
		// 					fAddCreateBefore = true;
		// 				}
		// 			}
		// 		}
		//
		// 		if( true == fAddCreateIn )
		// 		{
		// 			CString strItemText = strCreateIn.Right( strCreateIn.GetLength() - strCreateIn.Find( '\n' ) );
		// 			pContextMenu->ModifyMenu( ID_TABPROJ_CREATEIN, MF_BYCOMMAND, ID_TABPROJ_CREATEIN, strItemText );
		// 			m_mapUpdateMenuTooltip[ID_TABPROJ_CREATEIN] = strCreateIn.Left( strCreateIn.Find( '\n' ) );
		// 			pMainFrame->EnableMenuResID( ID_TABPROJ_CREATEIN );
		// 		}
		// 		else
		// 		{
		// 			pContextMenu->RemoveMenu( ID_TABPROJ_CREATEIN, MF_BYCOMMAND );
		// 		}
		//
		// 		if( true == fAddCreateBefore )
		// 		{
		// 			CString strItemText = strCreateBefore.Right( strCreateBefore.GetLength() - strCreateBefore.Find( '\n' ) );
		// 			pContextMenu->ModifyMenu( ID_TABPROJ_CREATEBEFORE, MF_BYCOMMAND, ID_TABPROJ_CREATEBEFORE, strItemText );
		// 			m_mapUpdateMenuTooltip[ID_TABPROJ_CREATEBEFORE] = strCreateBefore.Left( strCreateBefore.Find( '\n' ) );
		// 			pMainFrame->EnableMenuResID( ID_TABPROJ_CREATEBEFORE );
		// 		}
		// 		else
		// 		{
		// 			pContextMenu->RemoveMenu( ID_TABPROJ_CREATEBEFORE, MF_BYCOMMAND );
		// 		}

		str = pMainFrame->GetSplitStringFromResID( IDS_TABPROJ_CREATEMODULE );
		pContextMenu->ModifyMenu( ID_TABPROJ_CREATEIN, MF_BYCOMMAND, ID_TABPROJ_CREATEIN, str );

		if( false == bIsTerminalUnit )
		{
			pMainFrame->EnableMenuResID( ID_TABPROJ_CREATEIN );
		}
		else
		{
			pMainFrame->DisableMenuResID( ID_TABPROJ_CREATEIN );
		}

		// TODO: for a future release.
		pContextMenu->RemoveMenu( ID_TABPROJ_CREATEBEFORE, MF_BYCOMMAND );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Add valves' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( true == bIsTerminalUnit )
		{
			pMainFrame->DisableMenuResID( ID_TABPROJ_ADDVALVE );
		}
		else
		{
			pMainFrame->EnableMenuResID( ID_TABPROJ_ADDVALVE );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Edit' and 'Rename' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// With the 'Edit' item, we launch 'CDlgCreateModule' and we authorize user to change reference (name of the module).
		// Thus we can also enable 'Rename'.
		if( 1 == iNbrProductSelected )
		{
			// Available only when there is only one line selected.
			pMainFrame->EnableMenuResID( ID_TABPROJ_EDIT );
			pMainFrame->EnableMenuResID( ID_TABPROJ_RENAME );
		}
		else
		{
			// If more than one line are selected, we disable these two menu items.
			pMainFrame->DisableMenuResID( ID_TABPROJ_EDIT );
			pMainFrame->DisableMenuResID( ID_TABPROJ_RENAME );	
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Delete the logging' menu item unused here.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pContextMenu->RemoveMenu( ID_TABPROJ_DELETE_LD, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_FINDINDEXCIRCUIT, MF_BYCOMMAND );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Delete' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// We can always enable 'Delete' menu item and we have these cases:
		//  - If the selected line is the parent HM, all the branch is deleted (no need to execute other selected lines under parent).
		//  - If the selected line is a child and it's a module, this module and all of its children are deleted.
		//  - If the selected line is a circuit, simply delete this circuit.
		// Remark: user can delete a object that is in the cut mode. In this case, only this object will be erased and the cut operation
		//         will be reseted.
		pMainFrame->EnableMenuResID( ID_TABPROJ_DELETE );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Cut' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// User can cut in the following cases:
		//  - The parent HM ONLY if it's the only one that is selected.
		//  - all children circuits and modules ONLY if the parent is not selected.
		// Remark: - if more than one line is selected, 'pHMRightClicked' is NULL.
		//         - if at least one object in the selection is in the cut mode, it's not a problem. The previous cut operation
		//           will be reseted with the new one.
		pMainFrame->DisableMenuResID( ID_TABPROJ_CUT );

		if( 1 == iNbrProductSelected && pParentHM == pHMRightClicked )
		{
			pMainFrame->EnableMenuResID( ID_TABPROJ_CUT );
		}
		else if( false == bParentInSelection )
		{
			pMainFrame->EnableMenuResID( ID_TABPROJ_CUT );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Copy' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// User can copy what he wants EXCEPT root pump circuit. It's when using 'paste' that it will be applied some restrictions.
		// Remark: if at least one object in the selection is in the cut mode, it's not a problem. The previous cut operation
		//         will be reseted with the copy operation.
		pMainFrame->EnableMenuResID( ID_TABPROJ_COPY );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Paste', 'Paste in' and 'Paste before' menu items.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool fAddPasteIn = false;
		bool fAddPasteBefore = false;
		bool fDisablePasteIn = false;
		bool fDisablePasteBefore = false;
		CString strPasteIn( _T( "" ) );
		CString strPasteBefore( _T( "" ) );

		if( iNbrProductInClipboard > 0 )
		{
			if( 1 == iNbrProductSelected )
			{
				if( true == pHMRightClicked->IsaModule() )
				{
					// If there is only one line selected and it's a module, user can paste in.
					FormatString( strPasteIn, IDS_RVHMCALC_PASTEIN, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
					fAddPasteIn = true;

					// If the module is not the parent, user can insert before.
					if( false == bParentInSelection )
					{
						FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
						fAddPasteBefore = true;
					}
				}
				else
				{
					// If there is only one line selected and it's a circuit, user can only insert.
					FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
					fAddPasteBefore = true;
				}
			}
			else
			{
				if( true == bCircuitExistInSelection && false == bModuleExistInSelection )
				{
					// There are only circuit selected, user can only insert before the first one.
					FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
					fAddPasteBefore = true;
				}
				else if( false == bCircuitExistInSelection && true == bModuleExistInSelection )
				{
					// There are only modules selected, user can paste in each modules.
					strPasteIn = TASApp.LoadLocalizedString( IDS_RVHMCALC_PASTEINALLMODULES );
					fAddPasteIn = true;

					// If there is no the parent in the selection, user can insert before.
					if( false == bParentInSelection )
					{
						FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
						fAddPasteBefore = true;
					}
				}
				else
				{
					// At least one module and one circuit are selected. User can't paste.

					// But user can insert before the first object ONLY if the module selected is not the parent.
					if( false == bParentInSelection )
					{
						FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
						fAddPasteBefore = true;
					}
					else
					{
						strPasteIn = TASApp.LoadLocalizedString( ID_TABPROJ_PASTEIN );
						fAddPasteIn = true;
						fDisablePasteIn = true;
					}
				}
			}

			// Particular case for 'Cut' operation.
			if( ( ( true == fAddPasteIn && false == fDisablePasteIn ) || true == fAddPasteBefore )
				&& LastOperation::Cut == m_eLastOperation )
			{
				// We must verify that that the line (or lines) selected is not the same of the objects in the clipboard or one of its children.

				// Run all objects in the clipboard.
				bool fForbidden = false;
				bool fOnItself = false;
				IDPTR IDPtr = m_pTADS->GetFirstHMFromClipBoard();

				while( NULL != IDPtr.MP && false == fForbidden )
				{
					CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( ( CData * )IDPtr.MP );

					// Run all line selected.
					for( int iLoopArray = 0; iLoopArray < iNbrProductSelected && false == fForbidden; iLoopArray++ )
					{
						CDS_HydroMod *pHMSelected = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( arlProductSelected.GetAt( iLoopArray ) );

						if( NULL != pHMSelected )
						{
							// Remark: compare with the name because pointer on the HM in clipboard is never the same as in the circuit
							//         (because to save in the clipboard we create a new object).
							if( 0 == _tcscmp( pHMSelected->GetHMName(), pHMInClipboard->GetHMName() ) )
							{
								// Try to cut/paste on object on itself! That's not possible.
								fForbidden = true;
								fOnItself = true;
							}
							else
							{
								// Check if 'pHMSelected' is not a child of 'pHMInClipboard'.
								CDS_HydroMod *pParentHMSelected = dynamic_cast<CDS_HydroMod *>( ( CData * )pHMSelected->GetIDPtr().PP );

								while( NULL != pParentHMSelected && false == fForbidden )
								{
									// Remark: compare with the name because pointer on the HM in clipboard is never the same as in the circuit
									//         (because to save in the clipboard we create a new object).
									if( 0 == _tcscmp( pParentHMSelected->GetHMName(), pHMInClipboard->GetHMName() ) )
									{
										fForbidden = true;
									}
									else
									{
										pParentHMSelected = dynamic_cast<CDS_HydroMod *>( ( CData * )pParentHMSelected->GetIDPtr().PP );
									}
								}
							}
						}
					}

					IDPtr = m_pTADS->GetNextHMFromClipBoard();
				}

				if( true == fForbidden )
				{
					// If user tries to paste an object at the level of one of its children...
					if( false == fOnItself )
					{
						// We also disable the possibility to 'Paste before' this child.
						fAddPasteBefore = false;
					}

					if( false == fAddPasteBefore )
					{
						// If 'Paste before' menu item is not displayed, we add 'Paste' menu item but we disable it only.
						strPasteIn = TASApp.LoadLocalizedString( ID_TABPROJ_PASTEIN );
						fAddPasteIn = true;
						fDisablePasteIn = true;
					}
					else
					{
						// If 'Paste before' is already exist, we don't show 'Paste in' menu item.
						fAddPasteIn = false;
					}
				}
			}
		}
		else
		{
			// Add 'Paste module' menu item but disabled it.
			strPasteIn = TASApp.LoadLocalizedString( ID_TABPROJ_PASTEIN );
			fAddPasteIn = true;
			fDisablePasteIn = true;
		}

		if( true == fAddPasteIn )
		{
			CString strItemText = strPasteIn.Right( strPasteIn.GetLength() - strPasteIn.Find( '\n' ) );
			pContextMenu->ModifyMenu( ID_TABPROJ_PASTEIN, MF_BYCOMMAND, ID_TABPROJ_PASTEIN, strItemText );
			m_mapUpdateMenuTooltip[ID_TABPROJ_PASTEIN] = strPasteIn.Left( strPasteIn.Find( '\n' ) );

			if( false == fDisablePasteIn )
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

		if( true == fAddPasteBefore )
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
		// No possibility to lock/unlock if we are not in HM calculation mode.
		pContextMenu->RemoveMenu( ID_TABPROJ_LOCK, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_UNLOCK, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_LOCKMODULE, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_UNLOCKMODULE, MF_BYCOMMAND );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Print' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Remark: for the moment this option is only available for one line.
		if( 1 == iNbrProductSelected && true == bModuleExistInSelection )
		{
			// Available only if there is at least one module in all selected lines.
			pMainFrame->EnableMenuResID( ID_TABPROJ_PRINT );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_TABPROJ_PRINT, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Print recursive' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Remark: for the moment this option is only available for one line.
		if( 1 == iNbrProductSelected && true == bModuleExistInSelection && true == bChildModuleExist )
		{
			// Available only if there is at least one module and this module has at least one module child.
			pMainFrame->EnableMenuResID( ID_TABPROJ_PRINTRECURSIVE );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_TABPROJ_PRINTRECURSIVE, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Export' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Remark: for the moment this option is only available for one line.
		if( 1 == iNbrProductSelected && true == bModuleExistInSelection )
		{
			// Available only if there is at least one module in all selected lines.
			pMainFrame->EnableMenuResID( ID_TABPROJ_EXPORT );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Export' menu item for quick measurement unused here.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_QM, MF_BYCOMMAND );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Export recursive' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Remark: for the moment this option is only available for one line.
		if( 1 == iNbrProductSelected && true == bModuleExistInSelection && true == bChildModuleExist )
		{
			// Available only if there is at least one module and this module has at least one module child.
			pMainFrame->EnableMenuResID( ID_TABPROJ_EXPORTRECURSIVE );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_TABPROJ_EXPORTRECURSIVE, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Export logged data chart' and 'Export logged data list' menu items unused here.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_CHART, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_LDLIST, MF_BYCOMMAND );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Full catalog sheet' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pMainFrame->DisableMenuResID( ID_TABPROJ_GETFULLINFO );
		m_pTAPForFullCatalog = m_pSheetHMCalc->GetTAProductUnderMouse( point );

		if( NULL != m_pTAPForFullCatalog )
		{
			if( true == m_pTAPForFullCatalog->VerifyOneCatExist() )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_GETFULLINFO );
			}
		}

		// Show the popup menu.
		TASApp.GetContextMenuManager()->ShowPopupMenu( HMENU( *pContextMenu ), point.x, point.y, this, TRUE );
	}
	else
	{
		// We are in HMCalc mode.

		CDS_HydroMod *pParentHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpParentHM();

		if( NULL == pParentHM || true == pParentHM->IsForHub() )
		{
			return;
		}

		// Because in some cases we remove some items, we need to reload it at each time.
		// Why use menu with variable member? Because we need it for some methods as 'OnFltMenuRVHMCalcEditAccessories'.
		m_FltMenuRVHMCalc.DestroyMenu();

		if( FALSE == m_FltMenuRVHMCalc.LoadMenu( IDR_FLTMENU_RVHMCALC ) )
		{
			return;
		}

		CMenu *pContextMenu = m_FltMenuRVHMCalc.GetSubMenu( 0 );

		if( NULL == pContextMenu )
		{
			return;
		}

		// Load correct strings.
		CString str;
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_ADDCIRCUIT );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_ADDCIRCUIT, MF_BYCOMMAND, ID_FLTMRVHMCALC_ADDCIRCUIT, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_EDIT );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_EDIT, MF_BYCOMMAND, ID_FLTMRVHMCALC_EDIT, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_RENAME );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_RENAME, MF_BYCOMMAND, ID_FLTMRVHMCALC_RENAME, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_DELETE );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_DELETE, MF_BYCOMMAND, ID_FLTMRVHMCALC_DELETE, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_CUT );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_CUT, MF_BYCOMMAND, ID_FLTMRVHMCALC_CUT, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_COPY );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_COPY, MF_BYCOMMAND, ID_FLTMRVHMCALC_COPY, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_PASTEIN );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PASTEIN, MF_BYCOMMAND, ID_FLTMRVHMCALC_PASTEIN, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_PASTEBEFORE );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PASTEBEFORE, MF_BYCOMMAND, ID_FLTMRVHMCALC_PASTEBEFORE, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_EDITACCESSORIES );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_EDITACCESSORIES, MF_BYCOMMAND, ID_FLTMRVHMCALC_EDITACCESSORIES, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_COPYACCESSORIES );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_COPYACCESSORIES, MF_BYCOMMAND, ID_FLTMRVHMCALC_COPYACCESSORIES, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_PASTEACCESSORIES );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PASTEACCESSORIES, MF_BYCOMMAND, ID_FLTMRVHMCALC_PASTEACCESSORIES, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_LOCK );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_LOCK, MF_BYCOMMAND, ID_FLTMRVHMCALC_LOCK, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_UNLOCK );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_UNLOCK, MF_BYCOMMAND, ID_FLTMRVHMCALC_UNLOCK, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_LOCKCOLUMN );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_LOCKCOLUMN, MF_BYCOMMAND, ID_FLTMRVHMCALC_LOCKCOLUMN, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_UNLOCKCOLUMN );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_UNLOCKCOLUMN, MF_BYCOMMAND, ID_FLTMRVHMCALC_UNLOCKCOLUMN, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT, MF_BYCOMMAND, ID_FLTMRVHMCALC_LOCKALLINCIRCUIT, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT, MF_BYCOMMAND, ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_PRINT );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PRINT, MF_BYCOMMAND, ID_FLTMRVHMCALC_PRINT, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_PRINTRECURSIVE );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PRINTRECURSIVE, MF_BYCOMMAND, ID_FLTMRVHMCALC_PRINTRECURSIVE, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_EXPORT );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_EXPORT, MF_BYCOMMAND, ID_FLTMRVHMCALC_EXPORT, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_EXPORTRECURSIVE );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_EXPORTRECURSIVE, MF_BYCOMMAND, ID_FLTMRVHMCALC_EXPORTRECURSIVE, str );
		
		str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_GETFULLINFO );
		pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_GETFULLINFO, MF_BYCOMMAND, ID_FLTMRVHMCALC_GETFULLINFO, str );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Add circuit' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// In HM calculation mode, we change 'Create module' to 'Add circuit'.
		pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_ADDCIRCUIT );

		// If there is more than one line selected or the project is frozen user can't add circuit.
		if( iNbrProductSelected > 1 || true == bIsProjectFrozen )
		{
			pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_ADDCIRCUIT );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Edit' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( 1 == iNbrProductSelected && NULL != pHMRightClicked )
		{
			// If there is only one line selected we can enable 'Edit' menu item.
			// Remark: we can also enable 'Edit' when the project is frozen. Because user can view details about a circuit without the
			//         possibility to change it.
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_EDIT );
		}
		else
		{
			pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_EDIT );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Rename' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( 1 == iNbrProductSelected )
		{
			// Available only when there is only one line selected.
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_RENAME );
		}
		else
		{
			// If more than one line are selected, we disable the menu item.
			pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_RENAME );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Delete' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// We can always enable 'Delete' menu item (except when project is frozen) and we have these cases:
		//  - If the selected line is the parent HM, all the branch is deleted (no need to execute other selected lines under parent).
		//  - If the selected line is a child and it's a module, this module and all of its children are deleted.
		//  - If the selected line is a circuit, simply delete this circuit.
		// Remark: user can delete a object that is in the cut mode. In this case, only this object will be erased and the cut operation
		//         will be reseted.
		if( false == bIsProjectFrozen )
		{
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_DELETE );
		}
		else
		{
			pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_DELETE );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Cut' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// User can cut in the following cases:
		//  - The parent HM ONLY if it's the only one that is selected.
		//  - all children circuits and modules ONLY if the parent is not selected.
		// Remark: - if more than one line is selected, 'pHMRightClicked' is NULL.
		//         - if at least one object in the selection is in the cut mode, it's not a problem. The previous cut operation
		//           will be reseted with the new one.
		pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_CUT );

		if( false == bIsProjectFrozen )
		{
			if( 1 == iNbrProductSelected && pParentHM == pHMRightClicked )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_CUT );
			}
			else if( false == bParentInSelection )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_CUT );
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Copy' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// User can copy what he wants EXCEPT root pump circuit. It's when using 'paste' that it will be applied some restrictions.
		// Remark: if at least one object in the selection is in the cut mode, it's not a problem. The previous cut operation
		//         will be reseted with the copy operation.
		if( false == bIsProjectFrozen )
		{
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_COPY );
		}
		else
		{
			pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_COPY );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Paste', 'Paste in' and 'Paste before' menu items.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool bAddPasteIn = false;
		bool bAddPasteBefore = false;
		bool bDisablePasteIn = false;
		CString strPasteIn( _T( "" ) );
		CString strPasteBefore( _T( "" ) );

		if( false == bIsProjectFrozen && iNbrProductInClipboard > 0 )
		{
			if( 1 == iNbrProductSelected )
			{
				if( true == pHMRightClicked->IsaModule() )
				{
					// If there is only one line selected and it's a module, user can paste in.
					FormatString( strPasteIn, IDS_RVHMCALC_PASTEIN, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
					bAddPasteIn = true;

					// If the module is not the parent, user can insert before.
					if( false == bParentInSelection )
					{
						FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
						bAddPasteBefore = true;
					}
				}
				else
				{
					// If there is only one line selected and it's a circuit, user can only insert.
					FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
					bAddPasteBefore = true;
				}
			}
			else
			{
				if( true == bCircuitExistInSelection && false == bModuleExistInSelection )
				{
					// There are only circuit selected, user can only insert before the first one.
					FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
					bAddPasteBefore = true;
				}
				else if( false == bCircuitExistInSelection && true == bModuleExistInSelection )
				{
					// There are only modules selected, user can paste in each modules.
					strPasteIn = TASApp.LoadLocalizedString( IDS_RVHMCALC_PASTEINALLMODULES );
					bAddPasteIn = true;

					// If there is no the parent in the selection, user can insert before.
					if( false == bParentInSelection )
					{
						FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
						bAddPasteBefore = true;
					}
				}
				else
				{
					// At least one module and one circuit are selected. User can't paste.

					// But user can insert before the first object ONLY if the module selected is not the parent.
					if( false == bParentInSelection )
					{
						FormatString( strPasteBefore, IDS_RVHMCALC_PASTEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
						bAddPasteBefore = true;
					}
					else
					{
						strPasteIn = TASApp.LoadLocalizedString( ID_TABPROJ_PASTEIN );
						bAddPasteIn = true;
						bDisablePasteIn = true;
					}
				}
			}

			// Particular case for 'Cut' operation.
			// Remark: first line of condition is to test if there at least one 'Paste' menu item activated.
			if( ( ( true == bAddPasteIn && false == bDisablePasteIn ) || true == bAddPasteBefore )
				&& LastOperation::Cut == m_eLastOperation )
			{
				// We must verify that that the line (or lines) selected is not the same of the objects in the clipboard or one of its children.

				// Run all objects in the clipboard.
				bool fForbidden = false;
				bool fOnItself = false;
				IDPTR IDPtr = m_pTADS->GetFirstHMFromClipBoard();

				while( NULL != IDPtr.MP && false == fForbidden )
				{
					CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( ( CData * )IDPtr.MP );

					// Run all line selected.
					for( int iLoopArray = 0; iLoopArray < iNbrProductSelected && false == fForbidden; iLoopArray++ )
					{
						CDS_HydroMod *pHMSelected = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( arlProductSelected.GetAt( iLoopArray ) );

						if( NULL != pHMSelected )
						{
							// Remark: compare with the name because pointer on the HM in clipboard is never the same as in the circuit
							//         (because to save in the clipboard we create a new object).
							if( 0 == _tcscmp( pHMSelected->GetHMName(), pHMInClipboard->GetHMName() ) )
							{
								// Try to cut/paste on object on itself! That's not possible.
								fForbidden = true;
								fOnItself = true;
							}
							else
							{
								// Check if 'pHMSelected' is not a child of 'pHMInClipboard'.
								CDS_HydroMod *pParentHMSelected = dynamic_cast<CDS_HydroMod *>( ( CData * )pHMSelected->GetIDPtr().PP );

								while( NULL != pParentHMSelected && false == fForbidden )
								{
									// Remark: compare with the name because pointer on the HM in clipboard is never the same as in the circuit
									//         (because to save in the clipboard we create a new object).
									if( 0 == _tcscmp( pParentHMSelected->GetHMName(), pHMInClipboard->GetHMName() ) )
									{
										fForbidden = true;
									}
									else
									{
										pParentHMSelected = dynamic_cast<CDS_HydroMod *>( ( CData * )pParentHMSelected->GetIDPtr().PP );
									}
								}
							}
						}
					}

					IDPtr = m_pTADS->GetNextHMFromClipBoard();
				}

				if( true == fForbidden )
				{
					// If user tries to paste an object at the level of one of its children...
					if( false == fOnItself )
					{
						// We also disable the possibility to 'Paste before' this child.
						bAddPasteBefore = false;
					}

					if( false == bAddPasteBefore )
					{
						// If 'Paste before' menu item is not displayed, we add 'Paste' menu item but we disable it only.
						strPasteIn = TASApp.LoadLocalizedString( ID_FLTMRVHMCALC_PASTEIN );
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
			strPasteIn = TASApp.LoadLocalizedString( ID_FLTMRVHMCALC_PASTEIN );
			bAddPasteIn = true;
			bDisablePasteIn = true;
		}

		if( true == bAddPasteIn )
		{
			CString strItemText = strPasteIn.Right( strPasteIn.GetLength() - strPasteIn.Find( '\n' ) );
			pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PASTEIN, MF_BYCOMMAND, ID_FLTMRVHMCALC_PASTEIN, strItemText );
			m_mapUpdateMenuTooltip[ID_FLTMRVHMCALC_PASTEIN] = strPasteIn.Left( strPasteIn.Find( '\n' ) );

			if( false == bDisablePasteIn )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_PASTEIN );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_PASTEIN );
			}
		}
		else
		{
			pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_PASTEIN, MF_BYCOMMAND );
		}

		if( true == bAddPasteBefore )
		{
			CString strItemText = strPasteBefore.Right( strPasteBefore.GetLength() - strPasteBefore.Find( '\n' ) );
			pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PASTEBEFORE, MF_BYCOMMAND, ID_FLTMRVHMCALC_PASTEBEFORE, strItemText );
			m_mapUpdateMenuTooltip[ID_FLTMRVHMCALC_PASTEBEFORE] = strPasteBefore.Left( strPasteBefore.Find( '\n' ) );
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_PASTEBEFORE );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_PASTEBEFORE, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Edit accessories' and 'Copy accessories' menu items.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if( 1 == iNbrProductSelected )
		{
			// Retrieve the true spread coordinate where user has clicked.
			CRect rect;
			long lCol, lRow;
			CDC *pdc = GetDC();
			CPoint pointClient = point;
			m_pSheetHMCalc->ScreenToClient( &pointClient );
			this->ReleaseDC( pdc );
			m_pSheetHMCalc->GetCellFromPixel( &lCol, &lRow, pointClient.x, pointClient.y );

			CPipes *pPipe = NULL;
			bool fAccessoriesAvailable = m_pSheetHMCalc->IsEditAccAvailable( lCol, lRow, pPipe );

			if( true == fAccessoriesAvailable )
			{
				if( false == bIsProjectFrozen )
				{
					pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_EDITACCESSORIES );
					pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_COPYACCESSORIES );

					// Add current pipe in the menu.
					MENUITEMINFO MenuItemInfo;
					MenuItemInfo.cbSize = sizeof( MENUITEMINFO );
					MenuItemInfo.fMask = MIIM_DATA;
					MenuItemInfo.dwItemData = ( ULONG_PTR )pPipe;
					pContextMenu->SetMenuItemInfo( ID_FLTMRVHMCALC_EDITACCESSORIES, &MenuItemInfo );
					pContextMenu->SetMenuItemInfo( ID_FLTMRVHMCALC_COPYACCESSORIES, &MenuItemInfo );
				}
				else
				{
					pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_EDITACCESSORIES );
					pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_COPYACCESSORIES );
				}
			}
			else
			{
				// To avoid the context menu becomes too big, we remove when these items when not needed.
				pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_EDITACCESSORIES, MF_BYCOMMAND );
				pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_COPYACCESSORIES, MF_BYCOMMAND );
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Paste accessories' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		OpenClipboard();
		HGLOBAL hData = ::GetClipboardData( CF_PRIVATEFIRST );
		CloseClipboard();

		// Check if there is at least one selected line that can contain pipe in which we can paste accessories.
		bool fFound = false;

		for( int iLoopRow = 0; iLoopRow < iNbrProductSelected && false == fFound; iLoopRow++ )
		{
			CPipes *pPipe = NULL;

			if( true == m_pSheetHMCalc->IsEditAccAvailable( m_lColumn, arlProductSelected.GetAt( iLoopRow ), pPipe ) )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_PASTEACCESSORIES );
				fFound = true;
			}
		}

		if( true == fFound )
		{
			if( false == bIsProjectFrozen && NULL != hData )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_PASTEACCESSORIES );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_PASTEACCESSORIES );
			}
		}
		else
		{
			// To avoid the context menu becomes too big, we remove when this item when not needed.
			pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_PASTEACCESSORIES, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Lock' and 'Unlock' menu items.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_LOCK );
		pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_UNLOCK );

		if( false == bIsProjectFrozen && 1 == iNbrProductSelected && CDS_HydroMod::eNone != m_pSheetHMCalc->GetHMObj( m_lColumn, m_lRow ) )
		{
			// Remark: let menu items disabled if 'IsLockedTriState' returns 'eBool3::eb3Undef'.
			eBool3 eReturn = pHMRightClicked->IsLockedTriState( m_pSheetHMCalc->GetHMObj( m_lColumn, m_lRow ) );

			if( eBool3::eb3False == eReturn )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_LOCK );
			}
			else if( eBool3::eb3True == eReturn )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_UNLOCK );
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Lock column' and 'Unlock column' menu items.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_UNLOCKCOLUMN );
		pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_LOCKCOLUMN );
		CDS_HydroMod::eHMObj eObjectInColumn = CDS_HydroMod::eHMObj::eNone;

		if( false == bIsProjectFrozen && true == m_pSheetHMCalc->IsHMObjInColumn( m_lColumn, &eObjectInColumn ) )
		{
			// Check if all columns are already all locked or unlocked.
			std::vector<CDS_HydroMod *> vecAllHM;
			bool fAtLeastOneLocked = false;
			bool fAtLeastOneUnLocked = false;
			m_pSheetHMCalc->GetAllHMVisible( &vecAllHM );

			if( vecAllHM.size() > 0 )
			{
				eBool3 eLockStatus = eBool3::eb3Undef;

				for( int iLoopHM = 0; iLoopHM < ( int )vecAllHM.size(); iLoopHM++ )
				{
					eBool3 eReturn = vecAllHM[iLoopHM]->IsLockedTriState( eObjectInColumn );

					if( eBool3::eb3True == eReturn )
					{
						fAtLeastOneLocked = true;
					}
					else if( eBool3::eb3False == eReturn )
					{
						fAtLeastOneUnLocked = true;
					}
				}

				if( false == fAtLeastOneLocked && true == fAtLeastOneUnLocked )
				{
					// No object is locked but all is unlocked. We can thus lock this column.
					pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_LOCKCOLUMN );
				}
				else if( true == fAtLeastOneLocked && false == fAtLeastOneUnLocked )
				{
					// All objects are locked but no one is unlocked. We can thus unlock this column.
					pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_UNLOCKCOLUMN );
				}
				else if( false == fAtLeastOneLocked && false == fAtLeastOneUnLocked )
				{
					// All objects are not present or are in a undefined state. We can do nothing.
					pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_LOCKCOLUMN );
					pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_UNLOCKCOLUMN );
				}
				else
				{
					// We have some circuits locked and unlocked.
					pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_LOCKCOLUMN );
					pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_UNLOCKCOLUMN );
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Lock all in circuit' and 'Unlock all in circuit' menu items.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT );
		pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT );

		if( false == bIsProjectFrozen )
		{
			// Check if all circuits are already all locked or unlocked.
			bool fAtLeastOneLocked = false;
			bool fAtLeastOneUnLocked = false;
			eBool3 eLockStatus = eBool3::eb3Undef;

			for( int iLoopHM = 0; iLoopHM < iNbrProductSelected; iLoopHM++ )
			{
				CDS_HydroMod *pHMSelected = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( arlProductSelected.GetAt( iLoopHM ) );

				if( NULL == pHMSelected )
				{
					continue;
				}

				if( true == pHMSelected->IsAtLeastOneObjectLocked( true ) )
				{
					fAtLeastOneLocked = true;
				}

				if( true == pHMSelected->IsAtLeastOneObjectUnlocked( true ) )
				{
					fAtLeastOneUnLocked = true;
				}

				if( true == fAtLeastOneLocked && true == fAtLeastOneUnLocked )
				{
					break;
				}
			}

			if( false == fAtLeastOneLocked && true == fAtLeastOneUnLocked )
			{
				// It means that all objects are unlocked. We can thus lock all.
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT );
			}
			else if( true == fAtLeastOneLocked && false == fAtLeastOneUnLocked )
			{
				// It means that all objects are locked. We can thus lock all.
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT );
			}
			else if( false == fAtLeastOneLocked && false == fAtLeastOneUnLocked )
			{
				// All objects are not present or are in a undefined state. We can do nothing.
				pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT );
				pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT );
			}
			else
			{
				// We have some circuits locked and unlocked.
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT );
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT );
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Print' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( true == bModuleExistInSelection )
		{
			// Available only if there is at least one module in all selected lines.
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_PRINT );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_PRINT, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Print recursive' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( true == bModuleExistInSelection && true == bChildModuleExist )
		{
			// Available only if there is at least one module and this module has at least one module child.
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_PRINTRECURSIVE );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_PRINTRECURSIVE, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Export' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( true == bModuleExistInSelection )
		{
			// Available only if there is at least one module in all selected lines.
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_EXPORT );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_EXPORT, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Export recursive' menu item.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( true == bModuleExistInSelection && true == bChildModuleExist )
		{
			// Available only if there is at least one module and this module has at least one module child.
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_EXPORTRECURSIVE );
		}
		else
		{
			pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_EXPORTRECURSIVE, MF_BYCOMMAND );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Full catalog sheet' menu items.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_GETFULLINFO );
		m_pTAPForFullCatalog = m_pSheetHMCalc->GetTAProductUnderMouse( point );

		if( NULL != m_pTAPForFullCatalog )
		{
			if( true == m_pTAPForFullCatalog->VerifyOneCatExist() )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_GETFULLINFO );
			}
		}

		// Show the popup menu.
		TASApp.GetContextMenuManager()->ShowPopupMenu( HMENU( *pContextMenu ), point.x, point.y, this, TRUE );
	}
}

void CRViewHMCalc::OnFltMenuTabProjCreateIn()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_TabProjCreate( CreateMode::CreateIn );
}

void CRViewHMCalc::OnFltMenuTabProjCreateBefore()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_TabProjCreate( CreateMode::CreateBefore );
}

void CRViewHMCalc::OnFltMenuTabProjAddValve()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM || NULL == pDlgLeftTabProject )
	{
		return;
	}

	pDlgLeftTabProject->AddValve( pHM->GetPos() );
}

void CRViewHMCalc::OnFltMenuTabProjEdit()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() || NULL == pDlgLeftTabProject )
	{
		return;
	}

	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	// If project has been converted at least one time in a hydraulic calculation mode...
	bool fMustReset = false;

	if( eBool3::eb3True == m_pTADS->IsHMCalcMode( NULL, true ) )
	{
		// FF: I let the code here in the case we come back on our decision to not display anymore the warning.
		// 		// It's a particular case. User has created a network (in CBI mode (non HMCalc) or in HMCalc mode). If in CBI mode, he wants to edit a module/circuit
		// 		// tagged as 'HMCalc', we must ask to him if he wants to reset this module/circuit to a straight pipe default module/circuit.
		// 		INT_PTR nRet = IDOK;
		// 		if( true == TASApp.GetShowWarningEditHMCalc() )
		// 		{
		// 			CDlgWarningEditHMCalc clDlg( pHM );
		// 			nRet = clDlg.DoModal();
		// 		}
		// 		if( IDOK == nRet )
		// 		{
		if( true == pDlgLeftTabProject->EditHydroMod( pHM,
				( true == pHM->IsaModule() ) ? CDlgCreateModule::DialogMode::EditModule : CDlgCreateModule::DialogMode::EditValve ) )
		{
			pHM->SetHMCalcMode( false );
			fMustReset = true;
		}

		//		}
	}
	else
	{
		// The project was never been transformed to HM calculation mode. We can thus edit the module or the valve
		// in a simple way (with 'CDlgCreateModule').
		if( true == pDlgLeftTabProject->EditHydroMod( pHM,
				( true == pHM->IsaModule() ) ? CDlgCreateModule::DialogMode::EditModule : CDlgCreateModule::DialogMode::EditValve ) )
		{
			fMustReset = true;
		}
	}

	if( true == fMustReset )
	{
		ResetFlagCut();
		m_pTADS->CleanClipboard();
		pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )m_pSheetHMCalc->GetpParentHM() );
	}
}

void CRViewHMCalc::OnFltMenuTabProjRename()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	CDlgEditString dlg;
	CString strTitle = TASApp.LoadLocalizedString( IDS_RENAME );
	bool fMustReset = false;
	bool fStop = false;

	while( false == fStop )
	{
		CString strStatic = TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_NEWMODULENAME );
		CString strName = pHM->GetHMName();

		if( IDOK == dlg.Display( strTitle, strStatic, &strName ) )
		{
			// If name has changed...
			if( 0 != _tcscmp( ( LPCTSTR )strName, pHM->GetHMName() ) )
			{
				// If name is empty or prefixed with a '*'.
				if( true == strName.IsEmpty() || '*' == strName.GetAt( 0 ) )
				{
					// It means that it's an automatic name.
					pHM->SetHMName( _T("*") );

					if( 0 == pHM->GetLevel() )
					{
						m_pTADS->ComposeRMName( strName, pHM, pHM->GetPos() );
						pHM->SetHMName( ( LPCTSTR )strName );
					}
					else
					{
						IDPTR IDPtr = pHM->GetIDPtr();
						m_pTADS->ComposeValveName( IDPtr );
					}

					fStop = true;
					fMustReset = true;
				}
				else
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
						// Verify first if name is not already exist.
						CTable *pPipTab = m_pTADS->GetpHydroModTable();
						ASSERT( NULL != pPipTab );

						IDPTR IDPtr = _NULL_IDPTR;

						if( NULL == m_pTADS->FindHydroMod( strName, pPipTab ) )
						{
							pHM->SetHMName( ( LPCTSTR )strName );
							fStop = true;
							fMustReset = true;
						}
						else
						{
							CString str;
							::FormatString( str, IDS_HYDROMODREF_ALREADYUSED, ( LPCTSTR )strName );
							AfxMessageBox( str );
						}
					}
				}
			}
			else
			{
				fStop = true;
			}
		}
	}

	if( true == fMustReset )
	{
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )m_pSheetHMCalc->GetpParentHM() );
		}
	}
}

void CRViewHMCalc::OnFltMenuTabProjDelete()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuDelete();
}

void CRViewHMCalc::OnFltMenuTabProjCut()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CArray<long> aSelRows;
	m_pSheetHMCalc->GetSelectedRows( &aSelRows );

	if( 0 == aSelRows.GetCount() )
	{
		return;
	}

	BeginWaitCursor();

	CDS_HydroMod *pHM = NULL;
	m_pTADS->CleanClipboard();
	
	CTable *pTab = m_pTADS->GetpClipboardTable();
	ASSERT( NULL != pTab );
	
	std::vector<CDS_HydroMod *> vecHydromod;

	for( int i = aSelRows.GetCount() - 1; i >= 0; --i )
	{
		pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( aSelRows.GetAt( i ) );

		if( NULL == pHM )
		{
			continue;
		}

		m_pTADS->CopyHMToClipBoard( pHM, pTab );
		vecHydromod.push_back( pHM );

	}

	EndWaitCursor();

	if( vecHydromod.size() > 0 )
	{
		SetLastCopyCutOperation( LastOperation::Cut );
		SetFlagCut( &vecHydromod );

		if( NULL != m_pSheetHMCalc->GetpParentHM() )
		{
			RedrawHM( m_pSheetHMCalc->GetpParentHM() );
		}
	}
}

void CRViewHMCalc::OnFltMenuTabProjCopy()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuCopy();
}

void CRViewHMCalc::OnFltMenuTabProjPasteIn()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuPaste( PasteMode::PasteIn );
}

void CRViewHMCalc::OnFltMenuTabProjPasteBefore()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuPaste( PasteMode::PasteBefore );
}

void CRViewHMCalc::OnFltMenuTabProjPrint()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuPrint();
}

void CRViewHMCalc::OnFltMenuTabProjPrintRecursive()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuPrintRecursive();
}

void CRViewHMCalc::OnFltMenuTabProjExport()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuExport();
}

void CRViewHMCalc::OnFltMenuTabProjExportRecursive()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuExportRecursive();
}

void CRViewHMCalc::OnFltMenuRVHMCalcAddCircuit()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	// Clean cutting object if exist.
	ResetFlagCut();
	m_pTADS->CleanClipboard();

	CDlgWizCircuit dlg( this );
	dlg.Init( pHM, false );
	dlg.DisplayWizard();

 	if( true == dlg.IsAtLeastOneCircuitAdded() && NULL != pDlgLeftTabProject )
 	{
 		pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )dlg.GetTableWhereLastInsertOccured() );
 	}
}

void CRViewHMCalc::OnFltMenuRVHMCalcEdit()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	// Clean cutting object if exist.
	ResetFlagCut();
	m_pTADS->CleanClipboard();

	CDS_HydroMod *pSelectedHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->EnsureVisible( pSelectedHM, CDlgLeftTabProject::OMI_EditCircuit );
	}

	// Edit is only possible for one row. This is filtered in the 'OnContextMenu' method.
	m_pSheetHMCalc->EditHMRow();

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->EnsureVisible(  m_pSheetHMCalc->GetSelectedHM() );
	}
}

void CRViewHMCalc::OnFltMenuRVHMCalcRename()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	// Normally in the 'OnContextMenu' we allow 'Rename' menu item if there is ONLY one line selected.
	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	CDlgEditString dlg;
	CString strTitle = TASApp.LoadLocalizedString( IDS_RENAME );
	bool fStop = false;

	while( false == fStop )
	{
		CString strStatic = TASApp.LoadLocalizedString( IDS_RVHMCALC_NEWMODULENAME );
		CString strName = pHM->GetHMName();

		if( IDOK == dlg.Display( strTitle, strStatic, &strName ) )
		{
			if( 0 != _tcscmp( ( LPCTSTR )strName, pHM->GetHMName() ) )
			{
				if( strName.IsEmpty() )
				{
					strName = _T("*");
				}

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
						ResetFlagCut();
						m_pTADS->CleanClipboard();

						pHM->SetHMName( ( LPCTSTR )strName );

						// Is automatic name rename it.
						if( '*' == pHM->GetHMName().GetAt( 0 ) )
						{
							pHM->RenameMe();
						}

						if( NULL != pDlgLeftTabProject )
						{
							pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )m_pSheetHMCalc->GetpParentHM() );
						}

						fStop = true;
					}
					else
					{
						CString str;
						::FormatString( str, IDS_HYDROMODREF_ALREADYUSED, ( LPCTSTR )strName );
						AfxMessageBox( str );
					}
				}
			}
		}
		else
		{
			fStop = true;
		}
	}
}

void CRViewHMCalc::OnFltMenuRVHMCalcDelete()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuDelete();
}

void CRViewHMCalc::OnFltMenuRVHMCalcCut()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CArray<long> aSelRows;
	m_pSheetHMCalc->GetSelectedRows( &aSelRows );

	if( 0 == aSelRows.GetCount() )
	{
		return;
	}

	BeginWaitCursor();

	m_pTADS->CleanClipboard();

	// First step: sort selected lines by pos.
	std::map<int, CDS_HydroMod *> mapSelectedHM;

	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( aSelRows.GetAt( i ) );

		if( NULL == pHM )
		{
			continue;
		}

		mapSelectedHM[pHM->GetPos()] = pHM;
	}

	// Copy now in the clipboard in the right order.
	std::vector<CDS_HydroMod *> vecHydromod;
	
	CTable *pTab = m_pTADS->GetpClipboardTable();
	ASSERT( NULL != pTab );

	for( std::map<int, CDS_HydroMod *>::iterator iter = mapSelectedHM.begin(); iter != mapSelectedHM.end(); iter++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)iter->second;

		if( NULL == pHM )
		{
			continue;
		}

		m_pTADS->CopyHMToClipBoard( pHM, pTab );
		vecHydromod.push_back( pHM );
	}

	EndWaitCursor();

	if( vecHydromod.size() > 0 )
	{
		SetLastCopyCutOperation( LastOperation::Cut );
		SetFlagCut( &vecHydromod );

		if( NULL != m_pSheetHMCalc->GetpParentHM() )
		{
			RedrawHM( m_pSheetHMCalc->GetpParentHM() );
		}
	}
}

void CRViewHMCalc::OnFltMenuRVHMCalcCopy()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuCopy();
}

void CRViewHMCalc::OnFltMenuRVHMCalcPasteIn()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuPaste( PasteMode::PasteIn );
}

void CRViewHMCalc::OnFltMenuRVHMCalcPasteBefore()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuPaste( PasteMode::PasteBefore );
}

void CRViewHMCalc::OnFltMenuRVHMCalcEditAccessories()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	// Retrieve item data.
	CMenu *pContextMenu = m_FltMenuRVHMCalc.GetSubMenu( 0 );
	MENUITEMINFO MenuItemInfo;
	MenuItemInfo.cbSize = sizeof( MENUITEMINFO );
	MenuItemInfo.fMask = MIIM_DATA;

	if( 0 == pContextMenu->GetMenuItemInfo( ID_FLTMRVHMCALC_EDITACCESSORIES, &MenuItemInfo ) )
	{
		return;
	}

	CPipes *pPipe = ( CPipes * )MenuItemInfo.dwItemData;

	if( NULL == pPipe )
	{
		return;
	}

	bool fChangeDone;
	CDlgAccessory dlg( CTADatabase::FilterSelection::ForHMCalc );

	if( IDOK == dlg.Display( pHM, pPipe, &fChangeDone ) && true == fChangeDone )
	{
		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		m_pSheetHMCalc->GetpParentHM()->ComputeAll();

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )m_pSheetHMCalc->GetpParentHM() );
		}
	}
}

void CRViewHMCalc::OnFltMenuRVHMCalcCopyAccessories()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	// 'Copy accessories' menu item is available only for one selected row.

	// Retrieve pipe.
	CMenu *pContextMenu = m_FltMenuRVHMCalc.GetSubMenu( 0 );
	MENUITEMINFO MenuItemInfo;
	MenuItemInfo.cbSize = sizeof( MENUITEMINFO );
	MenuItemInfo.fMask = MIIM_DATA;

	if( 0 == pContextMenu->GetMenuItemInfo( ID_FLTMRVHMCALC_COPYACCESSORIES, &MenuItemInfo ) )
	{
		return;
	}

	CPipes *pPipes = ( CPipes * )MenuItemInfo.dwItemData;

	if( NULL == pPipes )
	{
		return;
	}

	int iTotalSize = pPipes->WriteAllSingularitiesToBuffer( NULL );

	if( 0 == iTotalSize )
	{
		return;
	}

	BYTE *pbSingularitiesBuffer = new BYTE[iTotalSize];

	if( NULL == pbSingularitiesBuffer )
	{
		return;
	}

	// Write singularities in the buffer.
	pPipes->WriteAllSingularitiesToBuffer( pbSingularitiesBuffer );

	// Save into the clipboard.
	HGLOBAL hData = GlobalAlloc( GMEM_FIXED, iTotalSize );
	LPVOID lpBuffer = GlobalLock( hData );
	memcpy_s( lpBuffer, iTotalSize, pbSingularitiesBuffer, iTotalSize );
	GlobalUnlock( hData );
	delete []pbSingularitiesBuffer;

	// Use the Windows clipboard.
	OpenClipboard();
	HANDLE hReturn = ::SetClipboardData( CF_PRIVATEFIRST, hData );
	CloseClipboard();
}

void CRViewHMCalc::OnFltMenuRVHMCalcPasteAccessories()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	BeginWaitCursor();

	// Retrieve the data in the Windows clipboard.
	OpenClipboard();
	HGLOBAL hData = ::GetClipboardData( CF_PRIVATEFIRST );
	CloseClipboard();

	// Read the total size.
	int iTotalSize;
	LPVOID lpBuffer = GlobalLock( hData );
	memcpy_s( &iTotalSize, sizeof( int ), lpBuffer, sizeof( int ) );

	if( 0 == iTotalSize )
	{
		return;
	}

	// Prepare buffer.
	BYTE *pbSingularitiesBuffer = new BYTE[iTotalSize];

	if( NULL == pbSingularitiesBuffer )
	{
		return;
	}

	// Read clipboard.
	memcpy_s( pbSingularitiesBuffer, iTotalSize, lpBuffer, iTotalSize );

	CArray<long> aSelRows;
	m_pSheetHMCalc->GetSelectedRows( &aSelRows );

	// For each selected target copy singularities.
	CDS_HydroMod *pHM = NULL;
	bool fChangeDone = false;

	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		CPipes *pDestPipe = NULL;

		if( false == m_pSheetHMCalc->IsEditAccAvailable( m_lColumn, aSelRows.GetAt( i ), pDestPipe ) )
		{
			continue;
		}

		pDestPipe->ReadAllSingularitiesFromBuffer( pbSingularitiesBuffer );
		fChangeDone = true;
	}

	if( true == fChangeDone )
	{
		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		m_pTADS->ComputeAllInstallation();

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )m_pSheetHMCalc->GetpParentHM() );
		}
	}

	EndWaitCursor();
}

void CRViewHMCalc::OnFltMenuRVHMCalcLock()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() || NULL == pDlgLeftTabProject )
	{
		return;
	}

	// 'Lock' menu item is available only for one selected row.
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetSelectedHM();

	if( NULL != pHM )
	{
		BeginWaitCursor();

		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		pHM->SetLock( m_pSheetHMCalc->GetHMObj( m_lColumn, m_lRow ), true );

		CDS_HydroMod *pTab = pDlgLeftTabProject->GetSelectedHM();

		if( NULL != pTab )
		{
			RedrawHM( pTab );
		}
		else
		{

			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )pHM );
		}

		EndWaitCursor();
	}
}

void CRViewHMCalc::OnFltMenuRVHMCalcUnLock()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() || NULL == pDlgLeftTabProject )
	{
		return;
	}

	// 'Unlock' menu item is available only for one selected row.
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetSelectedHM();

	if( NULL != pHM )
	{
		BeginWaitCursor();

		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		pHM->SetLock( m_pSheetHMCalc->GetHMObj( m_lColumn, m_lRow ), false );
		pHM->ComputeAll( true );

		CDS_HydroMod *pTab = pDlgLeftTabProject->GetSelectedHM();

		if( NULL != pTab )
		{
			RedrawHM( pTab );
		}
		else
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )pHM );
		}

		EndWaitCursor();
	}
}

void CRViewHMCalc::OnFltMenuRVHMCalcLockColumn()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	if( 0 == m_lColumn )
	{
		return;
	}

	BeginWaitCursor();

	bool fAtLeastOneLockColumnDone = false;

	for( long lLoopRow = CSheetHMCalc::RowDescription_Header::RD_Header_FirstCirc; lLoopRow <= m_pSheetHMCalc->GetMaxRows(); lLoopRow++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( lLoopRow );

		if( NULL == pHM )
		{
			continue;
		}

		if( CDS_HydroMod::eHMObj::eNone == m_pSheetHMCalc->GetHMObj( m_lColumn, lLoopRow ) )
		{
			continue;
		}

		pHM->SetLock( m_pSheetHMCalc->GetHMObj( m_lColumn, lLoopRow ), true );
		fAtLeastOneLockColumnDone = true;
	}

	if( true == fAtLeastOneLockColumnDone )
	{
		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )m_pSheetHMCalc->GetpParentHM() );
		}
	}

	EndWaitCursor();
}

void CRViewHMCalc::OnFltMenuRVHMCalcUnlockColumn()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	if( 0 == m_lColumn )
	{
		return;
	}

	BeginWaitCursor();

	bool fAtLeastOneUnLockColumnDone = false;

	for( long lLoopRow = CSheetHMCalc::RowDescription_Header::RD_Header_FirstCirc; lLoopRow <= m_pSheetHMCalc->GetMaxRows(); lLoopRow++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *) m_pSheetHMCalc->GetpHM( lLoopRow );

		if( NULL == pHM )
		{
			continue;
		}

		if( CDS_HydroMod::eHMObj::eNone == m_pSheetHMCalc->GetHMObj( m_lColumn, lLoopRow ) )
		{
			continue;
		}

		pHM->SetLock( m_pSheetHMCalc->GetHMObj( m_lColumn, lLoopRow ), false );
		fAtLeastOneUnLockColumnDone = true;
	}

	if( true == fAtLeastOneUnLockColumnDone )
	{
		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		m_pSheetHMCalc->GetpParentHM()->ComputeAll();

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )m_pSheetHMCalc->GetpParentHM() );
		}
	}

	EndWaitCursor();
}

void CRViewHMCalc::OnFltMenuRVHMCalcLockAll()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CArray<long> aSelRows;
	m_pSheetHMCalc->GetSelectedRows( &aSelRows );

	if( 0 == aSelRows.GetCount() )
	{
		return;
	}

	BeginWaitCursor();

	bool fAtLeastOneLockDone = false;
	CDS_HydroMod *pHM = NULL;

	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( aSelRows.GetAt( i ) );

		if( NULL == pHM )
		{
			continue;
		}

		pHM->SetLock( CDS_HydroMod::eHMObj::eALL, true );
		fAtLeastOneLockDone = true;
	}

	if( true == fAtLeastOneLockDone )
	{
		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		CDS_HydroMod *pTab = m_pSheetHMCalc->GetpParentHM();

		if( NULL != pTab )
		{
			RedrawHM( pTab );
		}
		else if( NULL != pHM && NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->ResetTreeAndSelectHM( pHM );
		}
	}

	EndWaitCursor();
}

void CRViewHMCalc::OnFltMenuRVHMCalcUnlockAll()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	CArray<long> aSelRows;
	m_pSheetHMCalc->GetSelectedRows( &aSelRows );

	if( 0 == aSelRows.GetCount() )
	{
		return;
	}

	BeginWaitCursor();

	bool fAtLeastOneUnLockDone = false;
	CDS_HydroMod *pHM = NULL;

	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( aSelRows.GetAt( i ) );

		if( NULL == pHM )
		{
			continue;
		}

		pHM->SetLock( CDS_HydroMod::eHMObj::eALL, false );
		fAtLeastOneUnLockDone = true;
	}

	if( true == fAtLeastOneUnLockDone )
	{
		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		m_pTADS->ComputeAllInstallation();

		CDS_HydroMod *pTab = m_pSheetHMCalc->GetpParentHM();

		if( NULL != pTab )
		{
			RedrawHM( pTab );
		}
		else if( NULL != pHM && NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->ResetTreeAndSelectHM( pHM );
		}
	}

	EndWaitCursor();
}

void CRViewHMCalc::OnFltMenuRVHMCalcPrint()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuPrint();
}

void CRViewHMCalc::OnFltMenuRVHMCalcPrintRecursive()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuPrintRecursive();
}

void CRViewHMCalc::OnFltMenuRVHMCalcExport()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuExport();
}

void CRViewHMCalc::OnFltMenuRVHMCalcExportRecursive()
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return;
	}

	_OnContextMenuExportRecursive();
}

void CRViewHMCalc::OnFltMenuRVHMCalcGetFullinfo()
{
	// No need to verify 'm_pTAPForQuickRefOrFullCat'. Because we can access to 'OnTabprojGetFullinfo' only if user gets context menu and click 'Full reference catalog'.
	// 'Full reference catalog' in context menu is valid only if there is a valid TA Product under mouse pointer. If there is a valid TA product,
	// 'm_pTAPForQuickRefOrFullCat' is updated at each call of context menu. Thus, if we enter in this method, 'm_pTAPForQuickRefOrFullCat' is valid!
	// HYS-1057: Delete DocSheetProduct and DocSheetType: We have one king of product only full catalog
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_DISPLAYDOCUMENTATION,0,
			( LPARAM )m_pTAPForFullCatalog->GetIDPtr().ID );
}

void CRViewHMCalc::OnUpdateMenuText( CCmdUI *pCmdUI )
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

LRESULT CRViewHMCalc::ComboDropDown( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return 0;
	}

	SS_CELLCOORD *pCellCoord = ( SS_CELLCOORD * )lParam;
	m_pSheetHMCalc->ComboDropDown( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CRViewHMCalc::DragDropBlock( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return 0;
	}

	SS_DRAGDROP *pDragDrop = ( SS_DRAGDROP * )lParam;
	m_pSheetHMCalc->DragDropBlock(	pDragDrop->BlockPrev.UL.Col, pDragDrop->BlockPrev.UL.Row, pDragDrop->BlockPrev.LR.Col, pDragDrop->BlockPrev.LR.Row,
									pDragDrop->BlockNew.UL.Col, pDragDrop->BlockNew.UL.Row, pDragDrop->BlockNew.LR.Col, pDragDrop->BlockNew.LR.Row,
									&pDragDrop->fCancel );
	return 0;
}

LRESULT CRViewHMCalc::EditChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return 0;
	}

	SS_CELLCOORD *pCellCoord = ( SS_CELLCOORD * )lParam;
	m_pSheetHMCalc->EditChange( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CRViewHMCalc::LeaveCell( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return 0;
	}

	SS_LEAVECELL *pLeaveCell = ( SS_LEAVECELL * )lParam;
	BOOL fCancel;
	m_pSheetHMCalc->LeaveCell( pLeaveCell->ColCurrent, pLeaveCell->RowCurrent, pLeaveCell->ColNew, pLeaveCell->RowNew, &fCancel );
	return fCancel;
}

LRESULT CRViewHMCalc::SheetChanged( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return 0;
	}

	Invalidate();
	m_pSheetHMCalc->SheetChanged( LOWORD( lParam ), HIWORD( lParam ) );
	return 0;
}

LRESULT CRViewHMCalc::TextTipFetch( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pSheetHMCalc || NULL == m_pSheetHMCalc->GetSafeHwnd() )
	{
		return 0;
	}

	SS_TEXTTIPFETCH *pTextTipFetch = ( SS_TEXTTIPFETCH * )lParam;
	// If hText is not empty, spread takes hText in place of szText!
	pTextTipFetch->hText = NULL;
	m_pSheetHMCalc->TextTipFetch( pTextTipFetch->Col, pTextTipFetch->Row, &pTextTipFetch->wMultiLine, &pTextTipFetch->nWidth, pTextTipFetch->szText,
								  &pTextTipFetch->fShow );
	return 0;
}

void CRViewHMCalc::_TabProjCreate( CreateMode eCreateMode )
{
	/* For a next release.
	CDlgCreateModuleTiny dlg;
	int iReturn = dlg.Display( CDlgCreateModuleTiny::DialogMode::CreateModule, 0.0, _T(""), enum_VDescriptionType::edt_TADBValve, 0.0, -1.0 );
	if( IDOK == iReturn )
	{
		CDS_HydroMod* pHMToReset = NULL;
		bool fCreateDone = false;
		CArray<long> arlProductSelected;
		int iNbrProductSelected = m_pSheetHMCalc->GetSelectedRows( &arlProductSelected );
		for( int iLoopSelectedLine = 0; iLoopSelectedLine < iNbrProductSelected; iLoopSelectedLine++ )
		{
			CDS_HydroMod* pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( arlProductSelected.GetAt( iLoopSelectedLine ) );
			if( NULL == pHM )
				continue;

			// Now we can run create on all selected lines.
			CTable* pDestTable = NULL;
			int iTargetLevel, iNewPos;
			if( CreateMode::CreateIn == eCreateMode )
			{
				// User create module on a module, we add these modules at the end of this module.

				// Destination is set to the current module selected.
				pDestTable = (CTable*)pHM;

				// Level will be the incremented by 1 because the object is copied under the module selected.
				iTargetLevel = pHM->GetLevel() + 1;

				// Added at the end of the module selected.
				iNewPos = pHM->GetCount() + 1;

				// We will reset on the module that has the lower level.
				if( NULL == pHMToReset )
					pHMToReset = pHM;
				else if( pHMToReset->GetLevel() > pHM->GetLevel() )
					pHMToReset = pHM;
			}
			else
			{
				// User wants to create object before the selected object.

				// Destination is set to the parent of the current line selected.
				pDestTable = pHM->GetParent();

				// Level is the same as the current object selected.
				iTargetLevel = pHM->GetLevel();

				// Added just before the selected object.
				iNewPos = pHM->GetPos();

				// We will reset on the module that has the lower level.
				if( NULL == pHMToReset )
					pHMToReset = pHM->GetParent();
				else if( pHMToReset->GetLevel() > pHM->GetParent()->GetLevel() )
					pHMToReset = pHM->GetParent();
			}

			// Create the module.
			IDPTR ModuleIDptr;
			if( false == m_pTADS->CreateObject( ModuleIDptr, CLASS(CDS_HydroMod ) ) )
				continue;
			pDestTable->Insert( ModuleIDptr );
			CDS_HydroMod* pHMCreated = (CDS_HydroMod *)ModuleIDptr.MP;
			if( NULL != pHMCreated )
			{
				// Reset level, pos and rename module.
				m_pTADS->SetHMLevel( pHMCreated, iTargetLevel );

				// If we insert, we must increment the position for object following the new inserted one.
				if( CreateMode::CreateBefore == eCreateMode )
				{
					for( IDPTR IDPtr = pDestTable->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pDestTable->GetNext() )
					{
						if( ( (CDS_HydroMod *)IDPtr.MP )->GetPos() >= iNewPos )
							( (CDS_HydroMod *)IDPtr.MP )->SetPos( ( (CDS_HydroMod *)IDPtr.MP )->GetPos() + 1 );
					}
				}

				pHMCreated->SetPos( iNewPos++ );
				pHMCreated->RenameMe();


				pHMCreated->SetQDesign( dlg.GetFlow() );
				pHMCreated->SetFlagModule( true );
				pHMCreated->SetCBIValveID( dlg.GetSelTADBID() );
				pHMCreated->SetUid( m_pTADS->GetpProjectParams()->GetNextUid() );
				pHMCreated->SetPresetting( dlg.GetPresetting() );

				// Complete the module to be compatible in HMCalc mode.
				pHMCreated->SetKvCv( dlg.GetKvCv() );
				pHMCreated->SetVDescrType( dlg.GetVDescrType() );

				fCreateDone = true;
			}
		}

		if( true == fCreateDone )
			pTabCDialogProj->PostMessage( WM_USER_RESETHMTREE, (WPARAM)false, (LPARAM)pHMToReset );
	}
	*/

	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM || NULL == pDlgLeftTabProject )
	{
		return;
	}

	pDlgLeftTabProject->AddModule( pHM->GetPos() );
}

void CRViewHMCalc::_OnContextMenuDelete()
{
	// Clean cutting object if exist.
	ResetFlagCut();
	m_pTADS->CleanClipboard();

	m_pSheetHMCalc->DeleteHMRow();
}

void CRViewHMCalc::_OnContextMenuCopy()
{
	CArray<long> aSelRows;
	m_pSheetHMCalc->GetSelectedRows( &aSelRows );

	if( 0 == aSelRows.GetCount() )
	{
		return;
	}

	BeginWaitCursor();

	// Clean cutting object if exist.
	ResetFlagCut();
	m_pTADS->CleanClipboard();

	// First pass all the selected line to see if there is different level or not.
	// Remark: the first key is for the level. The first key for the second map is for the position.
	std::map<int, std::map< int, CDS_HydroMod *> > mapHMToCopy;

	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( aSelRows.GetAt( i ) );

		if( NULL == pHM )
		{
			continue;
		}

		mapHMToCopy[pHM->GetLevel()][pHM->GetPos()] = pHM;
	}

	ASSERT( 1 == mapHMToCopy.size() || 2 == mapHMToCopy.size() );

	// Second pass, copy map beginning by the lowest level.
	// Remark: in 'SheetHMCalc', we have three possibilities of selection. Either user selects only the parent, either only one
	//         on more children or a mix of parent and its children. If level of the selected 'CDS_HydroMod' are all the same,
	//         we copy all objects directly with "CLIPBOARD_TABLE" as parent. Otherwise, we copy first level with "CLIPBOARD_TABLE"
	//         as parent, and children are copied with first level as parent.
	CTable *pClipboardTable = m_pTADS->GetpClipboardTable();
	ASSERT( NULL != pClipboardTable );
	
	std::map<int, std::map< int, CDS_HydroMod *> >::iterator mapLevelIter;
	bool bAtLeastOneCopyDone = false;
	CDS_HydroMod *pParentHM = NULL;

	for( mapLevelIter = mapHMToCopy.begin(); mapLevelIter != mapHMToCopy.end(); mapLevelIter++ )
	{
		// If only one level exist...
		if( 1 == ( int )mapHMToCopy.size() )
		{
			// Run each object and save it in the clipboard.
			std::map< int, CDS_HydroMod *>::iterator mapPosIter;

			for( mapPosIter = mapLevelIter->second.begin(); mapPosIter != mapLevelIter->second.end(); mapPosIter++ )
			{
				CDS_HydroMod *pHM = mapPosIter->second;
				m_pTADS->CreateCopyHM( pHM->GetIDPtr(), pClipboardTable, false );
				bAtLeastOneCopyDone = true;
			}
		}
		else
		{
			// If it's the first...
			if( mapLevelIter == mapHMToCopy.begin() )
			{
				// Save parent in the clipboard.
				// Remark: 'true' to not copy children. These ones will be copy below just for those that user
				//         has selected.
				std::map< int, CDS_HydroMod *>::iterator mapPosIter = mapLevelIter->second.begin();
				CDS_HydroMod *pHMToCopy = mapPosIter->second;
				IDPTR IDPtr = m_pTADS->CreateCopyHM( pHMToCopy->GetIDPtr(), pClipboardTable, true );

				if( _NULL_IDPTR == IDPtr || NULL == IDPtr.MP )
				{
					continue;
				}

				pParentHM = (CDS_HydroMod *)( IDPtr.MP );
				bAtLeastOneCopyDone = true;
			}
			else
			{
				// Save children in the clipboard.
				// Pay attention: in this case user can for example select a parent and a child at position 4. If we want to copy
				//                only these two objects, we have to change position of the child to 1.
				int iPos = 1;
				std::map< int, CDS_HydroMod *>::iterator mapPosIter;

				for( mapPosIter = mapLevelIter->second.begin(); mapPosIter != mapLevelIter->second.end(); mapPosIter++ )
				{
					CDS_HydroMod *pHMToCopy = mapPosIter->second;
					IDPTR IDPtrCopied = m_pTADS->CreateCopyHM( pHMToCopy->GetIDPtr(), pParentHM, false );

					if( _NULL_IDPTR == IDPtrCopied || NULL == IDPtrCopied.MP )
					{
						continue;
					}

					( (CDS_HydroMod *)( IDPtrCopied.MP ) )->SetPos( iPos++ );
					bAtLeastOneCopyDone = true;
				}
			}
		}
	}

	if( true == bAtLeastOneCopyDone )
	{
		SetLastCopyCutOperation( LastOperation::Copy );
	}

	EndWaitCursor();
}

void CRViewHMCalc::_OnContextMenuPaste( PasteMode ePasteMode )
{
	CArray<long> arlProductSelected;
	int iNbrProductSelected = m_pSheetHMCalc->GetSelectedRows( &arlProductSelected );
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
	std::vector<CDS_HydroMod *> vecHMSelected;
	CDS_HydroMod *pHMWhereToInsert = NULL;

	for( int iLoopSelectedLine = 0; iLoopSelectedLine < iNbrProductSelected; iLoopSelectedLine++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( arlProductSelected.GetAt( iLoopSelectedLine ) );

		if( NULL == pHM )
		{
			continue;
		}

		if( PasteMode::PasteBefore == ePasteMode && iNbrProductSelected > 1 )
		{
			if( NULL == pHMWhereToInsert || pHM->GetPos() < pHMWhereToInsert->GetPos() 
					|| ( pHM->GetLevel() == pHMWhereToInsert->GetLevel() && pHM->GetPos() < pHMWhereToInsert->GetPos() ) )
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
		CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( ( CData * )IDPtr.MP );

		if( pHMInClipboard != NULL )
		{
			// We take only the case for root module (level = 0) with pump that are not a circuit in injection.
			if( 0 == pHMInClipboard->GetLevel() && pHMInClipboard->GetpPump() != NULL &&
				(pHMInClipboard->GetpSchcat() != NULL && false == pHMInClipboard->GetpSchcat()->IsSecondarySideExist()))
			{
				bAtLeastOneRootPump = true;
			}
		}

		if( false == bAtLeastOneRootPump )
		{
			IDPtr = m_pTADS->GetNextHMFromClipBoard();
		}
	}

	if( true == bAtLeastOneRootPump )
	{
		// Because in the right view it's impossible to do a copy on the root level (only possible in the left tab view), if we have
		// a root pump circuit we can thus automatically disable this item.
		CString str = TASApp.LoadLocalizedString( IDS_RVHMCALC_CANTPASTEPUMPCIRCUIT );
		::AfxMessageBox( ( LPCTSTR )str, MB_OK | MB_ICONSTOP );
		return;
	}

	// Verify the return mode of the objects in the clipboard in regards to the objects where user wants to paste them.
	bool bAllCanPasteIn = true;
	bool bAllCanPasteBefore = true;
	IDPtr = m_pTADS->GetFirstHMFromClipBoard();

	while( NULL != IDPtr.MP && ( true == bAllCanPasteIn || true == bAllCanPasteBefore ) )
	{
		CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( ( CData * )IDPtr.MP );

		// Run all line selected.
		for( int iLoopArray = 0; iLoopArray < iNbrProductSelected && ( true == bAllCanPasteIn || true == bAllCanPasteBefore ); iLoopArray++ )
		{
			CDS_HydroMod *pHMSelected = (CDS_HydroMod *)m_pSheetHMCalc->GetpHM( arlProductSelected.GetAt( iLoopArray ) );

			if( NULL != pHMSelected )
			{
				// Test 'Paste in'.
				// Remark: if there is no child in the current selected module, than we can pass direct or return for
				//         the first time.
				if( pHMSelected->GetCount() > 0 )
				{
					CDS_HydroMod *pCHildHM = (CDS_HydroMod *)( pHMSelected->GetFirst().MP );

					if( NULL != pCHildHM && pHMInClipboard->GetReturnType() != pCHildHM->GetReturnType() )
					{
						bAllCanPasteIn = false;
					}
				}

				// Test 'Paste before'.
				if( pHMInClipboard->GetReturnType() != pHMSelected->GetReturnType() )
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

		if( IDNO == ::AfxMessageBox( ( LPCTSTR )str, MB_YESNO | MB_ICONWARNING ) )
		{
			return;
		}

		bApplyReturnCorrection = true;
	}

	// HYS-1716: verify if there is no error with secondary temperatures of main and child hydraulic circuits when pasting.
	IDPtr = m_pTADS->GetFirstHMFromClipBoard();
	bool bIsInjectionHydraulicCircuitTemperatureError = false;
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
				pclDestinationTable = iterSelectedLine->GetParent();
			}

			bIsInjectionHydraulicCircuitTemperatureError |= pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForPasteOperation( pHMInClipboard, pclDestinationTable, 
					&mmapInjectionHydraulicCircuitWithTemperatureError, &vecAllInjectionCircuitsWithTemperatureError );
		}
		
		IDPtr = m_pTADS->GetNextHMFromClipBoard();
	}

	CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode = CDlgInjectionError::ReturnCode::Undefined;

	if( true == bIsInjectionHydraulicCircuitTemperatureError )
	{
		// Show the dialog to ask user if he wants to apply this new temperature without changing all the children in errors, or if wants to apply and 
		// automatically correct the errors or if he wants to cancel.
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
	CDS_HydroMod *pHMToReset = NULL;
	int iTargetLevel, iNewPos;
	bool bPasteDone = false;

	for( std::vector<CDS_HydroMod *>::iterator iterSelectedLine = vecHMSelected.begin(); iterSelectedLine != vecHMSelected.end(); iterSelectedLine++ )
	{
		CDS_HydroMod *pHM = *iterSelectedLine;

		if( NULL == pHM )
		{
			continue;
		}

		if( PasteMode::PasteIn == ePasteMode )
		{
			// User pastes object on a module, we add these objects at the end of this module.

			// Destination is set to the current module selected.
			pDestTable = (CTable *)pHM;

			// Level will be the incremented by 1 because the object is copied under the module selected.
			iTargetLevel = pHM->GetLevel() + 1;

			// Added at the end of the module selected.
			iNewPos = pHM->GetCount() + 1;

			// We will reset on the module that has the lower level.
			if( NULL == pHMToReset )
			{
				pHMToReset = pHM;
			}
			else if( pHMToReset->GetLevel() > pHM->GetLevel() )
			{
				pHMToReset = pHM;
			}
		}
		else
		{
			// User wants to insert object before the selected object.

			// Destination is set to the parent of the current line selected.
			pDestTable = pHM->GetParent();

			// Level is the same as the current object selected.
			iTargetLevel = pHM->GetLevel();

			// Added just before the selected object.
			iNewPos = pHM->GetPos();

			// We will reset on the module that has the lower level.
			if( NULL == pHMToReset )
			{
				pHMToReset = pHM->GetParent();
			}
			else if( pHMToReset->GetLevel() > pHM->GetParent()->GetLevel() )
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

				// HYS-1716: if there ere errors and these one must be automatically corrected (Choice validated by user in the 'CDlgInjectionError' dialog above).
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
		if( LastOperation::Cut == GetLastCopyCutOperation() )
		{
			// We need to do this here. Because once hydromod will be deleted in the code below, pointer in the tree will be no more valid.
			ResetFlagCut();

			CTable *pPipTab = m_pTADS->GetpHydroModTable();
			ASSERT( NULL != pPipTab );

			std::vector<CDS_HydroMod *> vecHMToDelete;
			IDPTR IDPtrLoop = m_pTADS->GetFirstHMFromClipBoard();

			while( NULL != IDPtrLoop.MP )
			{
				CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( ( CData * )( IDPtrLoop.MP ) );

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

			for( int iLoopHM = 0; iLoopHM < ( int )vecHMToDelete.size(); iLoopHM++ )
			{
				m_pTADS->DeleteHM( vecHMToDelete[iLoopHM] );
			}

			// Also, if cut operation is done, we remove all objects in the clipboard (it's not the same as the copy operation where
			// user can in this case do copy more than one time).
			m_pTADS->CleanClipboard();
			SetLastCopyCutOperation( LastOperation::Undefined );
		}

		// HYS-1716: Now that the paste operation has been applied, we need to run all chidren injection circuit to update their
		// primary flows.
		// Remark: here we pass as argument the hydraulic circuit in which we have done the paste because we do not need to check all the network 
		//         but only the children of this hydraulic circuit.
		pclHydraulicNetwork->CorrectAllPrimaryFlow( pDestTable );

		// Compute all installation only if TASelect is in HM calculation mode and the project is not frozen.
		CDS_ProjectParams *pProjectParam = m_pTADS->GetpProjectParams();
		ASSERT( NULL != pProjectParam );

		if( true == pMainFrame->IsHMCalcChecked() && false == pProjectParam->IsFreezed() )
		{
			m_pTADS->ComputeAllInstallation();
		}

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, ( LPARAM )pHMToReset );
		}
	}

	EndWaitCursor();
}

void CRViewHMCalc::_OnContextMenuPrint()
{
	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	// Clean cutting object if exist.
	ResetFlagCut();
	m_pTADS->CleanClipboard();

	if( NULL != pRViewProj )
	{
		pRViewProj->ClearPrintingFlags();
		pRViewProj->PrintModule( pHM, false );
	}
}

void CRViewHMCalc::_OnContextMenuPrintRecursive()
{
	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	// Clean cutting object if exist.
	ResetFlagCut();
	m_pTADS->CleanClipboard();

	if( NULL != pRViewProj )
	{
		pRViewProj->ClearPrintingFlags();
		pRViewProj->PrintModule( pHM, true );
	}
}

void CRViewHMCalc::_OnContextMenuExport()
{
	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	CString PrjDir = GetProjectDirectory();

	// Load file filter, compose file name and initialize 'CFileDialog'.
	CString str = TASApp.LoadLocalizedString( IDS_EXPORTXLSFILTER );
	CString strExt = _T("xls");
	CString strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

	CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, ( LPCTSTR )str, NULL );
	dlg.m_ofn.lpstrInitialDir = ( LPCTSTR )PrjDir;

	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();

		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		// Create a 'SheetHMCalc' with multiple tabs.
		CSheetHMCalc SheetHMCalc;

		// Remark: for the parent, we intentionally take the main window. If we set 'CRViewHMCalc' as the parent, there are some refresh called
		//         (by who?) that cause flicking when exporting big project.
		SheetHMCalc.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), TASApp.GetMainWnd(), IDC_TMPSSHEET );
		SheetHMCalc.ModifyStyle( WS_VISIBLE, 0 );
		SheetHMCalc.Init( pHM, false, 0, true );
		SheetHMCalc.PrepareforExport();
		SheetHMCalc.UnSelectMultipleRows();
		SheetHMCalc.ExportExcelBook( dlg.GetPathName(), NULL );

		EndWaitCursor();
	}
}

void CRViewHMCalc::_OnContextMenuExportRecursive()
{
	CDS_HydroMod *pHM = m_pSheetHMCalc->GetSelectedHM();

	if( NULL == pHM )
	{
		return;
	}

	// Verify that the number of tabs in the sheet will not go upper than 1000 tabs.
	// By default and to be sure, we take a maximum of 950.
	// Remark: 'MAX_SHEETS' is defined in 'SSheet.h'.
	int iModuleCount = pHM->GetCount( true, true );

	if( ( iModuleCount * CSheetHMCalc::SheetDescription::SLast ) > ( MAX_SHEETS - 50 ) )
	{
		CString str;
		str.Format( TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_EXPORTXLS_EXCEDED ), iModuleCount, ( MAX_SHEETS - 50 ) / CSheetHMCalc::SheetDescription::SLast );
		CWnd::MessageBox( str, TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_EXPORTXLS_EXCEDED_TITLE ) , MB_ICONEXCLAMATION );
		return;
	}

	CString PrjDir = GetProjectDirectory();

	// Load file filter, compose file name and initialize 'CFileDialog'.
	CString str = TASApp.LoadLocalizedString( IDS_EXPORTXLSFILTER );
	CString strExt = _T("xls");
	CString strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

	CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, ( LPCTSTR )str, NULL );
	dlg.m_ofn.lpstrInitialDir = ( LPCTSTR )PrjDir;

	if( IDOK == dlg.DoModal() )
	{
		// Clean cutting object if exist.
		ResetFlagCut();
		m_pTADS->CleanClipboard();

		// Create a control bar dialog.
		CDlgProgBarExportXls dlgExp( 0, TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_EXPORTBOXTITLE ) );
		dlgExp.SetRange( 1, iModuleCount );

		// Create a sheetHMCalc with multiple tabs.
		CSheetHMCalc SheetHMCalc;

		// Remark: for the parent, we intentionally take the main window. If we set 'CRViewHMCalc' as the parent, there are some refresh called
		//         (by who?) that cause flicking when exporting big project.
		SheetHMCalc.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), TASApp.GetMainWnd(), IDC_TMPSSHEET );
		SheetHMCalc.ModifyStyle( WS_VISIBLE, 0 );

		dlgExp.SetParamForExport( &SheetHMCalc, pHM, ( MAX_SHEETS - 50 ) );
		dlgExp.Display( false );

		if( true == dlgExp.CanExport() )
		{
			BeginWaitCursor();
			SheetHMCalc.PrepareforExport();
			SheetHMCalc.UnSelectMultipleRows();
			SheetHMCalc.ExportExcelBook( dlg.GetPathName(), NULL );
			EndWaitCursor();
		}
	}
}
