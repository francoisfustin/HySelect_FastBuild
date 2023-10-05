// DlgExportConfigFile.cpp : implementation file
//

#include "stdafx.h"
#include <iostream>
#include "TASelect.h"
#include "HMInclude.h"
#include "DlgRef.h"
#include "HubHandler.h"
#include "Utilities.h"
#include "DlgExportConfigFile.h"
#include "..\ZipArchive\ZipArchive.h"

using namespace std;

#define HUBZIP
#define HUBIMAGE

#define XML_MODELPARTNUMBER _T("TA-HUB")
#define XML_REVISION		_T("2")
#define XSL_FILENAME		_T("TAHubStylesheet.xsl")


// DlgExportConfigFile dialog

IMPLEMENT_DYNAMIC(DlgExportConfigFile, CDialogEx)

DlgExportConfigFile::DlgExportConfigFile( CWnd* pParent )
	: CDialogEx( DlgExportConfigFile::IDD, pParent )
{
	m_pTADS = NULL;
	m_pTADB = NULL;
	m_iSelectedRow = -1;
}

DlgExportConfigFile::~DlgExportConfigFile()
{
}

void DlgExportConfigFile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTHUB, m_ListHub);
	DDX_Control(pDX, IDC_STATICIMG, m_Img);
	DDX_Control(pDX, IDC_EDITCUSTOMERNAME, m_EditCustomerName);
	DDX_Control(pDX, IDC_BUTTONPROJECTINFO, m_ButtonProjectInfo);
	DDX_Control(pDX, IDC_BUTTONUNSELECTALL, m_ButtonUnselectAll);
	DDX_Control(pDX, IDC_BUTTONSELECTALL, m_ButtonSelectAll);
	DDX_Control(pDX, IDC_STATICSELECTEDHUBS, m_StaticSelectedHubs);
	DDX_Control(pDX, IDC_STATICSELECTEDFOREXPORT, m_GroupSelForExp);
	DDX_Control(pDX, IDC_STATICPROJECTINFO, m_GroupProjInfo);
}

BEGIN_MESSAGE_MAP(DlgExportConfigFile, CDialogEx)
	ON_BN_CLICKED(IDOK, &DlgExportConfigFile::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &DlgExportConfigFile::OnBnClickedCancel)
	ON_BN_CLICKED( IDC_BUTTONSELECTALL, &DlgExportConfigFile::OnBnClickedButtonselectall)
	ON_BN_CLICKED( IDC_BUTTONUNSELECTALL, &DlgExportConfigFile::OnBnClickedButtonunselectall)
	ON_BN_CLICKED( IDC_BUTTONPROJECTINFO, &DlgExportConfigFile::OnBnClickedButtonprojectinfo)
	ON_BN_CLICKED( IDC_RADIOEXPORTALL, &DlgExportConfigFile::OnBnClickedRadioexportall)
	ON_BN_CLICKED( IDC_RADIOEXPORTSELECTEDCONFIGURATIONONLY, &DlgExportConfigFile::OnBnClickedRadioexportselectedconfigurationonly)
	ON_NOTIFY(NM_CLICK, IDC_LISTHUB, OnClickList)
	ON_WM_PAINT()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTHUB, &DlgExportConfigFile::OnLvnItemchangedListhub)
END_MESSAGE_MAP()


// DlgExportConfigFile message handlers

