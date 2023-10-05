//
// RViewLogData.cpp: implementation of the CRViewLogData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "Global.h"
#include "Utilities.h"

#include "DlgExportLdlist.h"
#include "RViewChart.h"
#include "RViewLogData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CRViewLogData *pRViewLogData = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE( CRViewLogData, CFormViewEx )

CRViewLogData::CRViewLogData() : CFormViewEx( CRViewLogData::IDD )
{
	pRViewLogData = this;
	m_fRedraw = true;
	m_pLogData = NULL;
}

CRViewLogData::~CRViewLogData()
{
	pRViewLogData = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CRightViewProj diagnostics

#ifdef _DEBUG
void CRViewLogData::AssertValid() const
{
	CFormViewEx::AssertValid();
}

void CRViewLogData::Dump( CDumpContext& dc ) const
{
	CFormViewEx::Dump( dc );
}

CTASelectDoc* CRViewLogData::GetDocument() // non-debug version is inline
{
	ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CTASelectDoc ) ) );
	return (CTASelectDoc*)m_pDocument;
}

#endif //_DEBUG


BEGIN_MESSAGE_MAP( CRViewLogData, CFormViewEx )
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_COMMAND( ID_APPMENU_EXPORT_LDLIST, OnFileExportLdlist )
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRViewLogData drawing

void CRViewLogData::SetRedraw( CLog *pLogData )
{
	if( NULL == m_SheetLogData.GetSafeHwnd() )
		return;
	
	m_fRedraw = true;
	if( NULL != pLogData )
		m_pLogData = pLogData;
	else
	{
		if( NULL == m_pLogData )
		{
			Invalidate();
			return;
		}
	}

	m_SheetLogData.SetRedraw( pLogData );
	ResizeColumnSheet();
	Invalidate();
}

void CRViewLogData::ResizeColumnSheet()
{
	if( NULL == m_SheetLogData.GetSafeHwnd() )
		return;
	
	CRect rect;
	GetClientRect( &rect );
	int	iWidth = rect.Width();
	iWidth = max( iWidth, RIGHTVIEWWIDTH );
	rect.right = iWidth;
	m_SheetLogData.ResizeColumns( iWidth );
}

/////////////////////////////////////////////////////////////////////////////
// CRightViewProj message handlers

BOOL CRViewLogData::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext ) 
{
	if( FALSE == CFormViewEx::Create( lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext ) )
		return FALSE;

	// Create a TSpread sheet.
	if( FALSE == m_SheetLogData.Create( ( dwStyle | WS_CHILD ) ^ WS_VISIBLE, rect, this, IDC_SHEETLOGDATA ) ) 
		return FALSE;

	return true;
}

LRESULT CRViewLogData::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	m_pTADS = TASApp.GetpTADS();

	// Retrieve last selected Page Setup from TADS.
	m_pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != m_pTADSPageSetup );

	m_pLogData = NULL;
	return 0;
}

void CRViewLogData::OnPaint() 
{
	if( NULL == m_SheetLogData.GetSafeHwnd() )
		return;

	CPaintDC dc( this ); // device context for painting
	
	if( NULL != m_pLogData )
	{
		CRect PageRect;
		GetClientRect( &PageRect );
		if( true == m_fRedraw )
		{
			if( NULL != m_SheetLogData.GetSafeHwnd() )
				m_SheetLogData.MoveWindow( 0, 0, PageRect.Width(), PageRect.Height(), TRUE );
		}
	}
	m_fRedraw = false;

	m_SheetLogData.Invalidate();
}

void CRViewLogData::OnSize( UINT nType, int cx, int cy ) 
{
	CFormViewEx::OnSize( nType, cx, cy );

	ResizeColumnSheet();
	m_fRedraw = true;
}

void CRViewLogData::OnFileExportLdlist() 
{
	if( NULL == m_SheetLogData.GetSafeHwnd() )
		return;

	// Set sheet name to "LogName" or "LogName - Part X".
	CString FileName = m_pLogData->GetName();
	m_SheetLogData.SetSheetName( m_SheetLogData.GetSheet(), FileName );

	CDlgExportLdlist dlg;
	dlg.Display( &m_SheetLogData, m_pLogData, FileName );
}
