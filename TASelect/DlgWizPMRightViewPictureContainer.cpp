#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"

#include "DlgWizPMRightViewPictureContainer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewPictureContainer::CDlgWizardPM_RightViewPictureContainer()
{
	m_iCurrentImageIDB = -1;
}

void CDlgWizardPM_RightViewPictureContainer::UpdatePicture( int iImageIDB )
{
	if( m_iCurrentImageIDB == iImageIDB )
	{
		// Do nothing if it's the same image.
		return;
	}

	if( INVALID_HANDLE_VALUE != m_hCurrentImage )
	{
		DeleteObject( m_hCurrentImage );
	}

	if( -1 == iImageIDB )
	{
		// No image to display.
		m_iCurrentImageIDB = -1;
		m_clPicture.SetBitmap( NULL );
		return;
	}

	m_hCurrentImage = (HBITMAP)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( iImageIDB ), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR );
	m_clPicture.SetBitmap( m_hCurrentImage );

	m_iCurrentImageIDB = iImageIDB;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewPictureContainer, CDialogExt )
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewPictureContainer::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICPICTURE, m_clPicture );
}

void CDlgWizardPM_RightViewPictureContainer::OnOK()
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

BOOL CDlgWizardPM_RightViewPictureContainer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}

BOOL CDlgWizardPM_RightViewPictureContainer::OnEraseBkgnd( CDC *pDC )
{
	CBrush clBrush;
	clBrush.CreateSolidBrush( _WHITE );

	CRect rectClient;
	GetClientRect( &rectClient );
	pDC->FillRect( rectClient, &clBrush );

	DeleteObject( &clBrush );

	return TRUE;
}

void CDlgWizardPM_RightViewPictureContainer::OnPaint()
{
	CPaintDC dc( this );
	CDialogExt::OnPaint();

	CBrush clBrush;
	clBrush.CreateSolidBrush( DLGWIZARDPM_RVPICTURE_SEPARATORCOLOR );

	CRect rectClient;
	GetClientRect( &rectClient );

	CRect rectSeparator = rectClient;
	rectSeparator.right = rectSeparator.left + 2;
	rectSeparator.bottom += 1;
	dc.FillRect( &rectSeparator, &clBrush );

	rectSeparator = rectClient;
	rectSeparator.bottom = rectSeparator.top + 2;
	rectSeparator.right += 1;
	dc.FillRect( &rectSeparator, &clBrush );

	DeleteObject( &clBrush );
}
