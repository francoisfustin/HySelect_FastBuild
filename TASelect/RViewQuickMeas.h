#pragma once


#include "Drawset.h"
#include "FormViewEx.h"
#include "SSheetQuickMeas.h"

class CRViewQuickMeas : public CFormViewEx , protected CDrawSet
{
DECLARE_DYNCREATE( CRViewQuickMeas )

public:
	enum { IDD = IDV_RVIEW };

	CRViewQuickMeas();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif	

	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );
	void SetRedraw();
	void CallOnFileExportQM() { OnFileExportQM(); }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void OnFileExportQM();
	LRESULT OnNewDocument( WPARAM wParam, LPARAM lParam );

	// Spread DLL message handlers.
	afx_msg LRESULT ClickFpspread( WPARAM wParam, LPARAM lParam );

// Protected variables.
protected:
	CSSheetQuickMeas m_SheetQuickMeas;

// Private methods.
private:
	void _ResizeColumnSheet( void );
};

extern CRViewQuickMeas* pRViewQuickMeas;