BOOL DlgExportConfigFile::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_pTADS = TASApp.GetpTADS();
	m_pTADB = TASApp.GetpTADB();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_TITLE );
	SetWindowText( str );

	// Iradio button state.
	( (CButton *)GetDlgItem( IDC_RADIOEXPORTALL ) )->SetCheck( 1 );
	
	// Init string.
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_EXPORTSELECTEDCONFIGONLY );
	( (CButton *)GetDlgItem( IDC_RADIOEXPORTSELECTEDCONFIGURATIONONLY ) )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_EXPORTALL );
	( (CButton *)GetDlgItem( IDC_RADIOEXPORTALL ) )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_PROJECTNAME ); 
	( (CStatic *)GetDlgItem( IDC_STATICPROJECTNAME ) )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_PROJECTINFO );
	m_GroupProjInfo.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_CUSTOMERNAME );
	( (CStatic *)GetDlgItem( IDC_STATICCUSTOMERNAME ) )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_STATICSELECTEDFOREXPORT );
	m_GroupSelForExp.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_EXPORT);
	( (CStatic *)GetDlgItem( IDOK ) )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );


	CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();
	ASSERT( NULL != pPrjRef );

	SetDlgItemText( IDC_EDITPROJECTNAME, pPrjRef->GetString( CDS_ProjectRef::Name ) );
	SetDlgItemText( IDC_EDITCUSTOMERNAME, pPrjRef->GetString( CDS_ProjectRef::CustName ) );

	m_ToolTip.Create( this, TTS_NOPREFIX );
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTSELECT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONSELECTALL ), TTstr );
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTUNSELECT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONUNSELECTALL ), TTstr );
	TTstr = TASApp.LoadLocalizedString( IDS_TTMODIFY_PRJREF );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONPROJECTINFO), TTstr);

	// Init images.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT ( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonProjectInfo.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_EditPrjInfo ) );
		m_ButtonSelectAll.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_SelectAll ) );
		m_ButtonUnselectAll.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_UnselectAll ) );
	}

	// Init list.
	m_ImageList.Create( IDB_CHECKEDUNCHECKED, 16, 1, _BLACK );
	m_ListHub.SetImageList( &m_ImageList, LVSIL_STATE );
	m_ListHub.DeleteAllItems();

	CDC *pDC = ( (CWnd *)this )->GetDC();

	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_COLUMNLABELID );
	
	int iWidthDesc, iWidthQuant, iWidthNumber;
	CSize size = pDC->GetTextExtent( str );
	m_ListHub.InsertColumn( 0, str, LVCFMT_LEFT, size.cx, 0 );

	iWidthDesc = size.cx; 
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_COLUMNDESCRIPTION );
	size = pDC->GetTextExtent( str );
	m_ListHub.InsertColumn( 1, str, LVCFMT_LEFT, size.cx, 1 );
	
	iWidthQuant = size.cx;
	str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_COLUMNNUMBEROFSTATION );
	size = pDC->GetTextExtent( str );
	m_ListHub.InsertColumn( 2, str, LVCFMT_LEFT, size.cx, 2 );
	
	iWidthNumber= size.cx;
	CStatic *pclStatic = ( (CStatic *)GetDlgItem( IDC_LISTHUB ) );
	CRect rect;
	pclStatic->GetWindowRect( &rect );
	int iWidthCtrl = rect.Width() - 4;

	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	( (CWnd *)this )->ReleaseDC( pDC );
		
	double dRatio = iWidthCtrl * 1.0 / ( iWidthDesc + iWidthQuant + iWidthNumber );
	m_ListHub.SetColumnWidth( 0, (int)( iWidthDesc  * dRatio ) );
	m_ListHub.SetColumnWidth( 1, (int)( iWidthQuant * dRatio ) );
	m_ListHub.SetColumnWidth( 2, (int)( iWidthNumber* dRatio ) );

	CTable *pTab = m_pTADS->GetpHUBSelectionTable();
	ASSERT( NULL != pTab );

	// Add all hub into the list
	LVITEM	lvItem;	
	lvItem.mask = LVIF_STATE | LVIF_PARAM | LVIF_TEXT; 
	lvItem.iItem = 0; 
	lvItem.iSubItem = 0; 
	lvItem.state = 0; 
	lvItem.stateMask = 0; 
	lvItem.pszText = NULL; 
	lvItem.cchTextMax = _TABLE_NAME_LENGTH; 
	lvItem.iImage = 0; 
	lvItem.lParam = NULL;
	lvItem.iIndent = 1;

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDS_HmHub *pHub = (CDS_HmHub *)( IDPtr.MP );
		lvItem.pszText = (TCHAR*)(LPCTSTR)pHub->GetHMName();
		lvItem.lParam = (LPARAM) pHub;
		m_ListHub.InsertItem( &lvItem );
		m_ListHub.SetItem( 0, 0, LVIF_TEXT, pHub->GetHMName(), 0, 0, 0, 0 );

		str.Format( _T("%s"), pHub->GetDescription( ) );
		m_ListHub.SetItem( 0, 1, LVIF_TEXT, str, 0, 0, 0, 0 );

		str.Format(_T("%d"),pHub->GetNumberOfStations( ) );
		m_ListHub.SetItem( 0, 2, LVIF_TEXT, str, 0, 0, 0, 0 );
	}
	
	// Set the check state outside the creation loop otherwise the first list item get no state image
	OnBnClickedRadioexportall();
	FormatStaticSelectedForExport();
	UpdateEnableOKButton();	
	
	return TRUE;
}

