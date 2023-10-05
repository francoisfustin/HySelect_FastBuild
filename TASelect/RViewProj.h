#pragma once


#include "RScrollView.h"

class CRank;
class CSheetHMCalc;
class CSSheetPanelCirc2;
class CSSheetLDSumm;
class CSSheetLogData;
class CSSheetQuickMeas;
class CRViewProj : public CRScrollView
{
public:
	CRViewProj();           // protected constructor used by dynamic creation
	virtual ~CRViewProj();

	DECLARE_DYNCREATE(CRViewProj)

// Attributes
public:
	CTASelectDoc* GetDocument();
	enum { IDD = IDV_RVIEW };

	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );
	virtual void OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnEndPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual BOOL OnPreparePrinting( CPrintInfo* pInfo );
	virtual void OnPrint( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnPrepareDC( CDC* pDC, CPrintInfo* pInfo = NULL );

	// Fill a CList with all pTab objects.
	void ChildTabList(CTable *pTab, int &iKey );
	void ClearPrintingFlags();

	// pHM pointer on first Hydraulic module
	// in case of pHM == NULL print all project
	void PrintModule( CTable * pTab, bool fRecursive = false );
	void PrintRecursive( CRank *pList, bool fPrint );
	void PrintTreeView( CTable *pTab, bool fPrint, bool fShort );
	void PrintSelectedProduct( bool fPrint );
	void PrintLogDataSumm( bool fPrint );
	void PrintLogData( bool fPrint );
	void PrintQuickMeas( CRank *pList, bool fPrint );
	void SetCurrentLog( CLog *pLogData ) { m_pLoggedData = pLogData; }
	CLog* GetCurrentLog() { return m_pLoggedData; }
	void PrintPreview(){ CScrollView::OnFilePrintPreview(); }

// Implementation
protected:
	typedef struct _Pages
	{
		LPARAM lpTab;
		LPARAM lpHM;
		LPARAM Sheet;
		LPARAM PageIndex;
		bool   fnewpage;
		bool	bDelayed;
	};

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

	// Generated message map functions
	DECLARE_MESSAGE_MAP()

	LRESULT OnNewDocument( WPARAM wParam, LPARAM lParam );
	CSize GetTextSize(CDC *pDC,CString str);
	
	// Draw main module info 
	int DrawHMInfo( CDC* pDC, CDS_HydroMod *pHM, int iXPos, int iYPos, int iWidth = RIGHTVIEWWIDTH, bool fDrawNow = true ); 
	CRect DrawHMNode( CDC *pDC, CDS_HydroMod *pHM, CPoint CurXY, bool fDraw = true );
	int PrintEachHMNode(CDC *pDC,CTable *pTab,CPoint &CurXY, CPoint ConnXY,CTable *pCurHM,bool bDraw=true,bool bFirst=false);
	int PrepareHMNodePrinting(CDC* pDC, CTable *pTab, int &iPage,CPoint &CurXY, CArray<CTable *> *pArReportedHM, bool bFirst = false);
	int HMNodePrinting(CDC* pDC, int iPage);

	CString FormatPipeAccessoryStr( CPipes *pPipe );

	// PrintPreview function created to solve spread preview printing
	// Param 'fAllSheetInOnePage' -> See 'OwnerPrintPreviewDraw' method in 'SSheet.h' file file for comments.
	bool PrintPreview( CDC *pDC, CSSheet *pSSheet, CRect rcSpread, CRViewProj::_Pages *pPage, bool fAllSheetInOnePage = false );
	
	// Dedicated OnPreparePrinting functions
	BOOL OPP_TreeView( CPrintInfo* pInfo );
	BOOL OPP_HMCalc( CPrintInfo* pInfo );
	BOOL OPP_Circuit( CPrintInfo* pInfo );
	BOOL OPP_SelectedProduct( CPrintInfo* pInfo );
	BOOL OPP_LogDataSumm( CPrintInfo* pInfo );
	BOOL OPP_LogData( CPrintInfo* pInfo );
	BOOL OPP_QuickMeas( CPrintInfo* pInfo );
	
	// Dedicated OnBeginPrinting functions
	void OBP_TreeView( CDC* pDC, CPrintInfo* pInfo );
	void OBP_HMCalc( CDC* pDC, CPrintInfo* pInfo );
	void OBP_Circuit( CDC* pDC, CPrintInfo* pInfo );
	void OBP_SelectedProduct( CDC* pDC, CPrintInfo* pInfo );
	void OBP_LogDataSumm( CDC* pDC, CPrintInfo* pInfo );
	void OBP_LogData( CDC* pDC, CPrintInfo* pInfo );
	void OBP_QuickMeas( CDC* pDC, CPrintInfo* pInfo );
	
	// Dedicated OnPrint functions
	void OP_TreeView( CDC* pDC, CPrintInfo* pInfo );
	void OP_HMCalc( CDC* pDC, CPrintInfo* pInfo );
	void OP_Circuit( CDC* pDC, CPrintInfo* pInfo );
	void OP_SelectedProduct( CDC* pDC, CPrintInfo* pInfo );
	void OP_LogDataSumm( CDC* pDC, CPrintInfo* pInfo );
	void OP_LogData( CDC* pDC, CPrintInfo* pInfo );
	void OP_QuickMeas( CDC* pDC, CPrintInfo* pInfo );
	
	// Dedicated OnEndPrinting functions
	void OEP_TreeView( CDC* pDC, CPrintInfo* pInfo );
	void OEP_HMCalc( CDC* pDC, CPrintInfo* pInfo );
	void OEP_Circuit( CDC* pDC, CPrintInfo* pInfo );
	void OEP_SelectedProduct( CDC* pDC, CPrintInfo* pInfo );
	void OEP_LogDataSumm( CDC* pDC, CPrintInfo* pInfo );	
	void OEP_LogData( CDC* pDC, CPrintInfo* pInfo );	
	void OEP_QuickMeas( CDC* pDC, CPrintInfo* pInfo );	

// Public variables
public:
	bool m_fPrintRecursive;
	bool m_fPrintTreeView;
	bool m_fPrintCircuit;
	bool m_fPrintSelProd;
	bool m_fPrintLogDataSumm;
	bool m_fPrintLogData;
	bool m_fPrintQuickMeas;

// Protected variables
protected:
	enum enum_CurPrintPos
	{
		ecppHeader,
		ecppFooter,
		ecppTitle,
		ecppHMInfo,
		ecppRows,
		ecppNextModule
	};
	CSheetHMCalc*		m_pSheetHMCalc;
	CSSheetPanelCirc2	m_SheetPC2;
	CSSheetLDSumm		m_SheetLDSumm;
	CSSheetLogData		m_SheetLogData;
	CSSheetQuickMeas	m_SheetQuickMeas;
	CLog*				m_pLoggedData;

	CTable				*m_pTab;
	CDS_HydroMod		*m_pHM,*m_pTabrecursive,*m_pHMItemCut;
	CString				m_HMID;
	CRect				m_SheetProjRect;
	CRect				m_PageRect;
	CRect				m_BmpRect;
	CRank				m_RankList;
	bool				m_fRClickOnSelectedLine;

//#define DEBUG_PRINTING
#ifdef DEBUG_PRINTING
	// Variables for Preview printing
	bool				m_fPrintPreviewMode;
	HDC					m_hDCMemory;
	CRect				m_recPrinterDP, m_recScreenDP;
	HBITMAP				m_hBitmap;
	HBITMAP				m_hBitmapOld;
#endif

	CArray <_Pages> m_Pagearray, m_aCurrentHM;
	CArray <LPARAM> m_aDelayedHM;
	int m_iPage;
	bool m_fShort;

	typedef struct sHMPage
	{
		CDS_HydroMod *pHM;
		CPoint	XY;
		bool	bLastSibling;
		bool	bContentReported;
		int		iRepPage;
	};
	typedef struct ArHMPage
	{
		CArray<sHMPage> ArHM;
		CArray<bool> ArIndent;
	};
	ArHMPage *m_pHMPage;
	typedef CMap<int, int , ArHMPage* , ArHMPage* > mapArHMPages;
	CArray< ArHMPage*,ArHMPage* > m_mapArHMPages;
	CMap< CData*, CData*, int, int > m_mapCrossRefReportedHM;
	int m_node, m_page;
	LPARAM m_pCurHM;
	CRect m_TreePrintRect;
	bool m_bTreePrintPreview;
	int m_iTabX;
	int m_iTabY;
	int m_iMm;
};

extern CRViewProj *pRViewProj;

#ifndef _DEBUG  // debug version in RViewSelP.cpp
inline CTASelectDoc* CRViewProj::GetDocument()
   { return (CTASelectDoc*)m_pDocument; }
#endif
