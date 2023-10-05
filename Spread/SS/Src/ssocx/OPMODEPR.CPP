// opmodepr.cpp : implementation file
//

#include "stdafx.h"
#include "ssocx.h"
#include "opmodepr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpModePropPage dialog

IMPLEMENT_DYNCREATE(COpModePropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(COpModePropPage, COlePropertyPage)
  //{{AFX_MSG_MAP(COpModePropPage)
  ON_BN_CLICKED(IDC_NORMAL, OnNormal)
  ON_BN_CLICKED(IDC_OPMODE_EXTEND, OnOpmodeExtend)
  ON_BN_CLICKED(IDC_OPMODE_MULTI, OnOpmodeMulti)
  ON_BN_CLICKED(IDC_OPMODE_READONLY, OnOpmodeReadonly)
  ON_BN_CLICKED(IDC_OPMODE_ROW, OnOpmodeRow)
  ON_BN_CLICKED(IDC_OPMODE_SINGLE, OnOpmodeSingle)
  ON_BN_CLICKED(IDC_SELBLKS_COLS, OnSelblksCols)
  ON_BN_CLICKED(IDC_SELBLKS_ROWS, OnSelblksRows)
  ON_BN_CLICKED(IDC_SELBLKS_BLKS, OnSelblksBlks)
  ON_BN_CLICKED(IDC_SELBLKS_ALL, OnSelblksAll)
  ON_BN_CLICKED(IDC_USER_RESIZE_COLS, OnUserResizeCols)
  ON_BN_CLICKED(IDC_USER_RESIZE_ROWS, OnUserResizeRows)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

#if defined(SS_SPREADOCX)
#if defined(_UNICODE)
// {F5CA1ACE-A9D8-11CF-875E-872A37290E19}
IMPLEMENT_OLECREATE_EX(COpModePropPage, "COpModePropPage0.COpModePropPage",
	0xf5ca1ace, 0xa9d8, 0x11cf, 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19)
#else
IMPLEMENT_OLECREATE_EX(COpModePropPage, "COpModePropPage0.COpModePropPage",
    0xc9047280, 0x848f, 0x101c, 0x93, 0x3e, 0x0, 0x0, 0xc0, 0x5, 0x95, 0x8c)
#endif
#elif defined(FP_GRIDOCX)
#pragma message (__FILE__ ": Todo : Add FPGrid ids")
#elif defined(SS_RSOCX)
// {5A27652D-C536-11CF-879A-00AA00BC8F75}
IMPLEMENT_OLECREATE_EX(COpModePropPage, "COpModePropPage0.COpModePropPage",
	0x5a27652d, 0xc536, 0x11cf, 0x87, 0x9a, 0x0, 0xaa, 0x0, 0xbc, 0x8f, 0x75)
#endif

/////////////////////////////////////////////////////////////////////////////
// COpModePropPage::COpModePropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for COpModePropPage
 
BOOL COpModePropPage::COpModePropPageFactory::UpdateRegistry(BOOL bRegister)
{
  // TODO: Define string resource for page type; replace '0' below with ID.

  if (bRegister)
    return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_OPMODE_PPG);
  else
    return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// COpModePropPage::COpModePropPage - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

COpModePropPage::COpModePropPage() :
    COlePropertyPage(IDD + LANGUAGE_BASE, IDS_OPMODE_PPG_CAPTION + LANGUAGE_BASE)
{
  //{{AFX_DATA_INIT(COpModePropPage)
  m_bAutoSize = FALSE;
  m_bNoBeep = FALSE;
  m_bNoBorder = FALSE;
  m_nOperationMode = -1;
  m_bProtect = FALSE;
  m_bRetainSelBlock = FALSE;
  m_nUnitType = -1;
  m_nUserResize = 0;
  m_nSelectBlockOptions = 0;
  //}}AFX_DATA_INIT
	SetHelpInfo(_T("Op Mode"), _T("ssovx25.hlp"), 6300);
}


/////////////////////////////////////////////////////////////////////////////
// COpModePropPage::DoDataExchange - Moves data between page and properties

void COpModePropPage::DoDataExchange(CDataExchange* pDX)
{
  // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
  //    DO NOT EDIT what you see in these blocks of generated code !
  //{{AFX_DATA_MAP(COpModePropPage)
  DDP_Check(pDX, IDC_AUTO_SIZE, m_bAutoSize, _T("AutoSize") );
  DDX_Check(pDX, IDC_AUTO_SIZE, m_bAutoSize);
  DDP_Check(pDX, IDC_NO_BEEP, m_bNoBeep, _T("NoBeep") );
  DDX_Check(pDX, IDC_NO_BEEP, m_bNoBeep);
  DDP_Check(pDX, IDC_NO_BORDER, m_bNoBorder, _T("NoBorder") );
  DDX_Check(pDX, IDC_NO_BORDER, m_bNoBorder);
  DDP_Radio(pDX, IDC_NORMAL, m_nOperationMode, _T("OperationMode") );
  DDX_Radio(pDX, IDC_NORMAL, m_nOperationMode);
  DDP_Check(pDX, IDC_PROTECT, m_bProtect, _T("Protect") );
  DDX_Check(pDX, IDC_PROTECT, m_bProtect);
  DDP_Check(pDX, IDC_RETAINSEL_BLK, m_bRetainSelBlock, _T("RetainSelBlock") );
  DDX_Check(pDX, IDC_RETAINSEL_BLK, m_bRetainSelBlock);
  DDP_Radio(pDX, IDC_UNIT_TYPE_NORM, m_nUnitType, _T("UnitType") );
  DDX_Radio(pDX, IDC_UNIT_TYPE_NORM, m_nUnitType);
  DDP_Text(pDX, IDC_USER_RESIZE, m_nUserResize, _T("UserResize") );
  DDX_Text(pDX, IDC_USER_RESIZE, m_nUserResize);
  DDP_Text(pDX, IDC_SEL_BLK_OPTS, m_nSelectBlockOptions, _T("SelectBlockOptions") );
  DDX_Text(pDX, IDC_SEL_BLK_OPTS, m_nSelectBlockOptions);
  //}}AFX_DATA_MAP
  DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// COpModePropPage message handlers



void COpModePropPage::OnNormal() 
{
  GetDlgItem(IDC_SELBLKS_ROWS)->EnableWindow(TRUE);
  GetDlgItem(IDC_SELBLKS_COLS)->EnableWindow(TRUE);
  GetDlgItem(IDC_SELBLKS_ALL)->EnableWindow(TRUE);
  GetDlgItem(IDC_SELBLKS_BLKS)->EnableWindow(TRUE);  
  CheckDlgButton(IDC_SELBLKS_ROWS,1); 
  CheckDlgButton(IDC_SELBLKS_COLS,1); 
  CheckDlgButton(IDC_SELBLKS_BLKS,1); 
  CheckDlgButton(IDC_SELBLKS_ALL,1);  
  SetDlgItemInt(IDC_SEL_BLK_OPTS,15);
}
     
void COpModePropPage::OnOpmodeExtend() 
{
  GetDlgItem(IDC_SELBLKS_ROWS)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_COLS)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_ALL)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_BLKS)->EnableWindow(FALSE);  
  CheckDlgButton(IDC_SELBLKS_ROWS,0); 
  CheckDlgButton(IDC_SELBLKS_COLS,0); 
  CheckDlgButton(IDC_SELBLKS_BLKS,0); 
  CheckDlgButton(IDC_SELBLKS_ALL,0);  
  SetDlgItemInt(IDC_SEL_BLK_OPTS,0);
}

