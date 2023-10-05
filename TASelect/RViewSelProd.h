#pragma once


#include "EnBitmapRow.h"
#include "DrawSet.h"
#include "FormViewEx.h"

class CRViewSelProd : public CFormViewEx , protected CDrawSet
{
	DECLARE_DYNCREATE( CRViewSelProd )
public:
	enum { IDD = IDV_RVIEW };
	
	CRViewSelProd();
	virtual ~CRViewSelProd();

	// These methods is called by 'CDlgLeftTabSelP' when user enter or quit current tab.
	void OnEnterTab( bool bComeFromProductSelTab );
	void OnLeaveTab( void );

	void RedrawRightView( bool bResetOrder = false, bool bPrint = false, bool bExport = false, bool bComeFromProductSelTab = false );
	
	CContainerForPrint *GetContainerToPrint( void );
	
	bool GoToPage( CDB_PageSetup::enCheck ePage );

	afx_msg void OnFileExportSelp( CString strFn = _T("") );
	afx_msg void OnFileExportSelpD82( bool bIsD82 = true );
	afx_msg void OnFileExportSelInOneSheet( CString strFn = _T("") );
	// HYS-1605: Export TA-Diagnostic and measurements
	afx_msg void OnFileExportTADiagnostic( CString strFn = _T("") );

	void WriteGaebPCounter( int countDigit, char *pcCounter );
	void WriteGaebCounter( int count, int countDigit, char *pcCounter );
	void WriteGaebLine21( FILE *fp, const char *pcCount, int iQuantity, int *piLineNum );
	int WriteGaebLine22( FILE *fp, const char *pcCount, double dPrice, int iQuantity, int *piLineNum );
	void WriteGaebLine25( FILE *fp, char *pstrUtf8, int *piLineNum );
	void WriteGaebLine26( FILE *fp, const wchar_t *pstrUtf8, int *piLineNum );
	void WriteGaebLine99( FILE *fp, int iTotalPrice, int iCount, int *piLineNum );
	// Public variables.
public:
	CTADatabase	*m_pTADB;

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );    // DDX/DDV support
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	LRESULT OnMainFrameResize( WPARAM wParam, LPARAM lParam );

// Protected variables.
protected:
	CSelProdDockPane m_SelProdDockPane;
	CContainerForExcelExport *m_pclContainerForExport;
	CContainerForPrint *m_pclContainerForPrint;
	WORD m_arwTemplate[1024];

// Private methods.
private:
	void _InitSelProdDockPane( void );
};

extern CRViewSelProd *pRViewSelProd;
