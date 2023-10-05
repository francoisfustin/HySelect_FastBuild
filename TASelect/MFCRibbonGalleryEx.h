#pragma once


// Class CMFCRibbonGalleryEx create to insert a LParam
// to each image index in the ImageList

class CMFCRibbonGalleryEx : public CMFCRibbonGallery
{
	DECLARE_DYNCREATE( CMFCRibbonGalleryEx )

public:
	CMFCRibbonGalleryEx();
	CMFCRibbonGalleryEx( UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, CMFCToolBarImages &imagesPalette );
	CMFCRibbonGalleryEx( UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, UINT uiImagesPaletteResID = 0, int cxPaletteImage = 0 );

	// Owner-draw icons constructor:
	CMFCRibbonGalleryEx( UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, CSize sizeIcon,
		int nIconsNum,					// If 0 - you should call AddGroup later
		BOOL bDefaultButtonStyle = TRUE);// If FALSE - you need to implement your own border/content drawing
	
	void ClearLparamArray() { m_ArrayLParam.RemoveAll(); }
	void SetLparamToImageIndex( int iIndexImage, LPARAM lparam );
	LPARAM GetLparamFromImageIndex( int iIndexImage );
	int GetImageIndexFromLparam( LPARAM lparam );

	void ClearEnabledArray() { m_arIconsEnabled.RemoveAll(); }
	void SetEnabledIcon( int iIndexIcon, bool bEnabled );

	// Set the number of icons to show in the gallery box. So if the box has enough space it will be adapted to total number of icons
	// It is done in the GetRegularSize function. By default the number of icons is 6 and it is too much in many case for us.
	void ShowXIcons( int iNbr) { m_iNbr = iNbr; }

// Protected methods.
protected:
	virtual CSize GetRegularSize( CDC *pDC );
	virtual CMFCRibbonBaseElement* HitTest( CPoint point );

// Protected variables.
protected:
	CArray <LPARAM> m_ArrayLParam;
	int m_iNbr;
	CArray<bool> m_arIconsEnabled;
};
