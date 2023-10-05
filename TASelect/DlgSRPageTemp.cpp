#include "stdafx.h"


#include "TASelect.h"
#include "HydroMod.h"
#include "HMPipes.h"
#include "HMTreeDroptarget.h"
#include "DlgSearchReplace.h"
#include "DlgReplacePopup.h"
#include "DlgSRPageTemp.h"


IMPLEMENT_DYNAMIC( CDlgSRPageTemp, CDlgCtrlPropPage )

CDlgSRPageTemp::CDlgSRPageTemp( CWnd *pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGSRTEMP_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGSRTEMP_PAGENAME );
	m_pTree = NULL;
	m_strStaticFind1 = L"";
	m_strStaticFind2 = L"";
	m_strStaticReplace = L"";
	m_pDlgReplacePopup = NULL;
	m_hTreeItem = NULL;
}

bool CDlgSRPageTemp::OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams )
{
	_ListHTreeItem();

	if( false == m_vHTreeItem.empty() )
	{
		m_hTreeItem = NULL;
		OnFindNextInjCircuit();
	}

	// Reset the edit info.
	m_EditInfo.SetWindowText( L"" );

	// Display the 'CDlgReplacePopup' dialog.
	CDlgReplacePopup Dlg( this );
	m_pDlgReplacePopup = &Dlg;
	m_pDlgReplacePopup->SetGroupFind( TASApp.LoadLocalizedString( IDS_TABDLGSRTEMP_POPUP_GROUPFIND ) );
	m_pDlgReplacePopup->SetGroupReplace( TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_POPUP_GROUPREPLACE ) );

	Dlg.DoModal();
	return true;
}

void CDlgSRPageTemp::SetTree( LPARAM pHMTreeListCtrl )
{
	// Define the tree from the parent.
	m_pTree = ( CHMTreeListCtrl * )pHMTreeListCtrl;

	// Fill the pipes.
	_FillComboBoxFindTemperatures();
	_UpdateStartBtn();
}

BEGIN_MESSAGE_MAP( CDlgSRPageTemp, CDlgCtrlPropPage )
	ON_MESSAGE( WM_USER_HMTREEITEMCHECKCHANGED, &CDlgSRPageTemp::OnTreeItemCheckStatusChanged )
	ON_MESSAGE( WM_USER_REPLACEPOPUPFINDNEXT, OnFindNextInjCircuit )
	ON_MESSAGE( WM_USER_REPLACEPOPUPREPLACE, OnReplaceTemperature )
	ON_MESSAGE( WM_USER_REPLACEPOPUPREPLACEALL, OnReplaceAll )
	ON_MESSAGE( WM_USER_CLOSEPOPUPWND, OnClosePopup )

	ON_CBN_SELCHANGE( IDC_COMBO_FINDPIPESERIE, OnCbnSelChangeFindTemperature )
	ON_EN_CHANGE( IDC_EDIT_REPLACESUPPLYTEMP, OnEditChangeSupplyTemperature )
	ON_EN_CHANGE( IDC_EDIT_REPLACERETURNTEMP, OnEditChangeReturnTemperature )
END_MESSAGE_MAP()

void CDlgSRPageTemp::DoDataExchange( CDataExchange *pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBO_FINDPIPESERIE, m_CBFindTemperature );
	DDX_Control( pDX, IDC_EDIT_REPLACESUPPLYTEMP, m_EditReplaceSupplyTemp );
	DDX_Control( pDX, IDC_EDIT_REPLACERETURNTEMP, m_EditReplaceReturnTemp );
	DDX_Control( pDX, IDC_EDIT_INFO, m_EditInfo );
}

BOOL CDlgSRPageTemp::OnInitDialog()
{
	// Do default function.
	BOOL Breturn = CDlgCtrlPropPage::OnInitDialog();

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_FINDWHAT );
	GetDlgItem( IDC_STATIC_FIND )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_REPLACEWITH );
	GetDlgItem( IDC_STATIC_REPLACE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_REPPIPES );
	GetDlgItem( IDC_STATIC_REP_INFO )->SetWindowText( str );

	m_EditReplaceSupplyTemp.SetLimitText( 8 );
	m_EditReplaceReturnTemp.SetLimitText( 8 );

	return Breturn;
}

LRESULT CDlgSRPageTemp::OnTreeItemCheckStatusChanged( WPARAM wParam, LPARAM lParam )
{
	_FillComboBoxFindTemperatures();
	_UpdateStartBtn();
	return 0;
}

