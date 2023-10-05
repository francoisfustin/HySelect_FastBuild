#include "stdafx.h"
#include <windowsx.h>
#include <windows.h>
#include "TASelect.h"
#include "Global.h"
#include "utilities.h"

#include "DlgPageField.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgPageField::CDlgPageField( CWnd *pParent )
	: CDialogEx( CDlgPageField::IDD, pParent )
{
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUSERDB = NULL;
	m_pTADSPageSetup = NULL;
	m_pTADSPageSetupBak = NULL;
	m_bModified = 0;
}

int CDlgPageField::Display()
{
	return DoModal();
}

BEGIN_MESSAGE_MAP( CDlgPageField, CDialogEx )
	ON_WM_PAINT()
	ON_NOTIFY( NM_CLICK, IDC_TREE, OnClickTree )
	ON_BN_CLICKED( IDC_BUTTONADDSTYLE, OnBnClickedAddStyle )
	ON_BN_CLICKED( IDC_BUTTONDELSTYLE, OnBnClickedDeleteStyle )
	ON_CBN_SELCHANGE( IDC_COMBOSTYLE, OnCbnSelChangeStyle )
	ON_MESSAGE( WM_USER_CHECKSTATECHANGE, OnCheckStateChange )
END_MESSAGE_MAP()

void CDlgPageField::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDOK, m_ButtonOk );
	DDX_Control( pDX, IDC_BUTTONDELSTYLE, m_ButtonDelStyle );
	DDX_Control( pDX, IDC_BUTTONADDSTYLE, m_ButtonAddStyle );
	DDX_Control( pDX, IDC_COMBOSTYLE, m_ComboStyle );
	DDX_Control( pDX, IDC_TREE, m_Tree );
}

