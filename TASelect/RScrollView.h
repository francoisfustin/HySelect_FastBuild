//
// RightScrollView.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DrawSet.h"
/////////////////////////////////////////////////////////////////////////////
// CRScrollView view

class CRScrollView : public CScrollView , protected CDrawSet
{
protected:
	CRScrollView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CRScrollView)

// Attributes
public:
	CTASelectDoc* GetDocument();

// Operations
public:
	virtual void Print() { CView::OnFilePrint(); }

// Overrides
	// ClassWizard generated virtual function overrides
	public:
	virtual void OnPrepareDC( CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual void OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting( CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting( CPrintInfo* pInfo);
	virtual void OnPrint( CDC* pDC, CPrintInfo* pInfo);

protected:
	virtual void OnDraw( CDC* pDC );				// overridden to draw this view
	virtual void OnInitialUpdate();					// first time after construct

// Implementation
protected:
	virtual ~CRScrollView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Private variables
protected:
	CSize m_ptSizeScroll;
};
#ifndef _DEBUG  // debug version in RightViewSelP.cpp

inline CTASelectDoc* CRScrollView::GetDocument()
{ 
	return (CTASelectDoc*)m_pDocument;
}
#endif