// headersp.cpp : implementation file
//

#include "stdafx.h"
#include "ssocx.h"
#include "headersp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHeadersPropPage dialog

IMPLEMENT_DYNCREATE(CHeadersPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CHeadersPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CHeadersPropPage)
    // NOTE - ClassWizard will add and remove message map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

#if defined(SS_SPREADOCX)
#if defined(_UNICODE)
// {F5CA1AD1-A9D8-11CF-875E-872A37290E19}
IMPLEMENT_OLECREATE_EX(CHeadersPropPage, "CHeadersPropPage0.CHeadersPropPage",
	0xf5ca1ad1, 0xa9d8, 0x11cf, 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19)
#else
IMPLEMENT_OLECREATE_EX(CHeadersPropPage, "CHeadersPropPage0.CHeadersPropPage",
    0x1b52ece2, 0x8483, 0x101c, 0x93, 0x3e, 0x0, 0x0, 0xc0, 0x5, 0x95, 0x8c)
#endif
#elif defined(SS_GRIDOCX)
#pragma message(__FILE__ ": Todo : Add FPGrid ids")
#elif defined(SS_RSOCX)
// {5A27652F-C536-11CF-879A-00AA00BC8F75}
IMPLEMENT_OLECREATE_EX(CHeadersPropPage, "CHeadersPropPage0.CHeadersPropPage",
	0x5a27652f, 0xc536, 0x11cf, 0x87, 0x9a, 0x0, 0xaa, 0x0, 0xbc, 0x8f, 0x75)
#endif

/////////////////////////////////////////////////////////////////////////////
// CHeadersPropPage::CHeadersPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CHeadersPropPage
 
BOOL CHeadersPropPage::CHeadersPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Define string resource for page type; replace '0' below with ID.

    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_HEADERS_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CHeadersPropPage::CHeadersPropPage - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CHeadersPropPage::CHeadersPropPage() :
    COlePropertyPage(IDD + LANGUAGE_BASE, IDS_HEADERS_PPG_CAPTION + LANGUAGE_BASE)
{
    //{{AFX_DATA_INIT(CHeadersPropPage)
    m_lFrozenCols = 0;
    m_lFrozenRows = 0;
    m_lMaxCols = 0;
    m_lMaxRows = 0;
    m_bRestrictCols = FALSE;
    m_bRestrictRows = FALSE;
    m_nColHeaderDisplay = -1;
    m_nRowHeaderDisplay = -1;
    m_lStartingCol = 0;
    m_lStartingRow = 0;
    m_lVisibleCols = 0;
    m_lVisibleRows = 0;
    m_bDisplayColHeaders = FALSE;
    m_bDisplayRowHeaders = FALSE;
    //}}AFX_DATA_INIT
	SetHelpInfo(_T("Headers"), _T("ssovx25.hlp"), 6400);
}


/////////////////////////////////////////////////////////////////////////////
// CHeadersPropPage::DoDataExchange - Moves data between page and properties

void CHeadersPropPage::DoDataExchange(CDataExchange* pDX)
{
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //{{AFX_DATA_MAP(CHeadersPropPage)
    DDP_Text(pDX, IDC_FROZEN_COLS, m_lFrozenCols, _T("ColsFrozen") );
    DDX_Text(pDX, IDC_FROZEN_COLS, m_lFrozenCols);
    DDP_Text(pDX, IDC_FROZEN_ROWS, m_lFrozenRows, _T("RowsFrozen") );
    DDX_Text(pDX, IDC_FROZEN_ROWS, m_lFrozenRows);
    DDP_Text(pDX, IDC_MAXCOLS, m_lMaxCols, _T("MaxCols") );
    DDX_Text(pDX, IDC_MAXCOLS, m_lMaxCols);
    DDP_Text(pDX, IDC_MAXROWS, m_lMaxRows, _T("MaxRows") );
    DDX_Text(pDX, IDC_MAXROWS, m_lMaxRows);
    DDP_Check(pDX, IDC_RESTRICT_COLS, m_bRestrictCols, _T("RestrictCols") );
    DDX_Check(pDX, IDC_RESTRICT_COLS, m_bRestrictCols);
    DDP_Check(pDX, IDC_RESTRICT_ROWS, m_bRestrictRows, _T("RestrictRows") );
    DDX_Check(pDX, IDC_RESTRICT_ROWS, m_bRestrictRows);
    DDP_CBIndex(pDX, IDC_COL_HEADER_DISPLAY, m_nColHeaderDisplay, _T("ColHeaderDisplay") );
    DDX_CBIndex(pDX, IDC_COL_HEADER_DISPLAY, m_nColHeaderDisplay);
    DDP_CBIndex(pDX, IDC_ROW_HEADER_DISPLAY, m_nRowHeaderDisplay, _T("RowHeaderDisplay") );
    DDX_CBIndex(pDX, IDC_ROW_HEADER_DISPLAY, m_nRowHeaderDisplay);
    DDP_Text(pDX, IDC_STARTING_COL, m_lStartingCol, _T("StartingColNumber") );
    DDX_Text(pDX, IDC_STARTING_COL, m_lStartingCol);
    DDP_Text(pDX, IDC_STARTING_ROW, m_lStartingRow, _T("StartingRowNumber") );
    DDX_Text(pDX, IDC_STARTING_ROW, m_lStartingRow);
    DDP_Text(pDX, IDC_VISIBLE_COLS, m_lVisibleCols, _T("VisibleCols") );
    DDX_Text(pDX, IDC_VISIBLE_COLS, m_lVisibleCols);
    DDP_Text(pDX, IDC_VISIBLE_ROWS, m_lVisibleRows, _T("VisibleRows") );
    DDX_Text(pDX, IDC_VISIBLE_ROWS, m_lVisibleRows);
    DDP_Check(pDX, IDC_DISPLAY_COL_HEADERS, m_bDisplayColHeaders, _T("DisplayColHeaders") );
    DDX_Check(pDX, IDC_DISPLAY_COL_HEADERS, m_bDisplayColHeaders);
    DDP_Check(pDX, IDC_DISPLAY_ROW_HEADERS, m_bDisplayRowHeaders, _T("DisplayRowHeaders") );
    DDX_Check(pDX, IDC_DISPLAY_ROW_HEADERS, m_bDisplayRowHeaders);
    //}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);                                    

#ifdef	SPREAD_JPN
	// Wei Feng for Incident 214
	if(!pDX->m_bSaveAndValidate)
	{
		GetDlgItem(IDC_FROZEN_COLS)->UpdateWindow();
		GetDlgItem(IDC_FROZEN_ROWS)->UpdateWindow();
		GetDlgItem(IDC_MAXCOLS)->UpdateWindow();
		GetDlgItem(IDC_MAXROWS)->UpdateWindow();
		GetDlgItem(IDC_STARTING_COL)->UpdateWindow();
		GetDlgItem(IDC_STARTING_ROW)->UpdateWindow();
		GetDlgItem(IDC_VISIBLE_COLS)->UpdateWindow();
		GetDlgItem(IDC_VISIBLE_ROWS)->UpdateWindow();
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
// CHeadersPropPage message handlers


