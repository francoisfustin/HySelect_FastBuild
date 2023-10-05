// virtualp.cpp : implementation file
//

#include "stdafx.h"
#include "ssocx.h"
#include "virtualp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVirtualPropPage dialog

IMPLEMENT_DYNCREATE(CVirtualPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CVirtualPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CVirtualPropPage)
    ON_BN_CLICKED(IDC_VIRTUAL_MODE, OnVirtualMode)
    ON_BN_CLICKED(IDC_VSCROLLSPEC, OnVscrollspec)
    ON_BN_CLICKED(IDC_VSSTYPE_HOMEEND, OnVsstypeHomeend)
    ON_BN_CLICKED(IDC_VSSTYPE_LINEUPDWN, OnVsstypeLineupdwn)
    ON_BN_CLICKED(IDC_VSSTYPE_PAGEUPDWN, OnVsstypePageupdwn)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

#if defined(SS_SPREADOCX)
#if defined(_UNICODE)
// {F5CA1AD7-A9D8-11CF-875E-872A37290E19}
IMPLEMENT_OLECREATE_EX(CVirtualPropPage, "CVirtualPropPage0.CVirtualPropPage",
  0xf5ca1ad7, 0xa9d8, 0x11cf, 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19)
#else
IMPLEMENT_OLECREATE_EX(CVirtualPropPage, "CVirtualPropPage0.CVirtualPropPage",
    0xc9047281, 0x848f, 0x101c, 0x93, 0x3e, 0x0, 0x0, 0xc0, 0x5, 0x95, 0x8c)
#endif
#elif defined(SS_FPGRIDOCX)
#pragma message(__FILE__ ": ToDo: Add FPGrid ids")
#elif defined(SS_RSOCX)
// {5A276535-C536-11CF-879A-00AA00BC8F75}
IMPLEMENT_OLECREATE_EX(CVirtualPropPage, "CVirtualPropPage0.CVirtualPropPage",
  0x5a276535, 0xc536, 0x11cf, 0x87, 0x9a, 0x0, 0xaa, 0x0, 0xbc, 0x8f, 0x75)
#endif

/////////////////////////////////////////////////////////////////////////////
// CVirtualPropPage::CVirtualPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CVirtualPropPage
 
BOOL CVirtualPropPage::CVirtualPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Define string resource for page type; replace '0' below with ID.

    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_VMODE_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CVirtualPropPage::CVirtualPropPage - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CVirtualPropPage::CVirtualPropPage() :
    COlePropertyPage(IDD + LANGUAGE_BASE, IDS_VMODE_PPG_CAPTION + LANGUAGE_BASE)
{
    //{{AFX_DATA_INIT(CVirtualPropPage)
    m_bVirtualMode = FALSE;
    m_lVMaxRows = 0;
    m_lVOverlap = 0;
    m_lVRows = 0;
    m_bVScrollBuffer = FALSE;
    m_bVScrollSpecial = FALSE;
    m_nVScrollSpecialType = 0;
    //}}AFX_DATA_INIT
  SetHelpInfo(_T("Virtual Mode"), _T("ssovx25.hlp"), 6600);
}


/////////////////////////////////////////////////////////////////////////////
// CVirtualPropPage::DoDataExchange - Moves data between page and properties

void CVirtualPropPage::DoDataExchange(CDataExchange* pDX)
{
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //{{AFX_DATA_MAP(CVirtualPropPage)
    DDP_Check(pDX, IDC_VIRTUAL_MODE, m_bVirtualMode, _T("VirtualMode") );
    DDX_Check(pDX, IDC_VIRTUAL_MODE, m_bVirtualMode);
    DDP_Text(pDX, IDC_VMAXROWS, m_lVMaxRows, _T("VirtualMaxRows") );
    DDX_Text(pDX, IDC_VMAXROWS, m_lVMaxRows);
    DDP_Text(pDX, IDC_VOVERLAP, m_lVOverlap, _T("VirtualOverlap") );
    DDX_Text(pDX, IDC_VOVERLAP, m_lVOverlap);
    DDP_Text(pDX, IDC_VROWS, m_lVRows, _T("VirtualRows") );
    DDX_Text(pDX, IDC_VROWS, m_lVRows);
    DDP_Check(pDX, IDC_VSCROLLBUFF, m_bVScrollBuffer, _T("VirtualScrollBuffer") );
    DDX_Check(pDX, IDC_VSCROLLBUFF, m_bVScrollBuffer);
    DDP_Check(pDX, IDC_VSCROLLSPEC, m_bVScrollSpecial, _T("VScrollSpecial") );
    DDX_Check(pDX, IDC_VSCROLLSPEC, m_bVScrollSpecial);
    DDP_Text(pDX, IDC_VSCROLL_SPEC_TYPE, m_nVScrollSpecialType, _T("VScrollSpecialType") );
    DDX_Text(pDX, IDC_VSCROLL_SPEC_TYPE, m_nVScrollSpecialType);
    //}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);

#ifdef	SPREAD_JPN
	// Wei feng for Incident 273
	if(!pDX->m_bSaveAndValidate)
	{
       BOOL bChecked = IsDlgButtonChecked(IDC_VIRTUAL_MODE);
	   GetDlgItem(IDC_VMAXROWS)->EnableWindow(bChecked);
       GetDlgItem(IDC_VOVERLAP)->EnableWindow(bChecked);
       GetDlgItem(IDC_VROWS)->EnableWindow(bChecked);
       GetDlgItem(IDC_VSCROLLBUFF)->EnableWindow(bChecked);
	   GetDlgItem(IDC_VMAXROWS)->UpdateWindow();
	   GetDlgItem(IDC_VOVERLAP)->UpdateWindow();
       GetDlgItem(IDC_VROWS)->UpdateWindow();
       GetDlgItem(IDC_VSCROLL_SPEC_TYPE)->UpdateWindow();

       bChecked = IsDlgButtonChecked(IDC_VSCROLLSPEC);
	   GetDlgItem(IDC_VSSTYPE_HOMEEND)->EnableWindow(bChecked);
       GetDlgItem(IDC_VSSTYPE_PAGEUPDWN)->EnableWindow(bChecked);
       GetDlgItem(IDC_VSSTYPE_LINEUPDWN)->EnableWindow(bChecked);
       GetDlgItem(IDC_VSSTYPE_HOMEEND)->UpdateWindow();
	   GetDlgItem(IDC_VSSTYPE_PAGEUPDWN)->UpdateWindow();
	   GetDlgItem(IDC_VSSTYPE_LINEUPDWN)->UpdateWindow();
	
	}
#endif

}


