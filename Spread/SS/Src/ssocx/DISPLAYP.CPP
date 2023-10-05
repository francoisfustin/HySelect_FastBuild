// displayp.cpp : implementation file
//

#include "stdafx.h"
#include "ssocx.h"
#include "displayp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDisplayPropPage dialog

IMPLEMENT_DYNCREATE(CDisplayPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CDisplayPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CDisplayPropPage)
    ON_CBN_SELENDOK(IDC_SCROLLBARS, OnSelendokScrollbars)
    ON_BN_CLICKED(IDC_BDM_CURRENT_CELL, OnBdmCurrentCell)
    ON_BN_CLICKED(IDC_BDM_CURRENT_COL, OnBdmCurrentCol)
    ON_BN_CLICKED(IDC_BDM_CURRENT_ROW, OnBdmCurrentRow)
    ON_BN_CLICKED(IDC_BDM_ALWAYS, OnBdmAlways)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

#if defined(SS_SPREADOCX)
#if defined(_UNICODE)
// {F5CA1AC9-A9D8-11CF-875E-872A37290E19}
IMPLEMENT_OLECREATE_EX(CDisplayPropPage, "CDisplayPropPage0.CDisplayPropPage",
	0xf5ca1ac9, 0xa9d8, 0x11cf, 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19)
#else
IMPLEMENT_OLECREATE_EX(CDisplayPropPage, "CDisplayPropPage0.CDisplayPropPage",
    0x1b52ece0, 0x8483, 0x101c, 0x93, 0x3e, 0x0, 0x0, 0xc0, 0x5, 0x95, 0x8c)
#endif
#elif defined(SS_GRIDOCX)
#pragma message(__FILE__ ": ToDo : Add FPGrid ids")
#elif defined(SS_RSOCX)
// {5A276529-C536-11CF-879A-00AA00BC8F75}
IMPLEMENT_OLECREATE_EX(CDisplayPropPage, "CDisplayPropPage0.CDisplayPropPage",
	0x5a276529, 0xc536, 0x11cf, 0x87, 0x9a, 0x0, 0xaa, 0x0, 0xbc, 0x8f, 0x75)
#endif

/////////////////////////////////////////////////////////////////////////////
// CDisplayPropPage::CDisplayPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CDisplayPropPage
 
BOOL CDisplayPropPage::CDisplayPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Define string resource for page type; replace '0' below with ID.

    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_DISPLAY_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CDisplayPropPage::CDisplayPropPage - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CDisplayPropPage::CDisplayPropPage() :
    COlePropertyPage(IDD + LANGUAGE_BASE, IDS_DISPLAY_PPG_CAPTION + LANGUAGE_BASE)
{
    //{{AFX_DATA_INIT(CDisplayPropPage)
    m_bGridShowHoriz = FALSE;
    m_bGridShowVert = FALSE;
    m_bGridSolid = FALSE;
    m_bRedraw = FALSE;  
    m_bScrollBarExt = FALSE;
    m_bScrollBarMaxAlign = FALSE;
    m_bScrollBarShowMax = FALSE;
    m_nScrollBars = -1;                                            //Both Vertical and Horizontal Scroll
    m_nButtonDrawMode = 0;
	m_bkColorStyle = -1;
	//}}AFX_DATA_INIT
	SetHelpInfo(_T("Display"), _T("ssovx25.hlp"), 6200);
}


/////////////////////////////////////////////////////////////////////////////
// CDisplayPropPage::DoDataExchange - Moves data between page and properties

