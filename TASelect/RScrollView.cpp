// RScrollView.cpp : implementation file
//
#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "Utilities.h"

#include "RScrollView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRScrollView

IMPLEMENT_DYNCREATE(CRScrollView, CScrollView)

CRScrollView::CRScrollView()
{
	m_ptSizeScroll.cx = m_ptSizeScroll.cy = 100;
}

CRScrollView::~CRScrollView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CRScrollView drawing

void CRScrollView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	// TODO: calculate the total size of this view
	SetScrollSizes( MM_TEXT, m_ptSizeScroll );
	m_Xfactor = 100;
}

void CRScrollView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
}

///////////////////////////////////////////////////////////////////////////////
// CRScrollView diagnostics

#ifdef _DEBUG
void CRScrollView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CRScrollView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
CTASelectDoc* CRScrollView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTASelectDoc)));
	return (CTASelectDoc*)m_pDocument;
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRScrollView message handlers

BOOL CRScrollView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	// default preparation
	m_CurPrintPos = 0;					// Start with header page
	m_CurPrintIndex = 0;
	return DoPreparePrinting(pInfo);
}

void CRScrollView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	// Compute Left and right Margin
	// Margin is fixed to 10 mm
	int iMargin=Getmm2pt(pDC,10,false);
	
	int factor;
	factor = 6;
	m_PrintTitleFont.DeleteObject();
	m_PrintSubTitleFont.DeleteObject();
	m_PrintText10.DeleteObject();
	m_PrintText11.DeleteObject();
	m_PrintTextBold.DeleteObject();

	m_PrintTitleFont.CreateFont(-22*factor,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Comic Sans MS"));		
	m_PrintSubTitleFont.CreateFont(-18*factor,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Comic Sans MS"));
	m_PrintText10.CreateFont(-10*factor,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_PrintText11.CreateFont(-11*factor,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_PrintTextBold.CreateFont(-11*factor,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	pInfo->SetMaxPage(1000);
	m_bContinuePrinting = true;
	m_CurrentPage = 1;
	
	CScrollView::OnBeginPrinting(pDC, pInfo);
}

void CRScrollView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	m_PrintTitleFont.DeleteObject();
	m_PrintSubTitleFont.DeleteObject();
	m_PrintText10.DeleteObject();
	m_PrintText11.DeleteObject();
	m_PrintTextBold.DeleteObject();

	int factor = 1;
	m_PrintTitleFont.CreateFont(-22*factor,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Comic Sans MS"));		
	m_PrintSubTitleFont.CreateFont(-18*factor,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Comic Sans MS"));
	m_PrintText10.CreateFont(-10*factor,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_PrintText11.CreateFont(-11*factor,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_PrintTextBold.CreateFont(-11*factor,0,0,0,FW_HEAVY,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_Xfactor = 100;
}

void CRScrollView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	if (pDC->IsPrinting())
	{
		if (!m_bContinuePrinting )
		{
			if (!pInfo->m_bPreview) 
				pInfo->SetMaxPage(-1);//pInfo->m_nCurPage-1);
			else
				pInfo->SetMaxPage(pInfo->m_nCurPage);
		}
		pInfo->m_bContinuePrinting=m_bContinuePrinting;
	}
	
	CScrollView::OnPrepareDC(pDC, pInfo);
}

void CRScrollView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	CScrollView::OnPrint(pDC, pInfo);
}
