// DlgHeader.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "Global.h"
#include "Units.h"
#include "utilities.h"
#include "DataBObj.h"

#include "DlgHeader.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgHeader dialog


CDlgHeader::CDlgHeader(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgHeader::IDD, pParent)
{
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUSERDB = NULL;
	m_pUnitDB = NULL;
	m_pPageSetup = NULL;
	m_pPageSetupBak = NULL;
	ZeroMemory( &m_lf, _PAGESETUP_ENUM * sizeof( LOGFONT ) );
	m_bModified = 0;
	m_iUnit = 0;
}

void CDlgHeader::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIOHEADERRIGHT, m_RadioHeaderRight);
	DDX_Control(pDX, IDC_RADIOHEADERLEFT, m_RadioHeaderLeft);
	DDX_Control(pDX, IDC_RADIOHEADERCENTERED, m_RadioHeaderCentered);
	DDX_Control(pDX, IDC_RADIOFOOTERRIGHT, m_RadioFooterRight);
	DDX_Control(pDX, IDC_RADIOFOOTERLEFT, m_RadioFooterLeft);
	DDX_Control(pDX, IDC_RADIOFOOTERCENTERED, m_RadioFooterCentered);
	DDX_Control(pDX, IDC_BUTTONFONTFOOTER, m_ButtonFontFooter);
	DDX_Control(pDX, IDC_BUTTONFONTHEADER, m_ButtonFontHeader);
	DDX_Control(pDX, IDC_EDITHEADER, m_EditHeader);
	DDX_Control(pDX, IDC_EDITFOOTER, m_EditFooter);
	DDX_Control(pDX, IDC_RADIO1STPAGE, m_Radio1StPage);
	DDX_Control(pDX, IDC_RADIOALLPAGES, m_RadioAllPages);
	DDX_Control(pDX, IDC_COMBOUNIT, m_ComboUnit);
	DDX_Control(pDX, IDC_STATICUNIT, m_StaticUnit);
	DDX_Control(pDX, IDC_STATICLOGO1, m_StaticLogo1);
	DDX_Control(pDX, IDC_STATICCUSTOLOGO, m_StaticCustoLogo);
	DDX_Control(pDX, IDC_EDITTOPMARGIN, m_EditTopMargin);
	DDX_Control(pDX, IDC_EDITBOTMARGIN, m_EditBotMargin);
	DDX_Control(pDX, IDC_CHECKLOGO, m_CheckLogo);
	DDX_Control(pDX, IDC_CHECKCUSTOLOGO, m_CheckCustoLogo);
	DDX_Control(pDX, IDC_CHECK1STPAGE, m_Check1StPage);
	DDX_Control(pDX, IDC_STATICHEADER, m_GroupHeader);
	DDX_Control(pDX, IDC_STATICFOOTER, m_GroupFooter);
}


BEGIN_MESSAGE_MAP(CDlgHeader, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO1STPAGE, OnRadio1stpage)
	ON_BN_CLICKED(IDC_RADIOALLPAGES, OnRadioallpages)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK1STPAGE, OnCheck1stpage)
	ON_CBN_SELCHANGE(IDC_COMBOUNIT, OnSelchangeCombounit)
	ON_EN_CHANGE(IDC_EDITTOPMARGIN, OnChangeEdittopmargin)
	ON_EN_CHANGE(IDC_EDITBOTMARGIN, OnChangeEditbotmargin)
	ON_EN_KILLFOCUS(IDC_EDITTOPMARGIN, OnKillfocusEdittopmargin)
	ON_EN_KILLFOCUS(IDC_EDITBOTMARGIN, OnKillfocusEditbotmargin)
	ON_BN_CLICKED(IDC_CHECKLOGO, OnChecklogo)
	ON_BN_CLICKED(IDC_CHECKCUSTOLOGO, OnCheckCustologo)
	ON_BN_CLICKED(IDC_BUTTONFONTFOOTER, OnButtonfontfooter)
	ON_BN_CLICKED(IDC_BUTTONFONTHEADER, OnButtonfontheader)
	ON_EN_CHANGE( IDC_EDITFOOTER, OnChangeEditfooter )
	ON_EN_KILLFOCUS(IDC_EDITFOOTER, OnKillfocusEditfooter)
	ON_EN_CHANGE( IDC_EDITHEADER, OnChangeEditheader )
	ON_EN_KILLFOCUS(IDC_EDITHEADER, OnKillfocusEditheader)
	ON_BN_CLICKED(IDC_RADIOFOOTERCENTERED, OnRadiofootercentered)
	ON_BN_CLICKED(IDC_RADIOFOOTERLEFT, OnRadiofooterleft)
	ON_BN_CLICKED(IDC_RADIOFOOTERRIGHT, OnRadiofooterright)
	ON_BN_CLICKED(IDC_RADIOHEADERCENTERED, OnRadioheadercentered)
	ON_BN_CLICKED(IDC_RADIOHEADERLEFT, OnRadioheaderleft)
	ON_BN_CLICKED(IDC_RADIOHEADERRIGHT, OnRadioheaderright)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTONSETUSERREF, OnButtonsetuserref)
	ON_WM_DESTROY()
