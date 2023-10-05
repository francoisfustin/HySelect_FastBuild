#include "stdafx.h"

#include <math.h>
#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"
#include "Global.h"
#include "utilities.h"
#include "DlgKvSeries.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgKvSeries::CDlgKvSeries( CWnd *pParent )
	: CDialogEx( CDlgKvSeries::IDD, pParent )
{
	m_pTADB = NULL;
	m_pUnitDB = NULL;
	m_pclDlgKvSeriesTabNomCond = NULL;
	m_pclDlgKvSeriesTabKvCv = NULL;
	m_pclDlgKvSeriesTabDzeta = NULL;
	m_pCurCDlg = NULL;
	m_CurrentDlg = eDlgType::NOMCOND;
}

int CDlgKvSeries::Display()
{
	m_pTADB = TASApp.GetpTADB();
	return DoModal();
}

void CDlgKvSeries::UpdateLstCtrl()
{
	// Scan all ListCtrl, find Kv introduced by q and Dp and compute the new value of Kv
	// with the new Rho
	double dq, dDp, dKv;
	int	j,k;

	// Get water characteristics ...rho
	double dRho = m_pclDlgKvSeriesTabNomCond->GetCurrentWaterChar().GetDens();

	for( int i = 0; i < m_List.GetItemCount(); i++ )
	{
		if( ( m_List.GetItemData( i ) >> 16 ) == 0 )		// Kv from q and Dp
		{
			CString str = m_List.GetItemText( i, 1 );				// Full string
			CString strq = str.Left( str.Find( _T(';') ) );			// q='value' units
			CString strDp = str.Right( str.Find( _T(';') ) );		// Dp='value' units

			// Extract q.
			j = strq.Find( _T('=') );
			strq.Delete( 0, j + 1 );
			j = strq.Find( _T(' ') );					// Index of space before units
			k = strq.GetLength();
			k -= j;										// char at the end will be removed
			strq.Delete( j, k );
			ReadDouble( strq, &dq );

			j = strDp.Find( _T('=') );
			strDp.Delete( 0, j + 1 );
			j = strDp.Find( _T(' ') );					// Index of space before units
			k = strDp.GetLength();
			k -= j;										// char at the end will be removed
			strDp.Delete( j, k );
			ReadDouble( strDp, &dDp );
			
			dq = CDimValue::CUtoSI( _U_FLOW, dq );		// TO SI
			dDp = CDimValue::CUtoSI( _U_DIFFPRESS, dDp );
			dKv = CalcKv( dq, dDp, dRho );				// Compute new Kv
			str.Format( WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, dKv), 4, 0, 1 ) );
			m_List.SetItemText(i,2,str);	// Update Ctrl List
		}
	}	

	_ComputeKvCv();		// Compute Sum
	UpdateCurrentKv();	
}

void CDlgKvSeries::UpdateCurrentKv() 
{
	CString	str, str1, str2;	
	double dKv, dRho, dDi;

	m_EditCurrentKvCv.SetWindowText( _T("") );

	// Determine which tab has been selected 
	// Get water characteristics ...rho 
	dRho = m_pclDlgKvSeriesTabNomCond->GetCurrentWaterChar().GetDens();
	UnitDesign_struct ud;	
	
	switch( m_TabCtrl.GetCurSel() )
	{
		// Nominal conditions.
		case 0:
		default:

			if( NULL == m_pclDlgKvSeriesTabNomCond )
			{
				return;
			}

			m_pclDlgKvSeriesTabNomCond->GetFlowText( str1 );

			if( true == str1.IsEmpty() )
			{
				return;
			}
			
			m_pclDlgKvSeriesTabNomCond->GetDpText( str1 );
			
			if( true == str1.IsEmpty() )
			{
				return;
			}
			
			// Dp and q must be > 0.0.
			if( m_pclDlgKvSeriesTabNomCond->GetFlowValue() <= 0.0 || m_pclDlgKvSeriesTabNomCond->GetDpValue() <= 0.0 )
			{
				return;
			}

			// Retrieve Kv from q and Dp.
			dKv = CalcKv( m_pclDlgKvSeriesTabNomCond->GetFlowValue(), m_pclDlgKvSeriesTabNomCond->GetDpValue(), dRho );
			break;

		// Kv.
		case 1:

			if( NULL == m_pclDlgKvSeriesTabKvCv )
			{
				return;
			}

			dKv = m_pclDlgKvSeriesTabKvCv->GetKvCvValue();

			if( dKv <= 0.0 )
			{
				return;
			}
			
			break;
		
		// Cv.
		case 2:
			
			if( NULL == m_pclDlgKvSeriesTabKvCv )
			{
				return;
			}

			dKv = m_pclDlgKvSeriesTabKvCv->GetKvCvValue();									// Convert Cv To Kv
			
			if( dKv <= 0.0 )
			{
				return;
			}
			
			ud = m_pUnitDB->GetUnit( _C_KVCVCOEFF, 1 );
			dKv = GetConvOf( ud ) * ( dKv + GetOffsetOf( ud ) );
			break;

		// Dzeta.
		case 3:

			if( NULL == m_pclDlgKvSeriesTabDzeta )
			{
				return;
			}

			if( m_pclDlgKvSeriesTabDzeta->GetDzetaValue() <= 0.0 )
			{
				return;
			}

			dDi = m_pclDlgKvSeriesTabDzeta->GetIntDiam();
			
			if( dDi <= 0.0 )
			{
				return;
			}

			dKv = CalcKvFromDzeta( m_pclDlgKvSeriesTabDzeta->GetDzetaValue(), dDi );
			break;
	};

	// Write Kv or Cv, convert to User Unit ....
	str.Format( WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, dKv ), 4, 0, 1 ) );
	m_EditCurrentKvCv.SetWindowText( str );
}

