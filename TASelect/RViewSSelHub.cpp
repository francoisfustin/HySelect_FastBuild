#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"

#include "HMInclude.h"
#include "DlgAccessory.h"
#include "HubHandler.h"
#include "SSheetHub.h"

#include "HMHub.h"
#include "RViewSSelSS.h"
#include "RViewSSelHub.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelHub.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SPREADYPOS 200
#define IMAGEWIDTH 10//320
#define IMAGEHEIGHT 10//190

CRViewSSelHub *pRViewSSelHub = NULL;

IMPLEMENT_DYNCREATE( CRViewSSelHub, CFormViewEx )

CRViewSSelHub::CRViewSSelHub()
	: CFormViewEx( CRViewSSelHub::IDD )
{
	m_pSheetTAHub =	NULL;
	m_pSheetTAStation = NULL;
	m_pCurSheet = NULL;
	m_lRow = 0;
	m_lColumn = 0;
	m_iXpos = 0;
	m_iYpos = 0;
	m_fMustbeSized = false;
	pRViewSSelHub = this;
}

CRViewSSelHub::~CRViewSSelHub()
{
	m_EnBmpHub.DeleteObject();
	m_EnBmpBox.DeleteObject();

	if( NULL != m_pSheetTAHub )
	{
		delete m_pSheetTAHub;
	}

	if( NULL != m_pSheetTAStation )
	{
		delete m_pSheetTAStation;
	}
	
	pRViewSSelHub = NULL;
}

void CRViewSSelHub::DoDataExchange( CDataExchange* pDX )
{
	CFormViewEx::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CRViewSSelHub, CFormViewEx )
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_COMMAND( ID_FLTMRVHMCALC_DELETE, OnFltRvSSelHubDelete )
	ON_COMMAND( ID_FLTMRVHMCALC_COPY, OnFltRvSSelHubCopy )
	ON_COMMAND( ID_FLTMRVHMCALC_PASTEIN, OnFltRvSSelHubPaste )
	ON_COMMAND( ID_FLTMRVHMCALC_LOCK, OnFltRvSSelHubLock )
	ON_COMMAND( ID_FLTMRVHMCALC_UNLOCK, OnFltRvSSelHubUnLock )
	ON_COMMAND( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT, OnFltRvSSelHubLockAll )
	ON_COMMAND( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT, OnFltRvSSelHubUnlockAll )
	ON_COMMAND( ID_FLTMRVHMCALC_LOCKCOLUMN, OnFltRvSSelHubLockcolumn )
	ON_COMMAND( ID_FLTMRVHMCALC_UNLOCKCOLUMN, OnFltRvSSelHubUnlockcolumn )
	ON_COMMAND( ID_FLTMRVHMCALC_EDITACCESSORIES, OnFltRvSSelHubEditaccessories )
	ON_COMMAND( ID_FLTMRVHMCALC_COPYACCESSORIES, OnFltRvSSelHubCopyAccessories )
	ON_COMMAND( ID_FLTMRVHMCALC_PASTEACCESSORIES, OnFltRvSSelHubPasteAccessories )
	ON_COMMAND( ID_FLTMRVHMCALC_GETFULLINFO, OnGetFullInfoHub )
	ON_UPDATE_COMMAND_UI_RANGE( ID_FLTMRVHMCALC_EDIT, ID_FLTMRVHMCALC_GETFULLINFO, OnUpdateMenuText )
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CREATE()

	// Spread DLL messages
	ON_MESSAGE( SSM_CLICK , ClickFpspread )
	ON_MESSAGE( SSM_COMBODROPDOWN, ComboDropDown )
	ON_MESSAGE( SSM_DBLCLK , DblClickFpspread )
	ON_MESSAGE( SSM_DRAGDROP, DragDropBlock )
	ON_MESSAGE( SSM_EDITCHANGE, EditChange )
	ON_MESSAGE( SSM_KEYDOWN, KeyDown )
	ON_MESSAGE( SSM_KEYPRESS, KeyPress )
	ON_MESSAGE( SSM_LEAVECELL, LeaveCell )
	ON_MESSAGE( SSM_RBUTTON, RightClick )
	ON_MESSAGE( SSM_SHEETCHANGED, SheetChanged )
	ON_MESSAGE( SSM_TEXTTIPFETCH, TextTipFetch )

END_MESSAGE_MAP()


// CRViewSSelHub diagnostics

#ifdef _DEBUG
void CRViewSSelHub::AssertValid() const
{
	CFormViewEx::AssertValid();
}

void CRViewSSelHub::Dump( CDumpContext& dc ) const
{
	CFormViewEx::Dump( dc );
}
#endif //_DEBUG

BOOL CRViewSSelHub::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext )
{
	// Create the RViewSSelHub
	if( !CFormViewEx::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, CRViewSSelHub::IDD, pContext ) )
		return FALSE;
	
	// Create the Spread Sheets
	if( NULL != m_pSheetTAHub )
		delete m_pSheetTAHub;
	m_pSheetTAHub = new CSSheetTAHub();			ASSERT( m_pSheetTAHub );
	if( NULL != m_pSheetTAHub )
	{
		if( !m_pSheetTAHub->Create( (GetStyle() | WS_CHILD ) ^ WS_VISIBLE /*^ WS_BORDER*/, rect, this, IDC_FPSPREADHUB ) )
			return FALSE;
	}
	
	if( NULL != m_pSheetTAStation )
		delete m_pSheetTAStation;
	m_pSheetTAStation = new CSSheetTAHub();		ASSERT( m_pSheetTAStation );
	if( NULL != m_pSheetTAStation )
	{
		if( !m_pSheetTAStation->Create( (GetStyle() | WS_CHILD ) ^ WS_VISIBLE /*^ WS_BORDER*/, rect, this, IDC_FPSPREADSTATION ) )
			return FALSE;
	}

	return TRUE;
}