LRESULT CDlgSRPageTemp::OnFindNextInjCircuit( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pDlgReplacePopup
		|| NULL == m_pDlgReplacePopup->GetSafeHwnd()
		|| m_vHTreeItem.empty() )
	{
		m_hTreeItem = NULL;
		return 0;
	}

	m_hTreeItem = m_vHTreeItem[0];
	m_vHTreeItem.erase( m_vHTreeItem.begin() );

	if( true == m_vHTreeItem.empty() )
	{
		m_pDlgReplacePopup->DisableFindNextBtn();
	}

	// Update data to the popup.
	_UpdatePopupData();

	return 0;
}

void CDlgSRPageTemp::_ListHTreeItem( HTREEITEM hItem )
{
	HTREEITEM l_hItem = hItem;

	if( NULL == l_hItem )
	{
		m_vHTreeItem.clear();
		l_hItem = m_pTree->GetRootItem();
	}

	for( HTREEITEM hChildItem = m_pTree->GetChildItem( l_hItem ); NULL != hChildItem; hChildItem = m_pTree->GetNextSiblingItem( hChildItem ) )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_pTree->GetItemData( hChildItem );

		if( INDEXTOSTATEIMAGEMASK( CRCImageManager::ILCB_Selected + 1 ) == m_pTree->GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) )
		{
			CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( pHM );

			bool bFilter = false;

			if( 0 == m_CBFindTemperature.GetCurSel() )
			{
				bFilter = true;
			}
			else if( NULL != pHMInj && CB_ERR != m_CBFindTemperature.GetCurSel() )
			{
				CString strFilter( _T("") );
				CString strCurrent( _T("") );
				m_CBFindTemperature.GetLBText( m_CBFindTemperature.GetCurSel(), strFilter );
				double dTs = CDimValue::SItoCU( _U_TEMPERATURE, pHMInj->GetDesignCircuitSupplySecondaryTemperature() );
				double dTr = CDimValue::SItoCU( _U_TEMPERATURE, pHMInj->GetDesignCircuitReturnSecondaryTemperature() );

				strCurrent.Format( _T("%g - %g"), dTs, dTr );

				if( 0 == strFilter.Compare( strCurrent ) )
				{
					bFilter = true;
				}
			}

			if( NULL != pHMInj && bFilter )
			{
				m_vHTreeItem.push_back( hChildItem );
			}
		}

		if( true == pHM->IsaModule() )
		{
			_ListHTreeItem( hChildItem );
		}
	}
}

LRESULT CDlgSRPageTemp::OnReplaceTemperature( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pDlgReplacePopup
		|| NULL == m_pDlgReplacePopup->GetSafeHwnd()
		|| NULL == m_hTreeItem )
	{
		m_hTreeItem = NULL;
		return 0;
	}

	// Recuperate the current pointer.
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_pTree->GetItemData( m_hTreeItem );
	CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( pHM );

	if( NULL != pHMInj )
	{
		CString strValue;
		double dValue;

		m_EditReplaceSupplyTemp.GetWindowTextW( strValue );
		ReadDouble( strValue, &dValue );
		pHMInj->SetDesignCircuitSupplySecondaryTemperature( CDimValue::CUtoSI( _U_TEMPERATURE, dValue ) );

		m_EditReplaceReturnTemp.GetWindowTextW( strValue );
		ReadDouble( strValue, &dValue );
		pHMInj->SetDesignCircuitReturnSecondaryTemperature( CDimValue::CUtoSI( _U_TEMPERATURE, dValue ) );
	}

	// Update the 'edit' box.
	_UpdateEditData();

	// Verify another pipe exist.
	if( NULL == pHMInj  || true == m_pDlgReplacePopup->IsFindNextBtnDisable() )
	{
		m_pDlgReplacePopup->OnBnClickedBtnClose();
		return 0;
	}

	// Jump to the next pipe.
	OnFindNextInjCircuit( wParam, lParam );

	return 0;
}

LRESULT CDlgSRPageTemp::OnReplaceAll( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pDlgReplacePopup || NULL == m_pDlgReplacePopup->GetSafeHwnd() )
	{
		return 0;
	}

	BeginWaitCursor();

	if( NULL == m_hTreeItem )
	{
		OnFindNextInjCircuit( wParam, lParam );
	}

	// Do a loop on the rank list and replace each pipe.
	while( NULL != m_hTreeItem )
	{
		OnReplaceTemperature( wParam, lParam );
	};

	EndWaitCursor();

	return 0;
}

LRESULT CDlgSRPageTemp::OnClosePopup( WPARAM wParam, LPARAM lParam )
{
	// Compute All is needed to correctly resize distribution pipe.
	TASApp.GetpTADS()->ComputeAllInstallation();
	m_pDlgReplacePopup = NULL;
	_FillComboBoxFindTemperatures();
	return TRUE;
}