END_MESSAGE_MAP()
	// Retrieve last selected Page Setup from TADS

/////////////////////////////////////////////////////////////////////////////
// 
int CDlgHeader::Display(CDB_PageSetup* pPageSetup)
{

	m_pPageSetup = pPageSetup;
	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();
	m_pUSERDB = TASApp.GetpUserDB();
	return DoModal();

}
/////////////////////////////////////////////////////////////////////////////
// CDlgHeader message handlers

BOOL CDlgHeader::OnInitDialog() 
{
	try
	{
		CDialogEx::OnInitDialog();

		// Initialize dialog strings.
		CString str = TASApp.LoadLocalizedString( IDS_DLGHEADER_CAPTION );
		SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_CHECK1STPAGE );
		GetDlgItem( IDC_CHECK1STPAGE )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_RADIO1STPAGE );
		GetDlgItem( IDC_RADIO1STPAGE )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_RADIOALLPAGES );
		GetDlgItem( IDC_RADIOALLPAGES )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_CHECKLOGO );
		GetDlgItem( IDC_CHECKLOGO )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_STATICHEADER );
		m_GroupHeader.SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_STATICFOOTER );
		m_GroupFooter.SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_STATICTOPMARGIN );
		GetDlgItem( IDC_STATICTOPMARGIN )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_STATICBOTMARGIN );
		GetDlgItem( IDC_STATICBOTMARGIN )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_DLGHEADER_CUSTOLOGO );
		GetDlgItem( IDC_CHECKCUSTOLOGO )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_OK );
		GetDlgItem(IDOK )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_CANCEL );
		GetDlgItem(IDCANCEL )->SetWindowText( str );

		TCHAR tcName[_MAXCHARS];
		int i;
		
		m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );
	
		// Copy the current TADS modified flag...
		m_bModified = m_pTADS->IsModified();
		
		// Create a CDB_PageSetup Backup.
		IDPTR IDPtr;
		m_pTADS->CreateObject( IDPtr, CLASS( CDB_PageSetup ) );
		m_pPageSetupBak = (CDB_PageSetup*)IDPtr.MP;
	
		// Load Bmp.
		m_BmpLogo.LoadBitmap( IDB_LOGOTA_SCREEN );
		m_StaticLogo1.SetBitmap( (HBITMAP)m_BmpLogo.GetSafeHandle() );

		CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
		
		if( NULL == pclImgListButton )
		{
			HYSELECT_THROW( _T("Internal error: Can't retrieve image list 'CRCImageManager::ILN_Button'.") );
		}

		m_ButtonFontHeader.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_Font ) );
		m_ButtonFontFooter.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_Font ) );

		m_RadioHeaderRight.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_AlignRight ) );
		m_RadioFooterRight.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_AlignRight ) );

		m_RadioHeaderLeft.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_AlignLeft ) );
		m_RadioFooterLeft.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_AlignLeft ) );

		m_RadioHeaderCentered.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_AlignCenter ) );
		m_RadioFooterCentered.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_AlignCenter ) );

		( (CButton *)GetDlgItem( IDC_BUTTONSETUSERREF ) )->SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_SetUserRef ) );

		str = TASApp.LoadLocalizedString( IDS_BUTTOOLUSERINFO );
		m_ToolTip.Create( this, TTS_NOPREFIX );
		m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONSETUSERREF ), str );

		// Init Text Size for Edit control.
		m_EditHeader.LimitText( _HEADER_LENGTH );
		m_EditFooter.LimitText( _HEADER_LENGTH );
		
		// HYS-1090.
		m_EditHeaderSaved = CteEMPTY_STRING;
		m_EditFooterSaved = CteEMPTY_STRING;
		
		// Init Unit ComboBox.
		m_pUnitDB = CDimValue::AccessUDB();
	
		for( i = 0; i < m_pUnitDB->GetLength( _U_DIAMETER ); i++ )
		{
			GetNameOf( m_pUnitDB->GetUnit( _U_DIAMETER, i ), tcName );
			m_ComboUnit.AddString( tcName );
		}
	
		// Copy Page Setup to another CDB_PageSetup m_pPageSetupBak
		// all modification will be applied into m_pPageSetupBak.
		m_pPageSetup->Copy( m_pPageSetupBak );

		if( false == m_bModified )
		{
			// Clear flag modified to monitor modification in dialog header
			// all modification in m_pPageSetupBak set flag Modified...
			m_pTADS->Modified( false );
		}
		
		// Initilise first page check box, if checked show radio buttons, otherwise hide it
		// if first page exist show it first.
		if( true == m_pPageSetupBak->GetFirstPageDifferent() )
		{
			m_Check1StPage.SetCheck( BST_CHECKED );
		}
		else
		{
			m_Check1StPage.SetCheck( BST_UNCHECKED );
		}

		// Set the text for the Internal Diameter units.
		m_iUnit = m_pUnitDB->GetDefaultUnitIndex( _U_DIAMETER );
		m_iUnit = (int)::AfxGetApp()->GetProfileInt( _T("Header"), _T("Unit"), m_iUnit );
		m_ComboUnit.SetCurSel( m_iUnit );
		m_iUnit = 0;
		SetValues2UUnit();

		for( i = 0; i < _PAGESETUP_ENUM; i++ )
		{
			m_pPageSetupBak->GetLogFont( (PageSetup_enum)i, &m_lf[i] );
			m_Font[i].CreateFontIndirect( &m_lf[i] );
		}

		OnCheck1stpage();	

		return TRUE;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgHeader::OnInitDialog'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgHeader::OnDestroy() 
{
	CDialogEx::OnDestroy();
	
	m_brWhiteBrush.DeleteObject();	
}