void CDlgKvSeries::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITCURRENTKVCV, m_EditCurrentKvCv );
	DDX_Control( pDX, IDC_STATICKVCV, m_StaticKvCv );
	DDX_Control( pDX, IDC_STATICTOTAL, m_StaticTotal );
	DDX_Control( pDX, IDC_STATICSUM, m_StaticSum );
	DDX_Control( pDX, IDOK, m_ButtonQuit );
	DDX_Control( pDX, IDC_BUTTONADD, m_ButtonAdd );
	DDX_Control( pDX, IDC_BUTTONREMOVE, m_ButtonRemove );
	DDX_Control( pDX, IDC_LIST, m_List );
	DDX_Control( pDX, IDC_TABCTRL, m_TabCtrl );
}

BEGIN_MESSAGE_MAP( CDlgKvSeries, CDialogEx )
	ON_NOTIFY( TCN_SELCHANGE, IDC_TABCTRL, OnSelChangeTabCtrl )
	ON_BN_CLICKED( IDC_BUTTONADD, OnButtonAdd )
	ON_NOTIFY( LVN_ENDLABELEDIT, IDC_LIST, OnEndLabelEditList )
	ON_BN_CLICKED( IDC_BUTTONREMOVE, OnButtonRemove )
	ON_NOTIFY( NM_CLICK, IDC_LIST, OnClickList )
	ON_BN_CLICKED( IDC_BUTTONPRINT, OnButtonPrint )
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CDlgKvSeries::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGKVSERIES_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGKVSERIES_STATICFROM );
	GetDlgItem( IDC_STATICFROM )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGKVSERIES_STATICTOTAL );
	GetDlgItem( IDC_STATICTOTAL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGKVSERIES_BUTTONADD );
	m_ButtonAdd.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGKVSERIES_BUTTONREMOVE );
	m_ButtonRemove.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGKVSERIES_OK );
	m_ButtonQuit.SetWindowText( str );

	// Disable button remove.
	m_ButtonRemove.EnableWindow( FALSE );
	
	// Init Unit Pointer.
	m_pUnitDB = CDimValue::AccessUDB();

	// Initialize Tab Control
	TC_ITEM tcinsert;
	tcinsert.mask = TCIF_TEXT;
	m_TabCtrl.SetMinTabWidth( 20 );
	
	str = TASApp.LoadLocalizedString( IDS_KVSERIES_TABDZETA );
	tcinsert.pszText = (TCHAR *)(LPCTSTR) str;
	m_TabCtrl.InsertItem( 0, &tcinsert );

	str = TASApp.LoadLocalizedString( IDS_KVSERIES_CV );
	tcinsert.pszText = (TCHAR *)(LPCTSTR) str;
	m_TabCtrl.InsertItem( 0, &tcinsert );

	str = TASApp.LoadLocalizedString( IDS_KVSERIES_KV );
	tcinsert.pszText = (TCHAR *)(LPCTSTR) str;
	m_TabCtrl.InsertItem( 0, &tcinsert );

	str = TASApp.LoadLocalizedString( IDS_KVSERIES_TABNC );
	tcinsert.pszText = (TCHAR *)(LPCTSTR) str;
	m_TabCtrl.InsertItem( 0, &tcinsert );

	// Create the child dialog.
	m_pclDlgKvSeriesTabNomCond = new CDlgKvSeriesTabNomCond();
	m_pclDlgKvSeriesTabNomCond->Create( IDD_DLGKVSERIESTABNOMCOND, GetDlgItem( IDC_TABCTRL ) );
	m_pclDlgKvSeriesTabNomCond->ShowWindow( SW_HIDE );

	m_pclDlgKvSeriesTabKvCv = new CDlgKvSeriesTabKvCv();
	m_pclDlgKvSeriesTabKvCv->Create( IDD_DLGKVSERIESTABKVCV, GetDlgItem( IDC_TABCTRL ) );	
	m_pclDlgKvSeriesTabKvCv->ShowWindow( SW_HIDE );

	m_pclDlgKvSeriesTabDzeta = new CDlgKvSeriesTabDzeta();
	m_pclDlgKvSeriesTabDzeta->Create( IDD_DLGKVSERIESTABDZETA, GetDlgItem( IDC_TABCTRL ) );
	m_pclDlgKvSeriesTabDzeta->ShowWindow( SW_HIDE );
	
	m_pCurCDlg = m_pclDlgKvSeriesTabNomCond;

	// Select Tab 0 : Nominal Condition
	m_TabCtrl.SetCurSel( 0 );
	OnSelChangeTabCtrl( NULL, NULL );

	// Initialize List Ctrl
	// Fill Column Name
	str = TASApp.LoadLocalizedString( IDS_KVSERIES_REFERENCE );
	m_List.InsertColumn( 0, str, LVCFMT_CENTER, 5 * 8, 0 );	//  5 char * 8 pixels

	str = TASApp.LoadLocalizedString( IDS_KVSERIES_VALUE );
	m_List.InsertColumn( 1, str, LVCFMT_CENTER, 20 * 8, 0 );	//  20 char * 8 pixels

	// Retrieve User default for KvCv and Initialize all static.
	if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_KVSERIES_KVTOTAL );
		m_StaticTotal.SetWindowText( str );

		str = TASApp.LoadLocalizedString( IDS_KVSERIES_CURRENTKV );
		m_StaticKvCv.SetWindowText( str );

		str = TASApp.LoadLocalizedString( IDS_KVSERIES_KV );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_KVSERIES_CVTOTAL );
		m_StaticTotal.SetWindowText( str );

		str = TASApp.LoadLocalizedString( IDS_KVSERIES_CURRENTCV );
		m_StaticKvCv.SetWindowText( str );

		str = TASApp.LoadLocalizedString( IDS_KVSERIES_CV );
	}

	m_List.InsertColumn( 2, str, LVCFMT_CENTER, 7 * 8, 0 );	// 56 => 7 char * 8 pixels

	// Get a full row selected.
	m_List.SetExtendedStyle( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

	// Set font for static SUM & TOTAL.
	VERIFY( m_font.CreateFont(
			16,							// nHeight
			0,							// nWidth
			0,							// nEscapement
			0,							// nOrientation
			FW_SEMIBOLD,				// nWeight
			FALSE,					    // bItalic
			FALSE,						// bUnderline
			0,							// cStrikeOut
			ANSI_CHARSET,				// nCharSet
			OUT_DEFAULT_PRECIS,			// nOutPrecision
			CLIP_DEFAULT_PRECIS,		// nClipPrecision
			DEFAULT_QUALITY,			// nQuality
			DEFAULT_PITCH | FF_SWISS,	// nPitchAndFamily
			_T("Arial") ) );            // lpszFacename

	m_StaticSum.SetFont( &m_font );
	m_StaticTotal.SetFont( &m_font );
	
	// Set total to ""	
	_ComputeKvCv();	
	return TRUE;
}