BOOL CDlgPageField::OnInitDialog() 
{
	try
	{
		CDialogEx::OnInitDialog();

		// Initialize dialog strings.
		CString str = TASApp.LoadLocalizedString( IDS_DLGPAGEFIELD_CAPTION );
		SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_OK );
		GetDlgItem( IDOK )->SetWindowText( str );
		
		str = TASApp.LoadLocalizedString( IDS_CANCEL );
		GetDlgItem( IDCANCEL )->SetWindowText( str );

		m_pTADB = TASApp.GetpTADB();
		m_pTADS = TASApp.GetpTADS();
		m_pUSERDB = TASApp.GetpUserDB();

		// Keep a trace of modified flag before loading Logged Data into the temporary table.
		m_bModified = m_pTADS->IsModified();
	
		// Create a backup table to store the current Page Setup Parameters.
		CTable *pTab = (CTable*)( m_pTADS->Get( _T("PARAM_TABLE") ).MP );
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'PARAM_TABLE' table from the datastruct.") );
		}

		IDPTR ObjIDptr;
		m_pTADS->CreateObject( ObjIDptr, CLASS(CDB_PageSetup), _T("PAGESETUP_BAK") );
		pTab->Insert( ObjIDptr );

		m_pTADSPageSetupBak = (CDB_PageSetup*)( ObjIDptr.MP );
	
		// Retrieve last selected Page Setup from TADS.
		m_pTADSPageSetup = m_pTADS->GetpPageSetup();
		
		if( NULL == m_pTADSPageSetup )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'PARAM_PAGESETUP' object from the datastruct.") );
		}

		// And backup it.
		m_pTADSPageSetup->Copy( m_pTADSPageSetupBak );

		// Create a ToolTipCtrl and add a tool tip for each button.
		m_ToolTip.Create( this, TTS_NOPREFIX );

		CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
		
		if( NULL == pclImgListButton )
		{
			HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CRCImageManager::ILN_Button' image list.") );
		}

		if( NULL != pclImgListButton )
		{
			m_ButtonDelStyle.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_TrashBin ) );
		}

		CString TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTDELSTYLE );
		m_ToolTip.AddToolWindow( &m_ButtonDelStyle,	TTstr );
	
		if( NULL != pclImgListButton )
		{
			m_ButtonAddStyle.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_SaveStyle ) );
		}

		TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTADDSTYLE );
		m_ToolTip.AddToolWindow(&m_ButtonAddStyle,	TTstr );

		// TREE
		// Root level.
		str = TASApp.LoadLocalizedString( IDS_SELP_1STREF );
		HTREEITEM hFirstRef = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfFIRSTREF, TVI_ROOT, TVI_LAST );
		str = TASApp.LoadLocalizedString( IDS_SELP_2NDREF );
		HTREEITEM hSecondRef = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSECONDREF, TVI_ROOT, TVI_LAST );
		str = TASApp.LoadLocalizedString( IDS_SELP_WATERINFO );
		HTREEITEM hWaterInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERINFO, TVI_ROOT, TVI_LAST );
	
		HTREEITEM hSeparatorInfo = NULL;
	
		if( true == TASApp.IsAirVentSepDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_SEPARATORINFO );
			hSeparatorInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hStaticoInfo = NULL;

		if( true == TASApp.IsPressureMaintenanceDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_STATICOINFO );
			hStaticoInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hTecBoxInfo = NULL;

		if( true == TASApp.IsPressureMaintenanceDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_TECBOXINFO );
			hTecBoxInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTECHBOXINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hWaterMakeUpInfo = NULL;

		if( true == TASApp.IsPressureMaintenanceDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_WATERMAKEUPINFO );
			hWaterMakeUpInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERMAKEUPINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hSafetyValveInfo = NULL;

		if( true == TASApp.IsSafetyValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_SAFETYVALVEINFO );
			hSafetyValveInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERMAKEUPINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hBlowTankInfo = NULL;

		if( true == TASApp.IsSafetyValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_BLOWTANKINFO );
			hBlowTankInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBLOWTANKINFO, TVI_ROOT, TVI_LAST );
		}

		// HYS-1741: Add pressure reducing valve
		HTREEITEM hPressureReducInfo = NULL;
		if( true == TASApp.IsSafetyValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRESSUREREDUCINFO );
			hPressureReducInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFO, TVI_ROOT, TVI_LAST );
		}

		str = TASApp.LoadLocalizedString( IDS_SELP_BVINFO );
		HTREEITEM hBvInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVINFO, TVI_ROOT, TVI_LAST );
	
		HTREEITEM hDpCBCVInfo = NULL;

		if( true == TASApp.IsDpCBCVDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_DPCBCVINFO );
			hDpCBCVInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hDpCInfo = NULL;

		if( true == TASApp.IsDpCDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_DPCINFO );
			hDpCInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hCvInfo = NULL;

		if( true == TASApp.IsCvDisplayed() || true == TASApp.IsPICvDisplayed() || true == TASApp.IsBCvDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_CVINFO );
			hCvInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hSmartControlValveInfo = NULL;

		if( true == TASApp.IsSmartControlValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_SMARTCONTROLVALVEINFO );
			hSmartControlValveInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hSmartDpCInfo = NULL;

		if( true == TASApp.IsSmartDpCDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_SMARTDPCINFO );
			hSmartDpCInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM h6WayValveInfo = NULL;

		if( true == TASApp.Is6WayCVDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_6WAYVALVEINFO );
			h6WayValveInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVEINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hShutoffValveInfo = NULL;

		if( true == TASApp.IsShutOffValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_SHUTOFFVALVEINFO );
			hShutoffValveInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hRadSetInfo = NULL;

		if( TASApp.IsTrvDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_RADSETINFO );
			hRadSetInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7INFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hFloorHeatingControlInfo = NULL;

		if( TASApp.IsFloorHeatingControlDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOORHEATINGCONTROLINFO );
			hFloorHeatingControlInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfFLOORHEATINGCONTROLINFO, TVI_ROOT, TVI_LAST );
		}

		HTREEITEM hTapWaterControlInfo = NULL;

		if( TASApp.IsTapWaterControlDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_TAPWATERCONTROLINFO );
			hTapWaterControlInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTAPWATERCONTROLINFO, TVI_ROOT, TVI_LAST );
		}
	
		str = TASApp.LoadLocalizedString( IDS_SELP_PARTDEFINFO );
		HTREEITEM hPartDefInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7INFO, TVI_ROOT, TVI_LAST );
	
		str = TASApp.LoadLocalizedString( IDS_SELP_ARTICLE );
		HTREEITEM hArticle = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfARTICLE, TVI_ROOT, TVI_LAST );
	
		str = TASApp.LoadLocalizedString( IDS_SELP_PIPEINFO );
		HTREEITEM hPipeInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPIPEINFO, TVI_ROOT, TVI_LAST );
	
		str = TASApp.LoadLocalizedString( IDS_SELP_QUANTITY );
		HTREEITEM hQuantity = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfQUANTITY, TVI_ROOT, TVI_LAST );
	
		HTREEITEM hSaleInfo;

		if( true == TASApp.IsPriceUsed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_SALEINFO );
			hSaleInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSALEINFO, TVI_ROOT, TVI_LAST );
		}
		str = TASApp.LoadLocalizedString( IDS_SELP_REMARK );
		HTREEITEM hRemark = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfREMARK, TVI_ROOT, TVI_LAST );
	
		// Separator branch.
		if( true == TASApp.IsAirVentSepDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hSeparatorAirVentProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTINFOPRODUCT, hSeparatorInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTINFOPRODUCTNAME, hSeparatorAirVentProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_DPP );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTINFOPRODUCTDPP, hSeparatorAirVentProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTINFOPRODUCTSIZE, hSeparatorAirVentProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTINFOPRODUCTCONNECTION, hSeparatorAirVentProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTINFOPRODUCTVERSION, hSeparatorAirVentProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTINFOPRODUCTPN, hSeparatorAirVentProduct, NULL );
	
			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hSeparatorTechnical = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTTECHINFO, hSeparatorInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTTECHINFOFLOW, hSeparatorTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_DP );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSEPARATORAIRVENTTECHINFODP, hSeparatorTechnical, NULL );
		}

		// Statico branch.
		if( true == TASApp.IsPressureMaintenanceDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
		
			HTREEITEM hStaticoProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFOPRODUCT, hStaticoInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFOPRODUCTNAME, hStaticoProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFOPRODUCTCONNECTION, hStaticoProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_NOMINALVOLUME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFOPRODUCTNOMINALVOLUME, hStaticoProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_PN );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFOPRODUCTMAXPRESSURE, hStaticoProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_TEMPRANGE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFOPRODUCTTEMPRANGE, hStaticoProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_WEIGHT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFOPRODUCTWEIGHT, hStaticoProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_MAXWEIGHT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSTATICOINFOPRODUCTMAXWEIGHT, hStaticoProduct, NULL );
		}

		// TecBox branch.
		if( true == TASApp.IsPressureMaintenanceDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
		
			HTREEITEM hTecBoxProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTECHBOXINFOPRODUCT, hTecBoxInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTECHBOXINFOPRODUCTNAME, hTecBoxProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_PN );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTECHBOXINFOPRODUCTPS, hTecBoxProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_POWER );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTECHBOXINFOPRODUCTPOWER, hTecBoxProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_SUPPLYVOLTAGE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTECHBOXINFOPRODUCTSUPPLYVOLTAGE, hTecBoxProduct, NULL );
		}

		// Water make-up branch.
		if( true == TASApp.IsPressureMaintenanceDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
		
			HTREEITEM hWaterMakeUpProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERMAKEUPINFOPRODUCT, hWaterMakeUpInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERMAKEUPINFOPRODUCTNAME, hWaterMakeUpProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_FUNCTIONS );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERMAKEUPINFOPRODUCTFUNCTIONS, hWaterMakeUpProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_CAPACITY );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERMAKEUPINFOPRODUCTCAPACITY, hWaterMakeUpProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_HEIGHT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERMAKEUPINFOPRODUCTHEIGHT, hWaterMakeUpProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_WEIGHT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfWATERMAKEUPINFOPRODUCTWEIGHT, hWaterMakeUpProduct, NULL );
		}

		// Safety valve branch.
		if( true == TASApp.IsSafetyValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hSafetyValveProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVEINFOPRODUCT, hSafetyValveInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVEINFOPRODUCTNAME, hSafetyValveProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVEINFOPRODUCTSIZE, hSafetyValveProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVEINFOPRODUCTCONNECTION, hSafetyValveProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_APPLICATIONTYPE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVEINFOPRODUCTPROJECTTYPE, hSafetyValveProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hSafetyValveTechnical = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVETECHINFO, hSafetyValveInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_SETPRESSURE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVETECHINFOSETPRESSURE, hSafetyValveTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_HEATGENERATORTYPE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVETECHINFOHEATGENERATORTYPE, hSafetyValveTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_POWERMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVETECHINFOPOWERMAX, hSafetyValveTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_POWERSYSTEM );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVETECHINFOPOWERSYSTEM, hSafetyValveTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_COLLECTORMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVETECHINFOCOLLECTORMAX, hSafetyValveTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_COLLECTOR );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSAFETYVALVETECHINFOCOLLECTOR, hSafetyValveTechnical, NULL );
		}

		// Blow tank Info branch.
		if( true == TASApp.IsSafetyValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hBlowTankProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBLOWTANKINFOPRODUCT, hBlowTankInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBLOWTANKINFOPRODUCTNAME, hBlowTankProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBLOWTANKINFOPRODUCTSIZE, hBlowTankProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hBlowTankTechnical = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBLOWTANKTECHINFO, hBlowTankInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBLOWTANKTECHINFOPS, hBlowTankTechnical, NULL );
		}
		
		// HYS-1741: Pressure reducer Info branch.
		if( true == TASApp.IsSafetyValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hPressureReducProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFOPRODUCT, hPressureReducInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFOPRODUCTNAME, hPressureReducProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_BDYMAT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFOPRODUCTBDYMATERIAL, hPressureReducProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFOPRODUCTCONNECTION, hPressureReducProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFOPRODUCTVERSION, hPressureReducProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFOPRODUCTPN, hPressureReducProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_INLETPRESSURE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFOPRODUCTINLETPRESS, hPressureReducProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_OUTLETPRESSURE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPRESSUREREDUCVALVEINFOPRODUCTOUTLETPRESS, hPressureReducProduct, NULL );

		}

		// BV Info branch.
		str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
		HTREEITEM hBvProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVINFOPRODUCT, hBvInfo, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVINFOPRODUCTNAME, hBvProduct, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVINFOPRODUCTSIZE, hBvProduct, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVINFOPRODUCTCONNECTION, hBvProduct, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVINFOPRODUCTVERSION, hBvProduct, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVINFOPRODUCTPN, hBvProduct, NULL );
	
		str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
		HTREEITEM hBvTechnical = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVTECHINFO, hBvInfo, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVTECHINFOFLOW, hBvTechnical, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_SELP_BVDP );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVTECHINFODP, hBvTechnical, NULL );
		
		str = TASApp.LoadLocalizedString( IDS_SELP_SETTING );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfBVTECHINFOSETTING, hBvTechnical, NULL );

		// Dpc Info Branch.
		if( true == TASApp.IsDpCDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hDpCProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCINFOPRODUCT, hDpCInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCINFOPRODUCTNAME, hDpCProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCINFOPRODUCTSIZE, hDpCProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCINFOPRODUCTCONNECTION, hDpCProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCINFOPRODUCTVERSION, hDpCProduct, NULL );		
			str = TASApp.LoadLocalizedString( IDS_SELP_DPLRANGE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCINFOPRODUCTDPLRANGE, hDpCProduct, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCINFOPRODUCTPN, hDpCProduct, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hDpCTechnical = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCTECHINFO, hDpCInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCTECHINFOFLOW, hDpCTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_DPC2NDDP );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCTECHINFO2NDDP, hDpCTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_DPMIN );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCTECHINFODPMIN, hDpCTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_SETTING );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCTECHINFOSETTING, hDpCTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_HMIN );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCTECHINFOHMIN, hDpCTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTSCHEME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCTECHINFOCONNECTSCHEME, hDpCTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_AUTHORITY );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCTECHINFOAUTHORITY, hDpCTechnical, NULL );
		}

		// DpcBCV Info Branch.
		if( true == TASApp.IsDpCBCVDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hDpCBCVProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVINFOPRODUCT, hDpCBCVInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVINFOPRODUCTNAME, hDpCBCVProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVINFOPRODUCTSIZE, hDpCBCVProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVINFOPRODUCTCONNECTION, hDpCBCVProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVINFOPRODUCTVERSION, hDpCBCVProduct, NULL );		
			
			str = TASApp.LoadLocalizedString( IDS_SELP_DPLRANGE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVINFOPRODUCTDPLRANGE, hDpCBCVProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVINFOPRODUCTPN, hDpCBCVProduct, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hDpCBCVTechnical = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVTECHINFO, hDpCBCVInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVTECHINFOFLOW, hDpCBCVTechnical, NULL );

			str = TASApp.LoadLocalizedString( IDS_DPMIN );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVTECHINFODPMIN, hDpCBCVTechnical, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_SETTING );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVTECHINFOSETTING, hDpCBCVTechnical, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_HMIN );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfDPCBCVTECHINFOHMIN, hDpCBCVTechnical, NULL );
		}

		// Cv Information.
		if( true == TASApp.IsCvDisplayed() || true == TASApp.IsPICvDisplayed() || true == TASApp.IsBCvDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hCvInfoProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVINFOPRODUCT, hCvInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVINFOPRODUCTNAME, hCvInfoProduct, NULL );
		
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		
			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_SELP_KVFC );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_SELP_CVFC );
			}

			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVINFOPRODUCTKV, hCvInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_BDYMAT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVINFOPRODUCTBDYMATERIAL, hCvInfoProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVINFOPRODUCTCONNECTION, hCvInfoProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVINFOPRODUCTVERSION, hCvInfoProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVINFOPRODUCTPN, hCvInfoProduct, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hCvTechInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVTECHINFO, hCvInfo, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVTECHINFOFLOW, hCvTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_CVDP );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVTECHINFODP, hCvTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_PS );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfCVTECHINFOSETTING, hCvTechInfo, NULL );
		}

		// Smart control valve information.
		if( true == TASApp.IsSmartControlValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hSmartControlValveInfoProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCT, hSmartControlValveInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTNAME, hSmartControlValveInfoProduct, NULL );
		
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		
			str = TASApp.LoadLocalizedString( IDS_SELP_BDYMAT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTBDYMATERIAL, hSmartControlValveInfoProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTCONNECTION, hSmartControlValveInfoProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTPN, hSmartControlValveInfoProduct, NULL );

			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_SELP_KVFC );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_SELP_CVFC );
			}

			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTKVS, hSmartControlValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_QNOM );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTQNOM, hSmartControlValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_POWERSUPPLY );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTPOWERSUPPLY, hSmartControlValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_INPUTSIGNAL );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTINPUTSIGNAL, hSmartControlValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_OUTPUTSIGNAL );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVEINFOPRODUCTOUTPUTSIGNAL, hSmartControlValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hSmartControlValveTechInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVETECHINFO, hSmartControlValveInfo, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVETECHINFOFLOW, hSmartControlValveTechInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPMIN );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVETECHINFODPMIN, hSmartControlValveTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVALVELOCALIZATION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVETECHINFOLOCALIZATION, hSmartControlValveTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVALVECONTROLMODE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVETECHINFOCONTROLMODE, hSmartControlValveTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEPOWERMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTCONTROLVALVETECHINFOPOWERMAX, hSmartControlValveTechInfo, NULL );
		}

		// Smart differential pressure controller information.
		if( true == TASApp.IsSmartDpCDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hSmartDpCInfoProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCINFOPRODUCT, hSmartDpCInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCINFOPRODUCTNAME, hSmartDpCInfoProduct, NULL );
		
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		
			str = TASApp.LoadLocalizedString( IDS_SELP_BDYMAT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCINFOPRODUCTBDYMATERIAL, hSmartDpCInfoProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCINFOPRODUCTCONNECTION, hSmartDpCInfoProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCINFOPRODUCTPN, hSmartDpCInfoProduct, NULL );

			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_SELP_KVFC );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_SELP_CVFC );
			}

			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCINFOPRODUCTKVS, hSmartDpCInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_QNOM );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCINFOPRODUCTQNOM, hSmartDpCInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hSmartDpCTechInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCTECHINFO, hSmartDpCInfo, NULL );
		
			// HYS-2067
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCTECHINFOFLOW, hSmartDpCTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEPOWERMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCTECHINFOPOWERMAX, hSmartDpCTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_DP );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCTECHINFODP, hSmartDpCTechInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_DPMIN );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCTECHINFODPMIN, hSmartDpCTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_SMARTDPCLOCALIZATION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSMARTDPCTECHINFOLOCALIZATION, hSmartDpCTechInfo, NULL );
		}

		// 6-way valve information.
		if( true == TASApp.Is6WayCVDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM h6WayValveInfoProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVEINFOPRODUCT, h6WayValveInfo, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVEINFOPRODUCTNAME, h6WayValveInfoProduct, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_KVS );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVEINFOPRODUCTKVS, h6WayValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_BDYMAT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVEINFOPRODUCTBDYMATERIAL, h6WayValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVEINFOPRODUCTCONNECTION, h6WayValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVEINFOPRODUCTVERSION, h6WayValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVEINFOPRODUCTPN, h6WayValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM h6WayValveTechInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVETECHINFO, h6WayValveInfo, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVETECHINFOFLOW, h6WayValveTechInfo, NULL );

			// HYS-2051: Add Dp item for 6-way valve tech. info.
			str = TASApp.LoadLocalizedString( IDS_SELP_DP );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epf6WAYVALVETECHINFODP, h6WayValveTechInfo, NULL );
		}

		// Shutoff valve information.
		if( true == TASApp.IsShutOffValveDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hShutoffValveInfoProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVINFOPRODUCT, hShutoffValveInfo, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVINFOPRODUCTNAME, hShutoffValveInfoProduct, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVINFOPRODUCTSIZE, hShutoffValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVINFOPRODUCTCONNECTION, hShutoffValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVINFOPRODUCTVERSION, hShutoffValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVINFOPRODUCTPN, hShutoffValveInfoProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hShutoffValveTechInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVTECHINFO, hShutoffValveInfo, NULL );
		
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVTECHINFOFLOW, hShutoffValveTechInfo, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_DP );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVTECHINFODP, hShutoffValveTechInfo, NULL );
		}

		// Radiator set information.
		if( true == TASApp.IsTrvDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hRadSetProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7INFOPRODUCT, hRadSetInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7INFOPRODUCTNAME, hRadSetProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSVINFOPRODUCTSIZE, hRadSetProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7INFOPRODUCTCONNECTION, hRadSetProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7INFOPRODUCTVERSION, hRadSetProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_PNTMINTMAX );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7INFOPRODUCTPN, hRadSetProduct, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
			HTREEITEM hRadSetTechnical = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7TECHINFO, hRadSetInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_FLOWRATE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7TECHINFOFLOW, hRadSetTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_DPC2NDDP );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7TECHINFODP, hRadSetTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7TECHINFOSETTING, hRadSetTechnical, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_DPTOT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7TECHINFODPTOT, hRadSetTechnical, NULL );

			str = TASApp.LoadLocalizedString( IDS_SELP_RADINFO );
			HTREEITEM hRadSetRadInfo = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7RADINFO, hRadSetInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_REQPOW );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7RADINFOREQPOW, hRadSetRadInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_INSTPOW );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7RADINFOINSTPOW, hRadSetRadInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_TROOM );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7RADINFOTROOM, hRadSetRadInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_TSUPPLY );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7RADINFOTSUPPLY, hRadSetRadInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_DELTAT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfRD7RADINFODELTAT, hRadSetRadInfo, NULL );
		}

		// Floor heating control information.
		if( true == TASApp.IsFloorHeatingControlDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hFloorHeatingControlProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfFLOORHEATINGCONTROLINFOPRODUCT, hFloorHeatingControlInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfFLOORHEATINGCONTROLINFOPRODUCTNAME, hFloorHeatingControlProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_ADDITIONALINFO );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfFLOORHEATINGCONTROLINFOPRODUCTADDINFO, hFloorHeatingControlProduct, NULL );
		}

		// Tap water control information.
		if( true == TASApp.IsTapWaterControlDisplayed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
			HTREEITEM hTapWaterControlProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTAPWATERCONTROLINFOPRODUCT, hTapWaterControlInfo, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTAPWATERCONTROLINFOPRODUCTNAME, hTapWaterControlProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_SETTINGRANGE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTAPWATERCONTROLINFOPRODUCTSETTINGRANGE, hTapWaterControlProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_BDYMAT );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTAPWATERCONTROLINFOPRODUCTBDYMATERIAL, hTapWaterControlProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_CONNECTION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTAPWATERCONTROLINFOPRODUCTCONNECTION, hTapWaterControlProduct, NULL );
			
			str = TASApp.LoadLocalizedString( IDS_SELP_VERSION );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfTAPWATERCONTROLINFOPRODUCTVERSION, hTapWaterControlProduct, NULL );
		}
	
		// Partially defined Info branch.
		str = TASApp.LoadLocalizedString( IDS_SELP_PRODUCT );
		HTREEITEM hPDProduct = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPDINFOPRODUCT, hPartDefInfo, NULL );
		str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPDINFOPRODUCTNAME, hPDProduct, NULL );
		str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPDINFOPRODUCTSIZE, hPDProduct, NULL );
		str = TASApp.LoadLocalizedString( IDS_SELP_TECHNICAL );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPDTECHINFO, hPartDefInfo, NULL );

		// Pipe information branch.
		str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPIPENAME, hPipeInfo, NULL );
		str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPIPESIZE, hPipeInfo, NULL );
		str = TASApp.LoadLocalizedString( IDS_SELP_WATERU );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPIPEWATERU, hPipeInfo, NULL );
		str = TASApp.LoadLocalizedString( IDS_SELP_DP );
		m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfPIPEDP, hPipeInfo, NULL );

		// Price information branch.
		if( true == TASApp.IsPriceUsed() )
		{
			str = TASApp.LoadLocalizedString( IDS_SELP_UNITPRICE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSALEUNITPRICE, hSaleInfo, NULL );
			str = TASApp.LoadLocalizedString( IDS_SELP_TOTALPRICE );
			m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, epfSALETOTALPRICE, hSaleInfo, NULL );
		}
	
		// Initialize ComboField with all possibilities existing in USERDB.
		m_ComboStyle.LimitText( _STRING_LENGTH );
		_InitPageFieldDD( m_pTADSPageSetup->GetString() );
	
		_InitPageField();
		return TRUE;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgPageField::OnInitDialog'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgPageField::OnPaint() 
{
	// Device context for painting.
	CPaintDC dc( this );

	_InitPageField();

	// Do not call CDialogEx::OnPaint() for painting messages
}