void CRViewSSelHub::RefreshMvLoc()
{
	if( NULL != pDlgIndSelHub )
	{
		pDlgIndSelHub->FillComboMvLoc();
	}
}

void CRViewSSelHub::RefreshScrollBars()
{
	SendMessage( WM_SIZE, SIZE_RESTORED, 0 );
	SendMessage( WM_PAINT, 0, 0 );
}

void CRViewSSelHub::RefreshSheets()
{
	TASApp.OverridedSetRedraw( this, FALSE );
	
	m_pSheetTAHub->FillSheetHub();
	m_pSheetTAStation->FillSheetHubStations();
	m_pSheetTAHub->RefreshCollapseExpandButton( m_pSheetTAStation );
	
	CHubHandler	mHubHandler;
	mHubHandler.Attach( m_pSheetTAHub->GetpHub()->GetIDPtr().ID );
	m_EnBmpHub.DeleteObject();
	m_EnBmpBox.DeleteObject();
	
	if( true == mHubHandler.GetHubImg( &m_EnBmpHub ) )
	{
		m_EnBmpHub.ResizeImage( 0.35 );
		mHubHandler.GetBoxImg( &m_EnBmpBox );
		m_EnBmpBox.ResizeImage( 0.35 );
	}
	
	TASApp.OverridedSetRedraw( this, TRUE );
	SendMessage( WM_SIZE,SIZE_RESTORED, 0 );
	SendMessage( WM_PAINT, 0, 0 );
}

void CRViewSSelHub::SetRedraw( CDS_HmHub *pHM )
{
	if( m_pSheetTAHub->GetSafeHwnd() && m_pSheetTAStation->GetSafeHwnd() )
	{ 
		TASApp.OverridedSetRedraw( this, FALSE );
		m_pSheetTAHub->SetMaxCols( 0 );
		m_pSheetTAHub->SetMaxRows( 0 );
		m_pSheetTAStation->SetMaxCols( 0 );
		m_pSheetTAStation->SetMaxRows( 0 );
		m_pSheetTAHub->Init( true, pHM );
		m_pSheetTAStation->Init( false, pHM );	
		TASApp.OverridedSetRedraw( this, TRUE );

		RefreshSheets();
		// To force scroll bar redraw if needed
	}
}

void CRViewSSelHub::UpdatePicture( bool fUseOldPos )
{
	int iXPos = ( true == fUseOldPos ) ? m_iXpos : GetScrollPos( SB_HORZ );
	int iYPos = ( true == fUseOldPos ) ? m_iYpos : GetScrollPos( SB_VERT );
	
	CDC *pDC = GetDC();
	CRect rect;
	GetClientRect( (LPRECT)rect );
	
	// Fill background with White brush
	CBrush BGBrush;
	BGBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );
	pDC->FillRect( rect, &BGBrush );
	BGBrush.DeleteObject();

	m_EnBmpHub.DrawOnDC( pDC, CRect( 30 - iXPos, 10 - iYPos, 0, 0 ), false );
	m_EnBmpBox.DrawOnDC( pDC, CRect( 500 - iXPos, 10 - iYPos, 0, 0 ), false );
	ReleaseDC( pDC );
}

void CRViewSSelHub::DragDropBlockStation( long lOldFromColumn, long lOldFromRow, long lOldToColumn, long lOldToRow, long lNewFromColumn, long lNewFromRow, long lNewToColumn, long lNewToRow, BOOL* pfCancel)
{
	m_pSheetTAStation->DragDropBlock( lOldFromColumn, lOldFromRow, lOldToColumn, lOldToRow, lNewFromColumn, lNewFromRow, lNewToColumn, lNewToRow, *pfCancel );
}

void CRViewSSelHub::LeaveCellHub( long lColumn, long lRow, long lNewColumn, long lNewRow, BOOL *pfCancel )
{
	m_pSheetTAHub->LeaveCell( lColumn, lRow, lNewColumn, lNewRow, pfCancel );
}

void CRViewSSelHub::LeaveCellStation( long lColumn, long lRow, long lNewColumn, long lNewRow,BOOL *pfCancel )
{
	m_pSheetTAStation->LeaveCell( lColumn, lRow, lNewColumn, lNewRow, pfCancel );
}

void CRViewSSelHub::ClickFpspreadHub( long lColumn, long lRow )
{
	m_pCurSheet = m_pSheetTAHub;
	
	bool fMustResize = false;
	m_pSheetTAHub->CellClicked( lColumn, lRow, m_pSheetTAStation, &fMustResize );
	
	if( true == fMustResize )
	{
		SendMessage( WM_SIZE,SIZE_RESTORED, 0 );
		SendMessage( WM_PAINT, 0, 0 );
	}
}

void CRViewSSelHub::ClickFpspreadStation( long lColumn, long lRow )
{
	m_pCurSheet = m_pSheetTAStation;
	
	bool fMustResize = false;
	m_pSheetTAStation->CellClicked( lColumn, lRow, m_pSheetTAHub, &fMustResize );
	
	if( true == fMustResize )
	{
		SendMessage( WM_SIZE,SIZE_RESTORED, 0 );
		SendMessage( WM_PAINT, 0, 0 );
	}
}

void CRViewSSelHub::RightClickHub( short nClickType, long lColumn, long lRow, long lMouseX, long lMouseY )
{
	m_pCurSheet = m_pSheetTAHub;
	if( true == IsLocked() )
		return;
	m_pCurSheet->SelectRow( lRow );
}

void CRViewSSelHub::RightClickStation( short nClickType, long lColumn, long lRow, long lMouseX, long lMouseY )
{
	m_pCurSheet = m_pSheetTAStation;
	if( true == IsLocked() )
		return;
	m_pCurSheet->SelectRow( lRow );
}