/////////////////////////////////////////////////////////////////////////////
// Convert a double Val from current unit to New Unit
double CDlgHeader::ChangeUnit(int PhysType,int CurrentUnit, int NewUnit, double val)
{
	double valSI;
	UnitDesign_struct ud = m_pUnitDB->GetUnit(PhysType,CurrentUnit);
	valSI = GetConvOf(ud)*(val+GetOffsetOf(ud));
	ud = m_pUnitDB->GetUnit(PhysType,NewUnit);
	val = valSI/GetConvOf(ud)-GetOffsetOf(ud);
	return val;
}
/////////////////////////////////////////////////////////////////////////////
// Set Edit (top and bottom margin) values according to the selected unit
//			
void CDlgHeader::SetValues2UUnit()
{
	double val;
	int		iNewUnit;
	TCHAR name[_MAXCHARS];

	iNewUnit = m_ComboUnit.GetCurSel();
	switch (ReadDouble(m_EditTopMargin,&val)) 
	{
	case RD_OK:
		val = ChangeUnit(_U_DIAMETER,m_iUnit,iNewUnit,val);
		m_EditTopMargin.SetWindowText(WriteDouble(val,3,1,true));
	break;
	};

	switch (ReadDouble(m_EditBotMargin,&val)) 
	{
	case RD_OK:
		val = ChangeUnit(_U_DIAMETER,m_iUnit,iNewUnit,val);
		m_EditBotMargin.SetWindowText(WriteDouble(val,3,1,true));
	break;
	};
	
	m_iUnit = iNewUnit;		
	GetNameOf(m_pUnitDB->GetUnit(_U_DIAMETER,m_iUnit),name);
	m_StaticUnit.SetWindowText(name);

}

/////////////////////////////////////////////////////////////////////////////////////////
// User switch between first and other page 
// Initialise all concerning control

