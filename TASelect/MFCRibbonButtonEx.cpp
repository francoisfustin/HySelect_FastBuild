#include "stdafx.h"


#include "TASelect.h"
#include "MFCRibbonButtonEx.h"


CMFCRibbonButtonEx::CMFCRibbonButtonEx( UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, BOOL bAlwaysShowDescription )
	: CMFCRibbonButton( nID, lpszText, nSmallImageIndex, nLargeImageIndex, bAlwaysShowDescription )
{
}

void CMFCRibbonButtonEx::OnDraw( CDC *pDC )
{
	if( TRUE == m_bIsVisible )
	{
		CMFCRibbonButton::OnDraw( pDC );
	}
}