void DlgExportConfigFile::OnBnClickedOk()
{

	CString PrjDir = GetProjectDirectory();
	CString str;
#ifdef HUBZIP
	CString strFilter;	strFilter=TASApp.LoadLocalizedString( IDS_HUBZIPSAVEFILEFILTER);
	CFileDialog dlg(false,_T("zip"),(LPCTSTR)str,OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,(LPCTSTR)strFilter, NULL );
#else
	CString strFilter;	strFilter=TASApp.LoadLocalizedString( IDS_HUBSAVEFILEFILTER);
	CFileDialog dlg(false,_T("xml"),(LPCTSTR)str,OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,(LPCTSTR)strFilter, NULL );
#endif
	dlg.m_ofn.lpstrInitialDir=(LPCTSTR)PrjDir;
	if (dlg.DoModal()==IDOK)
	{
		BeginWaitCursor();
		//Filename Exist
		try
		{
			CString Filename=dlg.GetPathName();
			CPath path(Filename);
			CString XmlFile = path.SplitPath((CPath::ePathFields)(CPath::ePathFields::epfDrive+CPath::ePathFields::epfDir+CPath::ePathFields::epfFName))+_T(".xml");
			CFile outf((TCHAR*)(LPCTSTR)XmlFile, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
			GenerateXML(outf);
			outf.Close();
#ifdef HUBZIP
			CString str = path.SplitPath((CPath::ePathFields)(CPath::ePathFields::epfDrive+CPath::ePathFields::epfDir+CPath::ePathFields::epfFName))+_T(".zip");
			CZipArchive zip;
			zip.Open(str, CZipArchive::zipCreate);
			zip.AddNewFile(XmlFile,-1,false);
			zip.Close();
			// remove uncompressed original
			RemoveFile(XmlFile);
#endif
#ifdef HUBIMAGE
			WriteJpgFiles(Filename);
#endif
			//Delete Filename...
		}
		catch(...)
		{
		}
		EndWaitCursor();
		OnOK();
	}
}

void DlgExportConfigFile::OnBnClickedCancel()
{
	OnCancel();
}

void DlgExportConfigFile::OnBnClickedButtonselectall()
{
	for (int i=0; i<m_ListHub.GetItemCount(); i++)
		m_ListHub.SetCheck(i,TRUE);
	UpdateEnableOKButton();
}

void DlgExportConfigFile::OnBnClickedButtonunselectall()
{
	for (int i=0; i<m_ListHub.GetItemCount(); i++)
		m_ListHub.SetCheck(i,FALSE);
	UpdateEnableOKButton();
}

void DlgExportConfigFile::OnBnClickedButtonprojectinfo()
{
	CDlgRef dlg;
	dlg.Display();
	SetDlgItemText( IDC_EDITPROJECTNAME, m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name ) );
	SetDlgItemText( IDC_EDITCUSTOMERNAME, m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::CustName ) );
}

void DlgExportConfigFile::OnBnClickedRadioexportall()
{
	m_ButtonSelectAll.EnableWindow( FALSE );
	m_ButtonUnselectAll.EnableWindow( FALSE );
	OnBnClickedButtonselectall();
}

void DlgExportConfigFile::OnBnClickedRadioexportselectedconfigurationonly()
{
	m_ButtonSelectAll.EnableWindow( TRUE );
	m_ButtonUnselectAll.EnableWindow( TRUE );
	UpdateEnableOKButton();
}

void DlgExportConfigFile::FormatStaticSelectedForExport()
{
	// number of different configurations selected
	int iNumOfConfig = 0;
	// total number of Hubs
	int iTotal = m_ListHub.GetItemCount();
	for (int i=0; i<iTotal; i++)
	{
		if (m_ListHub.GetCheck(i))
			iNumOfConfig++;
	}
	// Initialise m_StaticSelectedHubs
	CString str,strTotal, strNumOfConfig;
	strTotal.Format(_T("%d"),iTotal);
	strNumOfConfig.Format(_T("%d"),iNumOfConfig);
	if (iTotal>1)
	{
		if (iNumOfConfig>1)
			FormatString(str,IDS_DLGEXPCONFFILE_STATICSELECTEDHUBS_NM,strNumOfConfig,strTotal);
		else
			FormatString(str,IDS_DLGEXPCONFFILE_STATICSELECTEDHUBS_1N,strTotal);
	}
	else 
	if (iTotal == 1 && iNumOfConfig==1)
		str = TASApp.LoadLocalizedString( IDS_DLGEXPCONFFILE_STATICSELECTEDHUBS_11);

	m_StaticSelectedHubs.SetWindowText( str );
}

