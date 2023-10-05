#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"

#include "DlgWizPMRightViewInputBase.h"
#include "DlgWizPMRightViewPictureContainer.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"
#include "DlgWizPMRightViewTextContainer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewTextContainer::CDlgWizardPM_RightViewTextContainer()
{
}

CDlgWizardPM_RightViewTextContainer::~CDlgWizardPM_RightViewTextContainer()
{
	_Clear();
}

void CDlgWizardPM_RightViewTextContainer::UpdateText( CArray<CRViewWizardSelPM::CTextIDValue> *parTextIDSList )
{
	_Clear();

	if( 0 == parTextIDSList->GetCount() )
	{
		return;
	}
	
	CRect rectStatic;
	GetClientRect( &rectStatic );
	rectStatic.DeflateRect( 7, 7 );

	for( int iLoop = 0; iLoop < parTextIDSList->GetCount(); iLoop++ )
	{
		CExtStatic *pclNewExtStatic = new CExtStatic();

		if( NULL == pclNewExtStatic )
		{
			_Clear();
			return;
		}

		if( FALSE == pclNewExtStatic->Create( _T(""), SS_LEFT | SS_NOPREFIX, rectStatic, this, 30000 + iLoop ) )
		{
			_Clear();
			return;
		}

		CFont *pclFont = GetFont();
		pclNewExtStatic->SetFont( pclFont );

		int iFontSize = ( 0 == ( iLoop % 2 ) ) ? DLGWIZARDPM_RVTEXT_HEADERTEXTSIZE : DLGWIZARDPM_RVTEXT_NORMALTEXTSIZE;
		pclNewExtStatic->SetFontSize( iFontSize );

		bool bFontBold = ( 0 == ( iLoop % 2 ) ) ? DLGWIZARDPM_RVTEXT_HEADERTEXTBOLD : DLGWIZARDPM_RVTEXT_NORMALTEXTBOLD;
		pclNewExtStatic->SetFontBold( bFontBold );

		CString str = _T("");
		CRViewWizardSelPM::CTextIDValue clTextIDValue = parTextIDSList->GetAt( iLoop );

		if( 0 == clTextIDValue.m_vecValues.size() )
		{
			str = TASApp.LoadLocalizedString( clTextIDValue.m_iIDS );
		}
		else if( 1 == clTextIDValue.m_vecValues.size() )
		{
			FormatString( str, clTextIDValue.m_iIDS, WriteCUDouble( clTextIDValue.m_vecValues.at( 0 ).first, clTextIDValue.m_vecValues.at( 0 ).second, true ) );
		}
		else if( 2 == clTextIDValue.m_vecValues.size() )
		{
			CString strValue1 = WriteCUDouble( clTextIDValue.m_vecValues.at( 0 ).first, clTextIDValue.m_vecValues.at( 0 ).second, true );
			CString strValue2 = WriteCUDouble( clTextIDValue.m_vecValues.at( 1 ).first, clTextIDValue.m_vecValues.at( 1 ).second, true );
			FormatString( str, clTextIDValue.m_iIDS, strValue1, strValue2 );
		}
		else
		{
			// More than 2 variables is not implemented !!
			ASSERT( 0 );
		}

		pclNewExtStatic->SetWindowText( str );
		pclNewExtStatic->ShowWindow( SW_SHOW );
		m_arpExtStaticList.Add( pclNewExtStatic );
	}

	CRect rectClient;
	GetClientRect( &rectClient );
	int iHeightNeeded = _ComputeHeightNeeded( rectClient.Width(), rectClient.Height() );

	SCROLLINFO rScrollInfo;
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	rScrollInfo.nMin = 0;
	rScrollInfo.nMax = 0;
	rScrollInfo.nPage = 0;

	if( iHeightNeeded >= rectClient.Height() )
	{
		rScrollInfo.nMax = iHeightNeeded;
		rScrollInfo.nPage = rectClient.Height();
	}

	SetScrollInfo( SB_VERT, &rScrollInfo, TRUE );

	m_arTextIDSList.Copy( *parTextIDSList );

	Invalidate();
	UpdateWindow();
}