void CDlgKvSeries::OnDestroy() 
{
	CDialogEx::OnDestroy();
	
	m_font.DeleteObject();
}

void CDlgKvSeries::OnSelChangeTabCtrl( NMHDR *pNMHDR, LRESULT *pResult ) 
{
	if( NULL == m_pclDlgKvSeriesTabNomCond || NULL == m_pclDlgKvSeriesTabKvCv || NULL == m_pclDlgKvSeriesTabDzeta )
	{
		ASSERT_RETURN;
	}
	
	// Determine which tab has been selected and switch to the corresponding child dlg.
	CString	str;	

	switch( m_TabCtrl.GetCurSel() )
	{
		// Nominal conditions.
		case 0: 
		default:
			m_pclDlgKvSeriesTabNomCond->ResetAll();
			_DisplayTabCDlg( (CDialogExt *)( m_pclDlgKvSeriesTabNomCond ) );
			break;

		// Kv.
		case 1:
			m_pclDlgKvSeriesTabKvCv->ResetAll( true );
			_DisplayTabCDlg( (CDialogExt *)( m_pclDlgKvSeriesTabKvCv ) );
			break;

		// Cv.
		case 2:
			m_pclDlgKvSeriesTabKvCv->ResetAll( false );
			_DisplayTabCDlg( (CDialogExt *)( m_pclDlgKvSeriesTabKvCv ) );
			break;

		// Dzeta.
		case 3:
			m_pclDlgKvSeriesTabDzeta->ResetAll();
			_DisplayTabCDlg( (CDialogExt *)( m_pclDlgKvSeriesTabDzeta ) );
			break;
	};
	
	if( NULL != pResult )
	{
		*pResult = 0;
	}
}

