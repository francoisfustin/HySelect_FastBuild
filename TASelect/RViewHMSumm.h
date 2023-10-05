#pragma once

#include "Drawset.h"
#include "FormViewEx.h"
#include "SSheetHMSumm.h"

class CRViewHMSumm : public CFormViewEx , protected CDrawSet
{
DECLARE_DYNCREATE( CRViewHMSumm )

public:
	enum { IDD = IDV_RVIEW };
	
	CRViewHMSumm();
	virtual ~CRViewHMSumm();
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );
	void SetRedraw();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	LRESULT OnNewDocument( WPARAM wParam, LPARAM lParam );	

// Protected variables.
protected:
	CSSheetHMSumm m_SheetHMSumm;

// Private methods.
private:
	void _ResizeColumnSheet( void );
};

extern CRViewHMSumm *pRViewHMSumm;
