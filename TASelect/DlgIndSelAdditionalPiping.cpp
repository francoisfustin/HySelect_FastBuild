#include "stdafx.h"

#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "Utilities.h"
#include "Units.h"
#include "Select.h"
#include "SelectPM.h"
#include "ProductSelectionParameters.h"
#include "DlgIndSelAdditionalPiping.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelAdditionalPiping::CDlgIndSelAdditionalPiping( CProductSelelectionParameters *pclProductSelParams, CPipeList *pclPipeList, CWnd *pParent )
	: CDialogExt( IDD_DLGINDSELADDPIPES, pParent )
{
	m_pclProductSelParams = pclProductSelParams;
	m_bChangesDone = false;
	m_pclPipeList = pclPipeList;
	m_pclSSheet = 0;
	m_pclSDescription = 0;
	m_iPipeCount = 0;
	m_bLengthValid = true;
	m_PipeListRectClient = CRect( 0, 0, 0, 0 );
}

CDlgIndSelAdditionalPiping::~CDlgIndSelAdditionalPiping()
{
	while( m_ViewDescription.GetSheetNumber() )
	{
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetTopSheetDescription();
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
		pclSSheet->Detach();
		m_ViewDescription.RemoveOneSheetDescription( pclSheetDescription->GetSheetDescriptionID() );
		delete pclSSheet;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelAdditionalPiping, CDialogExt )
	ON_BN_CLICKED( IDC_RESETALL, &CDlgIndSelAdditionalPiping::OnBnClickedResetAll )
	ON_BN_CLICKED( IDCANCEL, &CDlgIndSelAdditionalPiping::OnBnClickedCancel )
	ON_BN_CLICKED( IDOK, &CDlgIndSelAdditionalPiping::OnBnClickedOk )
	ON_MESSAGE( SSM_CLICK, &CDlgIndSelAdditionalPiping::OnSSClick )
	ON_MESSAGE( SSM_EDITCHANGE, &CDlgIndSelAdditionalPiping::OnSSEditChange )
	ON_MESSAGE( SSM_COMBOCLOSEUP, &CDlgIndSelAdditionalPiping::OnComboDropCloseUp )
END_MESSAGE_MAP()

void CDlgIndSelAdditionalPiping::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICTOTWATVOLTEXT, m_clStaticTotalWaterVolumeText );
	DDX_Control( pDX, IDC_STATICTOTWATVOLVALUE, m_clStaticTotalWaterVolumeValue );
}

BOOL CDlgIndSelAdditionalPiping::OnInitDialog()
{
	if( NULL == m_pclPipeList )
	{
		ASSERTA_RETURN( FALSE );
	}

	CDialogExt::OnInitDialog();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_INDSELADDPIPES_CAPTION );
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_INDSELADDPIPES_TOTALWATERVOLUME );
	GetDlgItem( IDC_STATICTOTWATVOLTEXT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_INDSELADDPIPES_RESET );
	GetDlgItem( IDC_RESETALL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );

	// Set some static text in bold.
	m_clStaticTotalWaterVolumeText.SetFontBold( true );
	m_clStaticTotalWaterVolumeValue.SetFontBold( true );

	// Keep a copy in case of user abort.
	m_clPipeListCopy.CopyFrom( m_pclPipeList );

	bool pipeExist = m_pclPipeList->PipeExistInDB(); // TODO : Handling of user defined pipes

	_InitializeSSheet();
	_LoadPipeList();
	_UpdateStaticValue();

	return TRUE;
}

void CDlgIndSelAdditionalPiping::OnBnClickedResetAll()
{
	if( NULL == m_pclPipeList )
	{
		ASSERT_RETURN;
	}

	if( NULL != m_pclPipeList->GetPipeList() )
	{
		CString str = TASApp.LoadLocalizedString( AFXMSG_INDSELADDPIPES_RESETALL );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			m_pclPipeList->Reset();
			_UpdateStaticValue();

			// TODO: Clear all list and values.
			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_PipeList_FirstAvailRow;

			while( 0 != m_iPipeCount )
			{
				_RemovePipe( &cellCoord );
			}
		}
	}
}

