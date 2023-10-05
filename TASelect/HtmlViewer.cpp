//
// RightViewInfo.cpp : implementation of the CHtmlViewer class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "utilities.h"
#include "HtmlViewer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//IMPLEMENT_DYNCREATE(CHtmlViewer, CHtmlView)

BEGIN_MESSAGE_MAP(CHtmlViewer, CHtmlView)
	ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHtmlViewer construction/destruction

CHtmlViewer::CHtmlViewer()
{

}

CHtmlViewer::~CHtmlViewer()
{
	Stop();
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlViewer member functions

void CHtmlViewer::DisplayPage(CString URL)
{
	// Start navigation
	URL.MakeLower() ;
		
	if (m_OriginalPage!=URL) 
		Navigate2(m_OriginalPage = URL) ;
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlViewer message handlers


void CHtmlViewer::OnInitialUpdate() 
{
	Navigate2(m_OriginalPage);
}

// this method needs to be overridden to prevent 
//     an assertion
int CHtmlViewer::OnMouseActivate( CWnd* pDesktopWnd, 
                                  UINT nHitTest, 
                                  UINT message )
{
  return MA_ACTIVATE;
}