bool CRViewSSelHub::IsLocked()
{
	if( NULL != pDlgIndSelHub )
	{
		return pDlgIndSelHub->IsLocked();
	}
	else
	{
		return false;
	}
}

void CRViewSSelHub::CheckSelectButton( CDS_HmHub *pHmHub )
{
	if( NULL != pDlgIndSelHub ) 
	{
		pDlgIndSelHub->CheckSelectButton( pHmHub );
	}
}

void CRViewSSelHub::DrawScrollBars()
{
	if( !m_pSheetTAHub->GetSafeHwnd() || !m_pSheetTAStation->GetSafeHwnd() )
		return;

	CRect RectHub, RectHubStations;
	m_pSheetTAHub->GetClientRect( (LPRECT)RectHub );
	m_pSheetTAStation->GetClientRect( (LPRECT)RectHubStations );
	
	int iWidth = max( RectHub.Width(), RectHubStations.Width() );
	iWidth = max( iWidth, RIGHTVIEWWIDTH );
	m_pSheetTAStation->GetWindowRect( (LPRECT)RectHubStations );
	ScreenToClient( (LPRECT)RectHubStations );
	int iBottom = RectHubStations.bottom - 10;
	
	// Show Scroll bars if needed
	CRect rect;
	GetClientRect( (LPRECT)rect );
	int iSB = -1;
	
	// Horizontal SB
	if( iWidth > rect.Width() )
		iSB = SB_HORZ;
	
	// Vertical SB
	if( iBottom > rect.bottom )
		iSB = SB_VERT;
	if( iWidth > rect.Width() && iBottom > rect.bottom )
		iSB = SB_BOTH;

	SCROLLINFO ScrollInfo;
	if( SB_HORZ == iSB )
	{
		m_iYpos = 0;
		ShowScrollBar( iSB, true );
		ShowScrollBar( SB_VERT, false );
		GetScrollInfo( SB_HORZ, &ScrollInfo );
		ScrollInfo.nMax = max( iWidth, 0 );
		ScrollInfo.nMin = 0;
		ScrollInfo.nPage = rect.Width();
		SetScrollInfo( SB_HORZ, &ScrollInfo, false );
	}
	else if( SB_VERT == iSB )
	{
		m_iXpos = 0;
		ShowScrollBar( iSB, true );
		ShowScrollBar( SB_HORZ, false );
		GetScrollInfo( SB_VERT, &ScrollInfo );
		ScrollInfo.nMax = max( iBottom, 0 );
		ScrollInfo.nMin = 0;
		ScrollInfo.nPage = rect.Height();
		SetScrollInfo( SB_VERT, &ScrollInfo, false );

	}
	else if( SB_BOTH == iSB )
	{
		ShowScrollBar( iSB, true );
		SCROLLINFO ScrollInfo;
		GetScrollInfo( SB_HORZ, &ScrollInfo );
		ScrollInfo.nMax = max( iWidth, 0 );
		ScrollInfo.nMin = 0;
		ScrollInfo.nPage = rect.Width();
		SetScrollInfo( SB_HORZ, &ScrollInfo, false );
		GetScrollInfo( SB_VERT, &ScrollInfo );
		ScrollInfo.nMax = max( iBottom, 0 );
		ScrollInfo.nMin = 0;
		ScrollInfo.nPage = rect.Height();
		SetScrollInfo( SB_VERT, &ScrollInfo, false );
	}
	else 
	{
		m_iXpos = 0;
		m_iYpos = 0;
		ShowScrollBar( SB_BOTH, false );
	}
}

void CRViewSSelHub::SetHMSheetsPos()
{
	if( !m_pSheetTAHub->GetSafeHwnd() || !m_pSheetTAStation->GetSafeHwnd() )
		return;

	const int iSpace = 20;
	CRect RectHub = m_pSheetTAHub->GetSheetSizeInPixels();
	CRect RectHubStations = m_pSheetTAStation->GetSheetSizeInPixels();
	int iWidth = max( RectHub.Width(), RectHubStations.Width() );
	iWidth += 50;
	CRect rect;
	GetClientRect( (LPRECT)rect );
	iWidth = max( rect.Width(), iWidth );
	iWidth = max( iWidth, RIGHTVIEWWIDTH );
	int iXPos = 0;
	int iYPos = SPREADYPOS - m_iYpos;

	m_pSheetTAHub->MoveWindow( iXPos - m_iXpos, iYPos, iWidth, RectHub.Height() + iSpace / 2, FALSE );
	m_pSheetTAStation->MoveWindow( iXPos - m_iXpos, iYPos + RectHub.Height() + iSpace, iWidth, RectHubStations.Height() + iSpace, FALSE );
	m_fMustbeSized = false;
}

void CRViewSSelHub::OnPaint()
{
	CPaintDC dc( this ); // device context for painting
	m_pSheetTAHub->SetRedraw();
	m_pSheetTAStation->SetRedraw();
	m_pSheetTAHub->Invalidate();
	m_pSheetTAStation->Invalidate();
	// Picture updated after scrolling
	UpdatePicture( m_fMustbeSized );
}

void CRViewSSelHub::OnSize( UINT nType, int cx, int cy)
{
	CFormViewEx::OnSize( nType, cx, cy );
	
	CRect PageRect;
	GetClientRect( &PageRect );
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
		m_PrintRect.right = RIGHTVIEWWIDTH;
	m_iLeftMargin = 3;
	m_fMustbeSized = true;
	SetHMSheetsPos();
	DrawScrollBars();
	SetScrollPos( SB_HORZ, m_iXpos );
	SetScrollPos( SB_VERT, m_iYpos );
}