void CDlgPageField::OnOK() 
{
	// Delete backup object.
	IDPTR IDPtr = m_pTADSPageSetupBak->GetIDPtr();
	( (CTable*)IDPtr.PP )->Remove( IDPtr );
	m_pTADS->DeleteObject( IDPtr );

	CDialogEx::OnOK();
}

void CDlgPageField::OnCancel() 
{
	// Retrieve last saved parameters.
	m_pTADSPageSetupBak->Copy( m_pTADSPageSetup );
	
	// Delete backup object.
	IDPTR IDPtr = m_pTADSPageSetupBak->GetIDPtr();
	( (CTable*)IDPtr.PP )->Remove( IDPtr );
	m_pTADS->DeleteObject( IDPtr );

	// Reset the 'Modified' flag.
	m_pTADS->Modified( m_bModified );
	m_pTADS->RefreshResults( false );

	CDialogEx::OnCancel();
}

void CDlgPageField::OnClickTree( NMHDR *pNMHDR, LRESULT *pResult ) 
{
	if( NULL == pNMHDR || NULL == pResult )
	{
		ASSERT_RETURN;
	}

	// On a TreeView Control with the CheckBoxes, there is no notification that the check state
	// of the item has been changed, you can just determine that the user has clicked the state Icon
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is 
	// post a user defined message as a notification that the check state has changed
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = GET_X_LPARAM( dwpos );
	ht.pt.y = GET_Y_LPARAM( dwpos );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( TVHT_ONITEMSTATEICON == ( TVHT_ONITEMSTATEICON & ht.flags ) )
	{
		::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, 0, (LPARAM)ht.hItem );
	}

	*pResult = 0;
}