void CDlgIndSelAdditionalPiping::OnBnClickedCancel()
{
	if( NULL == m_pclPipeList )
	{
		ASSERT_RETURN;
	}

	if( true == m_bChangesDone )
	{
		CString str = TASApp.LoadLocalizedString( AFXMSG_INDSELADDPIPES_ABORT );

		if( IDNO == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			return;
		}

		// Restore the copy.
		m_pclPipeList->CopyFrom( &m_clPipeListCopy );
	}

	CDialogExt::OnCancel();
}

void CDlgIndSelAdditionalPiping::OnBnClickedOk()
{
	PREVENT_ENTER_KEY

	CDialog::OnOK();
}

void CDlgIndSelAdditionalPiping::_UpdateStaticValue( void )
{
	if( NULL == m_pclPipeList )
	{
		ASSERT_RETURN;
	}

	double dPipeContent = m_pclPipeList->GetTotalContent();
	m_clStaticTotalWaterVolumeValue.SetWindowText( WriteCUDouble( _U_VOLUME, dPipeContent, true ) );
}

void CDlgIndSelAdditionalPiping::_InitializeSSheet()
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheet = new CSSheet();
	m_pclSSheet->Attach( GetDlgItem( IDC_SSPIPELIST )->GetSafeHwnd() );
	m_pclSSheet->Reset();

	m_pclSDescription = m_ViewDescription.AddSheetDescription( SD_PipeList, -1, m_pclSSheet, CPoint( 0, 0 ) );

	if( NULL == m_pclSDescription )
	{
		return;
	}

	m_pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	m_pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	m_pclSSheet->SetBool( SSB_VERTSCROLLBAR, TRUE );
	m_pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	m_pclSSheet->SetBool( SSB_EDITMODEPERMANENT, TRUE );
	m_pclSSheet->SetButtonDrawMode( SS_BDM_ALWAYSCOMBO );
	m_pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	m_pclSSheet->SetMaxRows( RD_PipeList_FirstAvailRow );
	m_pclSSheet->SetFreeze( 0, RD_PipeList_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	m_pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Add columns.
	m_pclSDescription->AddColumn( CD_PipeList_FirstColumn, 1 );
	m_pclSDescription->AddColumn( CD_PipeList_PipeSeries, 1 );
	m_pclSDescription->AddColumn( CD_PipeList_Size, 1 );
	m_pclSDescription->AddColumn( CD_PipeList_Length, 1 );

	// Resize columns.
	_CheckPipeListColumnWidth();

	// Row name.
	m_pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheet->SetStaticText( CD_PipeList_FirstColumn, RD_PipeList_ColName, _T( "" ) );
	m_pclSSheet->SetStaticText( CD_PipeList_FirstColumn, RD_PipeList_UnitName, _T( "" ) );
	m_pclSSheet->SetStaticText( CD_PipeList_PipeSeries, RD_PipeList_ColName, IDS_INDSELADDPIPES_PIPESERIES );
	m_pclSSheet->SetStaticText( CD_PipeList_PipeSeries, RD_PipeList_UnitName, _T( "" ) );
	m_pclSSheet->SetStaticText( CD_PipeList_Size, RD_PipeList_ColName, IDS_INDSELADDPIPES_SIZE );
	m_pclSSheet->SetStaticText( CD_PipeList_Size, RD_PipeList_UnitName, _T( "" ) );
	m_pclSSheet->SetStaticText( CD_PipeList_Length, RD_PipeList_ColName, IDS_INDSELADDPIPES_LENGTH );
	m_pclSSheet->SetStaticText( CD_PipeList_Length, RD_PipeList_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );

	// To fill a icon.
	m_pclSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_PipeList_FirstColumn, RD_PipeList_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
}