void CRViewSSelHub::OnContextMenu( CWnd* pWnd, CPoint point )
{
	if( true == IsLocked() || m_pCurSheet != m_pSheetTAStation && m_pCurSheet != m_pSheetTAHub )
		return;
	
	// Enable all Menu ResID by default
	pMainFrame->EnableMenuResID( NULL );

	CMenu menu, popup;
	menu.LoadMenu( IDR_FLTMENU_RVHMCALC );
	CMenu* pContextMenu = menu.GetSubMenu( 0 );
	
	// Load Correct strings 
	CString str;
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_EDIT );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_EDIT, MF_BYCOMMAND, ID_FLTMRVHMCALC_EDIT, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_DELETE );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_DELETE, MF_BYCOMMAND, ID_FLTMRVHMCALC_DELETE, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_COPY );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_COPY, MF_BYCOMMAND, ID_FLTMRVHMCALC_COPY, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_PASTEIN );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PASTEIN, MF_BYCOMMAND, ID_FLTMRVHMCALC_PASTEIN, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_LOCK );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_LOCK, MF_BYCOMMAND, ID_FLTMRVHMCALC_LOCK, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_UNLOCK );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_UNLOCK, MF_BYCOMMAND, ID_FLTMRVHMCALC_UNLOCK, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT, MF_BYCOMMAND, ID_FLTMRVHMCALC_LOCKALLINCIRCUIT, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT, MF_BYCOMMAND, ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_UNLOCKCOLUMN );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_UNLOCKCOLUMN, MF_BYCOMMAND, ID_FLTMRVHMCALC_UNLOCKCOLUMN, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_LOCKCOLUMN );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_LOCKCOLUMN, MF_BYCOMMAND, ID_FLTMRVHMCALC_LOCKCOLUMN, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_EDITACCESSORIES );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_EDITACCESSORIES, MF_BYCOMMAND, ID_FLTMRVHMCALC_EDITACCESSORIES, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_COPYACCESSORIES );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_COPYACCESSORIES, MF_BYCOMMAND, ID_FLTMRVHMCALC_COPYACCESSORIES, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_PASTEACCESSORIES );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_PASTEACCESSORIES, MF_BYCOMMAND, ID_FLTMRVHMCALC_PASTEACCESSORIES, str );
	str = pMainFrame->GetSplitStringFromResID( ID_FLTMRVHMCALC_GETFULLINFO );
	pContextMenu->ModifyMenu( ID_FLTMRVHMCALC_GETFULLINFO, MF_BYCOMMAND, ID_FLTMRVHMCALC_GETFULLINFO, str );

	CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pPrjParam );
	
	// Disable or Remove menu items
	pContextMenu->RemoveMenu( ID_FLTMRVHMCALC_EDIT, MF_BYCOMMAND );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_COPY );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_DELETE );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_PASTEIN );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_LOCK );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_UNLOCK );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_UNLOCKCOLUMN );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_LOCKCOLUMN );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_EDITACCESSORIES );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_COPYACCESSORIES );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_PASTEACCESSORIES );
	pMainFrame->DisableMenuResID( ID_FLTMRVHMCALC_GETFULLINFO );

	bool fShowMenu = false;
	CRect rect;
	long lColumn, lRow;

	CDC* pdc = GetDC( );
	CPoint pointClient = point;
	m_pCurSheet->ScreenToClient( &pointClient );
	this->ReleaseDC( pdc );
	m_pCurSheet->GetCellFromPixel( &lColumn, &lRow, pointClient.x, pointClient.y );
	m_lRow = lRow;
	m_lColumn = lColumn;
	if( m_pCurSheet != m_pSheetTAHub && m_pTADS->IsClipBoardContainsOnlyHMModule() != eb3Undef )
	{
		CArray<long> aSelRows;
		m_pCurSheet->GetSelectedRows( &aSelRows );
		// Can paste clipboard content when only one line is selected	
		if( aSelRows.GetCount() <= 1 )
		{
			// Can paste only if clipboard contains CDS_HmHubStation and if the hub size is below MaxSize		
			// Check if Clipboard contains Hub Station
			//if (((CData *)m_pTADS->GetFirstHMFromClipBoard().MP)->IsClass(CLASS(CDS_HmHubStation))
			//	&& (m_pCurSheet->GetpHub()->GetNumberOfStations() < HUB_MAX_NUMBEROFSTATION))
			//	pMainFrame->EnableMenuResID(ID_FLTMRVHMCALC_PASTEIN);
		}
		if( true == m_pCurSheet->IsEditAccAvailable( lColumn, lRow ) )
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_PASTEACCESSORIES );
	}
	
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_pCurSheet->GetpHM( lRow );
	if( NULL != pHM ) 
	{
		// Enable/Disable some part of the menu
		if( m_pCurSheet != m_pSheetTAHub )
		{
			// Delete is possible only when the number of stations is bigger than HUB_MIN_NUMBEROFSTATION
			if( m_pCurSheet->GetpHub()->GetNumberOfStations() > HUB_MIN_NUMBEROFSTATION ) 
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_DELETE );
		}
		
		if( CDS_HydroMod::eHMObj::eNone != m_pCurSheet->GetHMObj( lColumn, lRow ) )
		{
			// When BV is used as a measuring valve keep Lock and Unlock menu disabled
			if( !( m_pCurSheet == m_pSheetTAHub && 
				!( (CDS_HmHub*)pHM)->GetBalTypeID().CompareNoCase( _T("DPCTYPE_STD") ) && 
				m_pCurSheet->GetHMObj( lColumn, lRow ) == CDS_HydroMod::eHMObj::eBVprim ) )
			{
				if( false == pHM->IsLocked( m_pCurSheet->GetHMObj( lColumn, lRow ) ) )
					pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_LOCK );
				else
					pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_UNLOCK );
			}			
			
			if( m_pCurSheet != m_pSheetTAHub )
			{
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_UNLOCKCOLUMN );
				pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_LOCKCOLUMN );
			}
		}
		
		if( m_pCurSheet != m_pSheetTAHub && m_pCurSheet->IsEditAccAvailable( lColumn, lRow ) )
		{
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_COPYACCESSORIES );
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_EDITACCESSORIES );
		}
		
		pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_LOCKALLINCIRCUIT );
		pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_UNLOCKALLINCIRCUIT );
		fShowMenu = true;
	}

	// Enable Full Cat Sheet if it exist
	// !!!!!!!!!!!!!!!Attention!!!!!!!!!!!!!!!!!!
	// We first use Hub balancing valve/control valve to find the full cat sheet of TA HUB!!!!
	CDS_HmHubStation* pHmHubSta = dynamic_cast<CDS_HmHubStation*>( pHM );
	CDS_HmHub* pHmHub = dynamic_cast<CDS_HmHub*>( pHM );
	if( NULL != pHmHubSta )
	{
		IDPTR idptrRetValv = pHmHubSta->GetReturnValveIDPtr();
		CDB_HubStaValv* pHubStaValv = dynamic_cast<CDB_HubStaValv*>( idptrRetValv.MP);						ASSERT( pHubStaValv );
		CDB_TAProduct* pRetValve = dynamic_cast<CDB_TAProduct*>( pHubStaValv->GetValveIDPtr().MP );			ASSERT( pRetValve );
		if( true == pRetValve->VerifyOneCatExist() )
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_GETFULLINFO );
	}
	else if( NULL != pHmHub )
	{
		IDPTR idptrRetValv = pHmHub->GetReturnValveIDPtr();
		CDB_HubValv* pHubValv = dynamic_cast<CDB_HubValv*>( idptrRetValv.MP );								ASSERT( pHubValv );
		CDB_TAProduct* pRetValve = dynamic_cast<CDB_TAProduct*>( pHubValv->GetValveIDPtr().MP );			ASSERT( pRetValve );
		if( true == pRetValve->VerifyOneCatExist() )
			pMainFrame->EnableMenuResID( ID_FLTMRVHMCALC_GETFULLINFO );
	}	
	
	if( true == fShowMenu )
	{
		// Show the popup menu
		TASApp.GetContextMenuManager()->ShowPopupMenu( HMENU(*pContextMenu), point.x, point.y, this, TRUE );
	}
}