void DlgExportConfigFile::UpdateEnableOKButton()
{
	FormatStaticSelectedForExport();
	((CButton *)GetDlgItem(IDOK))->EnableWindow(false);
	for (int i=0; i<m_ListHub.GetItemCount(); i++)
		if (m_ListHub.GetCheck(i))
	{
		((CButton *)GetDlgItem(IDOK))->EnableWindow(true);
		break;
	}	
}

void DlgExportConfigFile::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Retrieve where the user clicked
	LVHITTESTINFO	ht={0};
	DWORD dwpos = GetMessagePos();
	ht.pt.x= GET_X_LPARAM(dwpos);
	ht.pt.y= GET_Y_LPARAM(dwpos);

	::MapWindowPoints(HWND_DESKTOP, pNMHDR->hwndFrom,&ht.pt,1);
	m_ListHub.HitTest(&ht);

	// If the user click on the State Image, change selected to unselected and inversely
	CRect Rect,LabelRect;
	m_ListHub.GetItemRect( ht.iItem, &Rect, LVIR_BOUNDS);
	m_ListHub.GetItemRect( ht.iItem, &LabelRect, LVIR_LABEL);
	Rect.right = LabelRect.left-1;

	if ((!((CButton *)GetDlgItem( IDC_RADIOEXPORTALL))->GetCheck())&&(Rect.PtInRect(ht.pt)))
		m_ListHub.SetCheck(ht.iItem,!m_ListHub.GetCheck(ht.iItem ) );
	if (m_iSelectedRow != ht.iItem)
			SelectRowInTheList(ht.iItem);
	UpdateEnableOKButton();
	*pResult = 0;
}