void CDlgSRPageTemp::OnCbnSelChangeFindTemperature()
{
	// Temperature to find changed
	_UpdateStartBtn();
}

void CDlgSRPageTemp::OnEditChangeSupplyTemperature()
{
	// Temperature to replace for supply
	// Check if input is valid
	CString strValue;
	m_EditReplaceSupplyTemp.GetWindowTextW( strValue );
	CheckTempEditIsValid( strValue, m_EditReplaceSupplyTemp );
	_UpdateStartBtn();
}

void CDlgSRPageTemp::CheckTempEditIsValid( CString &strValue, CEdit &ceTemp )
{
	double dValue;

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

				if( RD_OK == ReadDouble( strValue, &dValue ) && i != 0 )
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

				// HYS-1307: If "-" sign is first allow to continue ( Not set the cell with the string without "-" sign ).
				if( true == strValue.IsEmpty() )
				{
					strValue = _T( "0.0" );
				}

				if( RD_OK == ReadDouble( strValue, &dValue ) )
				{
					break;
				}
			}

			ceTemp.SetWindowTextW( strValue );
		}
		break;

		case RD_EMPTY:
			break;

		case RD_OK:
			if( CDimValue::CUtoSI( _U_TEMPERATURE, dValue ) < -273.15
				|| CDimValue::CUtoSI( _U_TEMPERATURE, dValue ) > 374.1 )
			{
				strValue = strValue.Mid( 0, strValue.GetLength() - 1 );
				ceTemp.SetWindowTextW( strValue );
			}

			break;
	}
}

void CDlgSRPageTemp::OnEditChangeReturnTemperature()
{
	// Temperature to replace for return
	// Check if input is valid
	CString strValue;
	m_EditReplaceReturnTemp.GetWindowTextW( strValue );
	CheckTempEditIsValid( strValue, m_EditReplaceReturnTemp );
	_UpdateStartBtn();
}

void CDlgSRPageTemp::_GetTempListFromTree( std::vector<std::pair<double, double> > &vPairTemperature, HTREEITEM hItem /*= NULL*/ )
{
	if( NULL == hItem )
	{
		hItem = m_pTree->GetRootItem();
	}

	// Do a loop on the tree.
	for( HTREEITEM hChildItem = m_pTree->GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_pTree->GetNextSiblingItem( hChildItem ) )
	{
		if( INDEXTOSTATEIMAGEMASK( CRCImageManager::ILCB_Selected + 1 ) == m_pTree->GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) )
		{
			CDS_HydroMod *pHM = (CDS_HydroMod *)m_pTree->GetItemData( hChildItem );
			CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( pHM );

			if( NULL != pHMInj )
			{
				std::pair<double, double> pairTemp;
				pairTemp.first = CDimValue::SItoCU( _U_TEMPERATURE, pHMInj->GetDesignCircuitSupplySecondaryTemperature() );
				pairTemp.second = CDimValue::SItoCU( _U_TEMPERATURE, pHMInj->GetDesignCircuitReturnSecondaryTemperature() );
				vPairTemperature.push_back( pairTemp );
				
				// Remove duplicates.
				sort( vPairTemperature.begin(), vPairTemperature.end() );
				vPairTemperature.erase( unique( vPairTemperature.begin(), vPairTemperature.end() ), vPairTemperature.end() );
			}
		}

		// Do the recursive function.
		if( NULL != m_pTree->GetChildItem( hChildItem ) )
		{
			_GetTempListFromTree( vPairTemperature, hChildItem );
		}
	}
}

void CDlgSRPageTemp::_FillComboBoxFindTemperatures()
{
	std::vector<std::pair<double, double> > TempList;
	_GetTempListFromTree( TempList );
	_FillComboBox( TempList, &m_CBFindTemperature );

	// Add "** All temperatures **" as first element.
	if( false == TempList.empty() || 0 == m_CBFindTemperature.GetCount() )
	{
		m_CBFindTemperature.InsertString( 0, TASApp.LoadLocalizedString( IDS_COMBOTEXT_ALL_TEMPERATURES ) );
		m_CBFindTemperature.SetCurSel( 0 );
	}
}

void CDlgSRPageTemp::_FillComboBox( std::vector<std::pair<double, double> > &vPairTemperature, CExtNumEditComboBox *pCCombo )
{
	if( 0 == vPairTemperature.size() || NULL == pCCombo )
	{
		pCCombo->EnableWindow( FALSE );
		return;
	}

	pCCombo->ResetContent();

	// Transfer the list to the combo Box.
	for( unsigned int i = 0; i < vPairTemperature.size(); i++ )
	{
		CString strTemp;
		strTemp.Format( L"%g - %g", vPairTemperature[i].first, vPairTemperature[i].second );
		pCCombo->AddString( strTemp );
	}

	// Set the first item has the selected one.
	pCCombo->SetCurSel( 0 );

	// Enable/Disable the window depending on the number of items.
	if( pCCombo->GetCount() < 1 )
	{
		pCCombo->EnableWindow( FALSE );
	}
	else
	{
		pCCombo->EnableWindow( TRUE );
	}
}