void CDlgHeader::OnRadio1stpage() 
{
CFont ft;
	// if Radio 1St page is checked show first page header and footer
	if (m_Radio1StPage.GetCheck())
	{
		// Header and footer text
		m_pPageSetupBak->GetLogFont(epsFIRSTHEADER,&m_lf[(int)epsFIRSTHEADER]);
		m_Font[(int)epsFIRSTHEADER].DeleteObject();
		m_Font[(int)epsFIRSTHEADER].CreateFontIndirect(&m_lf[(int)epsFIRSTHEADER]);
		m_EditHeader.SetFont(&m_Font[(int)epsFIRSTHEADER]);
		m_EditHeader.SetWindowText(m_pPageSetupBak->GetText(epsFIRSTHEADER));
		// HYS-1090: Fill m_EditHeaderSaved
		m_EditHeader.GetWindowTextW( m_EditHeaderSaved );
		m_pPageSetupBak->GetLogFont(epsFIRSTFOOTER,&m_lf[(int)epsFIRSTFOOTER]);
		m_Font[(int)epsFIRSTFOOTER].DeleteObject();
		m_Font[(int)epsFIRSTFOOTER].CreateFontIndirect(&m_lf[(int)epsFIRSTFOOTER]);
		m_EditFooter.SetFont(&m_Font[(int)epsFIRSTFOOTER]);
		m_EditFooter.SetWindowText(m_pPageSetupBak->GetText(epsFIRSTFOOTER));
		// HYS-1090: Fill m_EditFooterSaved
		m_EditFooter.GetWindowTextW( m_EditFooterSaved );

		double val;
		// SI to User unit
		val = ChangeUnit(_U_DIAMETER,0,m_iUnit,m_pPageSetupBak->GetMargin(epsFIRSTHEADER));
		m_EditTopMargin.SetWindowText(WriteDouble(val,3,1,true));
		val = ChangeUnit(_U_DIAMETER,0,m_iUnit,m_pPageSetupBak->GetMargin(epsFIRSTFOOTER));
		m_EditBotMargin.SetWindowText(WriteDouble(val,3,1,true));
		// Logo
		m_CheckLogo.SetCheck(m_pPageSetupBak->GetFirstLogo());
		// HYS-1090: Set customer first logo display
		m_CheckCustoLogo.SetCheck( m_pPageSetupBak->GetFirstCustoLogo() );
		OnCheckCustologo();
		// Alignment
		m_RadioHeaderLeft.SetCheck(false);
		m_RadioHeaderCentered.SetCheck(false);
		m_RadioHeaderRight.SetCheck(false);
		switch (m_pPageSetupBak->GetAlignment(epsFIRSTHEADER))
		{
		case epaTextLeft: m_RadioHeaderLeft.SetCheck(true); break;
		case epaTextCenter: m_RadioHeaderCentered.SetCheck(true); break;
		case epaTextRight: m_RadioHeaderRight.SetCheck(true); break;
		}
		m_RadioFooterLeft.SetCheck(false);
		m_RadioFooterCentered.SetCheck(false);
		m_RadioFooterRight.SetCheck(false);
		switch (m_pPageSetupBak->GetAlignment(epsFIRSTFOOTER))
		{
		case epaTextLeft: m_RadioFooterLeft.SetCheck(true); break;
		case epaTextCenter: m_RadioFooterCentered.SetCheck(true); break;
		case epaTextRight: m_RadioFooterRight.SetCheck(true); break;
		}
	}
	else
	{
		// Header and footer text	

		m_pPageSetupBak->GetLogFont(epsHEADER,&m_lf[(int)epsHEADER]);
		m_Font[(int)epsHEADER].DeleteObject();
		m_Font[(int)epsHEADER].CreateFontIndirect(&m_lf[(int)epsHEADER]);
		m_EditHeader.SetFont(&m_Font[(int)epsHEADER]);
		m_EditHeader.SetWindowText(m_pPageSetupBak->GetText(epsHEADER));
		// HYS-1090: Fill m_EditHeaderSaved
		m_EditHeader.GetWindowTextW( m_EditHeaderSaved );
		m_pPageSetupBak->GetLogFont(epsFOOTER,&m_lf[(int)epsFOOTER]);
		m_Font[(int)epsFOOTER].DeleteObject();
		m_Font[(int)epsFOOTER].CreateFontIndirect(&m_lf[(int)epsFOOTER]);
		m_EditFooter.SetFont(&m_Font[(int)epsFOOTER]);
		m_EditFooter.SetWindowText(m_pPageSetupBak->GetText(epsFOOTER));
		// HYS-1090: Fill m_EditFooterSaved
		m_EditFooter.GetWindowTextW( m_EditFooterSaved );

		double val;
		// SI to User unit
		val = ChangeUnit(_U_DIAMETER,0,m_iUnit,m_pPageSetupBak->GetMargin(epsHEADER));
		m_EditTopMargin.SetWindowText(WriteDouble(val,3,1,true));
		val = ChangeUnit(_U_DIAMETER,0,m_iUnit,m_pPageSetupBak->GetMargin(epsFOOTER));
		m_EditBotMargin.SetWindowText(WriteDouble(val,3,1,true));
		// Logo
		m_CheckLogo.SetCheck(m_pPageSetupBak->GetLogo());
		// HYS-1090: Set customer logo display
		m_CheckCustoLogo.SetCheck( m_pPageSetupBak->GetCustoLogo() );
		OnCheckCustologo();
		// Alignment
		m_RadioHeaderLeft.SetCheck(false);
		m_RadioHeaderCentered.SetCheck(false);
		m_RadioHeaderRight.SetCheck(false);
		switch (m_pPageSetupBak->GetAlignment(epsHEADER))
		{
		case epaTextLeft: m_RadioHeaderLeft.SetCheck(true); break;
		case epaTextCenter: m_RadioHeaderCentered.SetCheck(true); break;
		case epaTextRight: m_RadioHeaderRight.SetCheck(true); break;
		}
		m_RadioFooterLeft.SetCheck(false);
		m_RadioFooterCentered.SetCheck(false);
		m_RadioFooterRight.SetCheck(false);
		switch (m_pPageSetupBak->GetAlignment(epsFOOTER))
		{
		case epaTextLeft: m_RadioFooterLeft.SetCheck(true); break;
		case epaTextCenter: m_RadioFooterCentered.SetCheck(true); break;
		case epaTextRight: m_RadioFooterRight.SetCheck(true); break;
		}
	}
		
}

void CDlgHeader::OnRadioallpages() 
{
	OnRadio1stpage();
}


void CDlgHeader::OnClose() 
{
	for (int i=0; i<_PAGESETUP_ENUM; i++)
		m_Font[i].DeleteObject();
	
	CDialogEx::OnClose();
}