// Delete a line only available for stations
void CRViewSSelHub::OnFltRvSSelHubDelete()
{	
	if( m_pCurSheet != m_pSheetTAStation )
		return;
	
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_pCurSheet->GetpHM( m_lRow );
	if( NULL == pHM )
		return;
	
	CDS_HmHub *pHub = m_pCurSheet->GetpHub();
	if( pHub->GetNumberOfStations() <= 2 )
		return;
	
	CString str, strPos;
	strPos.Format( _T("%d"), pHM->GetPos() );
	FormatString( str, IDS_SSHEETTAHUB_DELETESTATION, strPos);
	if( ::AfxMessageBox( (LPCTSTR)str, MB_YESNO | MB_DEFBUTTON2 | MB_ICONSTOP ) != IDYES )
		return;
	
	BeginWaitCursor();
	CTable* pPTab = NULL;

	// Delete all children
	m_pTADS->DeleteTADSBranch( pHM );
	m_pTADS->DeleteValveFromTADS( pHM );	// If valve exist in SelP, delete it		

	// Delete pHM
	int iHMindex = pHM->GetPos();		// Keep the index before deleting pHM
	IDPTR HMidptr = pHM->GetIDPtr();
	str = ( (CDS_HydroMod *)HMidptr.PP )->GetHMName();
	TRACE( _T("Delete Branch :%s\n"), HMidptr.ID );
	pPTab = (CTable*)HMidptr.PP;
	pPTab->Remove( HMidptr );
	m_pTADS->DeleteObject( HMidptr );

	// Reindex the modules of pPTab if the deletion of pHM has created a hole in the indexing
	m_pTADS->RemoveIndexHole( pPTab, iHMindex );

	( (CDS_HydroMod *)pPTab )->ComputeAll();

	if( NULL != pDlgIndSelHub )
	{
		pDlgIndSelHub->RefreshNumberOfStations();
	}
	
	// Init the sheetTAStation with the new table
	TASApp.OverridedSetRedraw( this, FALSE );
	m_pSheetTAStation->Init( false, (CDS_HmHub *)pPTab );
	TASApp.OverridedSetRedraw( this, TRUE );
	
	// Change the name of NumberofStations.
	pHub->RenameStations();
	RefreshSheets();
	EndWaitCursor();
}

void CRViewSSelHub::OnFltRvSSelHubCopy()
{
	if( m_pCurSheet != m_pSheetTAStation )
	{
		return;
	}
	
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_pCurSheet->GetpHM( m_lRow );
	
	if( NULL == pHM )
	{
		return;
	}
	
	BeginWaitCursor();
	m_pTADS->CleanClipboard();	

	CTable *pTab = m_pTADS->GetpClipboardTable();
	ASSERT( NULL != pTab );

	m_pTADS->CopyHMToClipBoard( pHM, pTab );
	EndWaitCursor();
}