/////////////////////////////////////////////////////////////////////////////
// CVirtualPropPage message handlers



void CVirtualPropPage::OnVirtualMode() 
{
     BOOL b;
     b=IsDlgButtonChecked(IDC_VIRTUAL_MODE);
     GetDlgItem(IDC_VMAXROWS)->EnableWindow(b);
     GetDlgItem(IDC_VOVERLAP)->EnableWindow(b);
     GetDlgItem(IDC_VROWS)->EnableWindow(b);
     GetDlgItem(IDC_VSCROLLBUFF)->EnableWindow(b);
}

void CVirtualPropPage::OnVscrollspec() 
{
     BOOL b;
     b=IsDlgButtonChecked(IDC_VSCROLLSPEC);
     GetDlgItem(IDC_VSSTYPE_HOMEEND)->EnableWindow(b);
     GetDlgItem(IDC_VSSTYPE_PAGEUPDWN)->EnableWindow(b);
     GetDlgItem(IDC_VSSTYPE_LINEUPDWN)->EnableWindow(b); 
     if (b == FALSE)
     {   
       SetDlgItemInt(IDC_VSCROLL_SPEC_TYPE,0);
       CheckDlgButton(IDC_VSSTYPE_HOMEEND,0);
       CheckDlgButton(IDC_VSSTYPE_PAGEUPDWN,0);
       CheckDlgButton(IDC_VSSTYPE_LINEUPDWN,0);
     }
    
}

void CVirtualPropPage::OnVsstypeHomeend() 
{
    int vss = GetDlgItemInt(IDC_VSCROLL_SPEC_TYPE);
    
    if (IsDlgButtonChecked(IDC_VSSTYPE_HOMEEND))
         SetDlgItemInt(IDC_VSCROLL_SPEC_TYPE, vss | 1);
    else SetDlgItemInt(IDC_VSCROLL_SPEC_TYPE, vss & 6);
}

void CVirtualPropPage::OnVsstypeLineupdwn() 
{
    int vss = GetDlgItemInt(IDC_VSCROLL_SPEC_TYPE);
    
    if (IsDlgButtonChecked(IDC_VSSTYPE_LINEUPDWN))
         SetDlgItemInt(IDC_VSCROLL_SPEC_TYPE, vss | 4);
    else SetDlgItemInt(IDC_VSCROLL_SPEC_TYPE, vss & 3);
    
}

void CVirtualPropPage::OnVsstypePageupdwn() 
{
    int vss = GetDlgItemInt(IDC_VSCROLL_SPEC_TYPE);
    
    if (IsDlgButtonChecked(IDC_VSSTYPE_PAGEUPDWN))
         SetDlgItemInt(IDC_VSCROLL_SPEC_TYPE, vss | 2);
    else SetDlgItemInt(IDC_VSCROLL_SPEC_TYPE, vss & 5);
    
}

BOOL CVirtualPropPage::OnInitDialog() 
{
  COlePropertyPage::OnInitDialog();
  BOOL b = IsDlgButtonChecked(IDC_VIRTUAL_MODE);
  int vss = GetDlgItemInt(IDC_VSCROLL_SPEC_TYPE);

  GetDlgItem(IDC_VMAXROWS)->EnableWindow(b);
  GetDlgItem(IDC_VOVERLAP)->EnableWindow(b);
  GetDlgItem(IDC_VROWS)->EnableWindow(b);
  GetDlgItem(IDC_VSCROLLBUFF)->EnableWindow(b);
  if( vss & SS_VSCROLL_NOHOMEEND )
    CheckDlgButton(IDC_VSSTYPE_HOMEEND, 1);
  if( vss & SS_VSCROLL_NOPAGEUPDN )
    CheckDlgButton(IDC_VSSTYPE_PAGEUPDWN, 1);
  if( vss & SS_VSCROLL_NOUPDN )
    CheckDlgButton(IDC_VSSTYPE_LINEUPDWN, 1);
  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}