/////////////////////////////////////////////////////////////////////////
//	Check1StPage control unchecked --> Hide Radio Buttons and select Other Pages radio button 
//	Check1StPage control checked   --> Show Radio Buttons (user can choose between first page and other pages)
//								   --> copy all other pages parameters into first page parameters				
//
void CDlgHeader::OnCheck1stpage() 
{
	if (m_Check1StPage.GetCheck())
	{
		m_Radio1StPage.ShowWindow(TRUE);
		m_RadioAllPages.ShowWindow(TRUE);
		m_Radio1StPage.SetCheck(TRUE);
		m_RadioAllPages.SetCheck(FALSE);

		if (!m_pPageSetupBak->GetFirstPageDifferent())		// 
		{
			// Copy other pages parameters to 1St page parameters
			m_pPageSetupBak->SetText(epsFIRSTHEADER,(TCHAR*)m_pPageSetupBak->GetText(epsHEADER));
			m_pPageSetupBak->SetText(epsFIRSTFOOTER,(TCHAR*)m_pPageSetupBak->GetText(epsFOOTER));
			
			m_pPageSetupBak->SetFirstLogo(m_pPageSetupBak->GetLogo());
			m_pPageSetupBak->SetFirstCustoLogo(m_pPageSetupBak->GetCustoLogo());
	
			m_pPageSetupBak->SetMargin(epsFIRSTHEADER,m_pPageSetupBak->GetMargin(epsHEADER));
			m_pPageSetupBak->SetMargin(epsFIRSTFOOTER,m_pPageSetupBak->GetMargin(epsFOOTER));
		
			LOGFONT	lf;
			m_pPageSetupBak->GetLogFont(epsHEADER,&lf);
			m_pPageSetupBak->SetLogFont(epsFIRSTHEADER,&lf);
			m_pPageSetupBak->GetLogFont(epsFOOTER,&lf);
			m_pPageSetupBak->SetLogFont(epsFIRSTFOOTER,&lf);
			
			m_pPageSetupBak->SetFontColor(epsFIRSTHEADER,m_pPageSetupBak->GetFontColor(epsHEADER));
			m_pPageSetupBak->SetFontColor(epsFIRSTFOOTER,m_pPageSetupBak->GetFontColor(epsFOOTER));

			m_pPageSetupBak->SetAlignment(epsFIRSTHEADER,m_pPageSetupBak->GetAlignment(epsHEADER));
			m_pPageSetupBak->SetAlignment(epsFIRSTFOOTER,m_pPageSetupBak->GetAlignment(epsFOOTER));

		}
		m_pPageSetupBak->SetFirstPageDifferent(true);
		// refresh edit header and footer
		OnRadio1stpage();
	}
	else
	{
		m_pPageSetupBak->SetFirstPageDifferent(false);
		m_Radio1StPage.ShowWindow(FALSE);
		m_RadioAllPages.ShowWindow(FALSE);
		m_Radio1StPage.SetCheck(FALSE);
		m_RadioAllPages.SetCheck(TRUE);
		// refresh edit header and footer
		OnRadio1stpage();
	}
}
/////////////////////////////////////////////////////////////////////////
// Length Unit Change
void CDlgHeader::OnSelchangeCombounit() 
{
SetValues2UUnit();	
}

/////////////////////////////////////////////////////////////////////////
// top Margin is changing

void CDlgHeader::OnChangeEdittopmargin() 
{
	CString str;
	double dVal;
	switch (ReadDouble(m_EditTopMargin,&dVal)) 
	{
	case RD_EMPTY:
		m_EditTopMargin.SetWindowText(WriteDouble(0,3,1,true));
		dVal = 0;
		break;
	case RD_NOT_NUMBER:
		m_EditTopMargin.GetWindowText(str);
		if (str == _T(".") || str == _T(",") ) 
			return;
		else
			TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
		m_EditTopMargin.SetWindowText(WriteDouble(0,3,1,true));
		dVal = 0;
		break;
	case RD_OK:
	break;
	};

}

/////////////////////////////////////////////////////////////////////////
// Bottom Margin is changing
void CDlgHeader::OnChangeEditbotmargin() 
{
	CString str;
	double dVal;
	switch (ReadDouble(m_EditBotMargin,&dVal)) 
	{
	case RD_EMPTY:
		m_EditBotMargin.SetWindowText(WriteDouble(0,3,1,true));
		break;
	case RD_NOT_NUMBER:
		m_EditBotMargin.GetWindowText(str);
		if (str == _T(".") || str == _T(",") ) 
			return;
		else
			TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
		m_EditBotMargin.SetWindowText(WriteDouble(0,3,1,true));
		break;
	case RD_OK:
		break;
	};
}

/////////////////////////////////////////////////////////////////////////
// Top Margin  changed
void CDlgHeader::OnKillfocusEdittopmargin() 
{
	double val;
	switch (ReadDouble(m_EditTopMargin,&val))
	{
	case RD_EMPTY:
		val = 0;
		break;
	case RD_OK:
		if (val < 0.0)
		{
			TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
			m_EditTopMargin.SetFocus();
		}
		else
			m_EditTopMargin.SetWindowText(WriteDouble(val,3,1,true));
		break;
	};
	// Save Top Margin Value according to the current page
	val = ChangeUnit(_U_DIAMETER,m_iUnit,0,val);				// To SI
	if (m_Radio1StPage.GetCheck())
		m_pPageSetupBak->SetMargin(epsFIRSTHEADER,val);
	else
		m_pPageSetupBak->SetMargin(epsHEADER,val);
	
}

