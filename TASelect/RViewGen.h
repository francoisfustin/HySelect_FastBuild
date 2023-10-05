#pragma once


#include "Drawset.h"
#include "FormViewEx.h"
#include "SSheetGen.h"

class CRViewGen : public CFormViewEx , protected CDrawSet
{
public:
	enum { IDD = IDV_RVIEW };

	CRViewGen();
	virtual ~CRViewGen();

	DECLARE_DYNCREATE( CRViewGen )

	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );
	void SetRedraw();

// Protected members
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif	

	void ResizeColumnSheet();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );

// Protected variables
protected:
	enum enum_CurPrintPos
	{
		ecppHeader,
		ecppFooter,
		ecppLogDataTitle,
		ecppRows
	};

	CSSheetGen		m_SheetGen;

};

extern CRViewGen *pRViewGen;
