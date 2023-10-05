// editmode.cpp : implementation file
//

#include "stdafx.h"
#include "ssocx.h"
#include "editmode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditModePropPage dialog

IMPLEMENT_DYNCREATE(CEditModePropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CEditModePropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CEditModePropPage)
    // NOTE - ClassWizard will add and remove message map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

#if defined(SS_SPREADOCX)
#if defined(_UNICODE)
// {F5CA1AD4-A9D8-11CF-875E-872A37290E19}
IMPLEMENT_OLECREATE_EX(CEditModePropPage, "CEditModePropPage0.CEditModePropPage",
	0xf5ca1ad4, 0xa9d8, 0x11cf, 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19)
#else
IMPLEMENT_OLECREATE_EX(CEditModePropPage, "CEditModePropPage0.CEditModePropPage",
    0x1b52ece1, 0x8483, 0x101c, 0x93, 0x3e, 0x0, 0x0, 0xc0, 0x5, 0x95, 0x8c)
#endif
#elif defined(SS_GRIDOCX)
pragma message (__FLIE__ ": Todo: Add FPGrid ids");
#elif defined(SS_RSOCX)
// {5A276533-C536-11CF-879A-00AA00BC8F75}
IMPLEMENT_OLECREATE_EX(CEditModePropPage, "CEditModePropPage0.CEditModePropPage",
	0x5a276533, 0xc536, 0x11cf, 0x87, 0x9a, 0x0, 0xaa, 0x0, 0xbc, 0x8f, 0x75)
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditModePropPage::CEditModePropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CEditModePropPage
 
BOOL CEditModePropPage::CEditModePropPageFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Define string resource for page type; replace '0' below with ID.

    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_EDITMODE_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CEditModePropPage::CEditModePropPage - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CEditModePropPage::CEditModePropPage() :
    COlePropertyPage(IDD + LANGUAGE_BASE, IDS_EDITMODE_PPG_CAPTION + LANGUAGE_BASE)
{
    //{{AFX_DATA_INIT(CEditModePropPage)
    m_nEditEnterAction = -1;
    m_bArrowsExitEditMode = FALSE;
    m_bEditModePermanent = FALSE;
    m_bEditModeReplace = FALSE;
    m_bProcessTab = FALSE;
    //}}AFX_DATA_INIT
	SetHelpInfo(_T("Edit Mode"), _T("ssovx25.hlp"), 6500);
}


/////////////////////////////////////////////////////////////////////////////
// CEditModePropPage::DoDataExchange - Moves data between page and properties

void CEditModePropPage::DoDataExchange(CDataExchange* pDX)
{
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //{{AFX_DATA_MAP(CEditModePropPage)
    DDP_Radio(pDX, IDC_EEA_NONE, m_nEditEnterAction, _T("EditEnterAction") );
    DDX_Radio(pDX, IDC_EEA_NONE, m_nEditEnterAction);
    DDP_Check(pDX, IDC_EM_ARROWS_EXIT, m_bArrowsExitEditMode, _T("ArrowsExitEditMode") );
    DDX_Check(pDX, IDC_EM_ARROWS_EXIT, m_bArrowsExitEditMode);
    DDP_Check(pDX, IDC_EM_PERMANENT, m_bEditModePermanent, _T("EditModePermanent") );
    DDX_Check(pDX, IDC_EM_PERMANENT, m_bEditModePermanent);
    DDP_Check(pDX, IDC_EM_REPLACE, m_bEditModeReplace, _T("EditModeReplace") );
    DDX_Check(pDX, IDC_EM_REPLACE, m_bEditModeReplace);
    DDP_Check(pDX, IDC_PROCESS_TAB, m_bProcessTab, _T("ProcessTab") );
    DDX_Check(pDX, IDC_PROCESS_TAB, m_bProcessTab);
    //}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CEditModePropPage message handlers


