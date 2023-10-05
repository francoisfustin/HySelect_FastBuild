#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "TASelectView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CTASelectApp TASApp;

IMPLEMENT_DYNCREATE( CTASelectView, CFormViewEx )

CTASelectView::CTASelectView()
	: CFormViewEx( CTASelectView::IDD )
{
	m_fUpdateLoaded = false;
}

CTASelectView::~CTASelectView()
{
}

#ifdef _DEBUG
void CTASelectView::AssertValid() const
{
	CFormViewEx::AssertValid();
}

void CTASelectView::Dump( CDumpContext& dc ) const
{
	CFormViewEx::Dump( dc );
}

CTASelectDoc* CTASelectView::GetDocument() const
{
	ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CTASelectDoc ) ) );
	return (CTASelectDoc*)m_pDocument;
}
#endif

BEGIN_MESSAGE_MAP( CTASelectView, CFormViewEx )

END_MESSAGE_MAP()


void CTASelectView::OnInitialUpdate()
{
	// Only one OnInitialUpdate() is alowed.
	if( true == m_fUpdateLoaded )
		return;
	m_fUpdateLoaded = true;

	CFormViewEx::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	Invalidate();
	ResizeParentToFit();

	// Call initialization of other views form the main view.
	CMainFrame *pMainFrame = static_cast<CMainFrame *>( GetParentFrame() );
	pMainFrame->InitViews();

	// Check what is the lateral pane state when launching. If it is in the 'autoHide' mode, it will be fixeed to correspond
	// to the checkbox state (thus the mode is fixed). If we want the checkbox be updated in regards to its state, we must change
	// its state in regards to the value of 'IsAutoHideMode()'.
	if( TRUE == pMainFrame->m_wndTabs.IsAutoHideMode() )
	{
		CPaneDivider* pDefaultSlider = pMainFrame->m_wndTabs.GetDefaultPaneDivider();
		pMainFrame->m_wndTabs.SetAutoHideMode( FALSE, pDefaultSlider->GetCurrentAlignment() );
	}

	COLORREF clrDark;
	COLORREF clrBlack;
	COLORREF clrHighlight;
	COLORREF clrFace;
	COLORREF clrDarkShadow;
	COLORREF clrLight;
	CBrush* pbrFace = NULL;
	CBrush* pbrBlack = NULL;

	// recuperation des couleurs de l'interface pour changement de couleur sur le changement de look de l'application.
	CMFCVisualManager::GetInstance()->GetTabFrameColors( pMainFrame->m_wndTabs.GetMFCTabCtrl(), clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack );
		
	SetBackgroundColor(	clrDarkShadow );
}

void CTASelectView::OnRButtonUp( UINT nFlags, CPoint point )
{
	ClientToScreen( &point );
	OnContextMenu( (CDockablePane*)this, point );
}