void CDlgKvSeries::OnButtonAdd() 
{
	if( NULL == m_pclDlgKvSeriesTabNomCond || NULL == m_pclDlgKvSeriesTabKvCv || NULL == m_pclDlgKvSeriesTabDzeta )
	{
		ASSERT_RETURN;
	}

	CString	str, str1, str2;	
	TCHAR tcName[_MAXCHARS];
	double dKv, dRho, dDi;
	LPARAM lp;
	
	// Get water characteristics ...rho 
	dRho = m_pclDlgKvSeriesTabNomCond->GetCurrentWaterChar().GetDens();
	UnitDesign_struct ud;

	switch( m_TabCtrl.GetCurSel() )
	{
		// Nominal conditions.
		case 0:
		default:
			// Save type of data introduced
			lp = 0;
			m_pclDlgKvSeriesTabNomCond->GetFlowText( str1 );

			if( true == str1.IsEmpty() )
			{
				return;
			}

			m_pclDlgKvSeriesTabNomCond->GetDpText( str1 );

			if( true == str1.IsEmpty() )
			{
				return;
			}

			// Dp and q must be > 0.0.
			if( m_pclDlgKvSeriesTabNomCond->GetFlowValue() <= 0.0 || m_pclDlgKvSeriesTabNomCond->GetDpValue() <= 0.0 )
			{
				return;
			}

			// Retrieve Kv from q and Dp.
			dKv = CalcKv( m_pclDlgKvSeriesTabNomCond->GetFlowValue(), m_pclDlgKvSeriesTabNomCond->GetDpValue(), dRho );

			// Build a string with edit box values.
			GetNameOf( m_pUnitDB->GetUnit( _U_FLOW, m_pUnitDB->GetDefaultUnitIndex( _U_FLOW ) ), tcName );
			str1 = WriteDouble( CDimValue::SItoCU( _U_FLOW, m_pclDlgKvSeriesTabNomCond->GetFlowValue() ), 4, 0, true );
			str1 += _T(" ");
			str1 += tcName;
			
			GetNameOf( m_pUnitDB->GetUnit( _U_DIFFPRESS, m_pUnitDB->GetDefaultUnitIndex( _U_DIFFPRESS ) ), tcName );
			str2 = WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, m_pclDlgKvSeriesTabNomCond->GetDpValue() ), 4, 0, true );
			str2 += _T(" ");
			str2 += tcName;
			str.Format( IDS_KVSERIES_Q_DP, str1, str2 ); 
			break;

		// Kv.
		case 1:
			// Save type of data introduced in Msb.
			lp = 0x10000;
			dKv = m_pclDlgKvSeriesTabKvCv->GetKvCvValue();

			if( dKv <= 0.0 )
			{
				return;
			}

			// Build string with Kv value.
			str = TASApp.LoadLocalizedString( IDS_KVSERIES_KV );
			str += _T("=");
			str1 = WriteDouble( dKv, 4, 0, true );
			str += str1;
			break;

		// Cv.
		case 2:
			// Save type of data introduced in Msb.
			lp = 0x20000;

			// Convert Cv To Kv.
			dKv = m_pclDlgKvSeriesTabKvCv->GetKvCvValue();

			if( dKv <= 0.0 )
			{
				return;
			}
			
			ud = m_pUnitDB->GetUnit( _C_KVCVCOEFF, 1 );
			dKv = GetConvOf( ud ) * ( dKv + GetOffsetOf( ud ) );

			// Build string with Cv value.
			str = TASApp.LoadLocalizedString( IDS_KVSERIES_CV );
			str += _T("=");
			str1 = WriteDouble( m_pclDlgKvSeriesTabKvCv->GetKvCvValue(), 4, 0, true );
			str += str1;
			break;

		// Dzeta.
		case 3:
			// Save type of data introduced in Msb
			lp = 0x30000;

			if( m_pclDlgKvSeriesTabDzeta->GetDzetaValue() <= 0.0 )
			{
				return;
			}
			
			dDi = m_pclDlgKvSeriesTabDzeta->GetIntDiam();
			
			if( dDi <= 0.0 )
			{
				return;
			}
			
			dKv = CalcKvFromDzeta( m_pclDlgKvSeriesTabDzeta->GetDzetaValue(), dDi );

			// Build string with Cv value 
			str1 = WriteDouble( m_pclDlgKvSeriesTabDzeta->GetDzetaValue(), 4, 0, true );
			str2 = WriteDouble( CDimValue::SItoCU( _U_DIAMETER, dDi ), 4, 0, true );
			str2 += _T(" ");
			GetNameOf( m_pUnitDB->GetUnit( _U_DIAMETER, m_pUnitDB->GetDefaultUnitIndex( _U_DIAMETER ) ), tcName );
			str2 += tcName;
			str.Format( IDS_KVSERIES_DZETA_DI, str1, str2 );
			break;
	};

	// Add computed Kv, Value String and a index into ListCtrl 
	int iIndex = m_List.GetItemCount();

	if( iIndex < 0 )
	{
		iIndex = 0;
	}

	LVITEM	lvItem;								
	lvItem.mask = LVIF_TEXT | LVIF_PARAM; 

	if( iIndex <= 0 )
	{
		lp |= 0x01;									// first Item
	}
	else
	{
		lp |= ( m_List.GetItemData( iIndex - 1 ) & 0xFFFF ) + 1;	// Counter stored in lsb of lp
	}

	str1.Format( _T("%u"), lp & 0xFFFF );
	lvItem.iItem = iIndex;
	lvItem.iSubItem = 0; 
	lvItem.pszText = (TCHAR *)(LPCTSTR)str1; 
	lvItem.lParam = lp;
	m_List.InsertItem( &lvItem );

	// Write value.
	m_List.SetItemText( iIndex, 1, str );

	// Write Kv or Cv, convert to User Unit ....
	str.Format( WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, dKv ), 4, 0, 1 ) );
	m_List.SetItemText( iIndex, 2, str );

	// Update Sum.
	_ComputeKvCv();

	// Select last item added
	for( int i = 0; i < m_List.GetItemCount(); i++ )		// Unselect all selected item
	{
		if( LVIS_SELECTED == m_List.GetItemState( i, LVIS_SELECTED ) )
		{
			m_List.SetItemState( i, 0, LVIS_SELECTED );
		}
	}

	m_List.SetItemState( m_List.GetItemCount() - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	m_List.EnsureVisible( m_List.GetItemCount() - 1, FALSE );

}