/////////////////////////////////////////////////////////////////////////
// Bottom Margin  changed
void CDlgHeader::OnKillfocusEditbotmargin() 
{
	double val;
	switch (ReadDouble(m_EditBotMargin,&val))
	{
	case RD_EMPTY:
		val = 0;
		break;
	case RD_OK:
		if (val < 0.0)
		{
			TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
			m_EditBotMargin.SetFocus();
		}
		else
			m_EditBotMargin.SetWindowText(WriteDouble(val,3,1,true));
		break;
	};
	// Save Top Margin Value according to the current page
	val = ChangeUnit(_U_DIAMETER,m_iUnit,0,val);				// To SI
	if (m_Radio1StPage.GetCheck())
		m_pPageSetupBak->SetMargin(epsFIRSTFOOTER,val);
	else
		m_pPageSetupBak->SetMargin(epsFOOTER,val);
	
}
// Save Logo Check Box 
void CDlgHeader::OnChecklogo() 
{
	if (m_Radio1StPage.GetCheck())
		m_CheckLogo.GetCheck()?m_pPageSetupBak->SetFirstLogo(true):m_pPageSetupBak->SetFirstLogo(false);
	else
		m_CheckLogo.GetCheck()?m_pPageSetupBak->SetLogo(true):m_pPageSetupBak->SetLogo(false);
	
}

void CDlgHeader::OnCheckCustologo()
{
	if( BST_UNCHECKED  == m_CheckCustoLogo.GetCheck() )
	{
		m_CheckCustoLogo.SetCheck( BST_UNCHECKED );
		// Don't show the logo
		m_StaticCustoLogo.SetBitmap( NULL );
		m_BmpCustoLogo.Detach();
	}
	else
	{
		m_CheckCustoLogo.SetCheck( BST_CHECKED );
		if( NULL == m_BmpCustoLogo.GetSafeHandle() )
		{
			CDS_UserRef *pUserRef = (CDS_UserRef*)TASApp.GetpUserDB()->Get( _T("USER_REF") ).MP;
			if( 0 != StringCompare( pUserRef->GetString( CDS_UserRef::Path ), CteEMPTY_STRING ) )
			{
				HBITMAP hBitMap = m_BmpCustoLogo.LoadImageFile( pUserRef->GetString( CDS_UserRef::Path ), -1, 90, 70 );
				bool ret = m_BmpCustoLogo.Attach( hBitMap );
				if( true == ret )
				{
					m_StaticCustoLogo.SetBitmap( (HBITMAP)m_BmpCustoLogo.GetSafeHandle() );
				}
			}
		}
	}
	if( m_Radio1StPage.GetCheck() )
		m_CheckCustoLogo.GetCheck() ? m_pPageSetupBak->SetFirstCustoLogo( true ) : m_pPageSetupBak->SetFirstCustoLogo( false );
	else
		m_CheckCustoLogo.GetCheck() ? m_pPageSetupBak->SetCustoLogo( true ) : m_pPageSetupBak->SetCustoLogo( false );
}
///////////////////////////////////////////////////////////////////////////////
// OK Copy all parameters into TADS and delete pPageSetupBak
void CDlgHeader::OnOK() 
{
	PREVENT_ENTER_KEY
	
	// Is there a modification?
	char bModified = m_pTADS->IsModified();
		
	// Save Current selected unit
	::AfxGetApp()->WriteProfileInt(_T("Header"),_T("Unit"),m_iUnit);

	// Save all information into the orinal m_pPageSetup
	m_pPageSetupBak->Copy(m_pPageSetup);
	
	// Remove PageSetup Backup object
	IDPTR IDPtrPageSetup = ((CDB_PageSetup*) m_pPageSetupBak)->GetIDPtr();
	ASSERT(*IDPtrPageSetup.ID);
	VERIFY(m_pTADS->DeleteObject(IDPtrPageSetup));
	
	if (!m_bModified)								// No modification before call to Dialog Header
		m_pTADS->Modified(bModified);				// if there was no modification in Header dialog 
													// erase flag set by deleteobject...
	
	CDialogEx::OnOK();
}

///////////////////////////////////////////////////////////////////////////////
// CANCEL
void CDlgHeader::OnCancel() 
{
	// Save Current selected unit
	::AfxGetApp()->WriteProfileInt(_T("Header"),_T("Unit"),m_iUnit);
	// Remove PageSetup Backup object
	IDPTR IDPtrPageSetup = ((CDB_PageSetup*) m_pPageSetupBak)->GetIDPtr();
	ASSERT(*IDPtrPageSetup.ID);
	VERIFY(m_pTADS->DeleteObject(IDPtrPageSetup));
	// restore Modified flag
	m_pTADS->Modified(m_bModified);	
	CDialogEx::OnCancel();
}

///////////////////////////////////////////////////////////////////////////////
// FOOTER font is changing --> display font dialog  