void CRViewSSelHub::OnFltRvSSelHubPaste()
{
	if( m_pCurSheet != m_pSheetTAStation )
		return;

	// Target station will be copied just before this line
	CDS_HydroMod *pHM = m_pCurSheet->GetpHM( m_lRow );
	if (NULL == pHM )
		return;
	
	CDS_HmHub *pHub = m_pCurSheet->GetpHub();
	if( NULL == pHub )
		return;
	
	// Check if Clipboard contains Hub Station
	IDPTR idptr = m_pTADS->GetFirstHMFromClipBoard();
	if( NULL == *idptr.ID )
		return;
	
	if( 0 == ( (CData *)idptr.MP)->IsClass( CLASS( CDS_HmHubStation ) ) )
		return;
	
	// Check if there is some available space into the Hub
	if( pHub->GetNumberOfStations() >= HUB_MAX_NUMBEROFSTATION )
		return;
	
	BeginWaitCursor();
	
	// Added just before the selected target
	int iNewPos = pHM->GetPos();
	
	// Copy from the clipboard
	CTable *pTab = (CTable *)pHub;
	idptr = m_pTADS->CopyFirstHMFromClipBoard( pTab );
	if( NULL != *idptr.ID )
	{
		CDS_HydroMod *pHMpaste = (CDS_HydroMod *)idptr.MP;
		if( NULL != pHMpaste )
		{
			// Reset Level
			m_pTADS->SetHMLevel( pHMpaste, pHM->GetLevel() );
			
			// Free the position for the pasted circuit
			pHMpaste->SetPos( 0 );
			for( IDPTR IDPtr = pTab->GetFirst(); NULL != *IDPtr.ID; IDPtr = pTab->GetNext() )
			{
				if( ( (CDS_HydroMod *)IDPtr.MP )->GetPos() >= iNewPos )
					( (CDS_HydroMod *)IDPtr.MP)->SetPos( ( (CDS_HydroMod *)IDPtr.MP )->GetPos() + 1 );
			}
			pHMpaste->SetPos( iNewPos );
		}
	}
	pHub->ComputeAll();

	if( NULL != pDlgIndSelHub )
	{
		pDlgIndSelHub->RefreshNumberOfStations();
	}

	RefreshSheets();
	EndWaitCursor();
}

void CRViewSSelHub::OnFltRvSSelHubEditaccessories()
{
	if( m_pCurSheet != m_pSheetTAStation )
		return;
	
	CDS_HydroMod *pHM = m_pCurSheet->GetpHM( m_lRow );
	if( NULL == pHM )
		return;
	
	if( false == m_pCurSheet->IsEditAccAvailable( m_lColumn, m_lRow ) )
		return;

	CDlgAccessory dlg( CTADatabase::FilterSelection::ForHMCalc );
	dlg.Display( pHM, pHM->GetpCircuitPrimaryPipe() );
	BeginWaitCursor();
	pHM->ComputeAll();
	RefreshSheets();
	EndWaitCursor();
}

void CRViewSSelHub::OnFltRvSSelHubCopyAccessories()
{
	OnFltRvSSelHubCopy();
}

void CRViewSSelHub::OnFltRvSSelHubPasteAccessories()
{
	if( m_pCurSheet != m_pSheetTAStation )
		return;
	
	// Target station will be copied just before this line
	CDS_HydroMod *pHM = m_pCurSheet->GetpHM( m_lRow );
	if( NULL == pHM )
		return;
	
	CDS_HmHub *pHub = m_pCurSheet->GetpHub();
	if( NULL == pHub )
		return;
	
	// Check if Clipboard contains Hub Station
	IDPTR idptr = m_pTADS->GetFirstHMFromClipBoard();
	if( NULL == *idptr.ID )
		return;
	
	if( 0 == ( (CData *)idptr.MP)->IsClass( CLASS( CDS_HmHubStation ) ) )
		return;

	CDS_HydroMod *pCbHM = (CDS_HydroMod*)idptr.MP;
	bool fCircuitPipes = true;
	CPipes *pCbPipe = NULL;
	CPipes *pPipe = NULL;
	pCbPipe = pCbHM->GetpCircuitPrimaryPipe();
	CArray<long> aSelRows;
	m_pCurSheet->GetSelectedRows( &aSelRows );
	
	// For each selected target copy singularities from the ClipBoard
	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		pHM = (CDS_HydroMod *)m_pCurSheet->GetpHM( aSelRows.GetAt( i ) );			ASSERT( pHM );
		if( NULL == pHM )
			return;
		pPipe = pHM->GetpCircuitPrimaryPipe();												ASSERT( pCbPipe && pPipe );
		if( NULL == pCbPipe || NULL == pPipe )
			return;
		pCbPipe->CopyAllSingularities( pPipe );
	}
	
	BeginWaitCursor();
	m_pTADS->ComputeAllInstallation();
	RefreshSheets();
	EndWaitCursor();
}

void CRViewSSelHub::OnFltRvSSelHubLock()
{
	CDS_HydroMod *pHM = m_pCurSheet->GetpHM( m_lRow );
	if( NULL == pHM )
		return;
	pHM->SetLock( m_pCurSheet->GetHMObj( m_lColumn, m_lRow ), true );
	RefreshSheets();
}

void CRViewSSelHub::OnFltRvSSelHubUnLock()
{
	CDS_HydroMod *pHM = m_pCurSheet->GetpHM( m_lRow );
	if( NULL == pHM )
		return;
	
	pHM->SetLock( m_pCurSheet->GetHMObj( m_lColumn, m_lRow ), false );
	BeginWaitCursor();
	pHM->ComputeAll( true );
	RefreshSheets();
	EndWaitCursor();
}

void CRViewSSelHub::OnFltRvSSelHubLockAll()
{
	CDS_HydroMod *pHM = m_pCurSheet->GetpHM( m_lRow );
	if( NULL == pHM )
		return;
	CArray<long> aSelRows;
	m_pCurSheet->GetSelectedRows( &aSelRows );
	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		pHM = (CDS_HydroMod *)m_pCurSheet->GetpHM( aSelRows.GetAt( i ) );
		if( NULL != pHM )
			pHM->SetLock( CDS_HydroMod::eHMObj::eALL, true );
	}
	RefreshSheets();
}

