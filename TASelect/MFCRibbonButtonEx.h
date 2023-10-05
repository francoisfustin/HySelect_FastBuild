#pragma once


class CMFCRibbonButtonEx : public CMFCRibbonButton
{
public:
	CMFCRibbonButtonEx( UINT nID, LPCTSTR lpszText, int nSmallImageIndex = -1, int nLargeImageIndex = -1, BOOL bAlwaysShowDescription = FALSE );

	virtual void OnDraw( CDC *pDC );
};