BOOL CDlgWizardPM_RightViewTextContainer::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( SB_VERT, &rScrollInfo );

	BOOL bReturn = FALSE;

	if( 0 != rScrollInfo.nPage )
	{
		// Retrieve current position
		int nCurrentPos = GetScrollPos( SBS_VERT );
		int nPreviousPos = nCurrentPos;

		if( zDelta > 0 )
		{
			nCurrentPos = max( nCurrentPos - 20, 0);
		}
		else if( zDelta < 0 )
		{
			nCurrentPos = min( nCurrentPos + 20, rScrollInfo.nMax - (int)rScrollInfo.nPage );
		}

		if( nCurrentPos != nPreviousPos )
		{
			SetScrollPos( SB_VERT, nCurrentPos );
			ScrollWindow( 0, nPreviousPos - nCurrentPos );
		}

		bReturn = TRUE;
	}

	if( FALSE == bReturn )
	{
		bReturn = CDialogExt::OnMouseWheel( nFlags, zDelta, pt );
	}

	return bReturn;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewTextContainer, CDialogExt )
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewTextContainer::OnOK()
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgWizardPM_RightViewTextContainer::OnSize( UINT nType, int cx, int cy )
{
	CDialogExt::OnSize( nType, cx, cy );

	int iHeightNeeded = _ComputeHeightNeeded( cx, cy );

	SCROLLINFO rScrollInfo;
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	rScrollInfo.nMin = 0;
	rScrollInfo.nMax = 0;
	rScrollInfo.nPage = 0;

	if( iHeightNeeded >= cy )
	{
		rScrollInfo.nMax = iHeightNeeded;
		rScrollInfo.nPage = cy;
	}

	SetScrollInfo( SB_VERT, &rScrollInfo, TRUE );
}

void CDlgWizardPM_RightViewTextContainer::OnPaint()
{
	CPaintDC dc( this );
	CDialogExt::OnPaint();

	CBrush clBrush;
	clBrush.CreateSolidBrush( DLGWIZARDPM_RVTEXT_SEPARATORCOLOR );

	CRect rectClient;
	GetClientRect( &rectClient );

	rectClient.right = rectClient.left + 2;
	rectClient.bottom += 1;
	dc.FillRect( &rectClient, &clBrush );

	DeleteObject( &clBrush );

	if( 0 == m_arpExtStaticList.GetCount() )
	{
		return;
	}

	CDC *pDC = GetDC();

	// Create memory device context.
	CRect ClientRect;
	GetClientRect( &ClientRect );

	CDC dcMemory;
	dcMemory.CreateCompatibleDC( pDC );

	CBitmap clBitmap;
	clBitmap.CreateCompatibleBitmap( pDC, ClientRect.Width(), ClientRect.Height() );

	CBitmap *pOldBitmap = dcMemory.SelectObject( &clBitmap );

	CRect rectStatic;
	GetClientRect( &rectStatic );
	rectStatic.DeflateRect( 7, 7 );
	
	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( SB_VERT, &rScrollInfo );

	CPoint pt( 0, rScrollInfo.nPos );
	rectStatic -= pt;

	for( int iLoop = 0; iLoop < m_arpExtStaticList.GetCount(); iLoop++ )
	{
		CFont *pclStaticFont = m_arpExtStaticList.GetAt( iLoop )->GetFont();
		dcMemory.SelectObject( pclStaticFont );

		CRect textCalc = rectStatic;

		CString strText;
		m_arpExtStaticList.GetAt( iLoop )->GetWindowText( strText );
		dcMemory.DrawText( strText, &textCalc, DT_LEFT | DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK );

		m_arpExtStaticList.GetAt( iLoop )->SetWindowPos( NULL, rectStatic.left, rectStatic.top, rectStatic.Width(), textCalc.Height(), SWP_NOZORDER );
		// m_arpExtStaticList.GetAt( iLoop )->ShowWindow( SW_SHOW );

		int iSeparator = ( 0 == ( iLoop % 2 ) ) ? DLGWIZARDPM_RVTEXT_SPACEBETWEENHEADERNORMALTEXT : DLGWIZARDPM_RVTEXT_SPACEBETWEENTWOTEXT;
		rectStatic.top += ( textCalc.Height() + iSeparator );
	}

	dcMemory.SelectObject( pOldBitmap );
	clBitmap.DeleteObject();
	dcMemory.DeleteDC();
	ReleaseDC( pDC );
}