void CDlgHeader::OnButtonfontfooter() 
{
	LOGFONT	lf;

	if (m_Radio1StPage.GetCheck())		// Get Current Font
		m_pPageSetupBak->GetLogFont(epsFIRSTFOOTER,&lf);
	else
		m_pPageSetupBak->GetLogFont(epsFOOTER,&lf);
	
	CFontDialog	fontDlg(&lf);			// Diplay FONT Dialog
	if (m_Radio1StPage.GetCheck())		// Get Current Font
		fontDlg.m_cf.rgbColors = m_pPageSetupBak->GetFontColor(epsFIRSTFOOTER); 
	else
		fontDlg.m_cf.rgbColors = m_pPageSetupBak->GetFontColor(epsFOOTER); 

	if (fontDlg.DoModal()== IDOK)
	{
		fontDlg.GetCurrentFont(&lf);

		if (m_Radio1StPage.GetCheck())		// Save New Font
		{	
			m_pPageSetupBak->SetLogFont(epsFIRSTFOOTER,&lf);
			m_pPageSetupBak->SetFontColor(epsFIRSTFOOTER,fontDlg.m_cf.rgbColors);
		}
		else
		{
			m_pPageSetupBak->SetLogFont(epsFOOTER,&lf);
			m_pPageSetupBak->SetFontColor(epsFOOTER,fontDlg.m_cf.rgbColors);
		}
		// Redraw edits
		OnRadio1stpage();
	}
	
}

///////////////////////////////////////////////////////////////////////////////
// HEADER font is changing --> display font dialog  

void CDlgHeader::OnButtonfontheader() 
{
	LOGFONT	lf;

	if (m_Radio1StPage.GetCheck())		// Get Current Font
		m_pPageSetupBak->GetLogFont(epsFIRSTHEADER,&lf);
	else
		m_pPageSetupBak->GetLogFont(epsHEADER,&lf);
	
	CFontDialog	fontDlg(&lf);			// Diplay FONT Dialog
	if (m_Radio1StPage.GetCheck())		// Get Current Font
		fontDlg.m_cf.rgbColors = m_pPageSetupBak->GetFontColor(epsFIRSTHEADER); 
	else
		fontDlg.m_cf.rgbColors = m_pPageSetupBak->GetFontColor(epsHEADER); 

	if (fontDlg.DoModal()== IDOK)
	{
		fontDlg.GetCurrentFont(&lf);

		if (m_Radio1StPage.GetCheck())		// Save New Font
		{	
			m_pPageSetupBak->SetLogFont(epsFIRSTHEADER,&lf);
			m_pPageSetupBak->SetFontColor(epsFIRSTHEADER,fontDlg.m_cf.rgbColors);
		}
		else
		{
			m_pPageSetupBak->SetLogFont(epsHEADER,&lf);
			m_pPageSetupBak->SetFontColor(epsHEADER,fontDlg.m_cf.rgbColors);
		}
		// Redraw edits
		OnRadio1stpage();
	}
}

void CDlgHeader::OnChangeEditfooter()
{
	// If more than 4 lines take the last saved modification
	if( -1 == m_EditFooter.LineIndex( 4 ) )
	{
		m_EditFooter.GetWindowTextW( m_EditFooterSaved );
	}
	else if ( m_EditFooterSaved != CteEMPTY_STRING )
	{
		m_EditFooter.SetWindowTextW( m_EditFooterSaved );
	}
}

void CDlgHeader::OnKillfocusEditfooter() 
{
// Update members variables	according to the page selected (first or all pages)
// 
	CString str;

	if (m_Radio1StPage.GetCheck())
	{
		m_EditFooter.GetWindowText(str);
		m_pPageSetupBak->SetText(epsFIRSTFOOTER,(TCHAR*)(LPCTSTR) str);
	}
	else
	{
		m_EditFooter.GetWindowText(str);
		m_pPageSetupBak->SetText(epsFOOTER,(TCHAR*)(LPCTSTR) str);
	}
}

void CDlgHeader::OnChangeEditheader()
{
	// If more than 2 lines take the last saved modification
	if( -1 == m_EditHeader.LineIndex( 2 ) )
	{
		m_EditHeader.GetWindowTextW( m_EditHeaderSaved );
	}
	else if ( m_EditHeaderSaved != CteEMPTY_STRING )
	{
		m_EditHeader.SetWindowTextW( m_EditHeaderSaved );
	}
}

void CDlgHeader::OnKillfocusEditheader() 
{
// Update members variables	according to the page selected (first or all pages)
// 
	CString str;
	if (m_Radio1StPage.GetCheck())		// Update First Header
	{
		m_EditHeader.GetWindowText(str);
		m_pPageSetupBak->SetText(epsFIRSTHEADER,(TCHAR*)(LPCTSTR) str);
	}
	else
	{
		m_EditHeader.GetWindowText(str);
		m_pPageSetupBak->SetText(epsHEADER,(TCHAR*)(LPCTSTR) str);
	}
}

void CDlgHeader::OnRadiofootercentered() 
{
	if (m_Check1StPage.GetCheck())
		m_pPageSetupBak->SetAlignment(epsFIRSTFOOTER,epaTextCenter);
	else
		m_pPageSetupBak->SetAlignment(epsFOOTER,epaTextCenter);
}

