#include "stdafx.h"
#include <map>
#include "global.h"
#include "ExtMenu.h"


IMPLEMENT_DYNAMIC( CExtMenu, CEdit )

CExtMenu::CExtMenu()
{
}

BOOL CExtMenu::AppendMenuEx( UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, COLORREF textColor, COLORREF backColor )
{
	if( FALSE == CMenu::AppendMenu( MF_OWNERDRAW, nIDNewItem, lpszNewItem ) )
		return FALSE;

	DrawItemParam rDrawItemParam;
	rDrawItemParam.m_textColor = textColor;
	rDrawItemParam.m_backColor = backColor;
	rDrawItemParam.m_strText = CString( lpszNewItem );
	m_mapOwnerDrawnParams[nIDNewItem] = rDrawItemParam;
	return TRUE;
}

void CExtMenu::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
	int i = 0;
}

void CExtMenu::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if( 0 == m_mapOwnerDrawnParams.count( lpDrawItemStruct->itemID ) )
		return;
	
	CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );

	// Background.
	CBrush br( m_mapOwnerDrawnParams[lpDrawItemStruct->itemID].m_backColor );
	pDC->FillRect( &lpDrawItemStruct->rcItem, &br );

	// Text.
	pDC->SetBkColor( m_mapOwnerDrawnParams[lpDrawItemStruct->itemID].m_backColor );
	pDC->SetTextColor( m_mapOwnerDrawnParams[lpDrawItemStruct->itemID].m_textColor );
	CRect rectText( lpDrawItemStruct->rcItem );
	rectText.left += 5;
	rectText.top += 1;
	pDC->TextOut( rectText.left, rectText.top, (LPCTSTR)m_mapOwnerDrawnParams[lpDrawItemStruct->itemID].m_strText );
}


IMPLEMENT_DYNCREATE( CMFCToolBarMenuButtonEx, CMFCToolBarMenuButton )

CMFCToolBarMenuButtonEx::CMFCToolBarMenuButtonEx()
	: CMFCToolBarMenuButton()
{
	m_rDrawItemParam.m_fCustomized = false;
	m_rDrawItemParam.m_textColor = _BLACK;
	m_rDrawItemParam.m_backColor = _WHITE;
}

CMFCToolBarMenuButtonEx::CMFCToolBarMenuButtonEx( UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText, BOOL bUserButton )
	: CMFCToolBarMenuButton( uiID, hMenu, iImage, lpszText, bUserButton )
{
	m_rDrawItemParam.m_fCustomized = false;
	m_rDrawItemParam.m_textColor = _BLACK;
	m_rDrawItemParam.m_backColor = _WHITE;
}

void CMFCToolBarMenuButtonEx::SetItemTextColor( COLORREF clTextColor )
{
	m_rDrawItemParam.m_fCustomized = true;
	m_rDrawItemParam.m_textColor = clTextColor;
}

void CMFCToolBarMenuButtonEx::SetItemBackColor( COLORREF clBackColor )
{
	m_rDrawItemParam.m_fCustomized = true;
	m_rDrawItemParam.m_backColor = clBackColor;
}

void CMFCToolBarMenuButtonEx::SetSubItemTextColor( UINT uiID, COLORREF clTextColor )
{
	bool fFound = false;
	for( POSITION pos = m_listCommands.GetHeadPosition(); pos != NULL && false == fFound; )
	{
		CMFCToolBarMenuButton* pItem = (CMFCToolBarMenuButton*)m_listCommands.GetAt( pos );
		if( NULL != pItem && pItem->m_nID == uiID )
		{
			// Is already a 'CMFCToolBarMenuButtonEx'?
			if( NULL != dynamic_cast<CMFCToolBarMenuButtonEx*>( pItem ) )
			{
				( (CMFCToolBarMenuButtonEx*)pItem )->SetItemTextColor( clTextColor );
			}
			else
			{
				CMFCToolBarMenuButtonEx* pclNewButtonEx = new CMFCToolBarMenuButtonEx();				ASSERT( NULL != pclNewButtonEx );
				pclNewButtonEx->CopyFrom( *pItem );
				pclNewButtonEx->SetItemTextColor( clTextColor );
				m_listCommands.SetAt( pos, pclNewButtonEx );
				delete pItem;
			}
			fFound = true;
		}
		else
		{
			m_listCommands.GetNext( pos );
		}
	}
}

