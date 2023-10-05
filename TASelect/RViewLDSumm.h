#pragma once


#include "Drawset.h"
#include "FormViewEx.h"
#include "SSheetLDSumm.h"

class CRViewLDSumm : public CFormViewEx , protected CDrawSet
{
DECLARE_DYNCREATE( CRViewLDSumm )

public:
	enum { IDD = IDV_RVIEW };

	CRViewLDSumm();
	virtual ~CRViewLDSumm();

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
	CSSheetLDSumm m_SheetLDSumm;
	
// Private methods.
private:
	void _ResizeColumnSheet();
};

extern CRViewLDSumm *pRViewLDSumm;
