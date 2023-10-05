#pragma once


#include "FormViewEx.h"

class CTASelectView : public CFormViewEx
{
protected:
	DECLARE_DYNCREATE( CTASelectView )
	CTASelectView();

// Public methods.
public:
	virtual ~CTASelectView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
	CTASelectDoc* GetDocument() const;
#else
	inline CTASelectDoc* CTASelectView::GetDocument() const	{ return reinterpret_cast<CTASelectDoc*>(m_pDocument); }
#endif

// Public variables.
public:
	enum{ IDD = IDD_TASELECT_VIEW };

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()

	virtual void OnInitialUpdate();

	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnContextMenu( CDockablePane* pWnd, CPoint point ) {}

// Protected variables.
protected:
	bool m_fUpdateLoaded;
};

