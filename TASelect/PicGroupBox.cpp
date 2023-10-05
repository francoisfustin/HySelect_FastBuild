
#include "StdAfx.h"
#include "resource.h"
#include "EnBitmap.h"

#include "PicGroupBox.h"

CPicGroupBox::CPicGroupBox()
{
	m_PicPos = CPoint(5,0);
	m_bTransparent = true;
}

CPicGroupBox::~CPicGroupBox(void)
{
	m_eBmp.DeleteObject();
}

void CPicGroupBox::SetPicture(int PicID, CPoint ptPos /*= CPoint(5,0)*/, bool bTransparent /*= true*/)
{
	m_eBmp.DeleteObject();
	//m_eBmp.LoadImage(PicID,_T("Bitmap"));
	m_eBmp.LoadBitmap(PicID);
	m_PicPos = ptPos;
	m_bTransparent = bTransparent;
}

BEGIN_MESSAGE_MAP(CPicGroupBox, CButton)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CPicGroupBox::OnPaint()
{
	CPaintDC dc(this); // device context for painting
   // Create a memory DC compatible with the paint DC
    CDC memDC;
    memDC.CreateCompatibleDC( &dc );
 
    CRect rcClip, rcClient;
    dc.GetClipBox( &rcClip );
    GetClientRect(&rcClient);
 
    // Select a compatible bitmap into the memory DC
    CBitmap bitmap;
    bitmap.CreateCompatibleBitmap( &dc, rcClient.Width(), rcClient.Height() );
    memDC.SelectObject( &bitmap );
    
    // Set clip region to be same as that in paint DC
    CRgn rgn;
    rgn.CreateRectRgnIndirect( &rcClip );
    memDC.SelectClipRgn(&rgn);
    rgn.DeleteObject();
 	
	CBrush Brush;
	Brush.CreateSysColorBrush(COLOR_BTNFACE);
	memDC.FillRect(rcClip, &Brush);
	Brush.DeleteObject();
    
	// Draw group box into the bitmap.
    CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
    
 	//Draw Bitmap
	m_eBmp.DrawOnDC(&memDC, m_PicPos, m_bTransparent);
    // Draw on screen
    dc.BitBlt( rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC, 
                rcClip.left, rcClip.top, SRCCOPY );

    bitmap.DeleteObject();
}