void CRViewSSelHub::OnFltRvSSelHubUnlockAll()
{
	CDS_HydroMod *pHM = m_pCurSheet->GetpHM( m_lRow );
	if( NULL == pHM )
		return;
	
	CArray<long> aSelRows;
	m_pCurSheet->GetSelectedRows( &aSelRows );
	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		pHM = (CDS_HydroMod *)m_pCurSheet->GetpHM( aSelRows.GetAt( i ) );
		if( NULL != pHM )
			pHM->SetLock( CDS_HydroMod::eHMObj::eALL, false );
	}
	
	BeginWaitCursor();
	m_pCurSheet->GetpHub()->ComputeAll( true );
	RefreshSheets();
	EndWaitCursor();
}

void CRViewSSelHub::OnFltRvSSelHubLockcolumn()
{
	if( m_pCurSheet != m_pSheetTAStation )
		return;
	
	if( 0 == m_lColumn )
		return;
	
	for( long lLoopRow = m_pCurSheet->m_HeaderRow.FirstCirc; lLoopRow <= m_pCurSheet->GetMaxRows(); lLoopRow++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_pCurSheet->GetpHM( lLoopRow );
		if( NULL == pHM )
			continue;
		if( CDS_HydroMod::eHMObj::eNone == m_pCurSheet->GetHMObj( m_lColumn, lLoopRow) )
			continue;
		pHM->SetLock( m_pCurSheet->GetHMObj( m_lColumn, lLoopRow ), true );
	}
	BeginWaitCursor();
	RefreshSheets();
	EndWaitCursor();
}

void CRViewSSelHub::OnFltRvSSelHubUnlockcolumn()
{
	if( m_pCurSheet != m_pSheetTAStation )
		return;

	if( 0 == m_lColumn )
		return;

	for( long lLoopRow = m_pCurSheet->m_HeaderRow.FirstCirc; lLoopRow <= m_pCurSheet->GetMaxRows(); lLoopRow++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_pCurSheet->GetpHM( lLoopRow );
		if( NULL == pHM )
			continue;
		if( CDS_HydroMod::eHMObj::eNone == m_pCurSheet->GetHMObj(m_lColumn, lLoopRow) )
			continue;
		pHM->SetLock(m_pCurSheet->GetHMObj( m_lColumn, lLoopRow ), false );
	}
	
	BeginWaitCursor();
	m_pCurSheet->GetpHub()->ComputeAll( true );
	RefreshSheets();
	EndWaitCursor();
}

void CRViewSSelHub::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	if( SB_THUMBPOSITION == nSBCode )
		m_iXpos = (int)nPos;
	__super::OnHScroll( nSBCode, nPos, pScrollBar );
}

void CRViewSSelHub::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	if( SB_THUMBPOSITION == nSBCode)
		m_iYpos = (int)nPos;
	__super::OnVScroll( nSBCode, nPos, pScrollBar );
}

void CRViewSSelHub::OnGetFullInfoHub()
{
	CDS_HmHubStation *pHmHubSta = dynamic_cast<CDS_HmHubStation*>( m_pCurSheet->GetpHM( m_lRow ) );
	CDS_HmHub* pHmHub = dynamic_cast<CDS_HmHub*>( m_pCurSheet->GetpHM( m_lRow ) );
	
	if( NULL != pHmHubSta )
	{
		IDPTR idptrRetValv = pHmHubSta->GetReturnValveIDPtr();
		CDB_HubStaValv* pHubStaValv = dynamic_cast<CDB_HubStaValv*>( idptrRetValv.MP );						ASSERT( pHubStaValv );
		CDB_TAProduct* pRetValve = dynamic_cast<CDB_TAProduct*>( pHubStaValv->GetValveIDPtr().MP );			ASSERT( pRetValve );
		if( NULL != pRetValve )
			::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_DISPLAYDOCUMENTATION, 0, (LPARAM)pRetValve->GetIDPtr().ID );
	}
	else if( NULL != pHmHub )
	{
		IDPTR idptrRetValv = pHmHub->GetReturnValveIDPtr();
		CDB_HubValv* pHubValv = dynamic_cast<CDB_HubValv*>( idptrRetValv.MP );								ASSERT( pHubValv );
		CDB_TAProduct* pRetValve = dynamic_cast<CDB_TAProduct*>( pHubValv->GetValveIDPtr().MP );			ASSERT( pRetValve );
		if( NULL != pRetValve )
			::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_DISPLAYDOCUMENTATION, 0, (LPARAM)pRetValve->GetIDPtr().ID );
	}
}