LRESULT CDlgIndSelAdditionalPiping::OnSSEditChange( WPARAM wParam, LPARAM lParam )
{
	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );

	if( NULL == pclSDesc )
	{
		ASSERTA_RETURN( 0 );
	}

	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		ASSERTA_RETURN( 0 );
	}

	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	if( NULL == pclCellCoord )
	{
		ASSERTA_RETURN( 0 );
	}

	// Intercept click on length only.
	if( pclCellCoord->Col != CD_PipeList_Length	|| pclCellCoord->Row < RD_PipeList_FirstAvailRow )
	{
		return 0;
	}

	if( CD_PipeList_Length == pclCellCoord->Col )
	{
		CString strValue = pclSSheet->GetCellText( pclCellCoord->Col, pclCellCoord->Row );
		double dValue;
		m_bChangesDone = true;

		switch( ReadDouble( strValue, &dValue ) )
		{
			case RD_NOT_NUMBER:
			{
				// Number is incomplete ?
				// Try to remove partial engineering notation if exist
				int i = strValue.FindOneOf( _T("eE") );

				if( i != -1 )
				{
					strValue.Delete( i );

					if( RD_OK == ReadDouble( strValue, &dValue ) )
					{
						break;
					}
				}

				i = strValue.FindOneOf( _T("abcdfghijklmnopqrstuvwxyzABCDFGHIJKLMNOPQRSTUVWXYZ") );

				if( i != -1 )
				{
					strValue.Delete( i );

					if( RD_OK == ReadDouble( strValue, &dValue ) )
					{
						break;
					}
				}

				i = strValue.FindOneOf( _T("-+") );

				if( i != -1 )
				{
					strValue.Delete( i );

					if( RD_OK == ReadDouble( strValue, &dValue ) )
					{
						break;
					}
				}

				m_bLengthValid = false;
				pclSSheet->SetCellText( pclCellCoord->Col, pclCellCoord->Row, strValue );
			}
			break;

			case RD_EMPTY:
				m_bLengthValid = true;
				break;

			case RD_OK:
				m_bLengthValid = true;
				break;
		}

	}

	if( IDC_SSPIPELIST == wParam )
	{
		_UpdatePipeList();
		_ButtonPlusPipeEnabled();
	}

	return 0;
}

LRESULT CDlgIndSelAdditionalPiping::OnSSClick( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclPipeList )
	{
		ASSERTA_RETURN( 0 );
	}

	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	if( NULL == pclCellCoord )
	{
		ASSERTA_RETURN( 0 );
	}

	int iRecordIndex = pclCellCoord->Row - RD_PipeList_FirstAvailRow;

	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );

	if( NULL == pclSDesc )
	{
		ASSERTA_RETURN( 0 );
	}

	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		ASSERTA_RETURN( 0 );
	}

	// Intercept click on the first column and below first row only.
	if( pclCellCoord->Col != 1 || pclCellCoord->Row < RD_PipeList_FirstAvailRow )
	{
		return 0;
	}

	if( IDC_SSPIPELIST == wParam )
	{
		if( 1 == pclCellCoord->Col )
		{
			if( _ButtonPlusPipeEnabled() && iRecordIndex == m_pclPipeList->GetPipeCount() ) // plus clicked
			{
				// New pipe must be from the same type as previous.
				CString previousPipeSerie;

				if( m_pclPipeList->GetPipeCount() > 0 )
				{
					_FindSerieID( pclSSheet, pclCellCoord->Row - 1, previousPipeSerie );
				}

				_AddPipe( pclSSheet, previousPipeSerie );
				m_bChangesDone = true;
				pclSSheet->ShowCell( CD_PipeList_FirstColumn, pclCellCoord->Row + 1, SS_SHOW_BOTTOMLEFT );
			}
			else if( m_pclPipeList->GetPipeCount() > 0 && iRecordIndex < m_pclPipeList->GetPipeCount() ) // trash clicked
			{
				_RemovePipe( pclCellCoord );
				m_bChangesDone = true;
			}

			_ButtonPlusPipeEnabled();
			return 1; // event processed
		}

		return 0;
	}

	return 1;
}