void CDlgKvSeries::OnEndLabelEditList( NMHDR *pNMHDR, LRESULT *pResult ) 
{
	LV_DISPINFO *pDispInfo = (LV_DISPINFO *)pNMHDR;
	*pResult = 0;

	int iItem = m_List.GetNextItem( -1, LVNI_SELECTED );

	if( iItem < 0 )
	{
		return;
	}
	
	CString str = pDispInfo->item.pszText;			// New string
	
	if( true == str.IsEmpty() )
	{
		return;						// return if new string is empty 
	}
	
	m_List.SetItemText( iItem, 0, str );				// set new string
}

void CDlgKvSeries::OnButtonRemove() 
{
	int iItem = m_List.GetNextItem( -1, LVNI_SELECTED );

	if( iItem < 0 )
	{
		return;							// Find current selected item
	}

	CString str;
	
	if( m_List.GetSelectedCount() > 1 )
	{
		str = TASApp.LoadLocalizedString( AFXMSG_DELETESELITEMS );		// Delete All selected Items
	}
	else
	{
		FormatString( str, AFXMSG_DELETEITEM, m_List.GetItemText( iItem, 0 ) ); // Delete one item 
	}

	if( AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) != IDYES )
	{
		return;
	}

	do
	{
		m_List.DeleteItem( iItem );
		iItem = m_List.GetNextItem( -1, LVNI_SELECTED );
	} while( iItem >= 0 );

	m_ButtonRemove.EnableWindow( FALSE );
	
	// Update Sum.
	_ComputeKvCv();	

}