void CDisplayPropPage::DoDataExchange(CDataExchange* pDX)
{
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //{{AFX_DATA_MAP(CDisplayPropPage)
	DDP_Check(pDX, IDC_GRIDSHOWHORIZ, m_bGridShowHoriz, _T("GridShowHoriz") );
	DDX_Check(pDX, IDC_GRIDSHOWHORIZ, m_bGridShowHoriz);
	DDP_Check(pDX, IDC_GRIDSHOWVERT, m_bGridShowVert, _T("GridShowVert") );
	DDX_Check(pDX, IDC_GRIDSHOWVERT, m_bGridShowVert);
	DDP_Check(pDX, IDC_GRIDSOLID, m_bGridSolid, _T("GridSolid") );
	DDX_Check(pDX, IDC_GRIDSOLID, m_bGridSolid);
	DDP_Check(pDX, IDC_REDRAW, m_bRedraw, _T("ReDraw") );
	DDX_Check(pDX, IDC_REDRAW, m_bRedraw);
	DDP_Check(pDX, IDC_SCROLLBAREXTMODE, m_bScrollBarExt, _T("ScrollBarExtMode") );
	DDX_Check(pDX, IDC_SCROLLBAREXTMODE, m_bScrollBarExt);
	DDP_Check(pDX, IDC_SCROLLBARMAXALIGN, m_bScrollBarMaxAlign, _T("ScrollBarMaxAlign") );
	DDX_Check(pDX, IDC_SCROLLBARMAXALIGN, m_bScrollBarMaxAlign);
	DDP_Check(pDX, IDC_SCROLLBARSHOWMAX, m_bScrollBarShowMax, _T("ScrollBarShowMax") );
	DDX_Check(pDX, IDC_SCROLLBARSHOWMAX, m_bScrollBarShowMax);
	DDP_CBIndex(pDX, IDC_SCROLLBARS, m_nScrollBars, _T("ScrollBars") );
	DDX_CBIndex(pDX, IDC_SCROLLBARS, m_nScrollBars);
	DDP_Text(pDX, IDC_BDM, m_nButtonDrawMode, _T("ButtonDrawMode") );
	DDX_Text(pDX, IDC_BDM, m_nButtonDrawMode);
	DDP_CBIndex(pDX, IDC_BKCOLORSTYLE, m_bkColorStyle, _T("BackColorStyle") );
	DDX_CBIndex(pDX, IDC_BKCOLORSTYLE, m_bkColorStyle);
	//}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CDisplayPropPage message handlers


void CDisplayPropPage::OnSelendokScrollbars() 
{        
   if (((CComboBox*)GetDlgItem(IDC_SCROLLBARS))->GetCurSel() == 0)
   { 
    GetDlgItem(IDC_SCROLLBARSHOWMAX)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCROLLBARMAXALIGN)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCROLLBAREXTMODE)->EnableWindow(FALSE);   
    }
    else
    { 
      GetDlgItem(IDC_SCROLLBARSHOWMAX)->EnableWindow(TRUE);
      GetDlgItem(IDC_SCROLLBARMAXALIGN)->EnableWindow(TRUE);
      GetDlgItem(IDC_SCROLLBAREXTMODE)->EnableWindow(TRUE);  
    }
}
BOOL CDisplayPropPage::OnInitDialog() 
{
    COlePropertyPage::OnInitDialog();
    
    int bdm;
    
    bdm=GetDlgItemInt(IDC_BDM);
    if (bdm == 0) CheckDlgButton(IDC_BDM_ALWAYS,1);
    if (bdm ==1 || bdm == 3 || bdm == 5 || bdm == 7) CheckDlgButton(IDC_BDM_CURRENT_CELL,1);
    if (bdm ==2 || bdm == 6 || bdm == 3 || bdm == 7) CheckDlgButton(IDC_BDM_CURRENT_COL,1);
    if (bdm >= 4) CheckDlgButton(IDC_BDM_CURRENT_ROW,1);
    if (m_nScrollBars == 0) 
    {
      GetDlgItem(IDC_SCROLLBAREXTMODE)->EnableWindow(FALSE);
      GetDlgItem(IDC_SCROLLBARSHOWMAX)->EnableWindow(FALSE);
      GetDlgItem(IDC_SCROLLBARMAXALIGN)->EnableWindow(FALSE);
    
    }
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDisplayPropPage::OnBdmCurrentCell() 
{
    int bdm;
    
    bdm=GetDlgItemInt(IDC_BDM);  
    if (IsDlgButtonChecked(IDC_BDM_CURRENT_CELL)) 
    {
           SetDlgItemInt(IDC_BDM,bdm | 1);
           CheckDlgButton(IDC_BDM_ALWAYS,0);
     }
    else  SetDlgItemInt(IDC_BDM,bdm & 6);
}

void CDisplayPropPage::OnBdmCurrentCol() 
{
    int bdm;
    
    bdm=GetDlgItemInt(IDC_BDM);  
    if (IsDlgButtonChecked(IDC_BDM_CURRENT_COL))  
    {
           SetDlgItemInt(IDC_BDM,bdm | 2);
           CheckDlgButton(IDC_BDM_ALWAYS,0);
        }
    else  SetDlgItemInt(IDC_BDM,bdm & 5);
}

void CDisplayPropPage::OnBdmCurrentRow() 
{
    int bdm;
    

    bdm=GetDlgItemInt(IDC_BDM);  
    if (IsDlgButtonChecked(IDC_BDM_CURRENT_ROW))
        {
           SetDlgItemInt(IDC_BDM,bdm | 4);
           CheckDlgButton(IDC_BDM_ALWAYS,0);
           }
    else  SetDlgItemInt(IDC_BDM,bdm & 3);
}

void CDisplayPropPage::OnBdmAlways() 
{
    if(IsDlgButtonChecked(IDC_BDM_ALWAYS))
    { 
      SetDlgItemInt(IDC_BDM,0);
      CheckDlgButton(IDC_BDM_CURRENT_CELL,0);
      CheckDlgButton(IDC_BDM_CURRENT_ROW,0);
      CheckDlgButton(IDC_BDM_CURRENT_COL,0);
    }
    
}