void COpModePropPage::OnOpmodeMulti() 
{
  GetDlgItem(IDC_SELBLKS_ROWS)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_COLS)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_ALL)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_BLKS)->EnableWindow(FALSE);  
  CheckDlgButton(IDC_SELBLKS_ROWS,0); 
  CheckDlgButton(IDC_SELBLKS_COLS,0); 
  CheckDlgButton(IDC_SELBLKS_BLKS,0); 
  CheckDlgButton(IDC_SELBLKS_ALL,0);
  SetDlgItemInt(IDC_SEL_BLK_OPTS,0);  
}

void COpModePropPage::OnOpmodeReadonly() 
{
  GetDlgItem(IDC_SELBLKS_ROWS)->EnableWindow(TRUE);
  GetDlgItem(IDC_SELBLKS_COLS)->EnableWindow(TRUE);
  GetDlgItem(IDC_SELBLKS_ALL)->EnableWindow(TRUE);
  GetDlgItem(IDC_SELBLKS_BLKS)->EnableWindow(TRUE);  
  CheckDlgButton(IDC_SELBLKS_ROWS,1); 
  CheckDlgButton(IDC_SELBLKS_COLS,1); 
  CheckDlgButton(IDC_SELBLKS_BLKS,1); 
  CheckDlgButton(IDC_SELBLKS_ALL,1); 
  SetDlgItemInt(IDC_SEL_BLK_OPTS,15); 
}

void COpModePropPage::OnOpmodeRow() 
{
  GetDlgItem(IDC_SELBLKS_ROWS)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_COLS)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_ALL)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_BLKS)->EnableWindow(FALSE);  
  CheckDlgButton(IDC_SELBLKS_ROWS,0); 
  CheckDlgButton(IDC_SELBLKS_COLS,0); 
  CheckDlgButton(IDC_SELBLKS_BLKS,0); 
  CheckDlgButton(IDC_SELBLKS_ALL,0); 
  SetDlgItemInt(IDC_SEL_BLK_OPTS,0);  
}