void CDlgKvSeries::OnOK() 
{
	PREVENT_ENTER_KEY	
	CDialogEx::OnOK();
}

void CDlgKvSeries::OnClickList( NMHDR *pNMHDR, LRESULT *pResult )
{
	int iItem = m_List.GetNextItem( -1, LVNI_SELECTED );

	if( iItem < 0 ) 
	{
		m_ButtonRemove.EnableWindow( FALSE );
		return;							// Find current selected item
	}

	m_ButtonRemove.EnableWindow( TRUE );
	*pResult = 0;
}


void CDlgKvSeries::OnButtonPrint() 
{
	return;
/*
	long lRow,lCol;
	lRow = m_List.GetItemCount();
	lRow += 5;			// Sheet Title, Column header, space, total

	lCol = 3;						

	m_F1Book.SetMaxRow(110);
//	m_F1Book.SetMaxCol(lCol);
//	m_F1Book.SetTextRC(1,1,"Title");

	CMetaFileDC	mfLogo;
	mfLogo.Create(NULL);		//Create a new meta file in Memory
	HMETAFILE hMF = mfLogo.Close();
	::CopyMetaFile(hMF,_T("c:\\DOC Alen\\Projets C++\\Phoenix\\Data\\logo IMI Indoor Climate.wmf"));
//	HENHMETAFILE hMF = GetEnhMetaFile("c:\\DOC Alen\\Projets C++\\Phoenix\\Data\\logo IMI Indoor Climate.emf");
//	HMETAFILE hMF = GetMetaFile("c:\\DOC Alen\\Projets C++\\Phoenix\\Data\\logo IMI Indoor Climate.wmf");

	CPictureHolder Picture;
	Picture.CreateFromMetafile((HMETAFILE)hMF,188,30);
	OLE_HANDLE OleHdle;
	Picture.m_pPict->get_Handle(&OleHdle);
	m_F1Book.ObjCreatePicture(1,1,2,2,OleHdle,7,188,30);

	CPictureHolder Picture;
	Picture.CreateFromBitmap(IDB_IMILOGO);
	OLE_HANDLE OleHdle;
	Picture.m_pPict->get_Handle(&OleHdle);
	m_F1Book.ObjCreatePicture(1,1,2,2,OleHdle,7,188,30);
	for (int i=0; i<lCol; i++)		// Fill Array
	{
		for (int j=0; j<100; j++)
		{
//			m_F1Book.SetTextRC(3+j,i+1,"test");	
		}
	}
//	m_F1Book.SetPrintTitles("A1:IV2");

	m_F1Book.SetShowColHeading(true);
	m_F1Book.SetColText(1,_T("Col 1"));
	//m_F1Book.SetPrintArea("A3:C100");

//	m_F1Book.FilePrintPreview();
//	DeleteEnhMetaFile(hMF);
*/
}

void CDlgKvSeries::_DisplayTabCDlg( CDialogExt *pTabCDlg )
{
	if( NULL == pTabCDlg )
	{
		return;
	}

	// Calculate correct size and position for pTabCDlg.
	CRect rect;
	m_TabCtrl.GetClientRect( rect );
	m_TabCtrl.AdjustRect( FALSE, &rect );

	// Display pTabCDlg and hide previous child dlg.
	pTabCDlg->SetWindowPos( &wndTop, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW );
	
	if( m_pCurCDlg != pTabCDlg )
	{
		m_pCurCDlg->ShowWindow( SW_HIDE );
		m_pCurCDlg = pTabCDlg;
	}
}

// Scan all ListCtrl and add all Kv/Cv and display the result 
void CDlgKvSeries::_ComputeKvCv()
{
	CString str = _T("");
	double dKv = 0.0;
	double dSum = 0.0;
	
	for( int i = 0; i < m_List.GetItemCount(); i++ )
	{
		str = m_List.GetItemText( i, 2 );
		ReadDouble( str, &dKv );

		if( m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )	// Cv
		{
			dKv = CDimValue::CUtoSI( _C_KVCVCOEFF, dKv );		// Convert to Kv
		}

		ASSERT( dKv > 0.0 );
		dSum += 1.0 / ( dKv * dKv );
	}

	if( dSum > 0.0 )
	{
		dKv = 1.0 / sqrt( dSum );
		str = WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, dKv ), 4, 0, 1 );
	}
	else
	{
		str = _T("");
	}
	
	m_StaticSum.SetWindowText( str );
}