LRESULT CDlgIndSelAdditionalPiping::OnComboDropCloseUp( WPARAM wParam, LPARAM lParam )
{
	SS_COMBOCLOSEUP *pComboCloseUp = ( SS_COMBOCLOSEUP * )lParam;

	if( NULL == pComboCloseUp )
	{
		ASSERTA_RETURN( 0 );
	}

	if( -1 == pComboCloseUp->dSelChange ) // no change in selection
	{
		return 0;
	}

	if( CD_PipeList_PipeSeries == pComboCloseUp->Col )
	{
		CString strSerieID;
		_FindSerieID( m_pclSSheet, pComboCloseUp->Row, strSerieID );

		_FillPipeSizesCombo( m_pclSSheet, CD_PipeList_Size, pComboCloseUp->Row, strSerieID );
	}

	_UpdatePipeList();
	m_bChangesDone = true;
	return 0;
}

bool CDlgIndSelAdditionalPiping::_IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row )
{
	if( NULL == pclSSheet )
	{
		ASSERTA_RETURN( false );
	}

	TCHAR tcCheckBox[16];
	pclSSheet->GetValue( col, row, tcCheckBox );

	if( _T( '0' ) == tcCheckBox[0] )
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CDlgIndSelAdditionalPiping::_ButtonPlusPipeEnabled()
{
	if( NULL == m_pclPipeList )
	{
		ASSERTA_RETURN( false );
	}

	// avoid to add a new pipe if some input are invalid
	// not valid :
	// * length <= 0
	// * NAN
	// * Empty Cell
	bool bEnable = true;
	int iLastDataRow = m_pclPipeList->GetPipeCount() + RD_PipeList_FirstAvailRow;

	for( int i = RD_PipeList_FirstAvailRow; i < iLastDataRow; i++ )
	{
		std::wstring strCapacity = m_pclSSheet->GetCellText( CD_PipeList_Length, i );
		double dCapacity = 0.0;

		if( RD_OK != ReadDoubleFromStr( strCapacity, &dCapacity ) || dCapacity <= 0.0 )
		{
			bEnable = false;
			break;
		}
	}

	if( true == bEnable )
	{
		m_pclSSheet->SetBackColor( 
				CD_PipeList_FirstColumn,
				RD_PipeList_FirstAvailRow + m_pclPipeList->GetPipeCount(),
				CD_PipeList_LastColumn,
				RD_PipeList_FirstAvailRow + m_pclPipeList->GetPipeCount(),
				_WHITE );

		m_bLengthValid = true;
	}
	else
	{
		m_pclSSheet->SetBackColor( 
				CD_PipeList_FirstColumn,
				RD_PipeList_FirstAvailRow + m_pclPipeList->GetPipeCount(),
				CD_PipeList_LastColumn,
				RD_PipeList_FirstAvailRow + m_pclPipeList->GetPipeCount(),
				_LIGHTGRAY );

		m_bLengthValid = false;
	}

	return bEnable;
}

void CDlgIndSelAdditionalPiping::_LoadPipeList()
{
	if( NULL == m_pclPipeList )
	{
		ASSERT_RETURN;
	}

	for( int i = 0; i < m_pclPipeList->GetPipeCount(); i++ )
	{
		_AddPipe( m_pclSSheet,
				  m_pclPipeList->GetPipe( i )->m_strPipeSerieID,
				  m_pclPipeList->GetPipe( i )->m_strPipeSizeID,
				  CDimValue::SItoCU( _U_LENGTH, m_pclPipeList->GetPipe( i )->m_dLength ),
				  true );
	}

	_UpdatePipeList();
}

void CDlgIndSelAdditionalPiping::_AddPipe( CSSheet *pclSSheet, CString strSerieID, CString strPipeID, double dLength, bool bOnlyGUI )
{
	if( NULL == pclSSheet || NULL == m_pclPipeList )
	{
		ASSERT_RETURN;
	}

	m_iPipeCount++;
	int iCurrentRow = RD_PipeList_FirstAvailRow + m_iPipeCount - 1;

	// Set max rows.
	pclSSheet->SetMaxRows( iCurrentRow + 1 );

	// Add plus icon.
	pclSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_PipeList_FirstColumn, iCurrentRow + 1, CSSheet::PictureCellType::Icon );

	// Add trash icon.
	pclSSheet->SetPictureCellWithID( IDI_TRASH, CD_PipeList_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add pipe series combo.
	pclSSheet->FormatComboList( CD_PipeList_PipeSeries, iCurrentRow );
	CString strSelectedSerie = _FillPipeSeriesCombo( pclSSheet, CD_PipeList_PipeSeries, iCurrentRow, strSerieID );

	// Add pipe size combo.
	pclSSheet->FormatComboList( CD_PipeList_Size, iCurrentRow );
	CString strSelectedSize = _FillPipeSizesCombo( pclSSheet, CD_PipeList_Size, iCurrentRow, strSelectedSerie, strPipeID );

	// Add length edit.
	pclSSheet->FormatEditDouble( CD_PipeList_Length, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	CString strLength;
	strLength.Format( _T("%g"), dLength );
	pclSSheet->SetCellText( CD_PipeList_Length, iCurrentRow, strLength );

	if( false == bOnlyGUI ) // Used to load data at dialog opening
	{
		m_pclPipeList->AddPipe( strSelectedSerie, strSelectedSize, dLength );
	}

	_CheckPipeListColumnWidth();
}

void CDlgIndSelAdditionalPiping::_RemovePipe( SS_CELLCOORD *plcCellCoord )
{
	if( NULL == plcCellCoord )
	{
		ASSERT_RETURN;
	}

	m_iPipeCount--;
	m_pclSSheet->DelRow( plcCellCoord->Row ); // drop a row
	m_pclSSheet->SetMaxRows( RD_PipeList_FirstAvailRow + m_iPipeCount ); // Set max rows.
	m_pclPipeList->RemovePipe( plcCellCoord->Row - RD_PipeList_FirstAvailRow );

	_CheckPipeListColumnWidth();
}

void CDlgIndSelAdditionalPiping::_UpdatePipeList()
{
	for( int i = 0; i < m_pclPipeList->GetPipeCount(); i++ )
	{
		int iRecordIndex = i;

		// Get series ID.
		CString strSerieID;
		_FindSerieID( m_pclSSheet, i + RD_PipeList_FirstAvailRow, strSerieID );

		// Get size ID.
		CString strSizeID;
		_FindSizeID( m_pclSSheet, i + RD_PipeList_FirstAvailRow, strSizeID );

		// Get Length in SI.
		TCHAR tcLength[16];
		double dLength;
		m_pclSSheet->GetValue( CD_PipeList_Length, i + RD_PipeList_FirstAvailRow, tcLength );
		ReadCUDoubleFromStr( _U_LENGTH, tcLength, &dLength );

		// Update database.
		m_pclPipeList->UpdatePipe( iRecordIndex, strSerieID, strSizeID, dLength );
	}

	_UpdateStaticValue();
}

void CDlgIndSelAdditionalPiping::_FindSerieID( CSSheet *pclSSheet, int row, CString &strID )
{
	if( NULL == pclSSheet )
	{
		ASSERT_RETURN;
	}

	strID = _T( "" );
	TCHAR tcComboValue[256];
	int iCurrentSelection = pclSSheet->ComboBoxSendMessage( CD_PipeList_PipeSeries, row, SS_CBM_GETCURSEL, 0, 0 );
	pclSSheet->ComboBoxSendMessage( CD_PipeList_PipeSeries, row, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )tcComboValue );

	for( std::map<CString, CTable *>::iterator mapIter = m_mapPipeSeriesType.begin(); mapIter != m_mapPipeSeriesType.end(); ++mapIter )
	{
		if( CString( mapIter->first.GetString() ).CompareNoCase( tcComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

void CDlgIndSelAdditionalPiping::_FindSizeID( CSSheet *pclSSheet, int row, CString &strID )
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB || NULL == pclSSheet )
	{
		ASSERT_RETURN;
	}

	CString strSerieID;
	_FindSerieID( pclSSheet, row, strSerieID );

	strID = _T( "" );
	TCHAR strComboValue[256];
	int iCurrentSelection = pclSSheet->ComboBoxSendMessage( CD_PipeList_Size, row, SS_CBM_GETCURSEL, 0, 0 );
	pclSSheet->ComboBoxSendMessage( CD_PipeList_Size, row, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )strComboValue );

	CTable *pTabPipe = dynamic_cast<CTable *>( TASApp.GetpPipeDB()->Get( strSerieID ).MP );
	ASSERT( pTabPipe );

	CRank PipeList;
	m_pclProductSelParams->m_pTADB->GetPipeSizeList( &PipeList, pTabPipe );

	CString str;
	LPARAM itemdata;

	for( BOOL cont = PipeList.GetFirst( str, itemdata ); cont; cont = PipeList.GetNext( str, itemdata ) )
	{
		CTable *pipe = ( CTable * )itemdata;

		if( _tcscmp( strComboValue, str.GetString() ) == 0 )
		{
			strID = pipe->GetIDPtr().ID;
			return;
		}
	};
}

CString CDlgIndSelAdditionalPiping::_FillPipeSeriesCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSerieSelect )
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB || NULL == pclSheet || NULL == ptcSerieSelect )
	{
		ASSERTA_RETURN( _T("" ) );
	}

	if( lCol < 1 || lCol > pclSheet->GetMaxCols() || lRow < 1 || lRow > pclSheet->GetMaxRows() )
	{
		return _T( "" );
	}

	CString strReturn;

	if( true == m_mapPipeSeriesType.empty() )
	{
		CRank PipeList;
		m_pclProductSelParams->m_pTADB->GetPipeSerieList( &PipeList );

		CString str;
		LPARAM itemdata;

		for( BOOL cont = PipeList.GetFirst( str, itemdata ); cont; cont = PipeList.GetNext( str, itemdata ) )
		{
			m_mapPipeSeriesType[str] = ( CTable * )itemdata;
		};
	}

	if( true == m_mapPipeSeriesType.empty() )
	{
		return _T( "" );
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_RESETCONTENT, 0, 0 );
	int iCount = 0;
	int iSelectIndex = 0;

	for( std::map<CString, CTable *>::iterator mapIter = m_mapPipeSeriesType.begin(); mapIter != m_mapPipeSeriesType.end(); ++mapIter )
	{
		pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_ADDSTRING, 0, ( LPARAM )( mapIter->first.GetString() ) );

		if( _tcscmp( ptcSerieSelect, mapIter->second->GetIDPtr().ID ) == 0 )
		{
			iSelectIndex = iCount;
			strReturn = ptcSerieSelect;
		}

		iCount++;

		// first item selected by default
		if( mapIter == m_mapPipeSeriesType.begin() )
		{
			strReturn = mapIter->second->GetIDPtr().ID;
		}
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_SETCURSEL, iSelectIndex, 0 );
	return strReturn;
}