void CDlgWizardPM_RightViewTextContainer::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
{
	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( SB_VERT, &rScrollInfo );

	// Retrieve current position
	int nCurrentPos = GetScrollPos( SBS_VERT );
	int nPreviousPos = nCurrentPos;

	switch(nSBCode)
	{
		// User has clicked on arrow up.
		case SB_LINEUP:
			nCurrentPos = max( nCurrentPos - 20, 0);
			break;

		// User has clicked on arrow down.
		case SB_LINEDOWN:
			nCurrentPos = min( nCurrentPos + 20, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// User has clicked on bar between arrow up and scroll box
		case SB_PAGEUP:
			nCurrentPos = max( nCurrentPos - (int)rScrollInfo.nPage, 0);
			break;

		// User has clicked on bar between arrow down and scroll box
		case SB_PAGEDOWN:
			nCurrentPos = min( nCurrentPos + (int)rScrollInfo.nPage, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// First one is when user drag the scroll box.
		// Second one is when user release the scroll box.
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			// Pay attention: 'nPos' is not really a UINT but a 16 bits value! To get position on 32 bits, we must use 'nTrackPos'.
			nCurrentPos = rScrollInfo.nTrackPos;
			break;

		case SB_ENDSCROLL:
			break;
	}

	if( nCurrentPos != nPreviousPos )
	{
		SetScrollPos( SB_VERT, nCurrentPos );
		ScrollWindow( 0, nPreviousPos - nCurrentPos );
	}
}

LRESULT CDlgWizardPM_RightViewTextContainer::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	for( int iLoop = 0; iLoop < m_arTextIDSList.GetCount(); iLoop++ )
	{
		CString str = _T("");
		CRViewWizardSelPM::CTextIDValue clTextIDValue = m_arTextIDSList.GetAt( iLoop );

		if( 0 == clTextIDValue.m_vecValues.size() )
		{
			str = TASApp.LoadLocalizedString( clTextIDValue.m_iIDS );
		}
		else if( 1 == clTextIDValue.m_vecValues.size() )
		{
			FormatString( str, clTextIDValue.m_iIDS, WriteCUDouble( clTextIDValue.m_vecValues.at( 0 ).first, clTextIDValue.m_vecValues.at( 0 ).second, true ) );
		}
		else if( 2 == clTextIDValue.m_vecValues.size() )
		{
			CString strValue1 = WriteCUDouble( clTextIDValue.m_vecValues.at( 0 ).first, clTextIDValue.m_vecValues.at( 0 ).second, true );
			CString strValue2 = WriteCUDouble( clTextIDValue.m_vecValues.at( 1 ).first, clTextIDValue.m_vecValues.at( 1 ).second, true );
			FormatString( str, clTextIDValue.m_iIDS, strValue1, strValue2 );
		}
		else
		{
			// More than 2 variables is not implemented !!
			ASSERT( 0 );
		}

		m_arpExtStaticList.GetAt( iLoop )->SetWindowText( str );
	}

	return 0;
}

void CDlgWizardPM_RightViewTextContainer::_Clear()
{
	if( m_arpExtStaticList.GetCount() > 0 )
	{
		for( int iLoop = 0; iLoop < m_arpExtStaticList.GetCount(); iLoop++ )
		{
			if( NULL != m_arpExtStaticList[iLoop] )
			{
				m_arpExtStaticList[iLoop]->DestroyWindow();
				delete m_arpExtStaticList[iLoop];
			}
		}
	}

	m_arpExtStaticList.RemoveAll();
}

int CDlgWizardPM_RightViewTextContainer::_ComputeHeightNeeded( int iWidth, int iHeight )
{
	if( 0 == m_arpExtStaticList.GetCount() )
	{
		return 0;
	}

	CDC *pDC = GetDC();

	// Create memory device context.
	CDC dcMemory;
	dcMemory.CreateCompatibleDC( pDC );

	CBitmap clBitmap;
	clBitmap.CreateCompatibleBitmap( pDC, iWidth, iHeight );

	CBitmap *pOldBitmap = dcMemory.SelectObject( &clBitmap );

	CRect rectStatic;
	GetClientRect( &rectStatic );
	rectStatic.DeflateRect( 7, 7 );

	for( int iLoop = 0; iLoop < m_arpExtStaticList.GetCount(); iLoop++ )
	{
		CFont *pclStaticFont = m_arpExtStaticList.GetAt( iLoop )->GetFont();
		dcMemory.SelectObject( pclStaticFont );

		CRect textCalc = rectStatic;

		CString strText;
		m_arpExtStaticList.GetAt( iLoop )->GetWindowText( strText );
		dcMemory.DrawText( strText, &textCalc, DT_LEFT | DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK );

		int iSeparator = ( 0 == ( iLoop % 2 ) ) ? DLGWIZARDPM_RVTEXT_SPACEBETWEENHEADERNORMALTEXT : DLGWIZARDPM_RVTEXT_SPACEBETWEENTWOTEXT;
		rectStatic.top += ( textCalc.Height() + iSeparator );
	}

	dcMemory.SelectObject( pOldBitmap );
	clBitmap.DeleteObject();
	dcMemory.DeleteDC();
	ReleaseDC( pDC );

	return rectStatic.top;
}