void CMFCToolBarMenuButtonEx::SetSubItemBackColor( UINT uiID, COLORREF clBackColor )
{
	bool fFound = false;
	for( POSITION pos = m_listCommands.GetHeadPosition(); pos != NULL && false == fFound; )
	{
		CMFCToolBarMenuButton* pItem = (CMFCToolBarMenuButton*)m_listCommands.GetAt( pos );
		if( NULL != pItem && pItem->m_nID == uiID )
		{
			// Is already a 'CMFCToolBarMenuButtonEx'?
			if( NULL != dynamic_cast<CMFCToolBarMenuButtonEx*>( pItem ) )
			{
				( (CMFCToolBarMenuButtonEx*)pItem )->SetItemBackColor( clBackColor );
			}
			else
			{
				CMFCToolBarMenuButtonEx* pclNewButtonEx = new CMFCToolBarMenuButtonEx();				ASSERT( NULL != pclNewButtonEx );
				pclNewButtonEx->CopyFrom( *pItem );
				pclNewButtonEx->SetItemBackColor( clBackColor );
				m_listCommands.SetAt( pos, pclNewButtonEx );
				delete pItem;
			}
			fFound = true;
		}
		else
		{
			m_listCommands.GetNext( pos );
		}
	}
}

void CMFCToolBarMenuButtonEx::CopyParamTo( DrawItemParam* pclDrawItemParam )
{
	pclDrawItemParam->m_fCustomized = m_rDrawItemParam.m_fCustomized;
	pclDrawItemParam->m_textColor = m_rDrawItemParam.m_textColor;
	pclDrawItemParam->m_backColor = m_rDrawItemParam.m_backColor;
}

void CMFCToolBarMenuButtonEx::OnDraw( CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons )
{
	if( true == m_rDrawItemParam.m_fCustomized )
	{
		m_rectArrow.SetRectEmpty();
		m_rectButton.SetRectEmpty();
		if( m_bMenuMode )
		{
			_DrawMenuItem( pDC, rect, pImages, bCustomizeMode, bHighlight, bGrayDisabledButtons );
			return;
		}
	}

	CMFCToolBarMenuButton::OnDraw( pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons );
}

void CMFCToolBarMenuButtonEx::CopyFrom( const CMFCToolBarButton& src )
{
	CMFCToolBarMenuButton::CopyFrom( src );
	CMFCToolBarButton* pclButton = (CMFCToolBarButton*)&src;
	CMFCToolBarMenuButtonEx* pclButtonEx = dynamic_cast<CMFCToolBarMenuButtonEx*>( pclButton );
	if( NULL != pclButtonEx )
	{
		pclButtonEx->CopyParamTo( &m_rDrawItemParam );
	}
}