void COpModePropPage::OnOpmodeSingle() 
{
  GetDlgItem(IDC_SELBLKS_ROWS)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_COLS)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_ALL)->EnableWindow(FALSE);
  GetDlgItem(IDC_SELBLKS_BLKS)->EnableWindow(FALSE);  
  CheckDlgButton(IDC_SELBLKS_ROWS,0); 
  CheckDlgButton(IDC_SELBLKS_COLS,0); 
  CheckDlgButton(IDC_SELBLKS_BLKS,0); 
  CheckDlgButton(IDC_SELBLKS_ALL,0);
  SetDlgItemInt(IDC_SEL_BLK_OPTS,0);  
}

void COpModePropPage::OnSelblksCols() 
{
  int sbo;
    
  sbo=GetDlgItemInt(IDC_SEL_BLK_OPTS);  
  if (IsDlgButtonChecked(IDC_SELBLKS_COLS))
    SetDlgItemInt(IDC_SEL_BLK_OPTS,sbo | 1);
  else
    SetDlgItemInt(IDC_SEL_BLK_OPTS,sbo  & 14);    
}

void COpModePropPage::OnSelblksRows() 
{
  int sbo;
    
  sbo=GetDlgItemInt(IDC_SEL_BLK_OPTS);  
  if (IsDlgButtonChecked(IDC_SELBLKS_ROWS))
    SetDlgItemInt(IDC_SEL_BLK_OPTS,sbo | 2);
  else
    SetDlgItemInt(IDC_SEL_BLK_OPTS,sbo  & 13);    
    
}

void COpModePropPage::OnSelblksBlks() 
{
  int sbo;
    
  sbo=GetDlgItemInt(IDC_SEL_BLK_OPTS);  
  if (IsDlgButtonChecked(IDC_SELBLKS_BLKS))
    SetDlgItemInt(IDC_SEL_BLK_OPTS,sbo | 4);
  else
    SetDlgItemInt(IDC_SEL_BLK_OPTS,sbo  & 11);    
    
}

void COpModePropPage::OnSelblksAll() 
{
  int sbo;
    
  sbo=GetDlgItemInt(IDC_SEL_BLK_OPTS);  
  if (IsDlgButtonChecked(IDC_SELBLKS_ALL))
    SetDlgItemInt(IDC_SEL_BLK_OPTS,sbo | 8);
  else
    SetDlgItemInt(IDC_SEL_BLK_OPTS,sbo  & 7); 
    
}

BOOL COpModePropPage::OnInitDialog() 
{
  COlePropertyPage::OnInitDialog();
    
  int sbo;
  int ur;
     
  ur=GetDlgItemInt(IDC_USER_RESIZE);
  sbo=GetDlgItemInt(IDC_SEL_BLK_OPTS);
    
  if (ur == 1 || ur == 3)
    CheckDlgButton(IDC_USER_RESIZE_COLS,1);
  if (ur >= 2)
    CheckDlgButton(IDC_USER_RESIZE_ROWS,1);
  if (sbo ==1 || sbo == 3 || sbo == 5 || sbo == 7 || sbo == 9 || sbo == 11 || sbo == 13 || sbo == 15)
    CheckDlgButton(IDC_SELBLKS_COLS,1);
  if (sbo ==2 || sbo == 6 || sbo == 3 || sbo == 7 || sbo == 10 || sbo == 11 || sbo == 14 || sbo == 15) 
    CheckDlgButton(IDC_SELBLKS_ROWS,1);
  if ((sbo>=4 && sbo <=7) || (sbo >=12)) 
    CheckDlgButton(IDC_SELBLKS_BLKS,1); 
  if (sbo >= 8)
    CheckDlgButton(IDC_SELBLKS_ALL,1);
    
  if( IsDlgButtonChecked(IDC_OPMODE_ROW) ||
      IsDlgButtonChecked(IDC_OPMODE_SINGLE) ||
      IsDlgButtonChecked(IDC_OPMODE_MULTI) ||
      IsDlgButtonChecked(IDC_OPMODE_EXTEND) )
  {
    GetDlgItem(IDC_SELBLKS_ROWS)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELBLKS_COLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELBLKS_ALL)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELBLKS_BLKS)->EnableWindow(FALSE);
  }
    
  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void COpModePropPage::OnUserResizeCols() 
{   
  int ur=GetDlgItemInt(IDC_USER_RESIZE);
    
  if (IsDlgButtonChecked(IDC_USER_RESIZE_COLS))
    SetDlgItemInt(IDC_USER_RESIZE,ur | 1);
  else
    SetDlgItemInt(IDC_USER_RESIZE, ur & 2);
}

void COpModePropPage::OnUserResizeRows() 
{
  int ur=GetDlgItemInt(IDC_USER_RESIZE);
    
  if (IsDlgButtonChecked(IDC_USER_RESIZE_ROWS))
    SetDlgItemInt(IDC_USER_RESIZE,ur | 2);
  else
    SetDlgItemInt(IDC_USER_RESIZE, ur & 1);    
}