CString CDlgIndSelAdditionalPiping::_FillPipeSizesCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcPipeSerie, const TCHAR *ptcSelect )
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB || NULL == pclSheet || NULL == ptcPipeSerie || NULL == ptcSelect )
	{
		ASSERTA_RETURN( _T("") );
	}

	if( lCol < 1 || lCol > pclSheet->GetMaxCols() || lRow < 1 || lRow > pclSheet->GetMaxRows() )
	{
		return _T( "" );
	}

	CTable *pTabPipe = dynamic_cast<CTable *>( TASApp.GetpPipeDB()->Get( ptcPipeSerie ).MP );
	ASSERT( NULL != pTabPipe );

	CRank PipeList;
	m_pclProductSelParams->m_pTADB->GetPipeSizeList( &PipeList, pTabPipe );

	CString str;
	LPARAM itemdata;

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_RESETCONTENT, 0, 0 );

	int iCount = 0;
	int iSelectIndex = 0;
	CString strReturn;

	for( BOOL bContinue = PipeList.GetFirst( str, itemdata ); TRUE == bContinue; bContinue = PipeList.GetNext( str, itemdata ) )
	{
		CTable *pipe = ( CTable * )itemdata;

		if( 0 == iCount )
		{
			strReturn = pipe->GetIDPtr().ID;
		}

		pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_ADDSTRING, 0, ( LPARAM ) str.GetString() );

		if( 0 == _tcscmp( ptcSelect, pipe->GetIDPtr().ID ) )
		{
			iSelectIndex = iCount;
			strReturn = ptcSelect;
		}

		iCount++;
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_SETCURSEL, iSelectIndex, 0 );

	return strReturn;
}