void CMFCToolBarMenuButtonEx::_DrawMenuItem( CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bCustomizeMode, BOOL bHighlight, BOOL bGrayDisabledButtons, BOOL bContentOnly )
{
	// The main part of this code directly comes from the MFC source.

	ASSERT_VALID( pDC );
	ASSERT_VALID( this );

	if( m_nID == AFX_MENU_GROUP_ID )
	{
		COLORREF clrText = CMFCVisualManager::GetInstance()->OnDrawMenuLabel( pDC, rect );
		COLORREF clrTextOld = pDC->SetTextColor( clrText );

		CRect rectText = rect;
		rectText.DeflateRect( AFX_TEXT_MARGIN, 0 );
		rectText.bottom -= 2;

		CFont* pOldFont = pDC->SelectObject( &afxGlobalData.fontBold );
		ASSERT_VALID( pOldFont );

		pDC->DrawText( m_strText, rectText, DT_SINGLELINE | DT_VCENTER );

		pDC->SetTextColor( clrTextOld );
		pDC->SelectObject( pOldFont );
		return;
	}

	BOOL bDisabled = ( bCustomizeMode && !IsEditable() ) || ( !bCustomizeMode && ( m_nStyle & TBBS_DISABLED ) );

	CMFCToolBarImages* pLockedImages = NULL;
	CMFCToolBarImages* pUserImages = NULL;
	CAfxDrawState ds;

	CMFCPopupMenuBar* pParentMenu = DYNAMIC_DOWNCAST( CMFCPopupMenuBar, m_pWndParent );

	CSize sizeMenuImage = CMFCToolBar::GetMenuImageSize();

	if( pParentMenu != NULL )
	{
		if( pParentMenu->IsDropDownListMode() )
		{
			CMFCDropDownListBox* pList = DYNAMIC_DOWNCAST( CMFCDropDownListBox, pParentMenu->GetParent() );
			if( pList != NULL )
			{
				COLORREF clrText = CMFCVisualManager::GetInstance()->GetMenuItemTextColor( this, bHighlight, FALSE );
				if( bHighlight )
				{
					CMFCVisualManager::GetInstance()-> OnHighlightMenuItem( pDC, this, rect, clrText );
				}

				COLORREF clrTextOld = pDC->SetTextColor( clrText );

				pList->OnDrawItem( pDC, this, bHighlight );

				pDC->SetTextColor( clrTextOld );
				return;
			}
		}

		if( pParentMenu->m_pRelatedToolbar != NULL && pParentMenu->m_pRelatedToolbar->IsLocked() )
		{
			pLockedImages = (CMFCToolBarImages*)pParentMenu->m_pRelatedToolbar->GetLockedMenuImages();
			if( pLockedImages != NULL )
			{
				CSize sizeDest( 0, 0 );

				if( sizeMenuImage != pParentMenu->GetCurrentMenuImageSize() )
				{
					sizeDest = sizeMenuImage;
				}

				pLockedImages->PrepareDrawImage( ds, sizeDest );

				pImages = pLockedImages;
			}
		}
	}

	BOOL bDisableImage = afxCommandManager->IsMenuItemWithoutImage( m_nID );
	if( m_nID == ID_AFXBARRES_TASKPANE_BACK || m_nID == ID_AFXBARRES_TASKPANE_FORWARD )
	{
		bDisableImage = TRUE;
	}

	CUserTool* pUserTool = NULL;
	if( afxUserToolsManager != NULL && !m_bUserButton )
	{
		pUserTool = afxUserToolsManager->FindTool( m_nID );
	}

	HICON hDocIcon = CMFCTabCtrl::GetDocumentIcon( m_nID );

	CSize sizeImage = CMenuImages::Size();

	if( m_pPopupMenu != NULL && !m_bToBeClosed )
	{
		bHighlight = TRUE;
	}

	// COLORREF clrText = CMFCVisualManager::GetInstance()->GetMenuItemTextColor( this, bHighlight, bDisabled );
	COLORREF clrText = m_rDrawItemParam.m_textColor;

	BOOL bDrawImageFrame = !CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem();

	if( bHighlight && !bContentOnly && CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem() )
	{
		CMFCVisualManager::GetInstance()->OnHighlightMenuItem( pDC, this, rect, clrText );
		bDrawImageFrame = FALSE;
	}

	if( ( m_nStyle & TBBS_CHECKED) && !CMFCVisualManager::GetInstance()->IsOwnerDrawMenuCheck() )
	{
		bDrawImageFrame = TRUE;
	}

	CFont* pOldFont = NULL;

	if( m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly && pParentMenu != NULL && pParentMenu->GetDefaultMenuId() == m_nID )
	{
		pOldFont = (CFont*)pDC->SelectObject( &afxGlobalData.fontBold );
	}

	CRect rectImage;
	rectImage = rect;
	rectImage.left += CMFCVisualManager::GetInstance()->GetMenuImageMargin();
	rectImage.right = rectImage.left + sizeMenuImage.cx + CMFCVisualManager::GetInstance()->GetMenuImageMargin();

	CRect rectFrameBtn = rectImage;

	if( CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem() )
	{
		rectFrameBtn = rect;

		rectFrameBtn.left += 2;
		rectFrameBtn.top++;
		rectFrameBtn.bottom -= 2;
		rectFrameBtn.right = rectImage.right;
	}
	else
	{
		rectFrameBtn.InflateRect( 1, -1 );
	}

	BOOL bIsRarelyUsed = ( CMFCMenuBar::IsRecentlyUsedMenus() && CMFCToolBar::IsCommandRarelyUsed( m_nID ) );
	if( bIsRarelyUsed )
	{
		bIsRarelyUsed = FALSE;

		CMFCPopupMenuBar* pParentMenuBar = DYNAMIC_DOWNCAST( CMFCPopupMenuBar, m_pWndParent );
		if( pParentMenuBar != NULL )
		{
			CMFCPopupMenu* pParentMenuCurr = DYNAMIC_DOWNCAST( CMFCPopupMenu, pParentMenuBar->GetParent() );
			if( pParentMenuCurr != NULL && pParentMenuCurr->HideRarelyUsedCommands() )
			{
				bIsRarelyUsed = TRUE;
			}
		}
	}

	BOOL bLightImage = FALSE;
	BOOL bFadeImage = !bHighlight && CMFCVisualManager::GetInstance()->IsFadeInactiveImage();

	if( bIsRarelyUsed )
	{
		bLightImage = TRUE;
		if( bHighlight && ( m_nStyle & ( TBBS_CHECKED | TBBS_INDETERMINATE ) ) )
		{
			bLightImage = FALSE;
		}

		if( GetImage() < 0 && !( m_nStyle & ( TBBS_CHECKED | TBBS_INDETERMINATE ) ) )
		{
			bLightImage = FALSE;
		}
	}
	else if( m_nStyle &( TBBS_CHECKED | TBBS_INDETERMINATE ) )
	{
		bLightImage = !bHighlight;
	}

	//----------------
	// Draw the image:
	//----------------
	if( !IsDrawImage() && hDocIcon == NULL ) // Try to find a matched image
	{
		BOOL bImageSave = m_bImage;
		BOOL bUserButton = m_bUserButton;
		BOOL bSuccess = TRUE;

		m_bImage = TRUE; // Always try to draw image!
		m_bUserButton = TRUE;

		if( GetImage() < 0 )
		{
			m_bUserButton = FALSE;
			if( GetImage() < 0 )
			{
				bSuccess = FALSE;
			}
		}

		if( !bSuccess )
		{
			m_bImage = bImageSave;
			m_bUserButton = bUserButton;
		}

		if( m_bUserButton && pImages != CMFCToolBar::GetUserImages() )
		{
			pUserImages = CMFCToolBar::GetUserImages();
			if( pUserImages != NULL )
			{
				ASSERT_VALID( pUserImages );

				pUserImages->PrepareDrawImage( ds );
				pImages = pUserImages;
			}
		}
	}

	BOOL bImageIsReady = FALSE;

	CRgn rgnClip;
	rgnClip.CreateRectRgnIndirect( &rectImage );

	if( bDrawImageFrame && !bContentOnly )
	{
		FillInterior( pDC, rectFrameBtn, bHighlight );
	}

	if( !bDisableImage && ( IsDrawImage() && pImages != NULL ) || hDocIcon != NULL )
	{
		BOOL bDrawImageShadow = bHighlight && !bCustomizeMode && CMFCVisualManager::GetInstance()->IsShadowHighlightedImage() &&
			!afxGlobalData.IsHighContrastMode() && ( ( m_nStyle & TBBS_CHECKED ) == 0 ) && ( ( m_nStyle & TBBS_DISABLED ) == 0 );

		pDC->SelectObject( &rgnClip );

		CPoint ptImageOffset( ( rectImage.Width() - sizeMenuImage.cx ) / 2, ( rectImage.Height() - sizeMenuImage.cy ) / 2 );
		if( ( m_nStyle & TBBS_PRESSED ) || !( m_nStyle & TBBS_DISABLED ) || !bGrayDisabledButtons || bCustomizeMode )
		{
			CRect rectIcon( CPoint( rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y ), sizeMenuImage );
			if( hDocIcon != NULL )
			{
				DrawDocumentIcon( pDC, rectIcon, hDocIcon );
			}
			else if( pUserTool != NULL )
			{
				pUserTool->DrawToolIcon( pDC, rectIcon );
			}
			else
			{
				CPoint pt = rectImage.TopLeft();
				pt += ptImageOffset;

				if( bDrawImageShadow )
				{
					pt.Offset( 1, 1 );

					pImages->Draw( pDC, pt.x, pt.y, GetImage(), FALSE, FALSE, FALSE, TRUE );

					pt.Offset( -2, -2 );
				}

				pImages->Draw( pDC, pt.x, pt.y, GetImage(), FALSE, bDisabled && bGrayDisabledButtons, FALSE, FALSE, bFadeImage );
			}

			bImageIsReady = TRUE;
		}

		if( !bImageIsReady )
		{
			CRect rectIcon( CPoint( rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y ), sizeMenuImage );

			if( hDocIcon != NULL )
			{
				DrawDocumentIcon( pDC, rectIcon, hDocIcon );
			}
			else if( pUserTool != NULL )
			{
				pUserTool->DrawToolIcon( pDC, rectIcon );
			}
			else
			{
				if( bDrawImageShadow )
				{
					rectImage.OffsetRect( 1, 1 );
					pImages->Draw( pDC, rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y, GetImage(), FALSE, FALSE, FALSE, TRUE );

					rectImage.OffsetRect( -2, -2 );
				}

				pImages->Draw( pDC, rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y, GetImage(), FALSE, bDisabled && bGrayDisabledButtons, FALSE, FALSE, bFadeImage );
			}

			bImageIsReady = TRUE;
		}
	}

	if( m_bAlwaysCallOwnerDraw || !bImageIsReady )
	{
		CFrameWnd* pParentFrame = m_pWndParent == NULL ? DYNAMIC_DOWNCAST( CFrameWnd, AfxGetMainWnd() ) : AFXGetTopLevelFrame( m_pWndParent );

		//------------------------------------
		// Get chance to user draw menu image:
		//------------------------------------
		CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST( CMDIFrameWndEx, pParentFrame );
		if( pMainFrame != NULL )
		{
			bImageIsReady = pMainFrame->OnDrawMenuImage( pDC, this, rectImage );
		}
		else // Maybe, SDI frame...
		{
			CFrameWndEx* pFrame = DYNAMIC_DOWNCAST( CFrameWndEx, pParentFrame );
			if( pFrame != NULL )
			{
				bImageIsReady = pFrame->OnDrawMenuImage( pDC, this, rectImage );
			}
			else // Maybe, OLE frame...
			{
				COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST( COleIPFrameWndEx, pParentFrame );
				if( pOleFrame != NULL )
				{
					bImageIsReady = pOleFrame->OnDrawMenuImage( pDC, this, rectImage );
				}
				else
				{
					COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST( COleDocIPFrameWndEx, pParentFrame );
					if( pOleDocFrame != NULL )
					{
						bImageIsReady = pOleDocFrame->OnDrawMenuImage( pDC, this, rectImage );
					}
				}
			}
		}
	}

	pDC->SelectClipRgn( NULL );

	if( m_nStyle & TBBS_CHECKED )
	{
		if( bDrawImageFrame )
		{
			UINT nStyleSaved = m_nStyle;

			if( bHighlight && CMFCVisualManager::GetInstance()->IsFrameMenuCheckedItems() )
			{
				m_nStyle |= TBBS_MARKED;
			}

			CMFCVisualManager::GetInstance()->OnDrawButtonBorder( pDC, this, rectFrameBtn, CMFCVisualManager::ButtonsIsPressed );

			m_nStyle = nStyleSaved;
		}

		if( !bImageIsReady )
		{
			CMFCVisualManager::GetInstance()->OnDrawMenuCheck( pDC, this, rectFrameBtn, bHighlight, m_bIsRadio );
		}
	}
	else if( !bContentOnly && bImageIsReady && bHighlight && bDrawImageFrame )
	{
		CMFCVisualManager::GetInstance()->OnDrawButtonBorder( pDC, this, rectFrameBtn, CMFCVisualManager::ButtonsIsHighlighted );
	}

	rectImage.InflateRect( 1, 0 );
	int iSystemImageId = -1;

	//-------------------------------
	// Try to draw system menu icons:
	//-------------------------------
	if( !bImageIsReady )
	{
		switch( m_nID )
		{
			case SC_MINIMIZE:
				iSystemImageId = CMenuImages::IdMinimize;
				break;

			case SC_RESTORE:
				iSystemImageId = CMenuImages::IdRestore;
				break;

			case SC_CLOSE:
				iSystemImageId = CMenuImages::IdClose;
				break;

			case SC_MAXIMIZE:
				iSystemImageId = CMenuImages::IdMaximize;
				break;
		}

		if( iSystemImageId != -1 )
		{
			CRect rectSysImage = rectImage;
			rectSysImage.DeflateRect( CMFCVisualManager::GetInstance()->GetMenuImageMargin(), CMFCVisualManager::GetInstance()->GetMenuImageMargin() );

			if( !bContentOnly && bDrawImageFrame )
			{
				FillInterior( pDC, rectFrameBtn, bHighlight );
			}

			CMenuImages::Draw( pDC, (CMenuImages::IMAGES_IDS)iSystemImageId, rectSysImage, bDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack );

			if( bHighlight && !bContentOnly && bDrawImageFrame )
			{
				CMFCVisualManager::GetInstance()->OnDrawButtonBorder( pDC, this, rectFrameBtn, CMFCVisualManager::ButtonsIsHighlighted );
			}
		}
	}

	//-------------------------------
	// Fill text area if highlighted:
	//-------------------------------
	CRect rectText = rect;
	rectText.left = rectFrameBtn.right + CMFCVisualManager::GetInstance()->GetMenuImageMargin() + 2;

	if( bHighlight )
	{
		if( !CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem() )
		{
			CRect rectFill = rectFrameBtn;

			if( ( m_nStyle &(TBBS_CHECKED) || bImageIsReady ) || iSystemImageId != -1 )
			{
				rectFill.left = rectText.left - 1;
			}

			rectFill.right = rect.right - 1;

			if( !bContentOnly )
			{
				CMFCVisualManager::GetInstance()->OnHighlightMenuItem( pDC, this, rectFill, clrText );
			}
			else
			{
				clrText = CMFCVisualManager::GetInstance()->GetHighlightedMenuItemTextColor( this );
			}
		}
		else if( bContentOnly )
		{
			clrText = CMFCVisualManager::GetInstance()->GetHighlightedMenuItemTextColor( this );
		}
	}

	//-------------------------
	// Find acceleration label:
	//-------------------------
	CString strText = m_strText;
	CString strAccel;

	int iTabOffset = m_strText.Find( _T('\t') );
	if( iTabOffset >= 0 )
	{
		strText = strText.Left( iTabOffset );
		strAccel = m_strText.Mid( iTabOffset + 1 );
	}

	//-----------
	// Draw text:
	//-----------
	COLORREF clrTextOld = pDC->GetTextColor();

	rectText.left += AFX_TEXT_MARGIN;

	if( !m_bWholeText )
	{
		CString strEllipses( _T("...") );
		while( strText.GetLength() > 0 && pDC->GetTextExtent( strText + strEllipses ).cx > rectText.Width() )
		{
			strText = strText.Left( strText.GetLength() - 1 );
		}

		strText += strEllipses;
	}

	if( bDisabled && !bHighlight && CMFCVisualManager::GetInstance()->IsEmbossDisabledImage() )
	{
		pDC->SetTextColor( afxGlobalData.clrBtnHilite );

		CRect rectShft = rectText;
		rectShft.OffsetRect(1, 1);
		pDC->DrawText( strText, &rectShft, DT_SINGLELINE | DT_VCENTER );
	}

	pDC->SetTextColor( clrText );
	pDC->DrawText( strText, &rectText, DT_SINGLELINE | DT_VCENTER );

	//------------------------
	// Draw accelerator label:
	//------------------------
	if( !strAccel.IsEmpty() )
	{
		CRect rectAccel = rectText;
		rectAccel.right -= AFX_TEXT_MARGIN + sizeImage.cx;

		if( bDisabled && !bHighlight && CMFCVisualManager::GetInstance()->IsEmbossDisabledImage() )
		{
			pDC->SetTextColor( afxGlobalData.clrBtnHilite );

			CRect rectAccelShft = rectAccel;
			rectAccelShft.OffsetRect( 1, 1 );
			pDC->DrawText( strAccel, &rectAccelShft, DT_SINGLELINE | DT_RIGHT | DT_VCENTER );
		}

		pDC->SetTextColor( clrText );
		pDC->DrawText( strAccel, &rectAccel, DT_SINGLELINE | DT_RIGHT | DT_VCENTER );
	}

	//--------------------------------------------
	// Draw triangle image for the cascade menues:
	//--------------------------------------------
	if( m_nID == (UINT) -1 || m_bDrawDownArrow || m_bMenuOnly )
	{
		CFont* pRegFont = pDC->SelectObject( &afxGlobalData.fontMarlett );
		ENSURE( pRegFont != NULL );

		CRect rectTriangle = rect;

		CString strTriangle = ( m_pWndParent->GetExStyle() & WS_EX_LAYOUTRTL ) ? _T("3") : _T("4"); // Marlett's right arrow

		if( m_bQuickCustomMode )
		{
			strTriangle = _T("6");  	// Marlett's down arrow
		}

		if( HasButton() )
		{
			m_rectButton = rect;

			m_rectButton.left = m_rectButton.right - pDC->GetTextExtent( strTriangle ).cx;

			CMFCVisualManager::GetInstance()->OnDrawMenuItemButton( pDC, this, m_rectButton, bHighlight, bDisabled );
		}

		pDC->DrawText( strTriangle, &rectTriangle, DT_SINGLELINE | DT_RIGHT | DT_VCENTER );

		pDC->SelectObject( pRegFont );
	}

	if( pOldFont != NULL )
	{
		pDC->SelectObject( pOldFont );
	}

	pDC->SetTextColor( clrTextOld );

	if( pLockedImages != NULL )
	{
		pLockedImages->EndDrawImage( ds );
	}

	if( pUserImages != NULL )
	{
		ASSERT_VALID( pUserImages );
		pUserImages->EndDrawImage( ds );
	}
}