#include "stdafx.h"


#include "mainfrm.h"
#include "taselectdoc.h"
#include "DlgSpecialActionExtractArticleWithoutPrice.h"

CDlgSpecialActionExtractArticleWithoutPrice::CDlgSpecialActionExtractArticleWithoutPrice( CWnd *pParent )
	: CDialogExt( CDlgSpecialActionExtractArticleWithoutPrice::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
	m_strFileName = _T("");
}

BEGIN_MESSAGE_MAP( CDlgSpecialActionExtractArticleWithoutPrice, CDialogExt )
	ON_BN_CLICKED( IDC_BUTTONFILETOSAVE, &OnBnClickedOpenFileToSave )
	ON_BN_CLICKED( IDC_BUTTONEXTRACT, &OnBnClickedExtract )
END_MESSAGE_MAP()

void CDlgSpecialActionExtractArticleWithoutPrice::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFILETOSAVE, m_EditFileToSave );
	DDX_Control( pDX, IDC_LIST, m_List );
}

BOOL CDlgSpecialActionExtractArticleWithoutPrice::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	m_strFileName = TASApp.GetProfileString( _T("Testing"), _T("ExtractArticleWithoutPrice"), _T("") );
	m_EditFileToSave.SetWindowText( m_strFileName );

	m_List.ModifyStyle( LBS_SORT, 0 );

	return TRUE;
}

void CDlgSpecialActionExtractArticleWithoutPrice::OnBnClickedOpenFileToSave()
{
    m_strFileName = _T("");
	CString strFilter = _T("Text files (*.txt)|*.txt|All Files (*.*)|*.*||");
	CFileDialog dlg( FALSE, _T(".txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, strFilter );

	if( IDOK == dlg.DoModal() )
	{
		m_strFileName = dlg.GetPathName();
		GetDlgItem( IDC_EDITFILETOSAVE )->SetWindowText( m_strFileName );

		TASApp.WriteProfileString( _T("Testing"), _T("ExtractArticleWithoutPrice"), m_strFileName );
	}
}

void CDlgSpecialActionExtractArticleWithoutPrice::OnBnClickedExtract()
{
	if( true == m_strFileName.IsEmpty() )
	{
		MessageBox( _T("File where to save must be defined."), _T("Error"), MB_OK );
		return;
	}

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Start to extract...") ) );

	CString str;

	CFile clFile( m_strFileName, CFile::modeWrite | CFile::modeCreate );

	if( CFile::hFileNull == clFile.m_hFile )
	{
		str.Format( _T("Can't open the file '%s'."), (LPCTSTR)m_strFileName );
		MessageBox( str, _T("Error"), MB_OK );
		return;
	}

	CDataList *pDL = TASApp.GetpTADB()->GetpDataList();

	if( NULL == pDL )
	{
		MessageBox( _T("Can't open the datalist."), _T("Error"), MB_OK );
		return;
	}

	USES_CONVERSION;
	CTADatabase *pclTADatabase = TASApp.GetpTADB();

	str = _T("ID|Article number|Name|Description|Type|Family|Version|Body material|Connection type|Kvs\n");
	clFile.Write( T2A( str.GetBuffer() ), str.GetLength() );

	for( IDPTR IDPtr = pDL->GetFirst(); IDPtr.MP != NULL; IDPtr = pDL->GetNext() )
	{
		CData *pclData = (CData *)( IDPtr.MP );

		if( false == pclData->IsSelectable() )
		{
			continue;
		}

		CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( IDPtr.MP );

		if( NULL == pclProduct )
		{
			// str.Format( _T("%s|not a product\n"), IDPtr.ID );
			// clFile.Write( T2A( str.GetBuffer() ), str.GetLength() );
			
			// str.Format( _T("%s -> not a product"), IDPtr.ID );
			// m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), str ) );
			continue;
		}

		CString strArticleNumber = pclProduct->GetArtNum();

		if( true == strArticleNumber.IsEmpty() )
		{
			str.Format( _T("%s|article number not defined\n"), IDPtr.ID );
			clFile.Write( T2A( str.GetBuffer() ), str.GetLength() );
			
			// str.Format( _T("%s -> article number not defined"), IDPtr.ID );
			// m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), str ) );
			continue;
		}

		double dPrice = pclTADatabase->GetPrice( strArticleNumber );

		if( dPrice <= 0.0 && true == TASApp.IsLocalArtNumberUsed() )
		{
			CString strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			strLocalArticleNumber.Trim();
			dPrice = pclTADatabase->GetPrice( strLocalArticleNumber );
		}

		if( dPrice <= 0.0 )
		{
			CString strName = pclProduct->GetName();
			CString strDescription = pclProduct->GetComment();
			CString strType = pclProduct->GetType();
			CString strFamily = pclProduct->GetFamily();
			CString strVersion = pclProduct->GetVersion();

			str.Format( _T("%s|%s|%s|%s|%s|%s|%s"), IDPtr.ID, (LPCTSTR)strArticleNumber, (LPCTSTR)strName, (LPCTSTR)strDescription, 
					(LPCTSTR)strType, (LPCTSTR)strFamily, (LPCTSTR)strVersion );

			if( NULL != dynamic_cast<CDB_TAProduct *>( pclProduct ) )
			{
				CDB_TAProduct *pclTAProduct = (CDB_TAProduct *)pclProduct;
				CString strBodyMaterial = pclTAProduct->GetBodyMaterial();
				CString strConnect = pclTAProduct->GetConnect();

				str.Format( _T("%s|%s|%s"), (LPCTSTR)str, (LPCTSTR)strBodyMaterial, (LPCTSTR)strConnect );

				if( NULL != dynamic_cast<CDB_ControlValve *>( pclProduct ) )
				{
					CDB_ControlValve *pclControlValve = (CDB_ControlValve *)pclProduct;
					CString strKvs = WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
					str.Format( _T("%s|%s"), (LPCTSTR)str, (LPCTSTR)strKvs );
				}
			}

			str += _T("\n");

			clFile.Write( T2A( str.GetBuffer() ), str.GetLength() );
			
			// str.Format( _T("%s -> price not defined"), IDPtr.ID );
			// m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), str ) );
			continue;
		}

		// str.Format( _T("%s -> price defined"), IDPtr.ID );
		// m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), str ) );
	}

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Finished.") ) );
}