void DlgExportConfigFile::SelectRowInTheList(int Row)
{
	m_iSelectedRow = Row;
	for (int i=0; i<m_ListHub.GetItemCount(); i++)
	{
		if (Row==i)
		{
			m_ListHub.SetItemState(Row,LVIS_SELECTED, LVIS_SELECTED);
			CDS_HmHub *pHub = (CDS_HmHub *)m_ListHub.GetItemData(Row);
			CHubHandler	mHubHandler;
			mHubHandler.Attach(pHub->GetIDPtr().ID);
			if (mHubHandler.BuildHubImg())
			{
				CRect Rect;
				m_Img.GetWindowRect(&Rect);
				CSize sizeImg(Rect.Width()-4, Rect.Height()-4);
				mHubHandler.GetHubImg()->ResizeImage(sizeImg);
				if (m_BmpHub.CopyImage(mHubHandler.GetHubImg()))
				{
					m_Img.SetBitmap(*mHubHandler.GetHubImg( ) );
					( (CStatic *)GetDlgItem( IDC_STATICIMG))->SetBitmap(*mHubHandler.GetHubImg( ) );
				}
			}	
		}
		else
		{
			m_ListHub.SetItemState(i,!LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}

void DlgExportConfigFile::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// select the first one in the list when the window is diplayed for the first time.
	// otherwise refresh
	if (m_iSelectedRow==-1)
		SelectRowInTheList(0);
	else
		SelectRowInTheList(m_iSelectedRow);
}

void DlgExportConfigFile::OnLvnItemchangedListhub(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if ((m_iSelectedRow!=-1)&&(pNMLV->uNewState == 3)&&(m_iSelectedRow != pNMLV->iItem))
		SelectRowInTheList(pNMLV->iItem);
	*pResult = 0;
}


void DlgExportConfigFile::WriteCString(CFile& outf, LPCTSTR string)
{
	ASSERT(string);
	// UTF-8 always use ASCII characters set
	CString strw= string;
	CStringA stra = CStringA(strw);
	outf.Write(stra.GetBuffer(), stra.GetLength( ) );
}

void DlgExportConfigFile::GenerateXML(CFile& outf)
{
	WriteCString(outf,_T("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n" ) );
	WriteCString(outf,_T("<?xml-stylesheet type=\"text/xsl\" href=\"" ) );
	WriteCString(outf,XSL_FILENAME);
	WriteCString(outf,_T("\"?>\r\n" ) );
	
	WriteCString(outf,_T("<actualConfiguration>\r\n" ) );

	WriteXmlHeadElt(outf);
	WriteXmlOrderLineElts(outf);

	WriteCString(outf,_T("</actualConfiguration>\r\n" ) );
}

void DlgExportConfigFile::WriteXmlHeadElt( CFile &outf )
{
	CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();
	ASSERT( NULL != pPrjRef );
	
	CString strProjectName = pPrjRef->GetString( CDS_ProjectRef::Name );
	CString strCustomerName = pPrjRef->GetString( CDS_ProjectRef::CustName );
	CString strCurrency = _T("-"); 
	
	if( true == TASApp.IsHubPriceUsed() )
	{
		CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

		CDB_Currency *pCur = (CDB_Currency *)TASApp.GetpTADB()->Get(pTech->GetCurrencyID()).MP;
		ASSERT( NULL != pCur );

		strCurrency =pCur->GetTAIS();
	}

	WriteCString(outf,_T("<head>\r\n" ) );
	
	if( true == strProjectName.IsEmpty() )
	{
		strProjectName = _T("-");
	}
	
	if( true == strCustomerName.IsEmpty() )
	{
		strCustomerName = _T("-");
	}
	
	if( true == strCurrency.IsEmpty() )
	{
		strCurrency = _T("-");
	}
	
	WriteCString(outf,_T("<projectName>") + strProjectName +  _T("</projectName>\r\n" ) ); 
	WriteCString(outf,_T("<customerName>") + strCustomerName + _T("</customerName>\r\n" ) );
	WriteCString(outf,_T("<currency>") + strCurrency + _T("</currency>\r\n" ) );
	WriteCString(outf,_T("</head>\r\n" ) );
}

void DlgExportConfigFile::WriteXmlOrderLineElts(CFile& outf)
{
	int i=0;
	for (int pos=0; pos<m_ListHub.GetItemCount(); pos++)
	{
		if (m_ListHub.GetCheck(pos))
		{
			i++;
			CDS_HmHub *pHmHub = ((CDS_HmHub*)m_ListHub.GetItemData(pos ) );
			CString str;
			str.Format(_T("%d"),i);
			WriteCString(outf,_T("<orderLine id=\"") + str + _T("\">" ) );

			WriteCString(outf,_T("<modelPartNumber>" ) );
			WriteCString(outf,XML_MODELPARTNUMBER); 
			WriteCString(outf,_T("</modelPartNumber>\r\n" ) );

			WriteCString(outf,_T("<revision>" ) );
			WriteCString(outf,XML_REVISION);
			WriteCString(outf,_T("</revision>\r\n" ) );

			if (!pHmHub->GetDescription().IsEmpty())
				WriteCString(outf,_T("<orderLineDescription>") + pHmHub->GetDescription() + _T("</orderLineDescription>\r\n" ) ); 
			else
				WriteCString(outf,_T("<orderLineDescription>-</orderLineDescription>\r\n" ) ); 

			WriteCString(outf,_T("<orderLineProductID>") + pHmHub->GetHMName() + _T("</orderLineProductID>\r\n" ) );
#ifdef HUBZIP
			CPath path(outf.GetFileName( ) );
			CString image = path.SplitPath((CPath::ePathFields)CPath::ePathFields::epfFName)+_T("-")+pHmHub->GetHMName()+_T(".jpg");
			WriteCString(outf,_T("<image>") + image + _T("</image>\r\n" ) );
#endif
			str.Format(_T("%d"),pHmHub->GetpSelectedInfos()->GetQuantity( ) );
			WriteCString(outf,_T("<quantity>") + str + _T("</quantity>\r\n" ) );

			str = _T("-");
			if (TASApp.IsHubPriceUsed())
			{
				if (pHmHub->GetpSelectedInfos()->GetPrice() != 0)
					str = WriteDouble(pHmHub->GetpSelectedInfos()->GetPrice(),2,0,1);
			}
			WriteCString(outf,_T("<orderLinePrice>") + str + _T("</orderLinePrice>\r\n" ) );

			WriteXmlConfigurationElt(outf,pHmHub);

			WriteCString(outf,_T("</orderLine>\r\n" ) );
		}
	}	
}

void DlgExportConfigFile::WriteXmlConfigurationElt(CFile& outf,CDS_HmHub *pHmHub)
{
	WriteCString(outf,_T("<configuration>\r\n" ) );

	//------------------------
	// General characteristics
	if (pHmHub->GetApplication() == CDS_HmHub::Application::Cooling) 
		WriteXmlConfElt(outf,_T("HUB_APPLICATION"),_T("COOLING" ) );
	else
		WriteXmlConfElt(outf,_T("HUB_APPLICATION"),_T("HEATING" ) );

	WriteXmlConfElt(outf,_T("HUB_BYPASS"),_T("1" ) );
		
	WriteXmlConfElt(outf,_T("HUB_CASING"),_T("1" ) );

	IDPTR IDPtr = pHmHub->GetStationsConnectIDPtr();
	CDB_MultiStringExt *pMS = (CDB_MultiStringExt *)IDPtr.MP;
	WriteXmlConfElt(outf,_T("HUB_CONNECTION"),pMS->GetString(CDS_HmHub::ExtConn::eTAISConnType ) ); 

	CString BalType = pHmHub->GetBalTypeID();
	if (BalType.Compare(_T("DPCTYPE_STD")))
		WriteXmlConfElt(outf,_T("HUB_DP_MEASURING_SIDE"),_T("N/A" ) ); 
	else	// DpC
	{
		ASSERT(pHmHub->GetpDpC( ) );
		if (pHmHub->GetpDpC() && pHmHub->GetpDpC()->GetMvLoc() == eMvLoc::MvLocSecondary)
			WriteXmlConfElt(outf,_T("HUB_DP_MEASURING_SIDE"),_T("SECONDARY" ) ); 
		else
			WriteXmlConfElt(outf,_T("HUB_DP_MEASURING_SIDE"),_T("PRIMARY" ) ); 
	}
	WriteXmlConfElt(outf,_T("HUB_END_OPTION"),_T("DRAIN" ) );

	WriteXmlConfElt(outf,_T("HUB_ID_NR"),pHmHub->GetHMName( ) ); 
	if (pHmHub->GetMainFeedPos() == CDS_HmHub::MainFeedPos::Left)
		WriteXmlConfElt(outf,_T("HUB_MAIN_FEED"),_T("LEFT" ) );
	else
		WriteXmlConfElt(outf,_T("HUB_MAIN_FEED"),_T("RIGHT" ) );

	CString str; str.Format(_T("%d"),pHmHub->GetNumberOfStations( ) );
	WriteXmlConfElt(outf,_T("HUB_NR_STATIONS"),str);

	str=_T("-");
	if (pHmHub->GetQ()) 
		str = WriteCUDouble(_U_FLOW,pHmHub->GetQ(),true);
	WriteXmlConfElt(outf,_T("HUB_TOTAL_FLOW"),str); 

	//--------------------------------------
	// Partner valve related characteristics
	IDPTR SupplyValveIDPtr = m_pTADB->Get(pHmHub->GetSupplyValveID( ) );
	IDPTR ReturnValveIDPtr = m_pTADB->Get(pHmHub->GetReturnValveID( ) );

	IDPtr = pHmHub->GetPartnerConnectIDPtr();
	pMS = (CDB_MultiStringExt *)IDPtr.MP;
	WriteXmlConfElt(outf,_T("HUB_PARTNER_CONNECTION"),pMS->GetString(CDS_HmHub::ExtConn::eTAISConnType ) ); 

	WriteXmlConfElt(outf,_T("HUB_PARTNER_SUPPLY"),((CDB_HubValv *) SupplyValveIDPtr.MP )->GetTAISPartner( ) );
	WriteXmlConfElt(outf,_T("HUB_PARTNER_RETURN"),((CDB_HubValv *) ReturnValveIDPtr.MP )->GetTAISPartner( ) );
	WriteXmlConfElt(outf,_T("HUB_PARTNER_SIZE"),((CDB_HubValv *) ReturnValveIDPtr.MP )->GetTAISSize( ) ); 

	if (BalType.Compare(_T("DPCTYPE_STD")))
		WriteXmlConfElt(outf,_T("HUB_PARTNER_DP_RANGE"),_T("N/A" ) );
	else
		WriteXmlConfElt(outf,_T("HUB_PARTNER_DP_RANGE"),((CDB_HubValv *) ReturnValveIDPtr.MP )->GetTAISDPRange( ) );
	
	//------------------------
	// Station characteristics
	CHubHandler HubHandler;
	HubHandler.Attach(pHmHub->GetIDPtr().ID);
	int iCount = pHmHub->GetCount();
	int i;
	for (int i=0;i<iCount;i++)
	{
		CString strTmp;
		CString strI; strI.Format(_T("%d"),i+1);
		IDPTR IDPtr = HubHandler.GetStationIDPTR(i+1);
		CDS_HmHubStation *currentStation= (CDS_HmHubStation *)IDPtr.MP;
		if (!currentStation->IsPlugged() && *IDPtr.ID)
		{
			IDPTR IDPtrStation = m_pTADB->Get(currentStation->GetSupplyValveID( ) );
			if (*IDPtrStation.ID)
			{
				FormatString(str,_T("HUB_%1_FLOW"),strI);
				WriteXmlConfElt(outf,str,WriteCUDouble(_U_FLOW,currentStation->GetQ(), true ) );
				FormatString(str,_T("HUB_%1_TERMINAL_ID"),strI);
				if (currentStation->GetHMName().IsEmpty())
					WriteXmlConfElt(outf,str,_T("-" ) );
				else
					WriteXmlConfElt(outf,str,currentStation->GetHMName( ) );
				FormatString(str,_T("HUB_SUPPLY_%1_TYPE"),strI);
				WriteXmlConfElt(outf,str,((CDB_HubStaValv *)IDPtrStation.MP)->GetTAISValve( ) );
				FormatString(str,_T("HUB_SUPPLY_%1_SIZE"),strI);
				WriteXmlConfElt(outf,str,((CDB_HubStaValv *)IDPtrStation.MP)->GetTAISSize( ) );
			}
			else
				ASSERT(0);

			IDPtrStation = m_pTADB->Get(currentStation->GetReturnValveID( ) );
			if (*IDPtrStation.ID)
			{
				FormatString(str,_T("HUB_RETURN_%1_TYPE"),strI);
				WriteXmlConfElt(outf,str,((CDB_HubStaValv *)IDPtrStation.MP)->GetTAISValve() );
				FormatString(str,_T("HUB_RETURN_%1_SIZE"),strI);
				WriteXmlConfElt(outf,str,((CDB_HubStaValv *)IDPtrStation.MP)->GetTAISSize() );
			}
			else
				ASSERT(0);
			CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>(m_pTADB->Get(_T("ACTUATOR_NONE")).MP);ASSERT(pStrID);
			CString strNoActuator = _T("0");//_T("NO_ACTUATOR");
			if (pStrID) strNoActuator = pStrID->GetIDstr();
			IDPTR IDPtrActr = m_pTADB->Get(currentStation->GetReturnActuatorID( ) );
			bool bFound = false;
			if (*IDPtrActr.ID)
			{	// Retrieve Actuator group tab and TAIS figure
				CTable *pTab = (CTable *)m_pTADB->Get(_T("HUBACTUATOR_TAB")).MP;
				for (IDPTR IDPtr=pTab->GetFirst(); *IDPtr.ID; IDPtr=pTab->GetNext())
				{
					pStrID = dynamic_cast<CDB_StringID *>(IDPtr.MP);
					if (pStrID)
					{
						CString str = pStrID->GetIDstr2();
						if (!str.IsEmpty())
						{
							CTable *pTabActrG = (CTable *)m_pTADB->Get( str ).MP;
							if (pTabActrG)
							{
								if (pTabActrG->Get(IDPtrActr.ID).MP)
								{	// Found, Take TAIS code
									FormatString(str,_T("HUB_RETURN_%1_ACTUATOR"),strI);
									WriteXmlConfElt(outf,str,pStrID->GetIDstr( ) );
									bFound = true;
									break;
								}
							}
						}
					}
				}
				ASSERT(bFound);
			}
			if (!bFound)
			{
				FormatString(str,_T("HUB_RETURN_%1_ACTUATOR"),strI);
				WriteXmlConfElt(outf,str,strNoActuator);
			}
		}
		else
		{	// Station Plugged
			CString strI; strI.Format(_T("%d"),i+1);
			FormatString(str,_T("HUB_%1_FLOW"),strI);
			WriteXmlConfElt(outf,str,WriteCUDouble(_U_FLOW,currentStation->GetQ(), true ) );
			FormatString(str,_T("HUB_%1_TERMINAL_ID"),strI);
			if (currentStation->GetHMName().IsEmpty())
				WriteXmlConfElt(outf,str,_T("-" ) );
			else
				WriteXmlConfElt(outf,str,currentStation->GetHMName( ) );
			FormatString(str,_T("HUB_SUPPLY_%1_TYPE"),strI);
			WriteXmlConfElt(outf,str,_T("PLUG" ) ); 
			FormatString(str,_T("HUB_SUPPLY_%1_SIZE"),strI);
			WriteXmlConfElt(outf,str,_T("-" ) );
			FormatString(str,_T("HUB_RETURN_%1_TYPE"),strI);
			WriteXmlConfElt(outf,str,_T("PLUG" ) );
			FormatString(str,_T("HUB_RETURN_%1_SIZE"),strI);
			WriteXmlConfElt(outf,str,_T("-" ) );
			FormatString(str,_T("HUB_RETURN_%1_ACTUATOR"),strI);
			WriteXmlConfElt(outf,str,_T("0" ) );
		}
	}
	// Plug station to fill empty slots
	for(i=iCount; i<(iCount+(int)HubHandler.GetNumberOfEmptySlots( ) ); i++)
	{
		CString strI; strI.Format(_T("%d"),i+1);
		FormatString(str,_T("HUB_%1_FLOW"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) ); 
		FormatString(str,_T("HUB_%1_TERMINAL_ID"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) );
		FormatString(str,_T("HUB_SUPPLY_%1_TYPE"),strI);
		WriteXmlConfElt(outf,str,_T("PLUG" ) ); 
		FormatString(str,_T("HUB_SUPPLY_%1_SIZE"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) );
		FormatString(str,_T("HUB_RETURN_%1_TYPE"),strI);
		WriteXmlConfElt(outf,str,_T("PLUG" ) );
		FormatString(str,_T("HUB_RETURN_%1_SIZE"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) );
		FormatString(str,_T("HUB_RETURN_%1_ACTUATOR"),strI);
		WriteXmlConfElt(outf,str,_T("0" ) );
	}
	// Fill to obtain 8 stations
	int iFill = iCount + HubHandler.GetNumberOfEmptySlots();
	// Dash 
	for(i=iFill; i<8; i++)
	{
		CString strI; strI.Format(_T("%d"),i+1);
		FormatString(str,_T("HUB_%1_FLOW"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) ); 
		FormatString(str,_T("HUB_%1_TERMINAL_ID"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) );
		FormatString(str,_T("HUB_SUPPLY_%1_TYPE"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) ); 
		FormatString(str,_T("HUB_SUPPLY_%1_SIZE"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) );
		FormatString(str,_T("HUB_RETURN_%1_TYPE"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) ); 
		FormatString(str,_T("HUB_RETURN_%1_SIZE"),strI);
		WriteXmlConfElt(outf,str,_T("-" ) );
		FormatString(str,_T("HUB_RETURN_%1_ACTUATOR"),strI);
		WriteXmlConfElt(outf,str,_T("0" ) );
	}

	WriteCString(outf,_T("</configuration>\r\n" ) );
}

void DlgExportConfigFile::WriteXmlConfElt(CFile& outf,CString cIDAttr,CString vIDAttr)
{
	WriteCString(outf,_T("<conf " ) );
	WriteCString(outf,_T("charId=\"" ) );
	WriteCString(outf,cIDAttr);
	WriteCString(outf,_T("\" valueId=\"" ) );
	WriteCString(outf,vIDAttr);
	WriteCString(outf,_T("\"/>\r\n" ) );
}

void DlgExportConfigFile::WriteJpgFiles(CString filename)
{
	int i=0;
	CPath path(filename);
	CString file = path.SplitPath((CPath::ePathFields)(CPath::ePathFields::epfDrive+CPath::ePathFields::epfDir+CPath::ePathFields::epfFName ) );
#ifdef HUBZIP
	// Zip Library already loaded
	CZipArchive zip;
	CString ZipFile = file + _T(".zip");
	zip.Open(ZipFile, CZipArchive::zipOpen);
#endif
	for (int pos=0; pos<m_ListHub.GetItemCount(); pos++)
	{
		if (m_ListHub.GetCheck(pos))
		{
			i++;
			CString strI; strI.Format(_T("%d"),i);
			CDS_HmHub *pHmHub = ((CDS_HmHub*)m_ListHub.GetItemData(pos ) );
			CHubHandler HubHandler;
			HubHandler.Attach(pHmHub->GetIDPtr().ID);
			HubHandler.BuildHubImg();
			CEnBitmap *pBmp=HubHandler.GetHubImg();
			
			//CString jpgfile = file+strI+_T(".jpg");
			CString jpgfile = file+_T("-")+pHmHub->GetHMName()+_T(".jpg");
			pBmp->SaveImageToFile(jpgfile);
#ifdef HUBZIP
			zip.AddNewFile(jpgfile,-1,false);
			RemoveFile(jpgfile);
#endif
		}
	}
#ifdef HUBZIP
	CFileStatus fs;
	CString xslfile = CString(_DATA_DIRECTORY) + CString(XSL_FILENAME);
	if (CFile::GetStatus(xslfile, fs))
	{
		zip.AddNewFile(xslfile,-1,false);
	}
	zip.Close();
#endif
}