void CRViewSSelHub::OnUpdateMenuText( CCmdUI* pCmdUI ) 
{
	// Update the status bar
	pMainFrame->UpdateMenuToolTip( pCmdUI );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Spread DLL message handlers
/////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CRViewSSelHub::ClickFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	if( IDC_FPSPREADHUB == wParam )
		ClickFpspreadHub( pCellCoord->Col, pCellCoord->Row );
	else if( IDC_FPSPREADSTATION == wParam )
		ClickFpspreadStation( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CRViewSSelHub::ComboDropDown( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	if( IDC_FPSPREADHUB == wParam )
		m_pSheetTAHub->ComboDropDown( pCellCoord->Col, pCellCoord->Row );
	else if( IDC_FPSPREADSTATION == wParam )
		m_pSheetTAStation->ComboDropDown( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CRViewSSelHub::DblClickFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	if( IDC_FPSPREADHUB == wParam )
		m_pSheetTAHub->CellDblClick( pCellCoord->Col, pCellCoord->Row );
	else if( IDC_FPSPREADSTATION == wParam )
		m_pSheetTAStation->CellDblClick( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CRViewSSelHub::DragDropBlock( WPARAM wParam, LPARAM lParam )
{
	SS_DRAGDROP *pDragDrop = (SS_DRAGDROP *)lParam;
	if( IDC_FPSPREADHUB == wParam )
		m_pSheetTAHub->DragDropBlock(	pDragDrop->BlockPrev.UL.Col, pDragDrop->BlockPrev.UL.Row, pDragDrop->BlockPrev.LR.Col, pDragDrop->BlockPrev.LR.Row,
										pDragDrop->BlockNew.UL.Col, pDragDrop->BlockNew.UL.Row, pDragDrop->BlockNew.LR.Col, pDragDrop->BlockNew.LR.Row,
										pDragDrop->fCancel );
	else if( IDC_FPSPREADSTATION == wParam )
		DragDropBlockStation(	pDragDrop->BlockPrev.UL.Col, pDragDrop->BlockPrev.UL.Row, pDragDrop->BlockPrev.LR.Col, pDragDrop->BlockPrev.LR.Row,
								pDragDrop->BlockNew.UL.Col, pDragDrop->BlockNew.UL.Row, pDragDrop->BlockNew.LR.Col, pDragDrop->BlockNew.LR.Row,
								&pDragDrop->fCancel );
	return 0;
}

LRESULT CRViewSSelHub::EditChange( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	if( IDC_FPSPREADHUB == wParam )
		m_pSheetTAHub->EditChange( pCellCoord->Col, pCellCoord->Row );
	else if( IDC_FPSPREADSTATION == wParam )
		m_pSheetTAStation->EditChange( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CRViewSSelHub::KeyDown( WPARAM wParam, LPARAM lParam )
{
	if( IDC_FPSPREADHUB == wParam )
		m_pSheetTAHub->KeyDown( LOWORD( lParam ), HIWORD( lParam ) );
	else if( IDC_FPSPREADSTATION == wParam )
		m_pSheetTAStation->KeyDown( LOWORD( lParam ), HIWORD( lParam ) );
	return 0;
}

LRESULT CRViewSSelHub::KeyPress( WPARAM wParam, LPARAM lParam )
{
	if( IDC_FPSPREADHUB == wParam )
		m_pSheetTAHub->KeyPress( (int *)lParam );
	else if( IDC_FPSPREADSTATION == wParam )
		m_pSheetTAStation->KeyPress( (int *)lParam );
	return 0;
}

LRESULT CRViewSSelHub::LeaveCell( WPARAM wParam, LPARAM lParam )
{
	SS_LEAVECELL *pLeaveCell = (SS_LEAVECELL *)lParam;
	BOOL fCancel = FALSE;
	if( IDC_FPSPREADHUB == wParam )
		LeaveCellHub( pLeaveCell->ColCurrent, pLeaveCell->RowCurrent, pLeaveCell->ColNew, pLeaveCell->RowNew, &fCancel );
	else if( IDC_FPSPREADSTATION == wParam )
		LeaveCellStation( pLeaveCell->ColCurrent, pLeaveCell->RowCurrent, pLeaveCell->ColNew, pLeaveCell->RowNew, &fCancel );
	return fCancel;
}

LRESULT CRViewSSelHub::RightClick( WPARAM wParam, LPARAM lParam )
{
	SS_RBUTTON *pRButton = (SS_RBUTTON *)lParam;
	if( IDC_FPSPREADHUB == wParam )
		RightClickHub( pRButton->RButtonType, pRButton->Col, pRButton->Row, pRButton->xPos, pRButton->yPos );
	else if( IDC_FPSPREADSTATION == wParam )
		RightClickStation( pRButton->RButtonType, pRButton->Col, pRButton->Row, pRButton->xPos, pRButton->yPos );
	return 0;
}

LRESULT CRViewSSelHub::SheetChanged( WPARAM wParam, LPARAM lParam )
{
	Invalidate();
	if( IDC_FPSPREADHUB == wParam )
		m_pSheetTAHub->SheetChanged( LOWORD(lParam), HIWORD(lParam) );
	else if( IDC_FPSPREADSTATION == wParam )
		m_pSheetTAStation->SheetChanged( LOWORD(lParam), HIWORD(lParam) );
	return 0;
}

LRESULT CRViewSSelHub::TextTipFetch( WPARAM wParam, LPARAM lParam )
{
	SS_TEXTTIPFETCH *pTextTipFetch = (SS_TEXTTIPFETCH *)lParam;
	// If hText is not empty, spread takes hText in place of szText!
	pTextTipFetch->hText = NULL;
	if( IDC_FPSPREADHUB == wParam )
		m_pSheetTAHub->TextTipFetch( pTextTipFetch->Col, pTextTipFetch->Row, &pTextTipFetch->wMultiLine, &pTextTipFetch->nWidth, pTextTipFetch->szText, &pTextTipFetch->fShow );
	else if( IDC_FPSPREADSTATION == wParam )
		m_pSheetTAStation->TextTipFetch( pTextTipFetch->Col, pTextTipFetch->Row, &pTextTipFetch->wMultiLine, &pTextTipFetch->nWidth, pTextTipFetch->szText, &pTextTipFetch->fShow );
	return 0;
}

BOOL CRViewSSelHub::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if( IDC_FPSPREADSTATION == LOWORD( wParam) && SSN_BUTTONDOWN == HIWORD( wParam ) )
	{
		long lColumn, lRow;
		m_pSheetTAStation->GetActiveCell( &lColumn, &lRow );
		m_pSheetTAStation->ButtonClickedFpspreadhubStation( lColumn, lRow, 1 );
	}

	return __super::OnCommand(wParam, lParam);
}
