// ssocxppg.cpp : Implementation of the CSsocxPropPage property page class.

#include "stdafx.h"
#include "ssocx.h"
#include "ssocxppg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CSsocxPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSsocxPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CSsocxPropPage)
    // NOTE - ClassWizard will add and remove message map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

#if defined(SS_SPREADOCX)
#ifdef _UNICODE
IMPLEMENT_OLECREATE_EX(CSsocxPropPage, "FPSpread.SsocxPropPage.1",
	0xf5ca1ac4, 0xa9d8, 0x11cf, 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19)
#else
IMPLEMENT_OLECREATE_EX(CSsocxPropPage, "FPSpread.SsocxPropPage.1",
    0xc75c4fe5, 0x848d, 0x11ce, 0xaf, 0x28, 0x86, 0x1b, 0xf4, 0x69, 0x9, 0xcc)
#endif
#elif defined(SS_GRIDOCX)
#pragma message(__FILE__ ": Todo: Add FPGrid ids")
#elif defined(SS_RSOCX)
IMPLEMENT_OLECREATE_EX(CSsocxPropPage, "RSSpread.SsocxPropPage.1",
	0x5a276524, 0xc536, 0x11cf, 0x87, 0x9a, 0, 0xaa, 0, 0xbc, 0x8f, 0x75)
#endif

/////////////////////////////////////////////////////////////////////////////
// CSsocxPropPage::CSsocxPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CSsocxPropPage

BOOL CSsocxPropPage::CSsocxPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_SSOCX_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CSsocxPropPage::CSsocxPropPage - Constructor

CSsocxPropPage::CSsocxPropPage() :
    COlePropertyPage(IDD + LANGUAGE_BASE, IDS_SSOCX_PPG_CAPTION + LANGUAGE_BASE)
{
    //{{AFX_DATA_INIT(CSsocxPropPage)
    m_bAllowCellOverflow = FALSE;
    m_bAllowDragDrop = FALSE;
    m_bAllowUserFormulas = FALSE;
    m_bAllowMultiBlocks = FALSE;
    m_bAutoCalc = FALSE;
    m_bAutoClipboard = FALSE;
    m_bFormulaSync = FALSE;
    m_bMoveActiveOnFocus = FALSE;
    //}}AFX_DATA_INIT
	SetHelpInfo(_T("General"), _T("ssovx25.hlp"), 6100);
}


/////////////////////////////////////////////////////////////////////////////
// CSsocxPropPage::DoDataExchange - Moves data between page and properties

void CSsocxPropPage::DoDataExchange(CDataExchange* pDX)
{
    //{{AFX_DATA_MAP(CSsocxPropPage)
    DDP_Check(pDX, IDC_ALLOW_CELL_OVERFLOW, m_bAllowCellOverflow, _T("AllowCellOverflow") );
    DDX_Check(pDX, IDC_ALLOW_CELL_OVERFLOW, m_bAllowCellOverflow);
    DDP_Check(pDX, IDC_ALLOW_DRAG_DROP, m_bAllowDragDrop, _T("AllowDragDrop") );
    DDX_Check(pDX, IDC_ALLOW_DRAG_DROP, m_bAllowDragDrop);
    DDP_Check(pDX, IDC_ALLOW_FORMULAS, m_bAllowUserFormulas, _T("AllowUserFormulas") );
    DDX_Check(pDX, IDC_ALLOW_FORMULAS, m_bAllowUserFormulas);
    DDP_Check(pDX, IDC_ALLOW_MULTIBLOCKS, m_bAllowMultiBlocks, _T("AllowMultiBlocks") );
    DDX_Check(pDX, IDC_ALLOW_MULTIBLOCKS, m_bAllowMultiBlocks);
    DDP_Check(pDX, IDC_AUTOCALC, m_bAutoCalc, _T("AutoCalc") );
    DDX_Check(pDX, IDC_AUTOCALC, m_bAutoCalc);
    DDP_Check(pDX, IDC_AUTOCLIPBOARD, m_bAutoClipboard, _T("AutoClipboard") );
    DDX_Check(pDX, IDC_AUTOCLIPBOARD, m_bAutoClipboard);
    DDP_Check(pDX, IDC_FORMULASYNC, m_bFormulaSync, _T("FormulaSync") );
    DDX_Check(pDX, IDC_FORMULASYNC, m_bFormulaSync);
    DDP_Check(pDX, IDC_MOVE_ON_FOCUS, m_bMoveActiveOnFocus, _T("MoveActiveOnFocus") );
    DDX_Check(pDX, IDC_MOVE_ON_FOCUS, m_bMoveActiveOnFocus);
    //}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CSsocxPropPage message handlers
