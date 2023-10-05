#include "stdafx.h"
#include "TASelect.h"
#include "SSheet.h"
#include "HydroMod.h"
#include "DlgSpreadCB.h"


IMPLEMENT_DYNAMIC( CDlgSpreadCB, CDialogExt )

CDlgSpreadCB *pDlgSpreadCB = NULL;

CDlgSpreadCB::CDlgSpreadCB( int iIDD, CWnd* pParent )
	: CDialogExt( iIDD, NULL )
{
	pDlgSpreadCB = this;
	m_bOpen = false;
	m_iIDD = iIDD;
	m_lparam = 0;
	m_pParent = pParent;
	m_nTimer = (UINT_PTR)0;
	m_pMainSheet = (CSSheet *)pParent;
	m_lMainSpreadColumn = m_lMainSpreadRow = 0;
	m_bModified = false;
	m_pHM = NULL;
	
	m_pTADS =TASApp.GetpTADS();
	ASSERT( NULL != m_pTADS );

	m_pTADB = TASApp.GetpTADB();
	ASSERT( NULL != m_pTADB );

	m_pUSERDB = TASApp.GetpUserDB();
	ASSERT( NULL != m_pUSERDB );
	
	m_pUnitDB = CDimValue::AccessUDB();
	ASSERT( NULL != m_pUnitDB );
	
	m_pclTableDN = (CTableDN *)( m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != m_pclTableDN );

	m_pTechParam = m_pTADS->GetpTechParams();
	ASSERT( NULL != m_pTechParam );
	
	m_pWC = m_pTADS->GetpWCForProject()->GetpWCData();
	ASSERT( NULL != m_pWC );
}

CDlgSpreadCB::~CDlgSpreadCB()
{
	DestroyWindow();
	pDlgSpreadCB = NULL;
}

void CDlgSpreadCB::DoDataExchange( CDataExchange* pDX )
{
}

void CDlgSpreadCB::SetMainSheetPos( CSSheet *pSheet, long lColumn, long lRow )
{
	m_pMainSheet = pSheet;
	m_lMainSpreadColumn = lColumn;
	m_lMainSpreadRow = lRow;
}

BOOL CDlgSpreadCB::Create()
{
	return CDialogExt::Create( m_iIDD );
}

CRect CDlgSpreadCB::GetComboPos( CRect rect )
{
	// Cell position in MainSheet
	long lWidth, lHeight;
	CRect rectCellPosition;
	long lColumnAnchor, lRowAnchor, lNumColumns, lNumRows;
	
	// in case of spanned cell compute total size for this group of cell
	if( m_pMainSheet->GetCellSpan( m_lMainSpreadColumn, m_lMainSpreadRow, &lColumnAnchor, &lRowAnchor, &lNumColumns, &lNumRows) != SS_SPAN_NO )
	{
		long lTotalWidth, lTotalHeight;
		
		lTotalWidth = lTotalHeight = 0;
		for( long lLoopColumn = lColumnAnchor; lLoopColumn < ( lColumnAnchor + lNumColumns ); lLoopColumn++ )
		{
			m_pMainSheet->GetCellRect( lLoopColumn, m_lMainSpreadRow, &rectCellPosition );
			lTotalWidth += rectCellPosition.Width();
		}		

		for( long lLoopRow = lRowAnchor; lLoopRow < ( lRowAnchor + lNumRows ); lLoopRow++ )
		{
			m_pMainSheet->GetCellRect( m_lMainSpreadColumn, lLoopRow, &rectCellPosition );
			lTotalHeight += rectCellPosition.Height();
		}

		m_pMainSheet->GetCellRect( lColumnAnchor, lRowAnchor, &rectCellPosition );

		lWidth = lTotalWidth;
		lHeight = lTotalHeight;
	}				
	else
	{
		m_pMainSheet->GetCellRect( m_lMainSpreadColumn, m_lMainSpreadRow, &rectCellPosition );
		lWidth = rectCellPosition.Width();
		lHeight = rectCellPosition.Height();
	}

	CRect Srect( rectCellPosition.left, rectCellPosition.top, rectCellPosition.left + lWidth, rectCellPosition.top + lHeight );
	
	int iCellWidth = Srect.Width();

	// Current position of MainSheet
	CRect MSheetrect;
	m_pMainSheet->GetWindowRect( (LPRECT)MSheetrect );
	Srect.MoveToXY( MSheetrect.left + 1 + Srect.left, MSheetrect.top + Srect.top + Srect.Height() );

	// Keep the combo inside the main sheet, shift left if necessary
	if( ( Srect.left + rect.Width() ) > MSheetrect.right )
	{
		Srect.right = Srect.left + iCellWidth;
		Srect.left = Srect.right - rect.Width();
	}
	return Srect;
}


BEGIN_MESSAGE_MAP( CDlgSpreadCB, CDialogExt )
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	// User Messages
	ON_MESSAGE( WM_USER_CLOSESPREADCOMBOBOX, OnCloseCB )
END_MESSAGE_MAP()


// CDlgSpreadCB message handlers
BOOL CDlgSpreadCB::OnInitDialog()
{
	pDlgSpreadCB = this;
	CDialogExt::OnInitDialog();
	return true;
}

void CDlgSpreadCB::OnDestroy()
{
	CDialogExt::OnDestroy();
}

LRESULT CDlgSpreadCB::OnCloseCB( WPARAM wParam, LPARAM lParam )
{
	m_pMainSheet->SetActiveCell( m_lMainSpreadColumn,m_lMainSpreadRow );
	EndDialog( true );
	m_bOpen = false;
	DestroyWindow();
	m_pMainSheet->CloseDialogSCB( this, m_bModified );
	return true;
}

void CDlgSpreadCB::InvalidateParentWnd()
{
	if( NULL != m_pParent )
		m_pParent->Invalidate();
}

void CDlgSpreadCB::OnActivate( UINT nState, CWnd* pWndOther, BOOL fMinimized )
{
	CDialogExt::OnActivate( nState, pWndOther, fMinimized );
	if( WA_ACTIVE == nState )
	{
		m_nTimer = SetTimer( _TIMERID_DLGSPREADCB, 5, &TimerProc );		// CDlgSpreadCB::OnTimer doesn't work why?
		m_bOpen = true;
	}
	
	if( WA_INACTIVE == nState )
	{
		PostMessage( WM_USER_CLOSESPREADCOMBOBOX );
	}
}

void CALLBACK TimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	KillTimer( hwnd, idEvent );

	if( NULL != pDlgSpreadCB )
	{
		pDlgSpreadCB->InvalidateParentWnd();
		pDlgSpreadCB->Refresh();
	}
}
