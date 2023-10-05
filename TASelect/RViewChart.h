#pragma once


#include "RScrollView.h"
#include "LoggedDataChart.h"

class CRViewChart : public CRScrollView
{
public:
	DECLARE_DYNCREATE( CRViewChart )
	CRViewChart();
	virtual ~CRViewChart();

	enum { IDD = IDV_RVIEW };

// Attributes
public:
	CTASelectDoc* GetDocument();

	void SetRedraw( CLog* pLoggedData );
	void CallOnChartTrack() { OnChartTrack(); }
	void CallOnChartRescale() { OnChartRescale(); }
	void CallOnChart2dzoom() { OnChart2dzoom(); }
	void CallOnChartDateaxis() { OnChartDateaxis(); }
	void CallOnFileExportChart() { OnFileExportChart(); }
	void CallOnEditCopy() { OnEditCopy(); }
	void CallOnChartLegend() { OnChartLegend(); }
	void CallOnUpdateChartTrack( CCmdUI* pCmdUI ) { OnUpdateChartTrack(pCmdUI); }
	void CallOnUpdateChart2dzoom( CCmdUI* pCmdUI ) { OnUpdateChart2dzoom(pCmdUI); }
	void CallOnUpdateChartDateaxis( CCmdUI* pCmdUI ) { OnUpdateChartDateaxis(pCmdUI); }
	void CallOnUpdateChartLegend( CCmdUI* pCmdUI ) { OnUpdateChartLegend(pCmdUI); }
	virtual void OnUpdate( CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/ );

// ClassWizard generated virtual function overrides.
public:
	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );
	virtual void OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnEndPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual BOOL OnPreparePrinting( CPrintInfo* pInfo );
	virtual void OnPrint( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnPrepareDC( CDC* pDC, CPrintInfo* pInfo = NULL );
protected:
	virtual void OnDraw( CDC* pDC );      // overridden to draw this view

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions.
protected:
	afx_msg void OnChartTrack();
	afx_msg void OnChart2dzoom();
	afx_msg void OnChartDateaxis();
	afx_msg void OnFileExportChart();
	afx_msg void OnChartLegend();
	afx_msg void OnCheckBoxFlow();
	afx_msg void OnCheckBoxDP();
	afx_msg void OnCheckBoxT1();
	afx_msg void OnCheckBoxT2();
	afx_msg void OnCheckBoxDT();
	afx_msg void OnCheckBoxPower();
	afx_msg void OnUpdateChartTrack( CCmdUI* pCmdUI );
	afx_msg void OnUpdateChart2dzoom( CCmdUI* pCmdUI );
	afx_msg void OnUpdateChartDateaxis( CCmdUI* pCmdUI );
	afx_msg void OnUpdateChartLegend( CCmdUI* pCmdUI );
	afx_msg void OnUpdateCheckBoxFlow( CCmdUI* pCmdUI );
	afx_msg void OnUpdateCheckBoxDP( CCmdUI* pCmdUI );
	afx_msg void OnUpdateCheckBoxT1( CCmdUI* pCmdUI );
	afx_msg void OnUpdateCheckBoxT2( CCmdUI* pCmdUI );
	afx_msg void OnUpdateCheckBoxDT( CCmdUI* pCmdUI );
	afx_msg void OnUpdateCheckBoxPower( CCmdUI* pCmdUI );
	afx_msg void OnUpdateCheckBoxSymbols( CCmdUI* pCmdUI );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnEditCopy();
	afx_msg void OnChartRescale();
	afx_msg void OnChartZoomIn();
	afx_msg void OnChartZoomOut();
	afx_msg void OnChartSymbols();
	afx_msg void OnCursorToolChangeTchart( long lTool, long lX, long lY, double dXVal, double dYVal, long lSeries, long lValueIndex );
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()

	// Draw Cursor tool info line.
	int DrawCursorToolInfo( CDC* pDC, CLog *pLD, int x, int y, int iWidth = RIGHTVIEWWIDTH, bool fDrawNow = true );
	LRESULT CRViewChart::OnNewDocument( WPARAM wParam, LPARAM lParam );

protected:
	enum enum_CurPrintPos
		{
			ecppHeader,
			ecppFooter,
			ecppChartTitle,
			ecppChartInfo,
			ecppChart
		};
	CLoggedDataChart m_Chart;
	#define MAXSTATICCURSOR	7
	CStatic m_StaticCursor[MAXSTATICCURSOR];
	CLog* m_pLoggedData;
	long m_lLegendLeft, m_lLegendRight, m_lLegendTop, m_lLegendBottom;
	CBrush m_brWhiteBrush;
	bool m_fNeedScroll;
};

extern CRViewChart *pRViewChart;

#ifndef _DEBUG  // debug version in RightViewSelP.cpp
inline CTASelectDoc* CRViewChart::GetDocument()
   { return (CTASelectDoc*)m_pDocument; }
#endif