void CDlgPageField::OnBnClickedAddStyle() 
{
	try
	{
		CString str,str1;
		m_ComboStyle.GetWindowText( str );
	
		if( _T("") == str )
		{
			return;
		}

		IDPTR IDPtr;
		CDB_PageField *pPageField;
	
		// Check if type exist.
		CTable *pPFTab = (CTable *)( m_pUSERDB->Get( _T("PAGE_FIELD_TAB") ).MP );
		
		if( NULL == pPFTab )
		{
			HYSELECT_THROW( _T("Internal error: 'PAGE_FIELD_TAB' table doesn't exist in the user database.") );
		}
	
		if( pPFTab->GetItemCount( CLASS( CDB_PageField ) ) > 0 )		// Check if this style already exist
		{
			for( IDPtr = pPFTab->GetFirst( CLASS( CDB_PageField ) ); _T('\0') != *IDPtr.ID; IDPtr = pPFTab->GetNext() )
			{
				pPageField = (CDB_PageField *)( IDPtr.MP );
				str1 = pPageField->GetString();
			
				if( str == str1 )								// Style already exist, warning message
				{
					if( false == m_pTADSPageSetup->CompareFields( pPageField ) )
					{
						FormatString( str1, AFXMSG_ENTRYEXIST, str ); 
					
						if( IDNO == AfxMessageBox( str1, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
						{	
							m_ComboStyle.SetWindowText( _T("") );
							m_ComboStyle.SetFocus();
							return;
						}
						else								// Update Current selected Style
						{
							pPageField->SetStyle( (CDB_PageField*)m_pTADSPageSetup ); 
							return;
						}
					}
					else
					{
						return;
					}
				}	
			}
		}
	
		// Insert new Style input.
		m_pUSERDB->CreateObject( IDPtr, CLASS( CDB_PageField ) );
		pPFTab->Insert( IDPtr );

		pPageField = (CDB_PageField*)( IDPtr.MP );
		pPageField->SetString( (TCHAR*)(LPCTSTR)str );

		_FindSelectedFields();
		pPageField->SetStyle( (CDB_PageField*)m_pTADSPageSetup );
		m_pTADSPageSetup->SetString( (TCHAR*)(LPCTSTR)str );
		_InitPageFieldDD( str );
		_InitPageField();
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgPageField::OnBnClickedAddStyle'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgPageField::OnBnClickedDeleteStyle() 
{
	CString str, str1;
	m_ComboStyle.GetWindowText( str );		//Check if one style entry is selected 

	if( _T("") == str )
	{
		return;
	}
	
	CDB_PageField *pPageField = (CDB_PageField *)m_ComboStyle.GetItemDataPtr( m_ComboStyle.GetCurSel() ); // retrieve glossary pointer
	FormatString( str1, AFXMSG_DELETEENTRY, str ); // Warning Message
	
	if( IDYES != AfxMessageBox( str1, MB_YESNO | MB_ICONQUESTION, 0 ) )
	{
		return;
	}

	IDPTR IDPtrPageField = ( (CDB_PageField*)pPageField )->GetIDPtr();
	ASSERT( _T('\0') != *IDPtrPageField.ID );

	// Get owner.
	CTable *pPFTab = (CTable *)( m_pUSERDB->Get( _T("PAGE_FIELD_TAB") ).MP );
	ASSERT( NULL != pPFTab );

	pPFTab->Remove( IDPtrPageField );
	VERIFY( m_pUSERDB->DeleteObject( IDPtrPageField ) );
	_InitPageFieldDD();
	m_pTADSPageSetup->SetString( _T("") );						// Current Setup is erased	
}

void CDlgPageField::OnCbnSelChangeStyle() 
{
	if( 0 == m_ComboStyle.GetCount() )
	{
		return;
	}
	
	if( m_ComboStyle.GetCurSel() < 0 )
	{
		return;			// No Selection
	}

	// retrieve Field pointer
	CDB_PageField *pPageField = (CDB_PageField *)m_ComboStyle.GetItemDataPtr( m_ComboStyle.GetCurSel() );
	ASSERT( NULL != pPageField );

	if( NULL == pPageField )
	{
		return;
	}
	
	m_pTADSPageSetup->SetStyle( pPageField );
	
	if( CB_ERR != m_ComboStyle.GetCurSel() )
	{
		CString str( _T("") );
		m_ComboStyle.GetLBText( m_ComboStyle.GetCurSel(), str );						// Save Style name
		m_pTADSPageSetup->SetString( (TCHAR *)(LPCTSTR)str );
	}

	_InitPageField();
}

////////////////////////////////////////////////////////////////////
// User click on the tree
// verify if one check box is checked or unchecked, if true check or
// uncheck child or parent according to the tree logic
// Tree Logic:
//			One Item is checked :	* check all children
//									* check parents
//			One Item is unchecked : * uncheck all children
//									* uncheck parent if all item sibling are unchecked
LRESULT CDlgPageField::OnCheckStateChange( WPARAM wParam, LPARAM lParam )
{
	HTREEITEM hTree = (HTREEITEM)lParam;

	if( NULL != hTree )	// One modified item found
	{
		HTREEITEM hTreeSave = hTree;
	
		if( TRUE == m_Tree.GetCheck( hTree ) )						// One Item Checked
		{	
			// Check all children.
			if( TRUE == m_Tree.ItemHasChildren( hTree ) )
			{
				HTREEITEM hTreeChild = m_Tree.GetChildItem( hTree );
				
				do
				{
					m_Tree.SetCheck( hTree, TRUE );
					hTree = _NextTreeItem( &m_Tree, hTree, true );
					
					if( NULL != hTree ) 
					{
						hTree = hTreeChild = m_Tree.GetNextSiblingItem( hTreeChild );
					}

				}while( NULL != hTree && hTree != hTreeSave );
			}
			
			hTree = hTreeSave;
			
			// Check all parents.
			while( NULL != hTree )
			{
				m_Tree.SetCheck( hTree, TRUE );
				hTree=m_Tree.GetParentItem( hTree );
			}
		}												// One Item Unchecked
		else
		{
			// Uncheck all children.
			if( TRUE == m_Tree.ItemHasChildren( hTree ) )
			{
				HTREEITEM hTreeChild = m_Tree.GetChildItem( hTree );
				
				do
				{
					m_Tree.SetCheck(hTree, FALSE );
					hTree = _NextTreeItem( &m_Tree, hTree, true );
					
					if( NULL == hTree ) 
					{
						hTree = hTreeChild = m_Tree.GetNextSiblingItem( hTreeChild );
					}

				}while( NULL != hTree && hTree != hTreeSave );
			}
			
			// Uncheck Parents if not any sibling is checked.
			HTREEITEM hTreeParent = m_Tree.GetParentItem( hTree );
			
			while( NULL != hTreeParent )
			{
				if( 0 == _IsChildChecked( &m_Tree, m_Tree.GetChildItem( hTreeParent ) ) )
				{
					m_Tree.SetCheck( hTreeParent, false );
					hTreeParent = m_Tree.GetParentItem( hTreeParent );
				}
				else
				{
					break;
				}
			}
		}
	}
	
	_FindSelectedFields();
	
	return 0;
}

void CDlgPageField::_InitPageField()
{
	HTREEITEM hTree = m_Tree.GetRootItem();

	// Set a larger string than the CTreeCtrl length to force the window to be updated.
	CString str = m_Tree.GetItemText( hTree );
	m_Tree.SetItemText( hTree, _T("123456789 123456789 123456789 123456789 123456789 123456789 123") ); // add a text that exceed the tree view
	m_Tree.SetItemText( hTree, (LPCTSTR)str ); // set back the previous text

	// Do a loop on all fields.
	while( NULL != hTree )
	{
		TRACE( _T("\nGet %s : %d"), m_Tree.GetItemText( hTree ), m_pTADSPageSetup->GetField( (PageField_enum)m_Tree.GetItemData( hTree ) ) );
		m_Tree.SetCheck( hTree, m_pTADSPageSetup->GetField( (PageField_enum)m_Tree.GetItemData( hTree ) ) );
		hTree=_NextTreeItem(&m_Tree, hTree);
	}
}

void CDlgPageField::_InitPageFieldDD( CString str )
{
	int i;
	m_ComboStyle.ResetContent();
	m_ComboStyle.LimitText( _STRING_LENGTH );
	
	// Initialize ComboField with all possibility existing in USERDB.
	CTable *pPFTab = (CTable *)( m_pUSERDB->Get( _T("PAGE_FIELD_TAB") ).MP );
	ASSERT( NULL != pPFTab );
	
	CDB_PageField *pPageField = NULL;
	
	if( 0 != pPFTab->GetItemCount( CLASS( CDB_PageField ) ) )							// String Exist
	{
		for( IDPTR IDPtr = pPFTab->GetFirst( CLASS( CDB_PageField ) ); _T('\0') != *IDPtr.ID; IDPtr = pPFTab->GetNext() )
		{
			pPageField = (CDB_PageField *)( IDPtr.MP );
			i = m_ComboStyle.AddString( pPageField->GetString() );
			m_ComboStyle.SetItemDataPtr( i, (void *)pPageField );
		}
	}

	if( _T("") != str )
	{
		i = m_ComboStyle.FindStringExact( 0, str );

		if( i < 0 )
		{
			return;
		}

		m_ComboStyle.SetCurSel( i );								// Select Item 
	}
}

void CDlgPageField::_FindSelectedFields()
{
	m_pTADSPageSetup->ClearAllFields();
	HTREEITEM hTree = m_Tree.GetRootItem();
	
	while( NULL != hTree )
	{
		if( TRUE == m_Tree.GetCheck( hTree ) ) 
		{
			m_pTADSPageSetup->SetField( (PageField_enum)m_Tree.GetItemData( hTree ), true );
		}
		
		TRACE( _T("\nSet %s : %d"), m_Tree.GetItemText( hTree ), m_pTADSPageSetup->GetField( (PageField_enum)m_Tree.GetItemData( hTree ) ) );
		hTree = _NextTreeItem( &m_Tree, hTree );
	}
}

HTREEITEM CDlgPageField::_NextTreeItem( CTreeCtrl *pTree, HTREEITEM hTree, bool bOneBranch )
{
	HTREEITEM hTreeSave = hTree;	

	if( TRUE == pTree->ItemHasChildren( hTree ) )								// Child exist
	{
		hTree = pTree->GetChildItem( hTree );							// one level down
	}
	else 
	{
		if( NULL != pTree->GetNextSiblingItem( hTree ) )						// Brother ?
		{
			hTree = pTree->GetNextSiblingItem( hTree );				// yes takes it
		}
		else
		{
			while( NULL != hTree )											
			{		
				// In case of hTree didn't have children or sibling
				if( true == bOneBranch && hTree == hTreeSave )
				{
					return NULL;	// don't take is Parent if bOneBranch = true
				}
				
				hTree = pTree->GetParentItem( hTree );				// go to one level up
				
				if( NULL != hTree )
				{
					if( true == bOneBranch && hTree == hTreeSave )
					{
						break;	// don't take sibling if bOneBranch = true
					}

					if( NULL != pTree->GetNextSiblingItem( hTree ) )	
					{
						hTree = pTree->GetNextSiblingItem( hTree );	// take next brother if exist
						break;
					}
				}
			}
		}
	}
	return hTree;
}

int CDlgPageField::_IsChildChecked( CTreeCtrl *pTree, HTREEITEM hTree )
{
	int iChildChecked = 0;
	
	while( NULL != hTree && 0 == iChildChecked )
	{
		// If child exist...
		if( TRUE == pTree->ItemHasChildren( hTree ) )
		{
			iChildChecked = _IsChildChecked( pTree, pTree->GetChildItem( hTree ) );	// one level down

			if( 1 == iChildChecked )
			{
				return true;
			}
		}
		else 
		{
			iChildChecked = pTree->GetCheck( hTree );
			
			if( 1 == iChildChecked )
			{
				return true;
			}
		}

		hTree = pTree->GetNextSiblingItem( hTree );			// Brother ?
		iChildChecked = _IsChildChecked( pTree, hTree );	
		
	}

	return iChildChecked;
}
