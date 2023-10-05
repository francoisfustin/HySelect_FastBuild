// RViewLogData.h: interface for the CRViewLogData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "FormViewEx.h"
#include "RScrollView.h"
#include "SSheetLogData.h"

class CRViewLogData : public CFormViewEx , protected CDrawSet
{
public:
	DECLARE_DYNCREATE( CRViewLogData )
	
	enum { IDD = IDV_RVIEW };
	
	CRViewLogData();
	virtual ~CRViewLogData();
	
	CTASelectDoc* GetDocument();
	void SetRedraw( CLog *pLogData = NULL );
	void CallOnFileExportLdlist() { OnFileExportLdlist(); }
	
	// !!! DrawLogInfo moved to CRightScrollView (Draw all log Information) 
	//int DrawLogInfo(CDC* pDC, CLog *pLD, int x, int y,int width=RIGHTVIEWWIDTH, bool DrawNow=true);
	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );

protected:

	void ResizeColumnSheet();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnPaint();
	afx_msg void OnFileExportLdlist();
	LRESULT OnNewDocument( WPARAM wParam, LPARAM lParam );

// Protected variables
protected:
	enum enum_CurPrintPos
	{
		ecppHeader,
		ecppFooter,
		ecppLogDataTitle,
		ecppLogDataInfo,
		ecppRows
	};

	CSSheetLogData	m_SheetLogData;
	CLog*			m_pLogData;
};

extern CRViewLogData *pRViewLogData;

#ifndef _DEBUG  // debug version in RightViewSelP.cpp
inline CTASelectDoc* CRViewLogData::GetDocument()
   { return (CTASelectDoc*)m_pDocument; }
#endif