void CDlgHeader::OnRadiofooterleft() 
{
	if (m_Check1StPage.GetCheck())
		m_pPageSetupBak->SetAlignment(epsFIRSTFOOTER,epaTextLeft);
	else
		m_pPageSetupBak->SetAlignment(epsFOOTER,epaTextLeft);
	
}

void CDlgHeader::OnRadiofooterright() 
{
	if (m_Check1StPage.GetCheck())
		m_pPageSetupBak->SetAlignment(epsFIRSTFOOTER,epaTextRight);
	else
		m_pPageSetupBak->SetAlignment(epsFOOTER,epaTextRight);
	
}

void CDlgHeader::OnRadioheadercentered() 
{
	if (m_Check1StPage.GetCheck())
		m_pPageSetupBak->SetAlignment(epsFIRSTHEADER,epaTextCenter);
	else
		m_pPageSetupBak->SetAlignment(epsHEADER,epaTextCenter);
	
	
}

void CDlgHeader::OnRadioheaderleft() 
{
	if (m_Check1StPage.GetCheck())
		m_pPageSetupBak->SetAlignment(epsFIRSTHEADER,epaTextLeft);
	else
		m_pPageSetupBak->SetAlignment(epsHEADER,epaTextLeft);
	
}

void CDlgHeader::OnRadioheaderright() 
{
	if (m_Check1StPage.GetCheck())
		m_pPageSetupBak->SetAlignment(epsFIRSTHEADER,epaTextRight);
	else
		m_pPageSetupBak->SetAlignment(epsHEADER,epaTextRight);
	
}

HBRUSH CDlgHeader::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	
	if (pWnd->m_hWnd == m_EditHeader.m_hWnd)
	{
		if (m_Radio1StPage.GetCheck())		// Save New Font
			pDC->SetTextColor(m_pPageSetupBak->GetFontColor(epsFIRSTHEADER));
		else
			pDC->SetTextColor(m_pPageSetupBak->GetFontColor(epsHEADER));
		pDC->SetBkColor(RGB(255,255,255));
		return (HBRUSH) m_brWhiteBrush;
	}
	if (pWnd->m_hWnd == m_EditFooter.m_hWnd)
	{
		if (m_Radio1StPage.GetCheck())		// Save New Font
			pDC->SetTextColor(m_pPageSetupBak->GetFontColor(epsFIRSTFOOTER));
		else
			pDC->SetTextColor(m_pPageSetupBak->GetFontColor(epsFOOTER));
		pDC->SetBkColor(RGB(255,255,255));
		return (HBRUSH) m_brWhiteBrush;
	}
	
	hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

void CDlgHeader::OnButtonsetuserref() 
{
	CDS_UserRef *pUserRef = (CDS_UserRef*) m_pUSERDB->Get(_T("USER_REF")).MP;				ASSERT(pUserRef);
	CString str,str1;
	str1=pUserRef->GetString(CDS_UserRef::Company);
	if (!str1.IsEmpty()) str = str1 + CString(_T("\r\n"));
	str1 = pUserRef->GetString(CDS_UserRef::Name);
	if (!str1.IsEmpty()) str += str1 + CString(_T("\r\n"));

	bool flag=false;

	str1 = pUserRef->GetString(CDS_UserRef::Address);
	if (!str1.IsEmpty())
	{
		str += pUserRef->GetString(CDS_UserRef::Address) + CString(_T(","));
		flag = true;
	}
	str1 = CString(pUserRef->GetString(CDS_UserRef::Zip));
	if (!str1.IsEmpty())
	{
		str += str1;
		str1 = CString(pUserRef->GetString(CDS_UserRef::Town));
		if (!str1.IsEmpty())
			str+=CString(" ");
		else
			str+=CString(",");

		flag = true;
	}
	str1 = CString(pUserRef->GetString(CDS_UserRef::Town));
	if (!str1.IsEmpty())
	{
		str += str1+ CString (",");
		flag = true;
	}
	str1=pUserRef->GetString(CDS_UserRef::Country);
	if (!str1.IsEmpty())
	{	
		str +=  str1;
		flag = true;
	}
	if (flag)
		str += CString("\r\n");
	
	str1=pUserRef->GetString(CDS_UserRef::Tel);
	CString str2;
	str2=TASApp.LoadLocalizedString( IDS_PRINT_TEL);
	if (!str1.IsEmpty()) str += str2 + pUserRef->GetString(CDS_UserRef::Tel) + CString(". ");
	str1=pUserRef->GetString(CDS_UserRef::Fax);
	str2=TASApp.LoadLocalizedString( IDS_PRINT_FAX);
	if (!str1.IsEmpty()) str += str2 + str1 + CString(". ");
	str1=pUserRef->GetString(CDS_UserRef::Email);
	str2=TASApp.LoadLocalizedString( IDS_PRINT_EMAIL);
	if (!str1.IsEmpty()) str += str2 + str1;

	m_EditFooter.SetWindowText( str );
	OnKillfocusEditfooter();
}