BOOL CDlgIndSelAdditionalPiping::PreTranslateMessage( MSG *pMsg )
{
	if( true == m_bLengthValid )
	{
		GetDlgItem( IDOK )->EnableWindow( TRUE );
	}
	else
	{
		GetDlgItem( IDOK )->EnableWindow( FALSE );
	}

	if( WM_LBUTTONDOWN == pMsg->message )
	{
		// PWU : The first click on the spread is ignored if the focus is elsewhere.
		// This code catch the first click and send it directly to the spread.
		CRect rectPipeList;
		GetDlgItem( IDC_SSPIPELIST )->GetWindowRect( &rectPipeList );

		CRect rectCurrentFocus;
		CWnd *pFocusedCtrl = GetFocus();

		if( NULL != pFocusedCtrl )
		{
			pFocusedCtrl->GetWindowRect( rectCurrentFocus );
		}

		CRect rectIntersectPipeList;
		rectIntersectPipeList.IntersectRect( rectCurrentFocus, rectPipeList );

		if( TRUE == rectPipeList.PtInRect( pMsg->pt ) && TRUE == rectIntersectPipeList.IsRectNull() )
		{
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( IDC_SSPIPELIST );
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			GetDlgItem( IDC_SSPIPELIST )->SetFocus();
		}
	}

	return __super::PreTranslateMessage( pMsg ); // allow default behavior (return TRUE if you want to discard message)
}

