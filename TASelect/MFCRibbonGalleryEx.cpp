#include "stdafx.h"
#include "TASelect.h"
#include "MFCRibbonGalleryEx.h"

const int nImageMargin = 4;
const int nBorderMarginX = 1;
const int nBorderMarginY = 3;


IMPLEMENT_DYNCREATE(CMFCRibbonGalleryEx, CMFCRibbonGallery)

CMFCRibbonGalleryEx::CMFCRibbonGalleryEx():CMFCRibbonGallery()
{
	m_iNbr = 6;
	m_ArrayLParam.RemoveAll();
}

CMFCRibbonGalleryEx::CMFCRibbonGalleryEx(UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, CMFCToolBarImages& imagesPalette)
					:CMFCRibbonGallery(nID,lpszText,nSmallImageIndex,nLargeImageIndex,imagesPalette)
{
	m_iNbr = 6;
	m_ArrayLParam.RemoveAll();
}

CMFCRibbonGalleryEx::CMFCRibbonGalleryEx( UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, UINT uiImagesPaletteResID, 
	int cxPaletteImage )
	:CMFCRibbonGallery( nID, lpszText, nSmallImageIndex, nLargeImageIndex, uiImagesPaletteResID, cxPaletteImage )
{
	m_iNbr = 6;
	m_ArrayLParam.RemoveAll();
}

CMFCRibbonGalleryEx::CMFCRibbonGalleryEx( UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, CSize sizeIcon, int nIconsNum, 
	BOOL bDefaultButtonStyle )
	:CMFCRibbonGallery( nID, lpszText, nSmallImageIndex, nLargeImageIndex, sizeIcon, nIconsNum, bDefaultButtonStyle )
{
	m_iNbr = 6;
	m_ArrayLParam.RemoveAll();
}

void CMFCRibbonGalleryEx::SetLparamToImageIndex( int iIndexImage, LPARAM lparam )
{
	m_ArrayLParam.SetAtGrow( iIndexImage, lparam );
}

LPARAM CMFCRibbonGalleryEx::GetLparamFromImageIndex( int iIndexImage )
{
	// Verify the LPARAM exist
	if( m_ArrayLParam.GetCount() < iIndexImage || m_ArrayLParam.GetSize() <= iIndexImage )
	{
		return NULL;
	}

	return m_ArrayLParam.GetAt( iIndexImage );

}

int CMFCRibbonGalleryEx::GetImageIndexFromLparam( LPARAM lparam )
{
	for( int i = 0; i < m_ArrayLParam.GetCount(); i++ )
	{
		if( m_ArrayLParam.GetAt( i ) == lparam )
		{
			return i;
		}
	}

	return 0;
}

void CMFCRibbonGalleryEx::SetEnabledIcon( int iIndexIcon, bool bEnabled )
{
	m_arIconsEnabled.SetAtGrow( iIndexIcon, bEnabled );
}

CSize CMFCRibbonGalleryEx::GetRegularSize( CDC *pDC )
{
	ASSERT_VALID( this );

	const CSize sizeImage = GetIconSize();
	CSize sizePanelSmallImage( 16, 16 );

	if( NULL != m_pParent )
	{
		ASSERT_VALID( m_pParent );
		sizePanelSmallImage = m_pParent->GetImageSize( FALSE );
	}

	m_bSmallIcons = (sizeImage.cx <= sizePanelSmallImage.cx * 3 / 2 );

	if( TRUE == m_bResetColumns && FALSE == m_bSmallIcons )
	{
		// Special treatment to resize correctly the Gallery Box.
		m_nPanelColumns = m_iNbr;

		if( m_pParentMenu != NULL && m_pParentMenu->GetCategory() == NULL )
		{
			// From the default panel button
			m_nPanelColumns = 3;
		}
	}

	m_bResetColumns = FALSE;

	if( TRUE == IsButtonLook() )
	{
		return CMFCRibbonButton::GetRegularSize( pDC );
	}

	if( 0 == m_arIcons.GetSize() )
	{
		CreateIcons();
	}

	ASSERT_VALID( m_pParent );

	const CSize sizePanelLargeImage = m_pParent == NULL ? CSize( 0, 0 ) : m_pParent->GetImageSize( TRUE );

	CSize size( 0, 0 );

	if( TRUE == m_bSmallIcons )
	{
		size.cx = sizeImage.cx * m_nPanelColumns;

		int m_iRows = 3;

		if( sizePanelLargeImage != CSize( 0, 0 ) && sizeImage.cy != 0 )
		{
			m_iRows = max( m_iRows, sizePanelLargeImage.cy * 2 / sizeImage.cy );
		}

		size.cy = m_iRows * sizeImage.cy + 2 * nBorderMarginY;
	}
	else
	{
		size.cx = ( sizeImage.cx + 2 * nImageMargin ) * m_nPanelColumns;
		size.cy = sizeImage.cy + 3 * nImageMargin + 2 * nBorderMarginY;
	}

	//---------------------------------------
	// Add space for menu and scroll buttons:
	//---------------------------------------
	size.cx += GetDropDownImageWidth() + 3 * nImageMargin;

	return size;
}

CMFCRibbonBaseElement *CMFCRibbonGalleryEx::HitTest( CPoint point )
{
	ASSERT_VALID( this );

	if( TRUE == IsDisabled() )
	{
		return NULL;
	}

	if( TRUE == IsButtonLook() )
	{
		return CMFCRibbonButton::HitTest( point );
	}

	int iImageIndex = 0;

	// For a gallery, we have in m_arIcons a CMFCRibbonPanel that is the name of the group of icons followed by
	// a CMFCRibbonGalleryIcon for each icons in the group. For the next group it's again a CMFRibbonPanel followed by CMFCRibbonGalleryIcon and so on.
	for( int i = 0; i < m_arIcons.GetSize(); i++ )
	{
		if( NULL == dynamic_cast<CMFCRibbonGalleryIcon*>( m_arIcons[i] ) )
		{
			// Don't care here about the group name (the 'CMFCRibbonPanel' element).
			continue;
		}

		CMFCRibbonGalleryIcon *pclRibbonGalleryIcon = dynamic_cast<CMFCRibbonGalleryIcon*>( m_arIcons[i] );

		if( m_arIcons[i]->GetRect().PtInRect( point ) )
		{
			if( pclRibbonGalleryIcon->GetIndex() < 0 )
			{
				// It's the case for three button up, down and menu in the gallery.
				return m_arIcons[i];
			}
			else if( true == m_arIconsEnabled[iImageIndex] )
			{
				return m_arIcons[i];
			}
			else
			{
				return NULL;
			}
		}

		iImageIndex++;
	}

	return NULL;
}