void CDlgSRPageTemp::_UpdatePopupData()
{
	if( NULL == m_hTreeItem )
	{
		return;
	}

	// Define the current pipe to show and his HydroMod.
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_pTree->GetItemData( m_hTreeItem );
	CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( pHM );

	// Define the text where the pipe came from.
	CString strPipeFrom = L"";

	if( NULL == pHM && NULL == pHMInj )
	{
		ASSERT( 0 );
	}

	// Define strings for 'm_strStaticFind1'.
	m_strStaticFind1 = pHM->GetHMName();

	// Define strings for 'm_strStaticFind2'.
	double dTs = CDimValue::SItoCU( _U_TEMPERATURE, pHMInj->GetDesignCircuitSupplySecondaryTemperature() );
	double dTr = CDimValue::SItoCU( _U_TEMPERATURE, pHMInj->GetDesignCircuitReturnSecondaryTemperature() );

	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pclUnitDB = CDimValue::AccessUDB();
	GetNameOf( pclUnitDB->GetDefaultUnit( _U_TEMPERATURE ), tcName );
	m_strStaticFind2.Format( _T("%g - %g %s"), dTs, dTr,  tcName );

	// Define strings for 'm_strStaticReplace'.
	CString strPipeSerie, strPipeSize;
	m_EditReplaceSupplyTemp.GetWindowTextW( strPipeSerie );
	m_EditReplaceReturnTemp.GetWindowTextW( strPipeSize );
	m_strStaticReplace.Format( _T("%s - %s %s"), strPipeSerie, strPipeSize, tcName );

	// Update text values in the popup window.
	m_pDlgReplacePopup->SetStaticFind1( m_strStaticFind1 );
	m_pDlgReplacePopup->SetStaticFind2( m_strStaticFind2 );
	m_pDlgReplacePopup->SetStaticReplace( m_strStaticReplace );
	m_pDlgReplacePopup->UpdateData();
}

void CDlgSRPageTemp::_UpdateEditData()
{
	if( NULL == m_hTreeItem )
	{
		return;
	}

	// Define the current pipe to show and his HydroMod.
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_pTree->GetItemData( m_hTreeItem );
	CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( pHM );

	CString strEditInfo = L"";
	m_EditInfo.GetWindowText( strEditInfo );

	// Define the text where the pipe came from.
	if( NULL != pHM && NULL != pHMInj )
	{
		CString str1;
		str1 =  pHM->GetHMName() + L": %1 - %2 %3";

		CString strFormated, strTs, strTr;
		m_EditReplaceSupplyTemp.GetWindowTextW( strTs );
		m_EditReplaceReturnTemp.GetWindowTextW( strTr );

		TCHAR tcName[_MAXCHARS];
		CUnitDatabase *pclUnitDB = CDimValue::AccessUDB();
		GetNameOf( pclUnitDB->GetDefaultUnit( _U_TEMPERATURE ), tcName );

		FormatString( strFormated, str1, strTs, strTr, tcName );

		strEditInfo += strFormated;
		strEditInfo += L"\r\n";
	}

	// Update text values in the edit ctrl.
	m_EditInfo.SetWindowText( strEditInfo );
}

void CDlgSRPageTemp::_UpdateStartBtn()
{
	BOOL bEnable = TRUE;

	_ListHTreeItem();

	// Variables.
	CTable *pCBFindTab = ( CTable * )m_CBFindTemperature.GetItemData( m_CBFindTemperature.GetCurSel() );

	// If one combo box is empty or no injection circuit is find disable the 'Start' button.
	bool b1 = ( m_CBFindTemperature.GetCount() > 0 ) ? false : true;
	bool b2 = ( m_EditReplaceSupplyTemp.GetWindowTextLength() > 0 ) ? false : true;
	bool b3 = ( m_vHTreeItem.size() > 0 ) ? false : true;
	bool b4 = ( m_EditReplaceReturnTemp.GetWindowTextLength() > 0 ) ? false : true;

	if( true == b1 || true == b2 || true == b3 || true == b4 )
	{
		bEnable = FALSE;
	}

	// Enable 'Start' button.
	if( NULL != pDlgSearchReplace )
	{
		pDlgSearchReplace->EnableButtonStart( bEnable );
	}
}