void CDlgIndSelAdditionalPiping::_CheckPipeListColumnWidth()
{
	if( NULL == m_pclSSheet )
	{
		return;
	}
	
	CRect rectClient;
	m_pclSSheet->GetClientRect( &rectClient );

	if( rectClient.Width() == m_PipeListRectClient.Width() )
	{
		return;
	}

	m_PipeListRectClient = rectClient;

	double dAvailableWidth = m_pclSSheet->LogUnitsToColWidthW( m_PipeListRectClient.Width() );
	double dStandardWidthInPixel = m_pclSSheet->ColWidthToLogUnits( 1 ) - 1;
	double dTotalColWidth = PLCW_FirstColumn + PLCW_PipeSeries + PLCW_Size + PLCW_Length;

	double dRatio = dAvailableWidth / dTotalColWidth * dStandardWidthInPixel;

	long lWidthInPixel = (long)( dRatio * PLCW_FirstColumn );
	long lTotalWidthInPixel = lWidthInPixel;
	m_pclSSheet->SetColWidthInPixels( CD_PipeList_FirstColumn, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * PLCW_PipeSeries );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheet->SetColWidthInPixels( CD_PipeList_PipeSeries, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * PLCW_Size );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheet->SetColWidthInPixels( CD_PipeList_Size, lWidthInPixel );

	m_pclSSheet->SetColWidthInPixels( CD_PipeList_Length, m_PipeListRectClient.Width() - lTotalWidthInPixel );
}
