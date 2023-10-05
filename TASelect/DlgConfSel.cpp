#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydromod.h"
#include "HubHandler.h"
#include "SelectPM.h"
#include "EnBitmapPatchWork.h"

#include "DlgInfoSSelDpC.h"
#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelTrv.h"
#include "afxdialogex.h"
#include "DlgConfSel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _MAX_QTY	50000

CDlgConfSel::CDlgConfSel( CProductSelelectionParameters *pclProdSelParams, CWnd *pParent )
	: CDialogEx( CDlgConfSel::IDD, pParent )
{
	// Will be verified in 'OnInitDialog' when calling 'DoModal'.
	m_pclProdSelParams = pclProdSelParams;

	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUSERDB = NULL;
	m_pUnitDB = NULL;
	m_pHmHub = NULL;
	m_pSelAcc = NULL;
	m_pSelAct = NULL;
	m_pSelBCv = NULL;
	m_pSelBv = NULL;
	m_pSelCv = NULL;
	m_pSelDpController = NULL;
	m_pSelDpReliefValve = NULL;
	m_pSelPICv = NULL;
	m_pSelRadSet = NULL;
	m_pSelAirVentSeparator = NULL;
	m_pSelPressureMaint = NULL;
	m_pSelDpCBCV = NULL;
	m_pSelSv = NULL;
	m_pSelSafetyValve = NULL;
	m_pSel6WayValve = NULL;
	m_pSelTapWaterControl = NULL;
	m_pSelSmartControlValve = NULL;
	m_pSelSmartDpC = NULL;
	m_pSelTALinkAlone = NULL;
	m_pSelFloorHeatingManifold = NULL;
	m_pSelFloorHeatingValve = NULL;
	m_pSelFloorHeatingController = NULL;
	m_bComboChanged = false;
	m_iFrom = 0;
	m_lQty = 0;
	m_strComment = L"";
	m_pProdPic = NULL;
	// HYS-1968: Use map instead of vector.
	m_mapEditableQuantity.clear();
	m_mapQuantityRef.clear();
	m_mapSelectAccessories.clear();
	m_bApplyDefaultQty = false;
	m_bApplyAccessoriesQty = false;
}

CDlgConfSel::~CDlgConfSel()
{
	if( m_mapQuantityRef.size() > 0 )
	{
		for( map<long, unsigned int*>::iterator it = m_mapQuantityRef.begin(); it != m_mapQuantityRef.end(); ++it )
		{
			delete it->second;
		}
	}
	m_mapQuantityRef.clear();
	m_mapSelectAccessories.clear();
	// HYS-1968
	m_mapEditableQuantity.clear();
}

BEGIN_MESSAGE_MAP( CDlgConfSel, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOREMARK, OnCbnSelChangeRemark )
	ON_CBN_EDITCHANGE( IDC_COMBOREMARK, OnCbnEditChangeRemark )
	ON_EN_CHANGE( IDC_EDITREMARK, OnEnChangeRemark )
	ON_BN_CLICKED( IDC_BUTTONDELETE, OnButtonDelete )
	ON_BN_CLICKED( IDC_BUTTONADD, OnButtonAdd )
	//HYS-987
	ON_BN_CLICKED( IDC_CHECKEDITACC, OnClickedCheckBox )
	ON_EN_KILLFOCUS( IDC_EDITQTY, OnEnKillFocusEditQty )
	ON_NOTIFY( NM_CLICK, IDC_LISTCTRL, OnNMClickList )
	ON_MESSAGE( WM_APP+1, OnNotifyDescriptionEdited )
END_MESSAGE_MAP()

void CDlgConfSel::Display( CData *pSSel )
{
	if( dynamic_cast<CDS_SSelPICv *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelPICv *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelRadSet *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelRadSet *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelCv *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelCv *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelBCv *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelBCv *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelDpC *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelDpC *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelDpReliefValve *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelDpReliefValve *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelBv *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelBv *)pSSel );
	}
	else if( dynamic_cast<CDS_Actuator *>( pSSel ) != NULL )
	{
		Display( (CDS_Actuator *)pSSel );
	}
	else if( dynamic_cast<CDS_Accessory *>( pSSel ) != NULL )
	{
		Display( (CDS_Accessory *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelAirVentSeparator *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelAirVentSeparator *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelPMaint *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelPMaint *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelDpCBCV *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelDpCBCV *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelSv *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelSv *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelSafetyValve *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelSafetyValve *)pSSel );
	}
	else if( dynamic_cast<CDS_SSel6WayValve *>( pSSel ) != NULL )
	{
		Display( (CDS_SSel6WayValve *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelTapWaterControl *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelTapWaterControl *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelSmartControlValve *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelSmartControlValve *)pSSel );
	}
	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	else if( dynamic_cast<CDS_SSelSmartDpC *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelSmartDpC *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelFloorHeatingManifold *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelFloorHeatingManifold *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelFloorHeatingValve *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelFloorHeatingValve *)pSSel );
	}
	else if( dynamic_cast<CDS_SSelFloorHeatingController *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelFloorHeatingController *)pSSel );
	}
	// HYS-1741
	else if( dynamic_cast<CDS_SelPWQAccServices *>( pSSel ) != NULL )
	{
		Display( (CDS_SelPWQAccServices*)pSSel );
	}
	// HYS 2007
	else if( dynamic_cast<CDS_SSelDpSensor *>( pSSel ) != NULL )
	{
		Display( (CDS_SSelDpSensor*)pSSel );
	}
	else
	{
		ASSERT( 0 );
	}
}

void CDlgConfSel::Display( CDS_SSelDpSensor* pSSel )
{
	m_pSelTALinkAlone = pSSel;

	if( NULL == m_pSelTALinkAlone )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelTALinkAlone );
}

void CDlgConfSel::Display( CDS_HmHub *pHmHub )
{
	m_pHmHub = pHmHub;

	if( NULL == m_pHmHub )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelH );
}

void CDlgConfSel::Display( CDS_SSelBCv *pSelBCv )
{
	m_pSelBCv = pSelBCv;

	if( NULL == m_pSelBCv )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelBCv );
}

void CDlgConfSel::Display( CDS_SSelBv *pSelBv )
{
	m_pSelBv = pSelBv;

	if( NULL == m_pSelBv )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelBV );
}

void CDlgConfSel::Display( CDS_SSelCv *pSelCv )
{
	m_pSelCv = pSelCv;

	if( NULL == m_pSelCv )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelCv );
}

void CDlgConfSel::Display( CDS_SSelDpC *pSelDpController )
{
	m_pSelDpController = pSelDpController;

	if( NULL == m_pSelDpController )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelDpController );
}

void CDlgConfSel::Display( CDS_SSelDpReliefValve *pSelDpReliefValve )
{
	m_pSelDpReliefValve = pSelDpReliefValve;

	if( NULL == m_pSelDpReliefValve )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelDpReliefValve );
}

void CDlgConfSel::Display( CDS_SSelPICv *pSelPICv )
{
	m_pSelPICv = pSelPICv;

	if( NULL == m_pSelPICv )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelPICv );
}

void CDlgConfSel::Display( CDS_SSelRadSet *pSelRadSet )
{
	m_pSelRadSet = pSelRadSet;

	if( NULL == m_pSelRadSet )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelTrv );
}

void CDlgConfSel::Display( CDS_SSelAirVentSeparator *pSelAirVentSeparator )
{
	m_pSelAirVentSeparator = pSelAirVentSeparator;

	if( NULL == m_pSelAirVentSeparator )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelAirVentSep );
}

void CDlgConfSel::Display( CDS_SSelPMaint *pSelPressureMaint )
{
	m_pSelPressureMaint = pSelPressureMaint;

	if( NULL == m_pSelPressureMaint )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelPMaint );
}

void CDlgConfSel::Display( CDS_SSelDpCBCV *pSelDpCBCV )
{
	m_pSelDpCBCV = pSelDpCBCV;

	if( NULL == m_pSelDpCBCV )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelDpCBCV );
}

void CDlgConfSel::Display( CDS_SSelSv *pSelSv )
{
	m_pSelSv = pSelSv;

	if( NULL == m_pSelSv )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelSv );
}

void CDlgConfSel::Display( CDS_SSelSafetyValve *pSelSafetyValve )
{
	m_pSelSafetyValve = pSelSafetyValve;

	if( NULL == m_pSelSafetyValve )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelSafetyValve );
}

void CDlgConfSel::Display( CDS_SSel6WayValve *pSel6WayValve )
{
	m_pSel6WayValve = pSel6WayValve;

	if( NULL == m_pSel6WayValve )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSel6WayValve );
}

void CDlgConfSel::Display( CDS_SSelTapWaterControl *pSelTapWaterControl )
{
	m_pSelTapWaterControl = pSelTapWaterControl;

	if( NULL == m_pSelTapWaterControl )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelTapWaterControl );
}

void CDlgConfSel::Display( CDS_SSelSmartControlValve *pSelSmartControlValve )
{
	m_pSelSmartControlValve = pSelSmartControlValve;

	if( NULL == m_pSelSmartControlValve )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelSmartControlValve );
}

void CDlgConfSel::Display( CDS_SSelSmartDpC *pSelSmartDpController )
{
	m_pSelSmartDpC = pSelSmartDpController;

	if( NULL == m_pSelSmartDpC )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelSmartDpController );
}

void CDlgConfSel::Display( CDS_SSelFloorHeatingManifold *pSelFloorHeatingManifold )
{
	m_pSelFloorHeatingManifold = pSelFloorHeatingManifold;

	if( NULL == m_pSelFloorHeatingManifold )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelFloorHeatingManifold );
}

void CDlgConfSel::Display( CDS_SSelFloorHeatingValve *pSelFloorHeatingValve )
{
	m_pSelFloorHeatingValve = pSelFloorHeatingValve;

	if( NULL == m_pSelFloorHeatingValve )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelFloorHeatingValve );
}

void CDlgConfSel::Display( CDS_SSelFloorHeatingController *pSelFloorHeatingController )
{
	m_pSelFloorHeatingController = pSelFloorHeatingController;

	if( NULL == m_pSelFloorHeatingController )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelFloorHeatingController );
}

void CDlgConfSel::Display( CDS_SelPWQAccServices* pSelPWQAccAndServices )
{
	m_pSelPWQAccAndServices = pSelPWQAccAndServices;

	if( NULL == m_pSelPWQAccAndServices )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelPWQAccAndServices );
}

void CDlgConfSel::Display( CDS_Actuator *pSelAct )
{
	m_pSelAct = pSelAct;

	if( NULL == m_pSelAct )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelAct );
}

void CDlgConfSel::Display( CDS_Accessory *pSelAcc )
{
	m_pSelAcc = pSelAcc;

	if( NULL == m_pSelAcc )
	{
		ASSERT_RETURN;
	}

	_Display( FromSSelAcc );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgConfSel::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTONADD, m_ButtonAdd );
	DDX_Control( pDX, IDOK, m_ButtonOk );
	DDX_Control( pDX, IDC_BUTTONDELETE, m_ButtonDelete );
	DDX_Control( pDX, IDC_EDITQTY, m_EditQty );
	DDX_Control( pDX, IDC_COMBOREMARK, m_ComboRemark );
	DDX_Control( pDX, IDC_EDIT1STREF, m_Edit1stRef );
	DDX_Control( pDX, IDC_EDIT2NDREF, m_Edit2ndRef );
	DDX_Control( pDX, IDC_EDITREMARK, m_EditRemark );
	DDX_Control( pDX, IDC_LISTCTRL, m_ListCtrl );
	DDX_Control( pDX, IDC_GROUPINFO, m_GroupInfo );
	DDX_Control( pDX, IDC_GROUPREMARK, m_GroupRemark );
	DDX_Control( pDX, IDC_STATICIMG, m_staticImg );
	DDX_Control( pDX, IDC_CHECKEDITACC, m_ButtonCheckBox );
}

BOOL CDlgConfSel::OnInitDialog()
{
	// These variables are initialized in the '_Display' method. This one is called before creating this dialog.
	if( NULL == m_pTADB || NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( FALSE );
	}

	static CString strTT1st, strTT2nd;
	CDialogEx::OnInitDialog();
	m_ButtonCheckBox.SetCheck( BST_UNCHECKED );
	m_bApplyDefaultQty = false;
	m_bApplyAccessoriesQty = false;

	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_CAPTION );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_GROUPINFO );
	m_GroupInfo.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_STATIC1STREF );
	GetDlgItem( IDC_STATIC1STREF )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_STATIC2NDREF );
	GetDlgItem( IDC_STATIC2NDREF )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_STATICQTY );
	GetDlgItem( IDC_STATICQTY )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_GROUPREMARK );
	m_GroupRemark.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_STATICSELREM );
	GetDlgItem( IDC_STATICSELREM )->SetWindowText( str );

	CDS_TechnicalParameter *pTech = m_pTADS->GetpTechParams();
	CDB_Currency *pCur = ( CDB_Currency * )( m_pTADB->Get( m_pTADS->GetpTechParams()->GetCurrencyID() ).MP );
	ASSERT( NULL != pCur );

	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );

	// Add bitmap to the Add and Delete Remark buttons.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonAdd.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_SaveRemark ) );
		m_ButtonDelete.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_TrashBin ) );
	}

	// Load the icons into the group boxes.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupInfo.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Info );
		m_GroupRemark.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_PostIt );
	}

	// Limit size of text entry according to the DB.
	m_ComboRemark.LimitText( _DBLSTRING_LENGTHSTR1 - 1 );
	m_EditRemark.SetLimitText( _DBLSTRING_LENGTHSTR2 - 1 );

	m_Edit1stRef.SetLimitText( _STRING_LENGTH - 1 );
	m_Edit2ndRef.SetLimitText( _STRING_LENGTH - 1 );

	// Initialize quantity and price....
	m_lQty = 1;
	str.Format( _T("%u"), ( int )m_lQty );
	m_EditQty.SetWindowText( str );

	// Open Glossary and fill dropdown list.
	_InitGlossaryDD( true );

	_InitListCtrl();

	m_ToolTip.Create( this, TTS_NOPREFIX );
	m_ToolTip.Activate( TRUE );

	Invalidate();
	return TRUE;
}

void CDlgConfSel::OnCbnSelChangeRemark()
{
	m_bComboChanged = false;

	if( 0 == m_ComboRemark.GetCount() )
	{
		return;
	}

	if( m_ComboRemark.GetCurSel() < 0 )
	{
		return;    // No Selection
	}

	// Retrieve glossary pointer.
	CDB_DblString *pDblStr = ( CDB_DblString * )m_ComboRemark.GetItemDataPtr( m_ComboRemark.GetCurSel() );
	m_EditRemark.SetWindowText( pDblStr->GetString2() );
}

void CDlgConfSel::OnCbnEditChangeRemark()
{
	// Change in course.
	m_bComboChanged = true;
}

void CDlgConfSel::OnOK()
{
	if( NULL == m_pTADS )
	{
		ASSERT_RETURN;
	}

	// PREVENT_ENTER_KEY
	// the new glossary entry is not yet saved, don't try to find it into the glossary

	CString str;

	// HYS-1133: to reactivate enter key ( it was disabled because onkillfocus is not called before ).
	long lQtyEdited = 0;
	ReadLong( m_EditQty, &lQtyEdited );

	if( lQtyEdited != m_lQty )
	{
		// Call onKillFocus to save values.
		OnEnKillFocusEditQty();
	}
	
	if( true == m_bComboChanged )
	{
		CString str1;
		m_ComboRemark.GetWindowText( str1 );
		FormatString( str, IDS_CONFSEL_NEWGLONOTSAVED, str1 );

		if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
		{
			OnButtonAdd();
		}
	}

	// HYS-987: If the 'Edit accessories quantity' checkbox is unchecked and user clicks on the 'Cancel' button the value must not be applied.
	if( true == m_bApplyDefaultQty )
	{
		for( map<long, SelectedAccessories>::iterator it = m_mapSelectAccessories.begin(); it != m_mapSelectAccessories.end(); ++it )
		{
			// Set all accessories quantity with default values.
			SelectedAccessories AccSelected = it->second;

			if( NULL != AccSelected.pclSelectedAccList || NULL != AccSelected.pclSelectedActuator )
			{
				if( NULL == AccSelected.pclSelectedActuator )
				{
					AccSelected.pclSelectedAccList->SetEditedQty( AccSelected.AccItemFromAccList, -1 );
				}
				else
				{
					AccSelected.pclSelectedActuator->SetEditedQty( AccSelected.AccItemFromActuator, -1 );
				}
			}
		}
	}

	// HYS-987: If accessories quantity is modified in confSel and user clicks doesn't click on the 'Cancel' button, we can save values.
	if( true == m_bApplyAccessoriesQty )
	{
		for( map<long, SelectedAccessories>::iterator it = m_mapSelectAccessories.begin(); it != m_mapSelectAccessories.end(); ++it )
		{
			// Set all accessories quantity with default values
			CString str = CteEMPTY_STRING;
			str = m_ListCtrl.GetItemText( it->first, 1 );
			int ivalueToSet = _ttol( str );
			SelectedAccessories AccSelected = it->second;
			
			if( NULL != AccSelected.pclSelectedAccList || NULL != AccSelected.pclSelectedActuator )
			{
				if( NULL == AccSelected.pclSelectedActuator )
				{
					AccSelected.pclSelectedAccList->SetEditedQty( AccSelected.AccItemFromAccList, ivalueToSet );
				}
				else
				{
					AccSelected.pclSelectedActuator->SetEditedQty( AccSelected.AccItemFromActuator, ivalueToSet );
				}
			}
		}
	}
	
	CTable *pDpCTab = NULL;
	CSelectedInfos *pSelInfo = NULL;
	IDPTR IDPtrSelDpC = _NULL_IDPTR;

	switch( m_iFrom )
	{
		case FromSSelAcc:
			
			if( NULL == pSelInfo && NULL != m_pSelAcc )
			{
				pSelInfo = m_pSelAcc->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelAct:
			
			if( NULL == pSelInfo && NULL != m_pSelAct )
			{
				pSelInfo = m_pSelAct->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelBCv:
			
			if( NULL == pSelInfo && NULL != m_pSelBCv )
			{
				pSelInfo = m_pSelBCv->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelBV:
			
			if( NULL == pSelInfo && NULL != m_pSelBv )
			{
				pSelInfo = m_pSelBv->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelCv:
			
			if( NULL == pSelInfo && NULL != m_pSelCv )
			{
				pSelInfo = m_pSelCv->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelDpController:
			
			if( NULL == pSelInfo && NULL != m_pSelDpController )
			{
				pSelInfo = m_pSelDpController->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelDpReliefValve:
			
			if( NULL == pSelInfo && NULL != m_pSelDpReliefValve )
			{
				pSelInfo = m_pSelDpReliefValve->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelPICv:
			
			if( NULL == pSelInfo && NULL != m_pSelPICv )
			{
				pSelInfo = m_pSelPICv->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelTrv:
			
			if( NULL == pSelInfo && NULL != m_pSelRadSet )
			{
				pSelInfo = m_pSelRadSet->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelAirVentSep:
			
			if( NULL == pSelInfo && NULL != m_pSelAirVentSeparator )
			{
				pSelInfo = m_pSelAirVentSeparator->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelPMaint:
			
			if( NULL == pSelInfo && NULL != m_pSelPressureMaint )
			{
				pSelInfo = m_pSelPressureMaint->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelDpCBCV:
			
			if( NULL == pSelInfo && NULL != m_pSelDpCBCV )
			{
				pSelInfo = m_pSelDpCBCV->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelSv:
			
			if( NULL == pSelInfo && NULL != m_pSelSv )
			{
				pSelInfo = m_pSelSv->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelSafetyValve:
			
			if( NULL == pSelInfo && NULL != m_pSelSafetyValve )
			{
				pSelInfo = m_pSelSafetyValve->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );

			}

			break;

		case FromSSel6WayValve:
			
			if( NULL == pSelInfo && NULL != m_pSel6WayValve )
			{
				pSelInfo = m_pSel6WayValve->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelTapWaterControl:
			
			if( NULL == pSelInfo && NULL != m_pSelTapWaterControl )
			{
				pSelInfo = m_pSelTapWaterControl->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelSmartControlValve:
			
			if( NULL == pSelInfo && NULL != m_pSelSmartControlValve )
			{
				pSelInfo = m_pSelSmartControlValve->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelSmartDpController:
			// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
			if( NULL == pSelInfo && NULL != m_pSelSmartDpC )
			{
				pSelInfo = m_pSelSmartDpC->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;
			
		case FromSSelTALinkAlone:
			// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
			if( NULL == pSelInfo && NULL != m_pSelTALinkAlone )
			{
				pSelInfo = m_pSelTALinkAlone->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelFloorHeatingManifold:
			
			if( NULL == pSelInfo && NULL != m_pSelFloorHeatingManifold )
			{
				pSelInfo = m_pSelFloorHeatingManifold->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelFloorHeatingValve:
			
			if( NULL == pSelInfo && NULL != m_pSelFloorHeatingValve )
			{
				pSelInfo = m_pSelFloorHeatingValve->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelFloorHeatingController:
			
			if( NULL == pSelInfo && NULL != m_pSelFloorHeatingController )
			{
				pSelInfo = m_pSelFloorHeatingController->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;
			
		// HYS-1741
		case FromSSelPWQAccAndServices:
			
			if( NULL == pSelInfo && NULL != m_pSelPWQAccAndServices )
			{
				pSelInfo = m_pSelPWQAccAndServices->GetpSelectedInfos();
				_SaveRefsAndRemarks( pSelInfo );
			}

			break;

		case FromSSelH:
			
			if( NULL != m_pHmHub )
			{
				m_Edit1stRef.GetWindowText( str );
				m_pHmHub->GetpSelectedInfos()->SetReference( CSelectedInfos::eReferences::eRef1, str );
				m_Edit2ndRef.GetWindowText( str );
				m_pHmHub->GetpSelectedInfos()->SetReference( CSelectedInfos::eReferences::eRef2, str );
				m_pHmHub->GetpSelectedInfos()->SetQuantity( ( int )m_lQty );
				m_EditRemark.GetWindowText( str );
				m_pHmHub->GetpSelectedInfos()->SetRemark( str );
				*m_pHmHub->GetpSelectedInfos()->GetpWCData() = *m_pHmHub->GetpWC();

				CHubHandler HubHandler;

				// If we are in edition mode replace the selected item by the new ones
				if( _T('\0') != *m_pHmHub->GetpSelectedInfos()->GetModifiedObjIDPtr().ID )
				{
					HubHandler.Attach( m_pHmHub->GetpSelectedInfos()->GetModifiedObjIDPtr().ID );
					HubHandler.Delete( m_pHmHub->GetpSelectedInfos()->GetModifiedObjIDPtr() );
				}

				m_pHmHub->GetpSelectedInfos()->ClearModifiedObjIDptr();
				HubHandler.Attach( m_pHmHub->GetIDPtr().ID );
				HubHandler.MoveToSelectedTab();
				IDPTR idptrNew = HubHandler.Copy( _T("TMPHUB_TAB") );
			}

			break;
	}

	m_pTADS->RefreshResults( true );
	CDialogEx::OnOK();

	// Reset the focus on the flow edit box in the 'CDlgXXXSel' if possible.
	CDlgSelectionBase *pCurrentDlgSel = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pCurrentDlgSel = pDlgLeftTabSelManager->GetCurrentLeftTabDialog();
	}

	if( NULL != pCurrentDlgSel )
	{
		pCurrentDlgSel->SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
	}
}

void CDlgConfSel::OnEnChangeRemark()
{
	CString str;
	m_ComboRemark.GetWindowText( str );

	if( _T("") == str )
	{
		return;
	}

	// The new glossary entry is not yet saved, don't try to find it into the glossary.
	if( true == m_bComboChanged )
	{
		return;
	}

	CDB_DblString *pDblStr = ( CDB_DblString * )m_ComboRemark.GetItemDataPtr( m_ComboRemark.GetCurSel() );
	m_EditRemark.GetWindowText( str );

	if( str != pDblStr->GetString2() )
	{
		m_ComboRemark.SetCurSel( -1 );
	}
}

void CDlgConfSel::OnButtonAdd()
{
	try
	{
		if( false == m_bComboChanged )
		{
			return;
		}

		CString str, str1;
		m_ComboRemark.GetWindowText( str );

		if( _T("") == str )
		{
			return;
		}

		m_EditRemark.GetWindowText( str1 );

		if( _T("") == str1 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_EMPTYREMARK );
			m_ComboRemark.SetWindowText( _T("") );
			return;
		}

		IDPTR IDPtr;
		CDB_DblString *pGlossaryEntry;

		// Check if glossary entry exist.
		CTable *pGTab = dynamic_cast<CTable *>( m_pUSERDB->Get( _T("GLOSSARY_TAB") ).MP );
		
		if( NULL == pGTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'GLOSSARY_TAB' table from the user database.") );
		}

		// If string exist...
		if( pGTab->GetItemCount( CLASS( CDB_DblString ) ) > 0 )
		{
			for( IDPtr = pGTab->GetFirst( CLASS( CDB_DblString ) ); _T('\0') != *IDPtr.ID; IDPtr = pGTab->GetNext() )
			{
				pGlossaryEntry = ( CDB_DblString * )IDPtr.MP;
				str1 = pGlossaryEntry->GetString1();

				// If glossary entry already exist...
				if( str == str1 )
				{
					m_EditRemark.GetWindowText( str1 );

					// If remarks are same it's OK.
					if( str1 == pGlossaryEntry->GetString2() )
					{
						return;
					}

					FormatString( str1, AFXMSG_KEYEXIST, str );
					AfxMessageBox( str1, MB_OK | MB_ICONEXCLAMATION, 0 );
					m_ComboRemark.SetWindowText( _T("") );
					m_ComboRemark.SetFocus();
					return;
				}
			}
		}

		// Insert new glossary input.
		m_pUSERDB->CreateObject( IDPtr, CLASS( CDB_DblString ) );
		pGTab->Insert( IDPtr );

		pGlossaryEntry = ( CDB_DblString * )IDPtr.MP;
		pGlossaryEntry->SetString1( ( TCHAR * )( LPCTSTR )str );
		m_EditRemark.GetWindowText( str );
		pGlossaryEntry->SetString2( ( TCHAR * )( LPCTSTR )str );
		
		m_ComboRemark.GetWindowText( str );
		_InitGlossaryDD( true, str );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgConfSel::OnButtonAdd'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgConfSel::OnButtonDelete()
{
	CString str, str1;

	// Check if one glossary entry is selected.
	m_ComboRemark.GetWindowText( str );

	if( _T("") == str )
	{
		return;
	}

	// Retrieve glossary pointer.
	CDB_DblString *pDblStr = (CDB_DblString *)m_ComboRemark.GetItemDataPtr( m_ComboRemark.GetCurSel() );
	FormatString( str1, AFXMSG_DELETEITEM, str );

	if( AfxMessageBox( str1, MB_YESNO | MB_ICONQUESTION, 0 ) != IDYES )
	{
		return;
	}

	IDPTR IDPtrDblStr = ( (CDB_DblString *)pDblStr )->GetIDPtr();
	ASSERT( *IDPtrDblStr.ID );

	// Get Owner.
	CTable *pGTab = (CTable *)( m_pUSERDB->Get( _T("GLOSSARY_TAB") ).MP );
	ASSERT( NULL != pGTab );

	pGTab->Remove( IDPtrDblStr );
	VERIFY( 0 != m_pUSERDB->DeleteObject( IDPtrDblStr ) );
	
	_InitGlossaryDD( false );
}

void CDlgConfSel::OnClickedCheckBox()
{
	if( BST_CHECKED == m_ButtonCheckBox.GetCheck() )
	{
		m_EditQty.EnableWindow( false );
		m_bApplyDefaultQty = false;
	}
	else
	{
		m_EditQty.EnableWindow( true );
		CString str = CteEMPTY_STRING;
		m_EditQty.GetWindowTextW( str );
		unsigned int iQty = _ttoi( str );
		_SetAllEditableQuantity( iQty );
	}
}

void CDlgConfSel::OnNMClickList( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( BST_CHECKED != m_ButtonCheckBox.GetCheck() )
	{
		m_ListCtrl.SendLButtonDown( MK_LBUTTON, GetListCtrlCursorPosition(), false );
		return;
	}
	
	CPoint pt = GetListCtrlCursorPosition();
	CPoint ptToSearch = pt;
	
	// Manage vertical scrollbar.
	int iPos;
	m_ListCtrl.GetVertScrollPosition( &iPos );
	
	if( iPos > 0 )
	{
		ptToSearch.y = -1;
	}
	
	bool bFound = false;
	// HYS-1968: Use map instead of vector
	map<long, RECT>::iterator it;

	for( it = m_mapEditableQuantity.begin(); it != m_mapEditableQuantity.end(); ++it )
	{
		CRect rQty = it->second;

		if( -1 == ptToSearch.y )
		{
			ptToSearch.y = pt.y + (rQty.Height() * iPos);
		}
		
		if( rQty.PtInRect( ptToSearch ) )
		{
			bFound = true;
			break;
		}	
	}
	
	if( bFound == true )
	{
		m_ListCtrl.SendLButtonDown( MK_LBUTTON, pt, true );
	}
	else
	{
		m_ListCtrl.SendLButtonDown( MK_LBUTTON, pt, false );
	}

	*pResult = 0;
}

void CDlgConfSel::OnEnKillFocusEditQty( )
{
	CString str = CteEMPTY_STRING;

	switch( ReadLong( m_EditQty, &m_lQty ) )
	{
		case RD_EMPTY:
			m_lQty = 1;
			str.Format( _T("%u"), (int)m_lQty );
			m_EditQty.SetWindowText( str );
			_SetAllEditableQuantity( m_lQty );
			m_EditQty.SetSel( 0, -1 );
			break;

		case RD_NOT_NUMBER:
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_lQty = 1;
			str.Format( _T("%u"), (int)m_lQty );
			m_EditQty.SetWindowText( str );
			_SetAllEditableQuantity( m_lQty );
			m_EditQty.SetSel( 0, -1 );
			break;

		case RD_OK:

			if( m_lQty > _MAX_QTY || m_lQty < 0 )									// Max quantity
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_lQty = _MAX_QTY;
				str.Format( _T("%u"), (int)m_lQty );
				m_EditQty.SetWindowText( str );
				_SetAllEditableQuantity( m_lQty );
				m_EditQty.SetSel( 0, -1 );
			}
			else if( m_EditQty.GetModify() && 0 != m_EditQty.GetWindowTextLengthW() )
			{
				_SetAllEditableQuantity( m_lQty );
			}

			break;
	}
}

// Obtain cursor position and offset it to position it at interview list control
CPoint CDlgConfSel::GetListCtrlCursorPosition() const
{
	DWORD pos = GetMessagePos();
	CPoint pt( GET_X_LPARAM( pos ), GET_Y_LPARAM( pos ) );
	ScreenToClient( &pt );

	CRect rect;
	CWnd *pWnd = GetDlgItem( IDC_LISTCTRL );
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );

	pt.x -= rect.left;
	pt.y -= rect.top;

	return pt;
}

LRESULT CDlgConfSel::OnNotifyDescriptionEdited( WPARAM wParam, LPARAM lParam )
{
	// Get the changed Description field text via the callback.
	LV_DISPINFO *dispinfo = reinterpret_cast<LV_DISPINFO *>( lParam );

	// Get the iRow changed.
	unsigned int *lpvalue = m_mapQuantityRef.at( dispinfo->item.iItem );
	int ivalueToSet = _ttol( dispinfo->item.pszText );

	if( ( 0 < ivalueToSet ) && ( CteEMPTY_STRING != dispinfo->item.pszText ) )
	{
		m_bApplyAccessoriesQty = true;
		
		// Persist the selected attachment details upon updating its text.
		m_ListCtrl.SetItemText( dispinfo->item.iItem, dispinfo->item.iSubItem, dispinfo->item.pszText );
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgConfSel::_Display( int iFrom )
{
	if( NULL != m_pclProdSelParams )
	{
		m_pTADB = m_pclProdSelParams->m_pTADB;
		m_pTADS = m_pclProdSelParams->m_pTADS;
		m_pUSERDB = m_pclProdSelParams->m_pUserDB;
	}
	else
	{
		m_pTADB = TASApp.GetpTADB();
		m_pTADS = TASApp.GetpTADS();
		m_pUSERDB = TASApp.GetpUserDB();
	}

	m_pUnitDB = CDimValue::AccessUDB();

	m_iFrom = iFrom;
}

void CDlgConfSel::_InitListCtrl()
{
	if( NULL == m_pTADB || NULL == m_pTADS )
	{
		ASSERT_RETURN;
	}

	// Get a full iRow selected.
	m_ListCtrl.SetExtendedStyle( m_ListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

	// Disable the possibility for someone to click on the m_ListCtrl.
	m_ListCtrl.SetClickListCtrlDisable( true );

	// Clean list control.
	m_ListCtrl.DeleteAllItems();

	// Clean the bitmap.
	m_Bmp.DeleteObject();

	CRect rect;
	m_ListCtrl.GetWindowRect( &rect );
	rect.right -= 4;

	CString str;
	CDB_TAProduct *pTAPpict = NULL;

	// Add column title.
	switch( m_iFrom )
	{
		case FromSSelAcc:
		case FromSSelAct:
		case FromSSelBCv:
		case FromSSelBV:
		case FromSSelCv:
		case FromSSelDpController:
		case FromSSelDpReliefValve:
		case FromSSelPICv:
		case FromSSelTrv:
		case FromSSelAirVentSep:
		case FromSSelPMaint:
		case FromSSelDpCBCV:
		case FromSSelSv:
		case FromSSelSafetyValve:
		case FromSSel6WayValve:
		case FromSSelTapWaterControl:
		case FromSSelSmartControlValve:
		case FromSSelSmartDpController:   // HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
		case FromSSelFloorHeatingManifold:
		case FromSSelFloorHeatingValve:
		case FromSSelFloorHeatingController:
		case FromSSelPWQAccAndServices:
		case FromSSelTALinkAlone:

			str = TASApp.LoadLocalizedString( IDS_CONFSEL_DESCRIPTION );
			m_ListCtrl.InsertColumn( 0, str, LVCFMT_LEFT, rect.Width()-89, 0 );
			
			str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_STATICQTY );
			m_ListCtrl.InsertColumn( 1, str, LVCFMT_CENTER, 59 );

			break;

		case FromSSelH:
			{
				// One column.
				ASSERT( NULL != m_pHmHub );

				if( NULL != m_pHmHub )
				{
					FormatString( str, IDS_DIALOGCONFSEL_TAHUB, m_pHmHub->GetHMName() );
					m_ListCtrl.InsertColumn( 0, str, LVCFMT_LEFT, rect.Width(), 0 );
				}
			}
			break;

		default:

			str = TASApp.LoadLocalizedString( IDS_CONFSEL_DESCRIPTION );
			m_ListCtrl.InsertColumn( 0, str, LVCFMT_LEFT, rect.Width()-89, 0 );
		
			str = TASApp.LoadLocalizedString( IDS_DLGCONFSEL_STATICQTY );
			m_ListCtrl.InsertColumn( 1, str, LVCFMT_CENTER, 59 );

			break;
	}

	IDPTR IDPtr;
	CSelectedInfos *pSelInfo = NULL;
	CDS_SSelCtrl *pSelCtrl = NULL;
	CDS_SSel *pSSel = NULL;

	switch( m_iFrom )
	{
		case FromSSelAcc:
		{
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( m_pSelAcc->GetAccessoryIDPtr().MP );
			VERIFY( NULL != pclAccessory );

			if( ( NULL == pclAccessory ) || ( false == pclAccessory->IsAnAccessory() ) )
			{
				break;
			}

			CString str;
			int row = 0;

			// Create empty lines for the actuator.
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_ACC );
			row = m_ListCtrl.AddItemText( str );
			m_ListCtrl.SetItemTextBold( row, 0, true );
			str = pclAccessory->GetName() + CString( _T(" : ") ) + pclAccessory->GetComment();
			row = m_ListCtrl.AddItemText( str );

			_InitRefsAndRemarks( m_pSelAcc->GetpSelectedInfos() );

		}
		break;

		case FromSSelAct:
		{
			// Change the CListCtrl height if it's a Control Valve Product.
			CRect rectImg, rectListCtrl;
			m_ListCtrl.GetWindowRect( &rectListCtrl );
			ScreenToClient( &rectListCtrl );
			GetDlgItem( IDC_STATICIMG )->GetWindowRect( &rectImg );
			ScreenToClient( &rectImg );

			m_ListCtrl.SetWindowPos( &CWnd::wndBottom, 0, 0, rectListCtrl.Width(), rectImg.bottom - rectListCtrl.top, SWP_NOZORDER | SWP_NOMOVE );
			GetDlgItem( IDC_STATICIMG )->ShowWindow( false );

			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( m_pSelAct->GetActuatorIDPtr().MP );
			VERIFY( NULL != pclActuator );

			if( NULL == pclActuator )
			{
				break;
			}

			_FillActuators( m_pSelAct->GetpSelectedInfos(), pclActuator, NULL );

			// Check to know if they are accessories or not.
			if( m_pSelAct->GetpActuatorAccessoryList()->size() > 0 )
			{
				// White line.
				m_ListCtrl.AddItemText( _T("") );
				str = TASApp.LoadLocalizedString( IDS_CONFSEL_ACTACCESSORY );
				long lRow = m_ListCtrl.AddItemText( str );
				m_ListCtrl.SetItemTextBold( lRow, 0, true );

				// HYS-987: Built-in accessories
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclActuator->GetAccessoriesGroupIDPtr().MP );
				std::vector<CData *> vecBuiltInAccessories;
				std::vector<CData *>::iterator itBI;
				if ( NULL != pclRuledTable )
					pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );

				for( int i = 0; i < ( int )m_pSelAct->GetpActuatorAccessoryList()->size(); i++ )
				{
					CDS_Actuator::AccessoryItem rAccessoryItem = m_pSelAct->GetpActuatorAccessoryList()->at( i );
					CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
					VERIFY( NULL != pclAccessory );

					if( ( NULL == pclAccessory ) || ( false == pclAccessory->IsAnAccessory() ) )
					{
						break;
					}
					int iQuantity = (true == rAccessoryItem.fByPair)? 2 : 1;

					CString str( _T("") );

					if( true == rAccessoryItem.fByPair )
					{
						str = _T("2x ");
					}

					str += pclAccessory->GetName() + CString( _T(" : ") ) + pclAccessory->GetComment();
					lRow = m_ListCtrl.AddItemText( str );

					// HYS-987.
					itBI = find( vecBuiltInAccessories.begin(), vecBuiltInAccessories.end(), pclAccessory );
					
					if( itBI != vecBuiltInAccessories.end() )
					{
						// Quantity of built-in accessories can't be edited.
						_AddQuantity( lRow, iQuantity, m_pSelAct->GetpSelectedInfos()->GetQuantity(), rAccessoryItem.lEditedQty, false );
					}
					else
					{
						_AddQuantity( lRow, iQuantity, m_pSelAct->GetpSelectedInfos()->GetQuantity(), rAccessoryItem.lEditedQty, true );
					}
					
					SelectedAccessories structAcc;
					structAcc.pclSelectedActuator = m_pSelAct;
					structAcc.AccItemFromActuator.fByPair = rAccessoryItem.fByPair;
					structAcc.AccItemFromActuator.IDPtr = rAccessoryItem.IDPtr;
					structAcc.AccItemFromActuator.lEditedQty = rAccessoryItem.lEditedQty;
					m_mapSelectAccessories.insert( make_pair( lRow, structAcc ) );
				}
			}

			_InitRefsAndRemarks( m_pSelAct->GetpSelectedInfos() );
		}
		break;

		case FromSSelBCv:
			if( NULL != m_pSelBCv )
			{
				_FillListCtrlForControlValve( m_pSelBCv->GetpSelectedInfos(), m_pSelBCv );
			}

			break;

		case FromSSelBV:
			if( NULL != m_pSelBv )
			{
				pSelInfo = m_pSelBv->GetpSelectedInfos();
				pSSel = m_pSelBv;
				CDB_TAProduct *pTAP = pSSel->GetProductAs<CDB_TAProduct>();

				if( NULL == pTAP )
				{
					ASSERT( 0 );
					break;
				}

				// If product contains a comment, we add it to the remark list.
				if( true == pSelInfo->GetRemark().IsEmpty() )
				{
					if( false == CString( pTAP->GetComment() ).IsEmpty() )
					{
						pSelInfo->SetRemark( pTAP->GetComment() );
					}
				}

				_InitRefsAndRemarks( pSelInfo );

				CAccessoryList *pclAccessoryList = pSSel->GetAccessoryList();

				long lRow = 0;
				lRow = m_ListCtrl.AddItemText( pTAP->GetType() );
				m_ListCtrl.SetItemTextBold( lRow, 0, true );

				lRow = m_ListCtrl.AddItemText( pTAP->GetName() );
				int iGroupQuantity = pSSel->GetpSelectedInfos()->GetQuantity();

				// HYS-987.
				_AddQuantity( lRow, 1, iGroupQuantity );
				lRow = _AddTAProductInfos( lRow, pTAP );

				// Add accessories infos.
				int iIDS = IDS_CONFSEL_BV_ACCESSORY;

				// HYS-987: Built-in accessories.
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pTAP->GetAccessoriesGroupIDPtr().MP );
				std::vector<CData *> vecBuiltInAccessories;

				if( NULL != pclRuledTable )
				{
					pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
				}

				_AddAccessoriesInfos( lRow, iIDS, pclAccessoryList, iGroupQuantity, vecBuiltInAccessories );
			}

			break;

		case FromSSelTrv:
			if( NULL != m_pSelRadSet )
			{
				_FillListCtrlForRadiatorSet( m_pSelRadSet->GetpSelectedInfos(), m_pSelRadSet );
			}

			break;

		case FromSSelAirVentSep:
			if( NULL != m_pSelAirVentSeparator )
			{
				_FillListCtrlForAirVentSep( m_pSelAirVentSeparator->GetpSelectedInfos(), m_pSelAirVentSeparator );
			}

			break;

		case FromSSelPMaint:
			if( NULL != m_pSelPressureMaint )
			{
				_FillListCtrlForPressMaint( m_pSelPressureMaint->GetpSelectedInfos(), m_pSelPressureMaint );
			}

			break;

		case FromSSelCv:
			if( NULL != m_pSelCv )
			{
				_FillListCtrlForControlValve( m_pSelCv->GetpSelectedInfos(), m_pSelCv );
			}

			break;

		case FromSSelDpController:
			if( NULL != m_pSelDpController )
			{
				pSelInfo = m_pSelDpController->GetpSelectedInfos();
				CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( m_pSelDpController->GetDpCIDPtr().MP );

				if( NULL == pclDpController )
				{
					ASSERT( 0 );
					break;
				}

				CDB_TAProduct *pMV = m_pSelDpController->GetProductAs<CDB_TAProduct>();
				long lRow = 0;

				// If product contains a comment, we add it to the remark list.
				// First the Dp controller valve.
				CString strRemark( _T("") );

				if( false == CString( pclDpController->GetComment() ).IsEmpty() )
				{
					strRemark = pclDpController->GetComment();
				}

				// Add remark on balancing valve if exist.
				if( NULL != pMV && false == CString( pMV->GetComment() ).IsEmpty() )
				{
					if( false == strRemark.IsEmpty() )
					{
						strRemark += _T("\r\n");
					}

					strRemark += pMV->GetComment();
				}

				// Set now the remark in 'pSelInfo'.
				if( false == strRemark.IsEmpty() )
				{
					pSelInfo->SetRemark( strRemark );
				}

				_InitRefsAndRemarks( pSelInfo );

				// Check if an DpCMvSet exist.
				IDPTR DpCMvIDPtr = m_pSelDpController->GetDpCMvPackageIDPtr();

				CString str;

				if( true == m_pSelDpController->IsSelectedAsAPackage() && NULL != dynamic_cast<CDB_Set *>( DpCMvIDPtr.MP ) )
				{
					CDB_Set *DpcMvSet = dynamic_cast<CDB_Set *>( DpCMvIDPtr.MP );
					str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPCMVSET );
					lRow = m_ListCtrl.AddItemText( str );
					m_ListCtrl.SetItemTextBold( lRow, 0, true );
					str = DpcMvSet->GetName();
					lRow = m_ListCtrl.AddItemText( str );
					//HYS-987
					int iGroupQuantity = pSelInfo->GetQuantity();
					_AddQuantity( lRow, 1, iGroupQuantity );
					// White line.
					lRow = m_ListCtrl.AddItemText( _T("") );
				}

				// DpC.
				lRow = m_ListCtrl.AddItemText( pclDpController->GetType() );
				m_ListCtrl.SetItemTextBold( lRow, 0, true );

				m_ListCtrl.AddItemText( pclDpController->GetName() );
				
				// HYS-987.
				int iGroupQuantity = pSelInfo->GetQuantity();
				_AddQuantity( lRow, 1, iGroupQuantity );
				lRow = _AddTAProductInfos( lRow, pclDpController );
				int iQuantity = m_pSelDpController->GetpSelectedInfos()->GetQuantity();
				
				// HYS-987: Built-int accessories.
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclDpController->GetAccessoriesGroupIDPtr().MP );
				std::vector<CData *> vecBuiltInAccessories;

				if( NULL != pclRuledTable )
				{
					pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
				}

				lRow = _AddAccessoriesInfos( lRow, IDS_CONFSEL_DPC_ACCESSORY, m_pSelDpController->GetDpCAccessoryList(), iQuantity, vecBuiltInAccessories );

				// Measuring valve.

				if( NULL != pMV )
				{
					lRow = m_ListCtrl.AddItemText( _T("") );
					lRow = m_ListCtrl.AddItemText( pMV->GetType() );
					m_ListCtrl.SetItemTextBold( lRow, 0, true );
					lRow = m_ListCtrl.AddItemText( pMV->GetName() );
					//HYS-987
					_AddQuantity( lRow, 1, iGroupQuantity );
					lRow = _AddTAProductInfos( lRow, pMV );

					// HYS-987: Built-in accessories
					CDB_RuledTable *pclRuledTableBv = dynamic_cast<CDB_RuledTable *>( pMV->GetAccessoriesGroupIDPtr().MP );
					std::vector<CData *> vecBuiltInAccessoriesBv;
					if( NULL != pclRuledTable )
						pclRuledTableBv->GetBuiltIn( &vecBuiltInAccessoriesBv );

					lRow = _AddAccessoriesInfos( lRow, IDS_CONFSEL_BV_ACCESSORY, m_pSelDpController->GetAccessoryList(), iQuantity, vecBuiltInAccessoriesBv );
				}
			}

			break;

		case FromSSelDpReliefValve:
			if( NULL != m_pSelDpReliefValve )
			{
				_FillListCtrlForDpReliefValve( m_pSelDpReliefValve->GetpSelectedInfos(), m_pSelDpReliefValve );
			}

			break;

		case FromSSelPICv:
			if( NULL != m_pSelPICv )
			{
				_FillListCtrlForControlValve( m_pSelPICv->GetpSelectedInfos(), m_pSelPICv );
			}

			break;

		case FromSSelDpCBCV:
			if( NULL != m_pSelDpCBCV )
			{
				_FillListCtrlForControlValve( m_pSelDpCBCV->GetpSelectedInfos(), m_pSelDpCBCV );
			}

			break;

		case FromSSelSv:
			if( NULL != m_pSelSv )
			{
				_FillListCtrlForShutOffValve( m_pSelSv->GetpSelectedInfos(), m_pSelSv );
			}

			break;

		case FromSSelSafetyValve:
			if( NULL != m_pSelSafetyValve )
			{
				_FillListCtrlForSafetyValve( m_pSelSafetyValve->GetpSelectedInfos(), m_pSelSafetyValve );
			}

			break;

		case FromSSel6WayValve:
			if( NULL != m_pSel6WayValve )
			{
				_FillListCtrlFor6WayValve( m_pSel6WayValve->GetpSelectedInfos(), m_pSel6WayValve );
			}

			break;

		case FromSSelTapWaterControl:
			if( NULL != m_pSelTapWaterControl )
			{
				_FillListCtrlForTapWaterControl( m_pSelTapWaterControl->GetpSelectedInfos(), m_pSelTapWaterControl );
			}

			break;

		case FromSSelSmartControlValve:
			if( NULL != m_pSelSmartControlValve )
			{
				_FillListCtrlForSmartControlValve( m_pSelSmartControlValve->GetpSelectedInfos(), m_pSelSmartControlValve );
			}

			break;

		case FromSSelSmartDpController:
			// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
			if( NULL != m_pSelSmartDpC )
			{
				_FillListCtrlForSmartDpC( m_pSelSmartDpC->GetpSelectedInfos(), m_pSelSmartDpC );
			}

			break;

		case FromSSelTALinkAlone:
			// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
			if( NULL != m_pSelTALinkAlone )
			{
				_FillListCtrlForTALinkAlone( m_pSelTALinkAlone->GetpSelectedInfos(), m_pSelTALinkAlone );
			}

			break;

		case FromSSelFloorHeatingManifold:
			if( NULL != m_pSelFloorHeatingManifold )
			{
				_FillListCtrlForFloorHeatingManifold( m_pSelFloorHeatingManifold->GetpSelectedInfos(), m_pSelFloorHeatingManifold );
			}

			break;

		case FromSSelFloorHeatingValve:
			if( NULL != m_pSelFloorHeatingValve )
			{
				_FillListCtrlForFloorHeatingValve( m_pSelFloorHeatingValve->GetpSelectedInfos(), m_pSelFloorHeatingValve );
			}

			break;

		case FromSSelFloorHeatingController:
			if( NULL != m_pSelFloorHeatingController )
			{
				_FillListCtrlForFloorHeatingController( m_pSelFloorHeatingController->GetpSelectedInfos(), m_pSelFloorHeatingController );
			}

			break;

		// HYS-1741
		case FromSSelPWQAccAndServices:
			if( NULL != m_pSelPWQAccAndServices )
			{
				_FillPWQAccAndServices( m_pSelPWQAccAndServices->GetpSelectedInfos(), m_pSelPWQAccAndServices );
			}

			break;

		case FromSSelH:

			// Create empty lines.
			for( int i = 0; i < 7; i++ )
			{
				m_ListCtrl.InsertItem( 0, _T("") );
			}

			if( NULL != m_pHmHub )
			{
				// Disable edit quantity.
				m_EditQty.EnableWindow( false );
				int row = 0;

				// Description.
				CString str = m_pHmHub->GetDescription();

				if( false == str.IsEmpty() )
				{
					m_ListCtrl.SetItem( row++, 0, LVIF_TEXT, ( LPCTSTR )str, 0, 0, 0, 0 );
				}

				// Number of stations.
				CString str1;
				str1.Format( _T("%d"), m_pHmHub->GetCount() );
				FormatString( str, IDS_DIALOGCONFSEL_NBSTATIONS, str1 );
				m_ListCtrl.SetItem( row++, 0, LVIF_TEXT, ( LPCTSTR )str, 0, 0, 0, 0 );

				// First reference LabelID.
				m_Edit1stRef.SetLimitText( TASApp.GetSiteNameMaxChar() );
				m_Edit1stRef.SetWindowText( m_pHmHub->GetHMName() );

				// Second reference description.
				m_Edit2ndRef.SetLimitText( HUB_MAXLENGTH_DESCRIPTION );
				m_Edit2ndRef.SetWindowText( m_pHmHub->GetDescription() );

				// Total flow.
				if( m_pHmHub->GetQ() != 0.0 )
				{
					str1 = WriteCUDouble( _U_FLOW, m_pHmHub->GetQ(), true );
					FormatString( str, IDS_DIALOGCONFSEL_TOTALFLOW, str1 );
					m_ListCtrl.SetItem( row++, 0, LVIF_TEXT, ( LPCTSTR )str, 0, 0, 0, 0 );
				}

				// Main supply.
				if( CDS_HmHub::MainFeedPos::Left == m_pHmHub->GetMainFeedPos() )
				{
					str = TASApp.LoadLocalizedString( IDS_DIALOGCONFSEL_MAINLEFTSUPPLY );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_DIALOGCONFSEL_MAINRIGHTSUPPLY );
				}

				m_ListCtrl.SetItem( row++, 0, LVIF_TEXT, ( LPCTSTR )str, 0, 0, 0, 0 );

				// Dp Controller.
				if( _T("DPCTYPE_STD") == m_pHmHub->GetBalTypeID() )
				{
					// Dp Controller.
					IDPTR IDPtr = m_pTADB->Get( m_pHmHub->GetReturnValveID() );
					ASSERT( IDPtr.MP );

					if( NULL != IDPtr.MP )
					{
						IDPTR IDPtrDpC = ( ( CDB_HubValv * )IDPtr.MP )->GetValveIDPtr();
						ASSERT( IDPtrDpC.MP );

						str1 = ( ( CDB_DpController * )IDPtrDpC.MP )->GetName() + CString( _T(" ") ) + ( ( CDB_DpController * )IDPtrDpC.MP )->GetFormatedDplRange().c_str();
						FormatString( str, IDS_DIALOGCONFSEL_DPCONTROLLER, str1 );
						m_ListCtrl.SetItem( row++, 0, LVIF_TEXT, ( LPCTSTR )str, 0, 0, 0, 0 );
						//HYS-987
						int iGroupQuantity = m_pHmHub->GetpSelectedInfos()->GetQuantity();
						_AddQuantity( row, 1, iGroupQuantity );
					}

					// Measuring valve.
					IDPtr = m_pTADB->Get( m_pHmHub->GetSupplyValveID() );
					ASSERT( IDPtr.MP );

					if( NULL != IDPtr.MP )
					{
						IDPTR IDPtrBv = ( ( CDB_HubValv * )IDPtr.MP )->GetValveIDPtr();
						ASSERT( IDPtrBv.MP );

						str1 = ( ( CDB_TAProduct * )IDPtrBv.MP )->GetName() ;
						FormatString( str, IDS_DIALOGCONFSEL_MEASURINGVALVE, str1 );
						m_ListCtrl.SetItem( row++, 0, LVIF_TEXT, ( LPCTSTR )str, 0, 0, 0, 0 );
						//HYS-987
						int iGroupQuantity = m_pHmHub->GetpSelectedInfos()->GetQuantity();
						_AddQuantity( row, 1, iGroupQuantity );
					}
				}

				// TA-Hub Connection.
				if( NULL != m_pHmHub->GetPartnerConnectIDPtr().MP )
				{
					CDB_MultiStringExt *pMS = ( CDB_MultiStringExt * )m_pHmHub->GetPartnerConnectIDPtr().MP;
					FormatString( str, IDS_DIALOGCONFSEL_TAHUBCONNECTION, pMS->GetString( CDS_HmHub::ExtConn::eConnectName ) );
					m_ListCtrl.SetItem( row++, 0, LVIF_TEXT, ( LPCTSTR )str, 0, 0, 0, 0 );
				}

				// Stations Connection.
				if( NULL != m_pHmHub->GetStationsConnectIDPtr().MP )
				{
					CDB_MultiStringExt *pMS = ( CDB_MultiStringExt * )m_pHmHub->GetStationsConnectIDPtr().MP;
					FormatString( str, IDS_DIALOGCONFSEL_STATIONCONNECTION, pMS->GetString( CDS_HmHub::ExtConn::eConnectName ) );
					m_ListCtrl.SetItem( row++, 0, LVIF_TEXT, ( LPCTSTR )str, 0, 0, 0, 0 );
				}

				CHubHandler HubHandler;
				HubHandler.Attach( m_pHmHub->GetIDPtr().ID );

				if( true == HubHandler.BuildHubImg() )
				{
					CRect rect;
					GetDlgItem( IDC_STATICIMG )->GetClientRect( &rect );
					HubHandler.GetHubImg()->ResizeImage( CSize( rect.Width() - 4, rect.Height() - 4 ) );
					m_Bmp.CopyImage( HubHandler.GetHubImg() );
					( ( CStatic * )GetDlgItem( IDC_STATICIMG ) )->SetBitmap( m_Bmp );
				}

				if( NULL != m_pHmHub->GetpSelectedInfos() )
				{
					// If selected info exist overwrite default informations by user informations, we are in re-edit mode.
					m_Edit1stRef.SetWindowText( m_pHmHub->GetpSelectedInfos()->GetReference( CSelectedInfos::eReferences::eRef1 ) );
					m_Edit2ndRef.SetWindowText( m_pHmHub->GetpSelectedInfos()->GetReference( CSelectedInfos::eReferences::eRef2 ) );
					str = m_pHmHub->GetpSelectedInfos()->GetRemark();

					if( false == str.IsEmpty() )
					{
						_InitRemark( str );
					}
				}

				if( FromSSelH == m_iFrom )
				{
					// Format the string before putting it into the remark list.
					CDB_HubValv *pHubValv = dynamic_cast<CDB_HubValv *>( m_pHmHub->GetSupplyValveIDPtr().MP );
					ASSERT( pHubValv );
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pHubValv->GetValveIDPtr().MP );
					ASSERT( pTAP );
					pTAP->ApplyComment( &m_strComment );

					pHubValv = dynamic_cast<CDB_HubValv *>( m_pHmHub->GetReturnValveIDPtr().MP );
					ASSERT( pHubValv );
					pTAP = dynamic_cast<CDB_TAProduct *>( pHubValv->GetValveIDPtr().MP );
					ASSERT( pTAP );
					pTAP->ApplyComment( &m_strComment );

					for( IDPTR idptr = m_pHmHub->GetFirst(); NULL != *idptr.ID; idptr = m_pHmHub->GetNext() )
					{
						CDS_HmHubStation *pHubStation = ( CDS_HmHubStation * )idptr.MP;
						CDB_HubStaValv *pHubStaValv = dynamic_cast<CDB_HubStaValv *>( pHubStation->GetSupplyValveIDPtr().MP );
						ASSERT( pHubStaValv );
						pTAP = dynamic_cast<CDB_TAProduct *>( pHubStaValv->GetValveIDPtr().MP );
						ASSERT( pTAP );
						pTAP->ApplyComment( &m_strComment );

						pHubStaValv = dynamic_cast<CDB_HubStaValv *>( pHubStation->GetReturnValveIDPtr().MP );
						ASSERT( pHubStaValv );
						pTAP = dynamic_cast<CDB_TAProduct *>( pHubStaValv->GetValveIDPtr().MP );
						ASSERT( pTAP );
						pTAP->ApplyComment( &m_strComment );
					}

					// Set the remark.
					m_EditRemark.SetWindowText( m_strComment.c_str() );
				}
			}

			break;
	}

	// Load the image.
	CDB_Product *pclProduct = NULL;

	if( NULL != m_pSelAcc )
	{
		pclProduct = dynamic_cast<CDB_Product *>( m_pSelAcc->GetIDPtr().MP );
	}
	else if( NULL != m_pSelAct )
	{
		pclProduct = dynamic_cast<CDB_Product *>( m_pSelAct->GetIDPtr().MP );
	}
	else if( NULL != m_pSelBCv )
	{
		pclProduct = m_pSelBCv->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelBv )
	{
		pclProduct = dynamic_cast<CDB_Product *>( m_pSelBv->GetProductIDPtr().MP );
	}
	else if( NULL != m_pSelCv )
	{
		pclProduct = m_pSelCv->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelDpController )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( m_pSelDpController->GetDpCIDPtr().MP );
		CDB_TAProduct *pValv = dynamic_cast<CDB_TAProduct *>( m_pSelDpController->GetProductIDPtr().MP );

		if( NULL == pValv && NULL != pDpC )
		{
			pclProduct = pDpC;
		}
		else
		{
			if( NULL == pValv || NULL == pDpC )
			{
				return;
			}

			// Just to get the picture.
			CDlgInfoSSelDpC *pSelInfoDpC = new CDlgInfoSSelDpC( m_pclProdSelParams, this );
			bool bDpOk = false;

			if( eDpStab::DpStabOnBranch == m_pSelDpController->GetDpStab() )
			{
				bDpOk = ( m_pSelDpController->GetDpL() > 0 );
			}
			else
			{
				bDpOk = ( m_pSelDpController->GetKv() > 0 );
			}

			pSelInfoDpC->SetSchemePict( m_pSelDpController->GetDpStab(), m_pSelDpController->GetMvLoc(), pDpC->GetDpCLoc(), bDpOk, m_pSelDpController->GetDpToStab(), m_pSelDpController->GetKv(),
					m_pSelDpController->IsSelectedAsAPackage() );

			CDB_ValveCharacteristic *pValveCharacteristic = pValv->GetValveCharacteristic();

			if( NULL != pValveCharacteristic )
			{
				CWaterChar *pWC = NULL;

				if( NULL != m_pclProdSelParams )
				{
					pWC = &m_pclProdSelParams->m_WC;
				}
				else
				{
					pWC = m_pTADS->GetpWCForProductSelection()->GetpWCData();
				}

				double dValveDp = 0.0;

				if( true == pValveCharacteristic->GetValveDp( m_pSelDpController->GetQ(), &dValveDp, m_pSelDpController->GetOpening(), pWC->GetDens(), pWC->GetKinVisc() ) )
				{
					double dDpFO = pValveCharacteristic->GetDpFullOpening( m_pSelDpController->GetQ(), pWC->GetDens(), pWC->GetKinVisc() );

					if( -1.0 != dDpFO )
					{
						pSelInfoDpC->UpdateInfoMV( pValv, dValveDp, dDpFO, m_pSelDpController->GetOpening() );
					}
				}
			}

			pSelInfoDpC->UpdateInfoDpC( pDpC, m_pSelDpController->GetDpMin(), true );

			CDC *pdc = GetDC();
			CEnBitmapPatchWork EnBmp;

			if( false == pSelInfoDpC->PrepareImage( *pdc, &EnBmp ) )
			{
				delete pSelInfoDpC;
				ReleaseDC( pdc );
				EnBmp.DeleteObject();
				return;
			}

			delete pSelInfoDpC;
			EnBmp.RotateImage( 90 );

			// Get client area of the dialog.
			CRect dcRect;
			m_staticImg.GetClientRect( &dcRect );
			EnBmp.ResizeImage( CSize( dcRect.Width(), dcRect.Height() ) );
			m_Bmp.CopyImage( EnBmp );
			m_staticImg.SetBitmap( (HBITMAP)m_Bmp );
			ReleaseDC( pdc );
			return;
		}
	}
	else if( NULL != m_pSelDpReliefValve )
	{
		pclProduct = m_pSelDpReliefValve->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelPICv )
	{
		pclProduct = m_pSelPICv->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelDpCBCV )
	{
		pclProduct = m_pSelDpCBCV->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelSv )
	{
		pclProduct = m_pSelSv->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelSafetyValve )
	{
		pclProduct = m_pSelSafetyValve->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pHmHub )
	{
		// Treated separately.
	}
	else if( NULL != m_pSelRadSet )
	{
		pclProduct = dynamic_cast<CDB_Product *>( m_pSelRadSet->GetSupplyValveIDPtr().MP );

		if( NULL != pclProduct )
		{
			pclProduct = dynamic_cast<CDB_Product *>( m_pSelRadSet->GetReturnValveIDPtr().MP );
		}
	}
	else if( NULL != m_pSelAirVentSeparator )
	{
		pclProduct = m_pSelAirVentSeparator->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelPressureMaint )
	{
		pclProduct = (CDB_Product *)( m_pSelPressureMaint->GetVesselIDPtr().MP );
	}
	else if( NULL != m_pSel6WayValve )
	{
		pclProduct = m_pSel6WayValve->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelTapWaterControl )
	{
		pclProduct = m_pSelTapWaterControl->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelSmartControlValve )
	{
		pclProduct = m_pSelSmartControlValve->GetProductAs<CDB_Product>();
	}
	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	else if( NULL != m_pSelSmartDpC )
	{
		pclProduct = m_pSelSmartDpC->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelTALinkAlone )
	{
		pclProduct = m_pSelTALinkAlone->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelFloorHeatingManifold )
	{
		pclProduct = m_pSelFloorHeatingManifold->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelFloorHeatingValve )
	{
		pclProduct = m_pSelFloorHeatingValve->GetProductAs<CDB_Product>();
	}
	else if( NULL != m_pSelFloorHeatingController )
	{
		pclProduct = m_pSelFloorHeatingController->GetProductAs<CDB_Product>();
	}
	// HYS-1741
	else if( NULL != m_pSelPWQAccAndServices )
	{
		pclProduct = m_pSelPWQAccAndServices->GetProductAs<CDB_Product>();
	}
	else
	{
		ASSERT( 0 );
	}

	if( NULL != pclProduct && true == TASApp.VerifyPicExist( pclProduct->GetProdPicID() ) )
	{
		m_pProdPic = TASApp.GetpProdPic( pclProduct->GetProdPicID(), &m_Bmp );

		// Verify the image exist.
		if( m_pProdPic && CProdPic::Pic == m_pProdPic->GetProdPicType() )
		{
			COLORREF cBck = m_Bmp.GetPixel( 0, 0 );
			m_Bmp.ReplaceColor( cBck, GetSysColor( COLOR_BTNFACE ) );

			if( epVertical != m_pProdPic->GetProdPosition() )
			{
				m_Bmp.RotateImage( -90, GetSysColor( COLOR_BTNFACE ) );
			}

			_SetImage();
		}
	}

	m_ListCtrl.SetColumnWidth( 0, LVSCW_AUTOSIZE );
}

void CDlgConfSel::_FillListCtrlForRadiatorSet( CSelectedInfos *pSelInfo, CDS_SSelRadSet *pSelRadSet )
{
	long lRow = 0;

	CDB_ThermostaticValve *pTrv = dynamic_cast<CDB_ThermostaticValve *>( pSelRadSet->GetSupplyValveIDPtr().MP );
	CDB_FlowLimitedControlValve *pFLCV = dynamic_cast<CDB_FlowLimitedControlValve *>( pSelRadSet->GetSupplyValveIDPtr().MP );
	CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( pSelRadSet->GetSupplyValveActuatorIDPtr().MP );
	CDB_RegulatingValve *pRetV =  dynamic_cast<CDB_RegulatingValve *>( pSelRadSet->GetReturnValveIDPtr().MP );

	CString str;

	int iQuantity = pSelRadSet->GetpSelectedInfos()->GetQuantity();
	if( NULL != pTrv || NULL != pFLCV )
	{
		// If product contains a comment, we add it to the remark list.
		if( true == pSelInfo->GetRemark().IsEmpty() )
		{
			// First the supply valve.
			CString strRemark( _T("") );

			if( NULL != pTrv && false == CString( pTrv->GetComment() ).IsEmpty() )
			{
				strRemark = pTrv->GetComment();
			}
			else if( NULL != pFLCV && false == CString( pFLCV->GetComment() ).IsEmpty() )
			{
				strRemark = pFLCV->GetComment();
			}

			// Second the return valve.
			if( NULL != pRetV && false == CString( pRetV->GetComment() ).IsEmpty() )
			{
				if( false == strRemark.IsEmpty() )
				{
					strRemark += _T("\r\n");
				}

				strRemark += pRetV->GetComment();
			}

			if( false == strRemark.IsEmpty() )
			{
				pSelInfo->SetRemark( strRemark );
			}
		}

		_InitRefsAndRemarks( pSelInfo );

		lRow = m_ListCtrl.AddItemText( ( NULL != pTrv ) ? pTrv->GetType() : pFLCV->GetType() );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		CDB_ControlValve *pclControlValve = ( NULL != pTrv ) ? ( CDB_ControlValve * )pTrv : ( CDB_ControlValve * )pFLCV;
		lRow = m_ListCtrl.AddItemText( pclControlValve->GetName() );
		//HYS-987
		_AddQuantity( lRow, 1, iQuantity );
		_AddTAProductInfos( lRow, pclControlValve );

		// Accessories.
		CAccessoryList *pclTrvAccessoryList = pSelRadSet->GetSupplyValveAccessoryList();
		int iNbrAcc = pclTrvAccessoryList->GetCount();

		if( pclTrvAccessoryList && iNbrAcc > 0 )
		{
			int iIDS = IDS_CONFSEL_TRV_ACCESSORIES;
			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclControlValve->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;
			if( NULL != pclRuledTable )
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			_AddAccessoriesInfos( 0, iIDS, pSelRadSet->GetSupplyValveAccessoryList(), iQuantity, vecBuiltInAccessories );
		}

		// If actuator exist...
		if( NULL != pSelRadSet->GetSupplyValveActuatorIDPtr().MP )
		{
			m_ListCtrl.AddItemText( _T("") );
			_FillActuators( pSelInfo, ( CDB_Actuator * )pSelRadSet->GetSupplyValveActuatorIDPtr().MP, pclControlValve );
		}

		// Actuator accessories.
		CAccessoryList *pclActrAccessoryList = pSelRadSet->GetSVActuatorAccessoryList();
		iNbrAcc = pclActrAccessoryList->GetCount();

		if( pclActrAccessoryList && iNbrAcc > 0 )
		{
			int iIDS = IDS_CONFSEL_ACTACCESSORY;
			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( ((CDB_Actuator *)pSelRadSet->GetSupplyValveActuatorIDPtr().MP)->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;
			if( NULL != pclRuledTable )
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );

			_AddAccessoriesInfos( 0, iIDS, pSelRadSet->GetSVActuatorAccessoryList(), iQuantity, vecBuiltInAccessories );
		}

		// White line.
		m_ListCtrl.AddItemText( _T("") );
	}
	else if( RVT_Inserts == ( RadiatorValveType )pSelRadSet->GetRadiatorValveType() &&
			 true == pSelRadSet->IsInsertInKv() )
	{
		// In case of insert, use can input supply valve by Kv. In this case, we have to check only with the return valve.

		// If product contains a comment, we add it to the remark list.
		if( true == pSelInfo->GetRemark().IsEmpty() )
		{
			CString strRemark( _T("") );

			// The return valve.
			if( NULL != pRetV && false == CString( pRetV->GetComment() ).IsEmpty() )
			{
				strRemark += pRetV->GetComment();
			}

			if( false == strRemark.IsEmpty() )
			{
				pSelInfo->SetRemark( strRemark );
			}
		}

		_InitRefsAndRemarks( pSelInfo );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_TRVNAME );
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		// Add name 'Other insert'.
		lRow = m_ListCtrl.AddItemText( TASApp.LoadLocalizedString( IDS_CONFSEL_OTHERINSERT ) );

		// Add Kvs (or CV depending of current unit).
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

		if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KVS );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CV );
		}

		str += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pSelRadSet->GetInsertKvValue(), false, 3, 2 );
		lRow = m_ListCtrl.AddItemText( str );

		// Blank line.
		lRow = m_ListCtrl.AddItemText( _T("") );
	}

	if( NULL != pRetV )
	{
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_RETVNAME );
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		lRow = m_ListCtrl.AddItemText( pRetV->GetName() );
		//HYS-987
		_AddQuantity( lRow, 1, iQuantity );
		_AddTAProductInfos( lRow, pRetV );

		// Accessories
		CAccessoryList *pclRvAccessoryList = pSelRadSet->GetReturnValveAccessoryList();
		int iNbrAcc = pclRvAccessoryList->GetCount();

		if( pclRvAccessoryList && iNbrAcc > 0 )
		{
			int iIDS = IDS_CONFSEL_RV_ACCESSORIES;
			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pRetV->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;
			if( NULL != pclRuledTable )
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			_AddAccessoriesInfos( 0, iIDS, pSelRadSet->GetReturnValveAccessoryList(), iQuantity, vecBuiltInAccessories );
		}
	}
}

void CDlgConfSel::_FillListCtrlForAirVentSep( CSelectedInfos *pSelInfo, CDS_SSelAirVentSeparator *pSelAirVentSep )
{
	long lRow = 0;

	CString str;
	CDB_TAProduct *pclTAProduct = dynamic_cast<CDB_TAProduct *>( pSelAirVentSep->GetProductIDPtr().MP );
	//HYS-987
	int iGroupQuantity = pSelInfo->GetQuantity();
	if( NULL != pclTAProduct )
	{
		CDB_AirVent *pclAirVent = dynamic_cast<CDB_AirVent *>( pclTAProduct );
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( pclTAProduct );

		if( NULL == pclAirVent && NULL == pclSeparator )
		{
			return;
		}

		// If product contains a comment, we add it to the remark list.
		if( true == pSelInfo->GetRemark().IsEmpty() )
		{
			if( NULL != pclAirVent && false == CString( pclAirVent->GetComment() ).IsEmpty() )
			{
				pSelInfo->SetRemark( pclAirVent->GetComment() );
			}
			else if( NULL != pclSeparator && false == CString( pclSeparator->GetComment() ).IsEmpty() )
			{
				pSelInfo->SetRemark( pclSeparator->GetComment() );
			}
		}

		_InitRefsAndRemarks( pSelInfo );

		lRow = m_ListCtrl.AddItemText( pclTAProduct->GetType() );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		lRow = m_ListCtrl.AddItemText( pclTAProduct->GetName() );
		//HYS-987
		_AddQuantity( lRow, 1, iGroupQuantity );
		// Add data.
		if( NULL != pclAirVent )
		{
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPP );
			str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, pclAirVent->GetDpp(), true );
			lRow = m_ListCtrl.AddItemText( str );
		}

		_AddTAProductInfos( lRow, pclTAProduct );

		// Accessories.
		CAccessoryList *pclSeparatorAccessoryList = pSelAirVentSep->GetAccessoryList();

		if( NULL != pclSeparatorAccessoryList && pclSeparatorAccessoryList->GetCount() > 0 )
		{
			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTAProduct->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;
			if( NULL != pclRuledTable )
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			_AddAccessoriesInfos( 0, ( NULL != pclAirVent ) ? IDS_CONFSEL_AIRVENT_ACCESSORIES : IDS_CONFSEL_SEPARATOR_ACCESSORIES, pclSeparatorAccessoryList, iGroupQuantity, vecBuiltInAccessories );
		}

		// White line.
		m_ListCtrl.AddItemText( _T("") );
	}
}

void CDlgConfSel::_FillListCtrlForPressMaint( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint )
{
	long lRow = 0;

	CDS_SSelPMaint::SelectionType eSelectionType = pclSelPressMaint->GetSelectionType();

	CString strRemark = _T("");

	if( true == pclSelPressMaint->IsFromDirSel() )
	{
		// Case of direct selection.

		// If vessel...
		if( NULL != dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetVesselIDPtr().MP ) ||
			NULL != dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetSecondaryVesselIDPtr().MP ) ||
			NULL != dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetIntermediateVesselIDPtr().MP ) )
		{
			CDB_Vessel *pclVessel;

			if( NULL != dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetVesselIDPtr().MP ) )
			{
				pclVessel = dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetVesselIDPtr().MP );
			}
			else if( NULL != dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetSecondaryVesselIDPtr().MP ) )
			{
				pclVessel = dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetSecondaryVesselIDPtr().MP );
			}
			else
			{
				pclVessel = dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetIntermediateVesselIDPtr().MP );
			}

			_FillPMVesselHelper( pclVessel, pclSelPressMaint, strRemark );
		}
		else if( NULL != dynamic_cast<CDB_TecBox *>( ( CData * )pclSelPressMaint->GetTecBoxCompTransfIDPtr().MP ) )
		{
			_FillPMCompressoTransferoHelper( pSelInfo, pclSelPressMaint, strRemark );
		}
		else if( NULL != dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pclSelPressMaint->GetTecBoxVentoIDPtr().MP ) )
		{
			_FillPMVentoHelper( pSelInfo, pclSelPressMaint, strRemark );
		}
		else if( NULL != dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pclSelPressMaint->GetTecBoxPlenoIDPtr().MP ) )
		{
			_FillPMPlenoHelper( pSelInfo, pclSelPressMaint, strRemark );
		}
		else if( NULL != dynamic_cast<CDB_PlenoRefill *>( ( CData * )pclSelPressMaint->GetPlenoRefillIDPtr().MP ) )
		{
			_FillPMPlenoRefillHelper( pSelInfo, pclSelPressMaint, strRemark );
		}
	}
	else
	{
		// Case of individual selection with pressurization.
		if( CDS_SSelPMaint::NoPressurization != eSelectionType )
		{
			if( CDS_SSelPMaint::SelectionType::Statico == eSelectionType )
			{
				// Fill Statico or streamline.
				CDB_Vessel *pclStatico = dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetVesselIDPtr().MP );
				ASSERT( NULL != pclStatico );

				if( NULL == pclStatico )
				{
					return;
				}

				_FillPMVesselHelper( pclStatico, pclSelPressMaint, strRemark );

				// Check if there is an intermediate vessel selected.
				CDB_Vessel *pclIntermediateVessel = dynamic_cast<CDB_Vessel *>( pclSelPressMaint->GetIntermediateVesselIDPtr().MP );

				if( NULL != pclIntermediateVessel )
				{
					// White line.
					m_ListCtrl.AddItemText( _T("") );
					_FillPMVesselHelper( pclIntermediateVessel, pclSelPressMaint, strRemark );
				}

				// Fill Vento if there are some.
				if( 0 != pclSelPressMaint->GetTecBoxVentoNumber() )
				{
					// White line.
					m_ListCtrl.AddItemText( _T("") );
					_FillPMVentoHelper( pSelInfo, pclSelPressMaint, strRemark );
				}

				// Fill Pleno if there are some.
				if( 0 != pclSelPressMaint->GetTecBoxPlenoNumber() )
				{
					// White line.
					m_ListCtrl.AddItemText( _T("") );
					_FillPMPlenoHelper( pSelInfo, pclSelPressMaint, strRemark );
				}

				// Fill Pleno Refill if there are some.
				if( 0 != pclSelPressMaint->GetPlenoRefillNumber() )
				{
					// White line.
					m_ListCtrl.AddItemText( _T("") );
					_FillPMPlenoRefillHelper( pSelInfo, pclSelPressMaint, strRemark );
				}
			}
			else
			{
				_FillPMCompressoTransferoHelper( pSelInfo, pclSelPressMaint, strRemark );
			}
		}
		else
		{
			// Case of individual selection with no pressurization.

			// Fill Vento if there are some.
			if( 0 != pclSelPressMaint->GetTecBoxVentoNumber() )
			{
				_FillPMVentoHelper( pSelInfo, pclSelPressMaint, strRemark );
			}

			// Fill Pleno if there are some.
			if( 0 != pclSelPressMaint->GetTecBoxPlenoNumber() )
			{
				// White line.
				if( 0 != pclSelPressMaint->GetTecBoxVentoNumber() )
				{
					m_ListCtrl.AddItemText( _T("") );
				}

				_FillPMPlenoHelper( pSelInfo, pclSelPressMaint, strRemark );
			}

			// Fill Pleno Refill if there are some.
			if( 0 != pclSelPressMaint->GetPlenoRefillNumber() )
			{
				// White line.
				m_ListCtrl.AddItemText( _T("") );

				_FillPMPlenoRefillHelper( pSelInfo, pclSelPressMaint, strRemark );
			}
		}
	}

	// Set now the remark in 'pSelInfo'.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == strRemark.IsEmpty() )
		{
			pSelInfo->SetRemark( strRemark );
		}
	}

	_InitRefsAndRemarks( pSelInfo );
}

void CDlgConfSel::_FillListCtrlForShutOffValve( CSelectedInfos *pSelInfo, CDS_SSelSv *pclSSelSv )
{
	long lRow = 0;

	CDB_ShutoffValve *pclSelShutOffValve = pclSSelSv->GetProductAs<CDB_ShutoffValve>();
	CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclSSelSv->GetActrIDPtr().MP );

	if( NULL == pclSelShutOffValve )
	{
		ASSERT_RETURN;
	}
	
	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclSelShutOffValve->GetComment() ).IsEmpty() )
		{
			pSelInfo->SetRemark( pclSelShutOffValve->GetComment() );
		}
	}

	_InitRefsAndRemarks( pSelInfo );

	lRow = m_ListCtrl.AddItemText( pclSelShutOffValve->GetType() );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	lRow = m_ListCtrl.AddItemText( pclSelShutOffValve->GetName() );
	//HYS-987
	int iGroupQuantity = pSelInfo->GetQuantity();
	_AddQuantity( lRow, 1, iGroupQuantity );
	lRow = _AddTAProductInfos( lRow, pclSelShutOffValve );

	// Add accessories infos.
	int iIDS = IDS_CONFSEL_SV_ACCESSORY;
	CAccessoryList *pclAccessoryList = pclSSelSv->GetAccessoryList();

	// HYS-987: Built-in accessories
	CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclSelShutOffValve->GetAccessoriesGroupIDPtr().MP );
	std::vector<CData *> vecBuiltInAccessories;
	if( NULL != pclRuledTable )
		pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
	lRow = _AddAccessoriesInfos( lRow, iIDS, pclAccessoryList, iGroupQuantity, vecBuiltInAccessories );

	// If actuator exist...
	if( NULL != pclActuator )
	{
		// Add a white line.
		if( lRow > 0 )
		{
			lRow = m_ListCtrl.AddItemText( _T("") );
		}

		_FillActuators( pSelInfo, pclActuator, NULL );

		CAccessoryList *pclActuatorAccessoryList = pclSSelSv->GetActuatorAccessoryList();

		if( NULL != pclActuatorAccessoryList && pclActuatorAccessoryList->GetCount() > 0 )
		{
			// White line.
			lRow = m_ListCtrl.AddItemText( _T("") );
			CString str = TASApp.LoadLocalizedString( IDS_CONFSEL_ACTACCESSORY );
			lRow = m_ListCtrl.AddItemText( str );
			m_ListCtrl.SetItemTextBold( lRow, 0, true );

			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclActuator->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;
			std::vector<CData *>::iterator itBI;
			if( NULL != pclRuledTable )
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );

			CAccessoryList::AccessoryItem rAccessoryItem = pclActuatorAccessoryList->GetFirst();

			while( rAccessoryItem.IDPtr.MP != NULL )
			{
				CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
				VERIFY( pclAccessory );

				if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
				{
					break;
				}

				str = pclAccessory->GetName() + CString( _T(" : ") ) + pclAccessory->GetComment();
				lRow = m_ListCtrl.AddItemText( str );

				int iQuantity = ( true == rAccessoryItem.fByPair ) ? 2 : 1;

				// HYS-987.
				itBI = find( vecBuiltInAccessories.begin(), vecBuiltInAccessories.end(), pclAccessory );
				
				if( itBI != vecBuiltInAccessories.end() )
				{
					// Quantity of built-in accessories can't be edited.
					_AddQuantity( lRow, iQuantity, iGroupQuantity, rAccessoryItem.lEditedQty, false );
				}
				else
				{
					_AddQuantity( lRow, iQuantity, iGroupQuantity, rAccessoryItem.lEditedQty, true );
				}
				
				SelectedAccessories structAcc;
				structAcc.pclSelectedAccList = pclActuatorAccessoryList;
				structAcc.AccItemFromAccList.eAccessoryType = rAccessoryItem.eAccessoryType;
				structAcc.AccItemFromAccList.fByPair = rAccessoryItem.fByPair;
				structAcc.AccItemFromAccList.fDistributed = rAccessoryItem.fDistributed;
				structAcc.AccItemFromAccList.IDPtr = rAccessoryItem.IDPtr;
				structAcc.AccItemFromAccList.lEditedQty = rAccessoryItem.lEditedQty;
				m_mapSelectAccessories.insert( make_pair( lRow, structAcc ) );
				rAccessoryItem = pclActuatorAccessoryList->GetNext();
			}
		}
	}
}

void CDlgConfSel::_FillListCtrlForSafetyValve( CSelectedInfos *pSelInfo, CDS_SSelSafetyValve *pclSSelSafetyValve )
{
	if( NULL == pSelInfo || NULL == pclSSelSafetyValve )
	{
		return;
	}

	// We can have here safety valve and blow tank from individual selection or safety/blow tank alone from direct selection.
	CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( (CData *)pclSSelSafetyValve->GetProductIDPtr().MP );
	CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( (CData *)pclSSelSafetyValve->GetBlowTankIDPtr().MP );
	CString str = _T("");
	long lRow = 0;
	//HYS-987
	int iGroupQuantity = pSelInfo->GetQuantity();
	// Update qty information
	_InitRefsAndRemarks( pSelInfo );

	// Safety valve.
	if( NULL != pclSafetyValve )
	{
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_SAFETYVALVETITLE );
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		m_ListCtrl.AddItemText( pclSafetyValve->GetName() );
		//HYS-987
		_AddQuantity( lRow, pclSSelSafetyValve->GetQuantityNeeded(), iGroupQuantity );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_BDYMAT );
		str += CString( _T(" : ") ) + pclSafetyValve->GetBodyMaterial();
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTION );
		str += CString( _T(" : ") ) + pclSafetyValve->GetConnect();
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_VERSION );
		str += CString( _T(" : ") ) + pclSafetyValve->GetVersion();
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_SAFETYVALVE_LIFTINGMECHANISM );
		str += CString( _T(" : ") ) + pclSafetyValve->GetLiftingType();
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_SAFETYVALVE_SETPRESSURE );
		str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclSafetyValve->GetSetPressure(), true );
		m_ListCtrl.AddItemText( str );

		//str = TASApp.LoadLocalizedString( IDS_CONFSEL_SAFETYVALVE_QUANTITYNEEED );
		//CString strQty;
		//strQty.Format( _T("%i"), pclSSelSafetyValve->GetQuantityNeeded() );
		//str += CString( _T(" : ") ) + strQty;
		//m_ListCtrl.AddItemText( str );

		// Add accessories infos if exist.
		int iIDS = IDS_CONFSEL_SAFETYVALVE_ACCESSORY;
		CAccessoryList *pclAccessoryList = pclSSelSafetyValve->GetAccessoryList();
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclSafetyValve->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
		if( NULL != pclRuledTable )
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		lRow = _AddAccessoriesInfos( lRow, iIDS, pclAccessoryList, iGroupQuantity, vecBuiltInAccessories, pclSSelSafetyValve->GetQuantityNeeded() );
	}

	if( NULL != pclBlowTank )
	{
		if( NULL != pclSafetyValve )
		{
			lRow = m_ListCtrl.AddItemText( _T("") );
		}

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_BLOWTANKTITLE );
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );
		m_ListCtrl.AddItemText( pclBlowTank->GetName() );
		//HYS-987
		_AddQuantity( lRow, pclSSelSafetyValve->GetQuantityNeeded(), iGroupQuantity );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_PN );
		str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclBlowTank->GetPmaxmax() );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_BDYMAT );
		str += CString( _T(" : ") ) + pclBlowTank->GetBodyMaterial();
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_BLOWTANK_SIZE );
		str += CString( _T(" : ") ) + pclBlowTank->GetAllSizesString();
		m_ListCtrl.AddItemText( str );

		//str = TASApp.LoadLocalizedString( IDS_CONFSEL_SAFETYVALVE_QUANTITYNEEED );
		//CString strQty;
		//strQty.Format( _T("%i"), pclSSelSafetyValve->GetQuantityNeeded() );
		//str += CString( _T(" : ") ) + strQty;
		//m_ListCtrl.AddItemText( str );

		// Add accessories infos if exist.
		int iIDS = IDS_CONFSEL_BLOWTANK_ACCESSORY;
		CAccessoryList *pclAccessoryList = pclSSelSafetyValve->GetBlowTankAccessoryList();
		// HYS-987: Built-int accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclBlowTank->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
		if( NULL != pclRuledTable )
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		lRow = _AddAccessoriesInfos( lRow, iIDS, pclAccessoryList, iGroupQuantity, vecBuiltInAccessories, pclSSelSafetyValve->GetQuantityNeeded() );
	}
}

void CDlgConfSel::_FillListCtrlFor6WayValve( CSelectedInfos *pSelInfo, CDS_SSel6WayValve *pclSSel6WayValve )
{
	long lRow = 0;

	CDB_6WayValve *pcl6WayValve = pclSSel6WayValve->GetProductAs<CDB_6WayValve>();

	if( NULL == pcl6WayValve )
	{
		return;
	}

	CDS_SSelPICv *pclSSelPIBCv1 = NULL;
	CDS_SSelPICv *pclSSelPIBCv2 = NULL;
	CDS_SSelBv *pclSSelBv1 = NULL;
	CDS_SSelBv *pclSSelBv2 = NULL;
	
	CDB_PIControlValve *pclPIBCValve1 = NULL;
	CDB_PIControlValve *pclPIBCValve2 = NULL;
	CDB_RegulatingValve *pclBalancingValve1 = NULL;
	CDB_RegulatingValve *pclBalancingValve2 = NULL;

	if( e6Way_EQMControl == pclSSel6WayValve->GetSelectionMode() )
	{
		pclSSelPIBCv1 = pclSSel6WayValve->GetCDSSSelPICv( BothSide );
		pclPIBCValve1 = pclSSelPIBCv1->GetProductAs<CDB_PIControlValve>();
		
		if( NULL == pclSSelPIBCv1 || NULL == pclPIBCValve1 )
		{
			ASSERT_RETURN;
		}
	}
	else if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
	{
		pclSSelPIBCv1 = pclSSel6WayValve->GetCDSSSelPICv( CoolingSide );
		pclSSelPIBCv2 = pclSSel6WayValve->GetCDSSSelPICv( HeatingSide );
		pclPIBCValve1 = pclSSelPIBCv1->GetProductAs<CDB_PIControlValve>();
		pclPIBCValve2 = pclSSelPIBCv2->GetProductAs<CDB_PIControlValve>();

		if( NULL == pclSSelPIBCv1 || NULL == pclSSelPIBCv2 || NULL == pclPIBCValve1 || NULL == pclPIBCValve2 )
		{
			ASSERT_RETURN;
		}
	}
	else if( e6Way_OnOffControlWithSTAD == pclSSel6WayValve->GetSelectionMode() )
	{
		pclSSelBv1 = pclSSel6WayValve->GetCDSSSelBv( CoolingSide );
		pclSSelBv2 = pclSSel6WayValve->GetCDSSSelBv( HeatingSide );
		pclBalancingValve1 = pclSSelBv1->GetProductAs<CDB_RegulatingValve>();
		pclBalancingValve2 = pclSSelBv2->GetProductAs<CDB_RegulatingValve>();

		if( NULL == pclSSelBv1 || NULL == pclSSelBv2 || NULL == pclBalancingValve1 || NULL == pclBalancingValve2 )
		{
			ASSERT_RETURN;
		}
	}

	CString str;
	CString strRemark( _T("") );

	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pcl6WayValve->GetComment() ).IsEmpty() )
		{
			strRemark = pcl6WayValve->GetComment();
		}
	}

	// Depending of the application type, we can have also either PIBCV or BV.
	if( e6Way_Alone != pclSSel6WayValve->GetSelectionMode() )
	{
		if( e6Way_EQMControl == pclSSel6WayValve->GetSelectionMode() || e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
		{
			if( false == CString( pclPIBCValve1->GetComment() ).IsEmpty() )
			{
				if( false == strRemark.IsEmpty() )
				{
					strRemark += _T("\r\n");
				}

				strRemark += pclPIBCValve1->GetComment();
			}
		}

		// Intentionally we don't put 'else' because if the circuit type is 'e6Way_OnOffControlWithPIBCV' we checked the heating
		// side above. Now we check the heating side.
		if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
		{
			if( false == CString( pclPIBCValve2->GetComment() ).IsEmpty() )
			{
				if( false == strRemark.IsEmpty() )
				{
					strRemark += _T("\r\n");
				}

				strRemark += pclPIBCValve2->GetComment();
			}
		}

		if( e6Way_OnOffControlWithSTAD == pclSSel6WayValve->GetSelectionMode() )
		{
			if( false == CString( pclBalancingValve1->GetComment() ).IsEmpty() )
			{
				if( false == strRemark.IsEmpty() )
				{
					strRemark += _T("\r\n");
				}

				strRemark += pclBalancingValve1->GetComment();
			}

			if( false == CString( pclBalancingValve2->GetComment() ).IsEmpty() )
			{
				if( false == strRemark.IsEmpty() )
				{
					strRemark += _T("\r\n");
				}

				strRemark += pclBalancingValve2->GetComment();
			}
		}
	}

	// Set now the remark in 'pSelInfo'.
	if( false == strRemark.IsEmpty() )
	{
		pSelInfo->SetRemark( strRemark );
	}

	_InitRefsAndRemarks( pSelInfo );

	// HYS-1877: Check if a TA-6-way-valve kit is selected.
	IDPTR idptr6WayValveKit = pclSSel6WayValve->GetCvActrSetIDPtr();
	if( true == pclSSel6WayValve->IsSelectedAsAPackage() && NULL != dynamic_cast<CDB_Set*>(idptr6WayValveKit.MP) )
	{
		// Write set information
		CDB_Set* TA6WayValveKit = dynamic_cast<CDB_Set*>(idptr6WayValveKit.MP);
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_6WAYVALVESET );
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );
		str = TA6WayValveKit->GetName();
		lRow = m_ListCtrl.AddItemText( str );
		// HYS-987
		int iGroupQuantity = pSelInfo->GetQuantity();
		_AddQuantity( lRow, 1, iGroupQuantity );
		// White line.
		lRow = m_ListCtrl.AddItemText( _T( "" ) );
	}

	// Add the title corresponding to the valve.
	lRow = m_ListCtrl.AddItemText( pcl6WayValve->GetType() );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	// Add the name of the valve.
	str = pcl6WayValve->GetName();
	lRow = m_ListCtrl.AddItemText( str );
	int iGroupQty = pclSSel6WayValve->GetpSelectedInfos()->GetQuantity();
	_AddQuantity( lRow, 1, iGroupQty );

	// Add the 'Kvs' value.
	CString str2;

	if( !m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str2 = TASApp.LoadLocalizedString( IDS_CONFSEL_KVS );
	}
	else
	{
		str2 = TASApp.LoadLocalizedString( IDS_CONFSEL_CV );
	}

	str2 += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pcl6WayValve->GetKvs() );
	lRow = m_ListCtrl.AddItemText( str2 );

	// Add the rangeability.
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_RANGEABILITY );
	str += CString( _T(" : ") ) + pcl6WayValve->GetStrRangeability().c_str();

	lRow = m_ListCtrl.AddItemText( str );

	// Add leakage.
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_CV_LEAKAGE );
	str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pcl6WayValve->GetLeakageRate() * 100 );
	str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_CONFSEL_CV_LEAKAGEUNIT );
	lRow = m_ListCtrl.AddItemText( str );

	// Format the string before putting it into the remark list.
	if( false == m_strComment.empty() )
	{
		pcl6WayValve->ApplyComment( &m_strComment );
	}
			
	int iQuantity = pclSSel6WayValve->GetpSelectedInfos()->GetQuantity();
			
	// Add infos about accessories on the valve.
	// HYS-987: Built-in accessories
	CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pcl6WayValve->GetAccessoriesGroupIDPtr().MP );
	std::vector<CData *> vecBuiltInAccessories;
			
	if( NULL != pclRuledTable )
	{
		pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
	}

	CString strTitle = pcl6WayValve->GetType() + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_CONFSEL_ACCESSORIES );
	lRow = _AddAccessoriesInfos( lRow, strTitle, pclSSel6WayValve->GetCvAccessoryList(), iQuantity, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory );

	// If actuator exist...
	if( NULL != pclSSel6WayValve->GetActrIDPtr().MP && NULL != dynamic_cast<CDB_Actuator *>( pclSSel6WayValve->GetActrIDPtr().MP ) )
	{
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclSSel6WayValve->GetActrIDPtr().MP );

		// Add a white line.
		if( lRow > 0 )
		{
			lRow = m_ListCtrl.AddItemText( _T("") );
		}

		CString strTitle = pcl6WayValve->GetType() + CString( _T(" ") );
		strTitle += CString( TASApp.LoadLocalizedString( IDS_CONFSEL_ACTUATOR ) ).MakeLower();
		_FillActuators( pclSSel6WayValve->GetpSelectedInfos(), pclActuator, pcl6WayValve, strTitle, _BLACK, false );

		strTitle = pcl6WayValve->GetType() + CString( _T(" ") );
		strTitle += CString( TASApp.LoadLocalizedString( IDS_CONFSEL_ACTACCESSORY ) ).MakeLower();
		CAccessoryList *pclActuatorAccessoryList = pclSSel6WayValve->GetActuatorAccessoryList();
		lRow = _FillActuatorAccessories( pclActuatorAccessoryList, pclActuator, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), lRow, strTitle );
	}

	// If pressure independent balancing & control valve is selected (For EQM Control of the On/Off control with PIBCV cooling side).
	if( NULL != pclSSelPIBCv1 && NULL != pclPIBCValve1 )
	{
		// Add a blank line.
		lRow = m_ListCtrl.AddItemText( _T("") );

		// Add the title corresponding to the valve.
		COLORREF clColor = _BLACK;
		str = pclPIBCValve1->GetType();

		if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
		{
			str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_COOLINGSIDE ) + _T(")");
			clColor = _IMI_TITLE_GROUPCOOLING;
		}

		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );
		m_ListCtrl.SetItemTextColor( lRow, 0, clColor, clColor );

		// Add the name of the valve.
		str = pclPIBCValve1->GetName();
		lRow = m_ListCtrl.AddItemText( str );
		int iGroupQty = pclSSel6WayValve->GetpSelectedInfos()->GetQuantity();
		_AddQuantity( lRow, 1, iGroupQty );

		// Add the rangeability.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_RANGEABILITY );
		str += CString( _T(" : ") ) + pclPIBCValve1->GetStrRangeability().c_str();

		lRow = m_ListCtrl.AddItemText( str );

		// Add leakage.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_CV_LEAKAGE );
		str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclPIBCValve1->GetLeakageRate() * 100 );
		str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_CONFSEL_CV_LEAKAGEUNIT );
		lRow = m_ListCtrl.AddItemText( str );

		// Format the string before putting it into the remark list.
		if( false == m_strComment.empty() )
		{
			pclPIBCValve1->ApplyComment( &m_strComment );
		}
			
		int iQuantity = pclSSel6WayValve->GetpSelectedInfos()->GetQuantity();

		// Add infos about accessories on the valve.
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclPIBCValve1->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
			
		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		str = pclPIBCValve1->GetType();
		str += _T(" ") + TASApp.LoadLocalizedString( IDS_CONFSEL_ACCESSORIES );

		if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
		{
			str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_COOLINGSIDE ) + _T(")");
		}

		long lRowStart = lRow;
		lRow = _AddAccessoriesInfos( lRow, str, pclSSelPIBCv1->GetCvAccessoryList(), iQuantity, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory, true, clColor );

		// HYS-1968: Disable editable status.
		if( true == pclSSel6WayValve->IsSelectedAsAPackage() )
		{
			// Remove editable accessories
			// lRowStart + 2 because str is not empty and 2 lines is used for title.
			_SetQuantityStatus( lRowStart + 2, lRow + 1, false );
		}

		// If actuator exist...
		if( NULL != pclSSelPIBCv1->GetActrIDPtr().MP && NULL != dynamic_cast<CDB_Actuator *>( pclSSelPIBCv1->GetActrIDPtr().MP ) )
		{
			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclSSelPIBCv1->GetActrIDPtr().MP );

			// Add a white line.
			if( lRow > 0 )
			{
				lRow = m_ListCtrl.AddItemText( _T("") );
			}

			str = pclPIBCValve1->GetType() + CString( _T(" ") );
			str += CString( TASApp.LoadLocalizedString( IDS_CONFSEL_ACTUATOR ) ).MakeLower();

			if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
			{
				str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_COOLINGSIDE ) + _T(")");
			}

			_FillActuators( pclSSel6WayValve->GetpSelectedInfos(), pclActuator, pcl6WayValve, str, clColor );

			CAccessoryList *pclActuatorAccessoryList = pclSSelPIBCv1->GetActuatorAccessoryList();

			str = pclPIBCValve1->GetType() + CString( _T(" ") );
			str += CString( TASApp.LoadLocalizedString( IDS_CONFSEL_ACTACCESSORY ) ).MakeLower();

			if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
			{
				str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_COOLINGSIDE ) + _T(")");
			}

			lRow = _FillActuatorAccessories( pclActuatorAccessoryList, pclActuator, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), lRow, str, clColor );
		}
	}

	// If pressure independent balancing & control valve is selected (For On/Off control with PIBCV heating side).
	if( NULL != pclSSelPIBCv2 && NULL != pclPIBCValve2 )
	{
		// Add a blank line.
		lRow = m_ListCtrl.AddItemText( _T("") );

		// Add the title corresponding to the valve.
		str = pclPIBCValve2->GetType();
		str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_HEATINGSIDE ) + _T(")");

		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );
		m_ListCtrl.SetItemTextColor( lRow, 0, _IMI_TITLE_GROUPHEATING, _IMI_TITLE_GROUPHEATING );

		// Add the name of the valve.
		str = pclPIBCValve2->GetName();
		lRow = m_ListCtrl.AddItemText( str );
		int iGroupQty = pclSSel6WayValve->GetpSelectedInfos()->GetQuantity();
		_AddQuantity( lRow, 1, iGroupQty );

		// Add the rangeability.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_RANGEABILITY );
		str += CString( _T(" : ") ) + pclPIBCValve2->GetStrRangeability().c_str();

		lRow = m_ListCtrl.AddItemText( str );

		// Add leakage.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_CV_LEAKAGE );
		str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclPIBCValve2->GetLeakageRate() * 100 );
		str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_CONFSEL_CV_LEAKAGEUNIT );
		lRow = m_ListCtrl.AddItemText( str );

		// Format the string before putting it into the remark list.
		if( false == m_strComment.empty() )
		{
			pclPIBCValve2->ApplyComment( &m_strComment );
		}
			
		int iQuantity = pclSSel6WayValve->GetpSelectedInfos()->GetQuantity();

		// Add infos about accessories on the valve.
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclPIBCValve2->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
			
		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		str = pclPIBCValve2->GetType();
		str += _T(" ") + TASApp.LoadLocalizedString( IDS_CONFSEL_ACCESSORIES );

		if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
		{
			str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_HEATINGSIDE ) + _T(")");
		}

		lRow = _AddAccessoriesInfos( lRow, str, pclSSelPIBCv2->GetCvAccessoryList(), iQuantity, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory, true, _IMI_TITLE_GROUPHEATING );

		// If actuator exist...
		if( NULL != pclSSelPIBCv2->GetActrIDPtr().MP && NULL != dynamic_cast<CDB_Actuator *>( pclSSelPIBCv2->GetActrIDPtr().MP ) )
		{
			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclSSelPIBCv2->GetActrIDPtr().MP );

			// Add a white line.
			if( lRow > 0 )
			{
				lRow = m_ListCtrl.AddItemText( _T("") );
			}

			str = pclPIBCValve2->GetType() + CString( _T(" ") );
			str += CString( TASApp.LoadLocalizedString( IDS_CONFSEL_ACTUATOR ) ).MakeLower();
			str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_HEATINGSIDE ) + _T(")");

			_FillActuators( pclSSel6WayValve->GetpSelectedInfos(), pclActuator, pcl6WayValve, str, _IMI_TITLE_GROUPHEATING );

			CAccessoryList *pclActuatorAccessoryList = pclSSelPIBCv2->GetActuatorAccessoryList();
			
			str = pclPIBCValve2->GetType() + CString( _T(" ") );
			str += CString( TASApp.LoadLocalizedString( IDS_CONFSEL_ACTACCESSORY ) ).MakeLower();
			str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_HEATINGSIDE ) + _T(")");
			lRow = _FillActuatorAccessories( pclActuatorAccessoryList, pclActuator, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), lRow, str, _IMI_TITLE_GROUPHEATING );
		}
	}

	// If balancing valve is selected (For On/Off control with STAD cooling side).
	if( NULL != pclSSelBv1 && NULL != pclBalancingValve1 && NULL != pclSSelBv2 && NULL != pclBalancingValve2 )
	{
		// Add a blank line.
		lRow = m_ListCtrl.AddItemText( _T("") );

		// Balancing valve - heating side.
		str = pclBalancingValve1->GetType();
		str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_COOLINGSIDE ) + _T(")");
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );
		m_ListCtrl.SetItemTextColor( lRow, 0, _IMI_TITLE_GROUPCOOLING, _IMI_TITLE_GROUPCOOLING );

		lRow = m_ListCtrl.AddItemText( pclBalancingValve1->GetName() );
		int iGroupQuantity = pclSSel6WayValve->GetpSelectedInfos()->GetQuantity();

		// HYS-987.
		_AddQuantity( lRow, 1, iGroupQuantity );
		lRow = _AddTAProductInfos( lRow, pclBalancingValve1 );

		// Add accessories infos.
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclBalancingValve1->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}
		
		str = pclBalancingValve1->GetType() + CString( _T(" ") );
		str += TASApp.LoadLocalizedString( IDS_CONFSEL_ACCESSORIES );
		str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_COOLINGSIDE ) + _T(")");

		_AddAccessoriesInfos( lRow, str, pclSSelBv1->GetAccessoryList(), iGroupQuantity, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory, true, _IMI_TITLE_GROUPCOOLING );

		// Add a blank line.
		lRow = m_ListCtrl.AddItemText( _T("") );

		// Balancing valve - heating side.
		str = pclBalancingValve2->GetType();
		str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_HEATINGSIDE ) + _T(")");
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );
		m_ListCtrl.SetItemTextColor( lRow, 0, _IMI_TITLE_GROUPHEATING, _IMI_TITLE_GROUPHEATING );

		lRow = m_ListCtrl.AddItemText( pclBalancingValve2->GetName() );

		// HYS-987.
		_AddQuantity( lRow, 1, iGroupQuantity );
		lRow = _AddTAProductInfos( lRow, pclBalancingValve2 );

		// Add accessories infos.
		// HYS-987: Built-in accessories
		pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclBalancingValve2->GetAccessoriesGroupIDPtr().MP );
		vecBuiltInAccessories.clear();

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		str = pclBalancingValve2->GetType() + CString( _T(" ") );
		str += TASApp.LoadLocalizedString( IDS_CONFSEL_ACCESSORIES );
		str += _T(" (") + TASApp.LoadLocalizedString( IDS_CONFSEL_HEATINGSIDE ) + _T(")");
		_AddAccessoriesInfos( lRow, str, pclSSelBv2->GetAccessoryList(), iGroupQuantity, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory, true, _IMI_TITLE_GROUPHEATING );
	}

	// Fill image.
	switch( pclSSel6WayValve->GetSelectionMode() )
	{
		case e6WayValveSelectionMode::e6Way_Alone:
			m_Bmp.LoadBitmap( IDB_CIRCUIT6W_D );
			break;

		case e6WayValveSelectionMode::e6Way_EQMControl:
			m_Bmp.LoadBitmap( IDB_CIRCUIT6W_A );
			break;

		case e6WayValveSelectionMode::e6Way_OnOffControlWithPIBCV:
			m_Bmp.LoadBitmap( IDB_CIRCUIT6W_B );
			break;

		case e6WayValveSelectionMode::e6Way_OnOffControlWithSTAD:
			m_Bmp.LoadBitmap( IDB_CIRCUIT6W_C );
			break;
	}

	_SetImage();
}

void CDlgConfSel::_FillListCtrlForSmartControlValve( CSelectedInfos *pSelInfo, CDS_SSelSmartControlValve *pclSSelSmartControlValve )
{
	long lRow = 0;

	CDB_SmartControlValve *pclSmartControlValve = pclSSelSmartControlValve->GetProductAs<CDB_SmartControlValve>();

	if( NULL == pclSmartControlValve || NULL == pclSmartControlValve->GetSmartValveCharacteristic() )
	{
		return;
	}

	CString str;
	CString strRemark( _T("") );

	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclSmartControlValve->GetComment() ).IsEmpty() )
		{
			strRemark = pclSmartControlValve->GetComment();
		}
	}

	// Set now the remark in 'pSelInfo'.
	if( false == strRemark.IsEmpty() )
	{
		pSelInfo->SetRemark( strRemark );
	}

	_InitRefsAndRemarks( pSelInfo );

	// Add the title corresponding to the valve.
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_SMARTCONTROLVALVE );
	lRow = m_ListCtrl.AddItemText( str );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	// Add the name of the valve.
	str = pclSmartControlValve->GetName();
	lRow = m_ListCtrl.AddItemText( str );
	int iGroupQty = pSelInfo->GetQuantity();
	_AddQuantity( lRow, 1, iGroupQty );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_BDYMAT );
	str += CString( _T(" : ") ) + pclSmartControlValve->GetBodyMaterial();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTION );
	str += CString( _T(" : ") ) + pclSmartControlValve->GetConnect();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PN );
	str += CString( _T(" : ") ) + pclSmartControlValve->GetPN().c_str();
	m_ListCtrl.AddItemText( str );

	// Add 'Kvs' for control valve.
	CString str2;

	if( !m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str2 = TASApp.LoadLocalizedString( IDS_CONFSEL_KVS );
	}
	else
	{
		str2 = TASApp.LoadLocalizedString( IDS_CONFSEL_CV );
	}

	str2 += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs() );
	lRow = m_ListCtrl.AddItemText( str2 );

	// Add the nominal flow.
	str = CString( _T("Qnom : ") ) + WriteCUDouble( _U_FLOW, pclSmartControlValve->GetSmartValveCharacteristic()->GetQnom(), true );
	lRow = m_ListCtrl.AddItemText( str );

	// HYS-1660
	// Power supply
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_POWSUP );
	str += CString( _T( " : " ) ) + pclSmartControlValve->GetPowerSupplyStr();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_INPUTSIG );
	str += CString( _T( " : " ) ) + pclSmartControlValve->GetInOutSignalsStr( true );
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_OUTPUTSIG );

	if( CString( _T( "" ) ) == pclSmartControlValve->GetInOutSignalsStr( false ) )
	{
		str += CString( _T( " :    -" ) );
	}
	else
	{
		str += CString( _T( " : " ) ) + pclSmartControlValve->GetInOutSignalsStr( false );
	}

	m_ListCtrl.AddItemText( str );
	// Format the string before putting it into the remark list.
	if( false == m_strComment.empty() )
	{
		pclSmartControlValve->ApplyComment( &m_strComment );
	}

	// HYS-2047: Fill smart control valve accessories if exists.
	if ( pclSSelSmartControlValve->IsAccessoryExist() )
	{
		CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>( pclSmartControlValve->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData*> vecBuiltInAccessories;

		if (NULL != pclRuledTable)
		{
			pclRuledTable->GetBuiltIn(&vecBuiltInAccessories);
		}

		CString strTitle = pclSmartControlValve->GetType() + CString(_T(" ")) + TASApp.LoadLocalizedString(IDS_CONFSEL_PRODUCTACCESSORY);
		lRow = _AddAccessoriesInfos(lRow, strTitle, pclSSelSmartControlValve->GetAccessoryList(), iGroupQty, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory);
	}
}

void CDlgConfSel::_FillListCtrlForSmartDpC( CSelectedInfos *pSelInfo, CDS_SSelSmartDpC *pclSSelSmartDpC )
{
	long lRow = 0;

	CDB_SmartControlValve *pclSmartDpC = pclSSelSmartDpC->GetProductAs<CDB_SmartControlValve>();

	if( NULL == pclSmartDpC || NULL == pclSmartDpC->GetSmartValveCharacteristic() )
	{
		return;
	}

	CString str;
	CString strRemark( _T("") );

	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclSmartDpC->GetComment() ).IsEmpty() )
		{
			strRemark = pclSmartDpC->GetComment();
		}
	}

	// Set now the remark in 'pSelInfo'.
	if( false == strRemark.IsEmpty() )
	{
		pSelInfo->SetRemark( strRemark );
	}

	_InitRefsAndRemarks( pSelInfo );

	// Check if a smart differential pressure controller with a set that is selected.
	IDPTR SetIDPtr = pclSSelSmartDpC->GetSetIDPtr();
	CDB_Set *pclSet = NULL;

	if( true == pclSSelSmartDpC->IsSelectedAsAPackage() && NULL != dynamic_cast<CDB_Set *>( SetIDPtr.MP ) )
	{
		// Write set information.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_SMARTDPCSET );
		lRow = m_ListCtrl.AddItemText( str );
		
		pclSet = (CDB_Set *)( SetIDPtr.MP );
		
		CDB_Product *pclSmartDpC = dynamic_cast<CDB_Product *>( pclSet->GetFirstIDPtr().MP );
		ASSERT( NULL != pclSmartDpC );

		CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>( pclSet->GetSecondIDPtr().MP );
		ASSERT( NULL != pclProductSet );

		str = _T("");

		if( NULL != pclSmartDpC && NULL != pclProductSet )
		{
			str = pclSmartDpC->GetName();
			str += _T(" + ");
			str += pclProductSet->GetName();
		}

		m_ListCtrl.SetItemTextBold( lRow, 0, true );
		lRow = m_ListCtrl.AddItemText( str );

		int iGroupQuantity = pSelInfo->GetQuantity();
		_AddQuantity( lRow, 1, iGroupQuantity );
		
		// White line.
		lRow = m_ListCtrl.AddItemText( _T( "" ) );
	}

	// Add the title corresponding to the valve.
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_SMARTDPCONTROLLER );
	lRow = m_ListCtrl.AddItemText( str );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	// Add the name of the valve.
	str = pclSmartDpC->GetName();
	long lSmartDpCNameRow = m_ListCtrl.AddItemText( str );
	int iGroupQty = pSelInfo->GetQuantity();
	_AddQuantity( lSmartDpCNameRow, 1, iGroupQty );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_BDYMAT );
	str += CString( _T(" : ") ) + pclSmartDpC->GetBodyMaterial();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTION );
	str += CString( _T(" : ") ) + pclSmartDpC->GetConnect();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PN );
	str += CString( _T(" : ") ) + pclSmartDpC->GetPN().c_str();
	m_ListCtrl.AddItemText( str );

	FormatString( str, IDS_CONFSEL_TEMPRANGE, pclSmartDpC->GetTempRange( true ) );
	lRow = m_ListCtrl.AddItemText( str );

	// Verify temperature.
	CString strErrorTemp = _T("");

	if( NULL != pclSSelSmartDpC->GetpSelectedInfos() && NULL != pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData() )
	{
		double dTemp = pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData()->GetTemp();

		if( dTemp < pclSmartDpC->GetTmin() )
		{
			m_ListCtrl.SetItemTextColor( lRow, 0, _RED, _RED );

			// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
			CString strProductName = pclSmartDpC->GetName();
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
			CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartDpC->GetTmin(), true );
			FormatString( strErrorTemp, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
			m_ListCtrl.AddToolTipText( lRow, 0, strErrorTemp );
		}
		else if( dTemp > pclSmartDpC->GetTmax() )
		{
			m_ListCtrl.SetItemTextColor( lRow, 0, _RED, _RED );

			// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
			CString strProductName = pclSmartDpC->GetName();
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
			CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartDpC->GetTmax(), true );
			FormatString( strErrorTemp, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
			m_ListCtrl.AddToolTipText( lRow, 0, strErrorTemp );
		}
	}

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPMAX );
	str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true );
	lRow = m_ListCtrl.AddItemText( str );

	// Verify Dp max input by user.
	CString strErrorDpMax = _T("");

	if( BST_CHECKED == pclSSelSmartDpC->GetCheckboxDpMax() && pclSSelSmartDpC->GetDpMaxValue() > 0.0 )
	{
		if( pclSSelSmartDpC->GetDpMaxValue() > pclSmartDpC->GetDpmax() )
		{
			m_ListCtrl.SetItemTextColor( lRow, 0, _RED, _RED );

			// The maximum differential pressure is above the limit of this product (%1 > %2).
			CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpMaxValue(), true, 3, 0 );
			CString str3 = WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true, 3, 0 );
			FormatString( strErrorDpMax, IDS_PRODUCTSELECTION_ERROR_DPMAX, str2, str3 );
			m_ListCtrl.AddToolTipText( lRow, 0, strErrorDpMax );
		}
	}

	// If error, set the smart differential pressure controller name in red and add tooltips.
	if( false == strErrorTemp.IsEmpty() || false == strErrorDpMax.IsEmpty() )
	{
		CString strToolTip = strErrorTemp;

		if( false == strErrorDpMax.IsEmpty() )
		{
			if( false == strToolTip.IsEmpty() )
			{
				strToolTip.Insert( 0, _T("- ") );
				strToolTip += _T("\r\n- ");
			}

			strToolTip += strErrorDpMax;
		}

		if( false == strToolTip.IsEmpty() )
		{
			m_ListCtrl.SetItemTextColor( lSmartDpCNameRow, 0, _RED, _RED );
			m_ListCtrl.AddToolTipText( lSmartDpCNameRow, 0, strToolTip );
		}
	}

	// Add 'Kvs' for smart differential pressure controller.
	CString str2;

	if( !m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str2 = TASApp.LoadLocalizedString( IDS_CONFSEL_KVS );
	}
	else
	{
		str2 = TASApp.LoadLocalizedString( IDS_CONFSEL_CV );
	}

	str2 += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclSmartDpC->GetSmartValveCharacteristic()->GetKvs() );
	lRow = m_ListCtrl.AddItemText( str2 );

	// Add the nominal flow.
	str = CString( _T("Qnom : ") ) + WriteCUDouble( _U_FLOW, pclSmartDpC->GetSmartValveCharacteristic()->GetQnom(), true );
	lRow = m_ListCtrl.AddItemText( str );

	// Power supply.
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_POWSUP );
	str += CString( _T(" : ") ) + pclSmartDpC->GetPowerSupplyStr();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_OUTPUTSIG );

	if( CString( _T("") ) == pclSmartDpC->GetInOutSignalsStr( false ) )
	{
		str += CString( _T(" :    -") );
	}
	else
	{
		str += CString( _T(" : ") ) + pclSmartDpC->GetInOutSignalsStr( false );
	}

	m_ListCtrl.AddItemText( str );

	// Format the string before putting it into the remark list.
	if( false == m_strComment.empty() )
	{
		pclSmartDpC->ApplyComment( &m_strComment );
	}

	// Fill smart differential pressure controller accessories if exist.
	if( pclSSelSmartDpC->IsAccessoryExist() )
	{
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclSmartDpC->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		CString strTitle = pclSmartDpC->GetType() + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_CONFSEL_PRODUCTACCESSORY );
		lRow = _AddAccessoriesInfos( lRow, strTitle, pclSSelSmartDpC->GetAccessoryList(), iGroupQty, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory );
	}

	CDB_DpSensor *pclDpSensor = NULL;

	if( NULL != pclSet )
	{
		pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSet->GetSecondIDPtr().MP );
	}
	else
	{
		pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSSelSmartDpC->GetDpSensorIDPtr().MP );
	}
	
	if( NULL == pclSet && NULL != pclDpSensor )
	{
		// Fill Dp sensor and accessories info.

		// Add a white line.
		if( lRow > 0 )
		{
			lRow = m_ListCtrl.AddItemText( _T("") );
		}

		// Add the title corresponding to the Dp sensor.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPSENSOR );
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		// Add name.
		lRow = m_ListCtrl.AddItemText( pclDpSensor->GetName() );
		
		// Dpl range .
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPLRANGE );
		str += CString( _T( " : " ) ) + pclDpSensor->GetFormatedDplRange( true ).c_str();
		lRow = m_ListCtrl.AddItemText( str );

		// Verify Dpl input by user.
		if( BST_CHECKED == pclSSelSmartDpC->GetCheckboxDpBranch() && pclSSelSmartDpC->GetDpBranchValue() > 0.0 )
		{
			if( pclSSelSmartDpC->GetDpBranchValue() < pclDpSensor->GetMinMeasurableDp() )
			{
				m_ListCtrl.SetItemTextColor( lRow, 0, _RED, _RED );

				// Dp to stabilize is below the minimum measurable pressure drop of the currenct selected Dp sensor (%1 < %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpBranchValue() );
				CString strMinMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMinMeasurableDp(), true );
				FormatString( str, IDS_HYDROMOD_SMARTDPCDPLTOOLOW, strDpl, strMinMeasurableDpl );
			}
			else if( pclSSelSmartDpC->GetDpBranchValue() > pclDpSensor->GetMaxMeasurableDp() )
			{
				// Dp to stabilize is above the maximum measurable pressure drop of the current selected Dp sensor (%1 > %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpBranchValue() );
				CString strMaxMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMaxMeasurableDp(), true );
				FormatString( str, IDS_HYDROMOD_SMARTDPCDPLTOOHIGH, strDpl, strMaxMeasurableDpl );
			}

			m_ListCtrl.SetItemTextColor( lRow, 0, _RED, _RED );
			m_ListCtrl.AddToolTipText( lRow, 0, str );
		}

		// Add PN.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_PN );
		str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclDpSensor->GetPmaxmax() );
		m_ListCtrl.AddItemText( str );

		// Add output signal
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_OUTPUTSIG );
		str += CString( _T( " : " ) ) + pclDpSensor->GetOutputSignalsStr( _T("\\") );
		m_ListCtrl.AddItemText( str );

		// Fill Dp sensor accessories if exist.
		if( pclSSelSmartDpC->IsDpSensorAccessoryExist() )
		{
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclDpSensor->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;

			if( NULL != pclRuledTable )
			{
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			}

			CString strTitle = pclDpSensor->GetType() + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_CONFSEL_PRODUCTACCESSORY );
			lRow = _AddAccessoriesInfos( lRow, strTitle, pclSSelSmartDpC->GetDpSensorAccessoryList(), iGroupQty, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory );
		}

		int iGroupQty = pSelInfo->GetQuantity();
		_AddQuantity( lRow, 1, iGroupQty );
	}
	else if( NULL != pclSet )
	{
		// Fill set content.
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclSet->GetAccGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		CString strTitle = TASApp.LoadLocalizedString( IDS_CONFSEL_SETCONTENT );
		lRow = _AddAccessoriesInfos( lRow, strTitle, pclSSelSmartDpC->GetSetContentAccessoryList(), iGroupQty, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_SetAccessory );
	}
	else if( NULL != pclSSelSmartDpC->GetSetIDPtr().MP )
	{
		// HYS-1992: Fill fictif set content.
		CDB_Product* pclFictifSet = (CDB_Product *)pclSSelSmartDpC->GetSetIDPtr().MP;
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>(pclFictifSet->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		// Fictif set title to get quantity on the line.
		long lFirstRow = lRow;
		m_ListCtrl.AddItemText( _T( "" ) );

		// HYS-2007
		CString strTitle = CteEMPTY_STRING;
		if( NULL != dynamic_cast<CDB_DpSensor*> (pclFictifSet ) )
		{
			strTitle = ((CDB_DpSensor*)pclFictifSet)->GetFullName();
		}
		else
		{
			// Connection set
			strTitle = pclFictifSet->GetName();
		}

		lRow = m_ListCtrl.AddItemText( strTitle );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );
		_AddQuantity( lRow, 1, iGroupQty );
		strTitle = _T( "" );
		lRow = _AddAccessoriesInfos( lRow, strTitle, pclSSelSmartDpC->GetSetContentAccessoryList(), iGroupQty, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_SetAccessory, false );
	}
}

void CDlgConfSel::_FillListCtrlForTALinkAlone( CSelectedInfos *pSelInfo, CDS_SSelDpSensor *pclSSel )
{
	long lRow = 0;

	CDB_DpSensor *pclTALink = pclSSel->GetProductAs<CDB_DpSensor>();

	if( NULL == pclTALink )
	{
		return;
	}

	CString str;
	CString strRemark( _T("") );

	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclTALink->GetComment() ).IsEmpty() )
		{
			strRemark = pclTALink->GetComment();
		}
	}

	// Set now the remark in 'pSelInfo'.
	if( false == strRemark.IsEmpty() )
	{
		pSelInfo->SetRemark( strRemark );
	}

	_InitRefsAndRemarks( pSelInfo );

	if( true == pclTALink->IsPartOfaSet() )
	{
		// HYS-2059: Fill fictif set content.
		CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>(pclTALink->GetAccessoriesGroupIDPtr().MP);
		std::vector<CData*> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		CString strTitle = ((CDB_DpSensor*)pclTALink)->GetFullName();
		lRow = m_ListCtrl.AddItemText( strTitle );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		int iGroupQty = pSelInfo->GetQuantity();
		_AddQuantity( lRow, 1, iGroupQty );
		strTitle = _T( "" );
		lRow = _AddAccessoriesInfos( lRow, strTitle, pclSSel->GetAccessoryList(), iGroupQty, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory, false );
	}
	else
	{
		// Add the title corresponding to the valve.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPSENSOR );
		lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		// Add the name of the valve.
		str = pclTALink->GetName();
		lRow = m_ListCtrl.AddItemText( str );
		int iGroupQty = pSelInfo->GetQuantity();
		_AddQuantity( lRow, 1, iGroupQty );

		// Dpl range.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPLRANGE );
		str += CString( _T( " : " ) ) + pclTALink->GetFormatedDplRange( true ).c_str();
		m_ListCtrl.AddItemText( str );

		// HYS-2007: Burst pressure
		if( 0 < pclTALink->GetBurstPressure() )
		{
			// Burst pressure: %1.
			CString strBurstPressure = WriteCUDouble( _U_DIFFPRESS, pclTALink->GetBurstPressure(), true );
			FormatString( str, IDS_TALINK_BURSTPRESSURE, strBurstPressure );
			m_ListCtrl.AddItemText( str );
		}

		// Output signal.
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_OUTPUTSIG );
		str += CString( _T( " : " ) ) + pclTALink->GetOutputSignalsStr();
		m_ListCtrl.AddItemText( str );

		// HYS-2007
		if( pclSSel->IsAccessoryExist() )
		{
			CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>(pclTALink->GetAccessoriesGroupIDPtr().MP);
			std::vector<CData*> vecBuiltInAccessories;

			if( NULL != pclRuledTable )
			{
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			}
			CString strTitle = pclTALink->GetType() + CString( _T( " " ) ) + TASApp.LoadLocalizedString( IDS_CONFSEL_PRODUCTACCESSORY );
			lRow = _AddAccessoriesInfos( lRow, strTitle, pclSSel->GetAccessoryList(), iGroupQty, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory );
		}
	}
}

void CDlgConfSel::_FillPMVesselHelper( CDB_Vessel *pclVessel, CDS_SSelPMaint *pclSelPressMaint, CString &strRemark, 
		CDB_TecBox::TBIntegratedVesselType eIntegratedVesselType, bool fShowAccessories )
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( NULL == pUnitDB )
	{
		return;
	}

	if( NULL == pclSelPressMaint || NULL == pclSelPressMaint->GetpInputUser() )
	{
		return;
	}

	// Add comment if exist.
	if( false == CString( pclVessel->GetComment() ).IsEmpty() )
	{
		if( false == strRemark.IsEmpty() )
		{
			strRemark += _T("\r\n");
		}

		strRemark = pclVessel->GetComment();
	}

	// Add vessel.
	CAccessoryList *pclVesselAccessoryList = NULL;
	int iIDSVesselAccessory;
	CString strVesselType;
	unsigned int uiQty = pclSelPressMaint->GetVesselNumber();

	if( CDB_TecBox::ePMIntVesselType_None == eIntegratedVesselType )
	{
		switch( pclVessel->GetVesselType() )
		{
			case CDB_Vessel::eVsslType_Statico:
				strVesselType = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_STATICOTITLE );
				pclVesselAccessoryList = pclSelPressMaint->GetVesselAccessoryList();
				iIDSVesselAccessory = IDS_CONFSEL_PRESSMAINT_VESSELACCESSORIES;
				break;

			case CDB_Vessel::eVsslType_StatMbr:
				strVesselType = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_MEMBRANEVESSEL );
				pclVesselAccessoryList = pclSelPressMaint->GetVesselAccessoryList();
				iIDSVesselAccessory = IDS_CONFSEL_PRESSMAINT_VESSELACCESSORIES;
				break;

			case CDB_Vessel::eVsslType_Prim:
				strVesselType = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_CPRSSOPRIMVESSELTITLE );
				pclVesselAccessoryList = pclSelPressMaint->GetVesselAccessoryList();
				iIDSVesselAccessory = IDS_CONFSEL_PRESSMAINT_PRIMVSSLACCTITLE;
				break;

			case CDB_Vessel::eVsslType_Sec:
				strVesselType = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_CPRSSOSECVESSELTITLE );
				pclVesselAccessoryList = NULL;
				iIDSVesselAccessory = -1;
				uiQty = pclSelPressMaint->GetSecondaryVesselNumber();
				break;

			case CDB_Vessel::eVsslType_Interm:
				strVesselType = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_INTERMVESSEL );
				pclVesselAccessoryList = pclSelPressMaint->GetIntermediateVesselAccessoryList();
				iIDSVesselAccessory = IDS_CONFSEL_PRESSMAINT_INTERMVESSELACCESSORIES;
				uiQty = pclSelPressMaint->GetIntermediateVesselNumber();
				break;

			case CDB_Vessel::eVsslType_Aqua:
				strVesselType = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_AQUAVESSEL );
				pclVesselAccessoryList = pclSelPressMaint->GetVesselAccessoryList();
				iIDSVesselAccessory = IDS_CONFSEL_PRESSMAINT_AQUAVESSELACCTITLE;
				break;
		}
	}
	else
	{
		if( CDB_TecBox::ePMIntVesselType_Primary == eIntegratedVesselType )
		{
			// Primary vessel integrated: it is the case for Simpl Compresso.
			strVesselType = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_INTEGRATEDVESSEL );
			// No accessories for integrated vessel.
			fShowAccessories = false;
		}
		else if( CDB_TecBox::ePMIntVesselType_Buffer == eIntegratedVesselType )
		{
			// Buffer vessel integraged: It was the case for TPV that are now deleted.
			strVesselType = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_BUFFERVESSEL );
			iIDSVesselAccessory = IDS_CONFSEL_PRESSMAINT_BUFFERVESSELACCTITLE;
		}
	}

	long lRow = m_ListCtrl.AddItemText( strVesselType );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );
	
	// HYS-987: Distributed quanttity for integrated
	if( CDB_TecBox::ePMIntVesselType_None == eIntegratedVesselType )
	{
		if( dynamic_cast<CDB_Vessel*>( pclSelPressMaint->GetTecBoxIntegratedVesselIDPtr().MP ) == pclVessel )
		{
			uiQty *= pclSelPressMaint->GetTecBoxCompTransfNumber();
		}
	}

	lRow = m_ListCtrl.AddItemText( pclVessel->GetName() );
	//HYS-987
	int iGroupQuantity = pclSelPressMaint->GetpSelectedInfos()->GetQuantity();
	_AddQuantity( lRow, uiQty, iGroupQuantity );

	CString str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_VESSELVOL );
	double dVesselVolume = pclVessel->GetNominalVolume();
	str += CString( _T(" : ") ) + WriteCUDouble( _U_VOLUME, dVesselVolume, true );
	lRow = m_ListCtrl.AddItemText( str );

	bool bCHVersion = ( 0 == TASApp.GetTADBKey().CompareNoCase( _T("CH") ) ) ? true : false;
	int iID = ( true == bCHVersion ) ? IDS_CONFSEL_PRESSMAINT_VESSELMAXPRESSCH : IDS_CONFSEL_PRESSMAINT_VESSELMAXPRESS;
	str = TASApp.LoadLocalizedString( iID );

	double dPS = ( true == bCHVersion ) ? pclVessel->GetPSch() : pclVessel->GetPmaxmax();
	str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dPS, true );
	lRow = m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_VESSELTEMPRANGE );
	str += CString( _T(" : " ) ) + pclVessel->GetTempRange() + _T( " ") + GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str();
	lRow = m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_VESSELWEIGHT );
	double dVesselWeight = pclVessel->GetWeight();
	str += CString( _T(" : ") ) + WriteCUDouble( _U_MASS, dVesselWeight, true );
	lRow = m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_VESSELMAXWEIGHT );
	double dVesselMaxWeight = pclSelPressMaint->GetpInputUser()->GetMaxWeight( dVesselWeight, dVesselVolume, pclSelPressMaint->GetpInputUser()->GetMinTemperature() );
	str += CString( _T(" : ") ) + WriteCUDouble( _U_MASS, dVesselMaxWeight, true );
	lRow = m_ListCtrl.AddItemText( str );


	// Add accessories if exist.
	if( true == fShowAccessories && NULL != pclVesselAccessoryList && pclVesselAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclVessel->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
		if( NULL != pclRuledTable )
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );

		_AddAccessoriesInfos( 0, iIDSVesselAccessory, pclVesselAccessoryList, iGroupQuantity, vecBuiltInAccessories, uiQty );
	}
}

void CDlgConfSel::_FillPMCompressoTransferoHelper( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint, CString &strRemark )
{
	if( NULL == pSelInfo || NULL == pclSelPressMaint )
	{
		return;
	}

	CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( ( CData * )pclSelPressMaint->GetTecBoxCompTransfIDPtr().MP );

	if( NULL == pclTecBox )
	{
		return;
	}

	// Add comment if exist.
	if( false == CString( pclTecBox->GetComment() ).IsEmpty() )
	{
		strRemark = pclTecBox->GetComment();
	}

	// Tecbox title.
	long lRow = 0;

	if( CDB_TecBox::etbtCompresso == pclTecBox->GetTecBoxType() )
	{
		lRow = m_ListCtrl.AddItemText( TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_CPRSSOTECBOXTITLE ) );
	}
	else if( CDB_TecBox::etbtTransfero == pclTecBox->GetTecBoxType() || CDB_TecBox::etbtTransferoTI == pclTecBox->GetTecBoxType() )
	{
		lRow = m_ListCtrl.AddItemText( TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TRSFROTECBOXTITLE ) );
	}

	m_ListCtrl.SetItemTextBold( lRow, 0, true );
	
	lRow = m_ListCtrl.AddItemText( pclTecBox->GetName() );
	int iGroupQty = pclSelPressMaint->GetpSelectedInfos()->GetQuantity();
	int iDistributedQty = pclSelPressMaint->GetTecBoxCompTransfNumber();
	_AddQuantity( lRow, iDistributedQty, iGroupQty );
	CString str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXPS );
	str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclTecBox->GetPmaxmax(), true );
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXPOWER );
	str += CString( _T(" : ") ) + WriteCUDouble( _U_ELEC_POWER, pclTecBox->GetPower(), true );
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXSUPPLYVOLTAGE );
	str += CString( _T(" : ") ) + pclTecBox->GetPowerSupplyStr();
	m_ListCtrl.AddItemText( str );

	// Check if there are accessories linked to the tecbox.
	CAccessoryList *pclTecBoxAccessoryList = pclSelPressMaint->GetTecBoxCompTransfAccessoryList();

	if( NULL != pclTecBoxAccessoryList && pclTecBoxAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTecBox->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
		if( NULL != pclRuledTable )
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );

		if( CDB_TecBox::etbtCompresso == pclTecBox->GetTecBoxType() )
		{
			_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_CPRSSOTECBOXACCTITLE, pclTecBoxAccessoryList, iGroupQty, vecBuiltInAccessories, iDistributedQty );
		}
		else if( CDB_TecBox::etbtTransfero == pclTecBox->GetTecBoxType() || CDB_TecBox::etbtTransferoTI == pclTecBox->GetTecBoxType() )
		{
			_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_TRSFROTECBOXACCTITLE, pclTecBoxAccessoryList, iGroupQty, vecBuiltInAccessories, iDistributedQty );
		}
	}

	bool bPrimaryVesselIntegrated = false;

	// Fill integrated vessel if exist.
	// Remark: for the moment, this "integrated" vessel can be the primary vessel (For Simply Compresso) or the selected buffer
	// vessel for Transfero.
	if( _NULL_IDPTR != pclSelPressMaint->GetTecBoxIntegratedVesselIDPtr() )
	{
		// White line.
		m_ListCtrl.AddItemText( _T("") );
	
		CDB_Vessel *pclIntegratedVessel = (CDB_Vessel *)( pclSelPressMaint->GetTecBoxIntegratedVesselIDPtr().MP );
		CDB_TecBox::TBIntegratedVesselType eIntegratedVesselType = pclTecBox->GetIntegratedVesselType();

		// For Simply Compresso, the 'CDB_TecBox::GetIntegratedVesselType' method will well return 'ePMVariant_IntegratedPrimaryVessel' because the primary
		// vessel CD 80.9 is well integrated in the TecBox.
		// For Transfero, the 'CDB_TecBox::GetIntegratedVesselType' method will either return a list of buffer vessel or return NULL if the buffer vessels are
		// not in a table but are fixed in one other way (Yes, all of this need a clarification) or if there is NO buffer vessel at all.
		// In this case for Transfero, if a buffer vessel was existing in the suggestion list (In the right view), the 'pclSelPressMaint->GetTecBoxIntegratedVesselIDPtr()'
		// will return the IDPTR of this buffer vessel. And to allow to have the write title in the '_FillPMVesselHelper' method, we will force the 4th argument to
		// the 'ePMIntVesselType_Buffer'.
		if( true == pclTecBox->IsFctPump() )
		{
			eIntegratedVesselType = CDB_TecBox::TBIntegratedVesselType::ePMIntVesselType_Buffer;
		}

		_FillPMVesselHelper( pclIntegratedVessel, pclSelPressMaint, strRemark, eIntegratedVesselType );

		bPrimaryVesselIntegrated = pclIntegratedVessel->IsPrimaryVesselIntegrated();
		
		// HYS-872: Check if there are accessories linked to the buffer vessel.
		CAccessoryList *pclTecBoxAccessoryList = pclSelPressMaint->GetTecBoxIntegratedVesselAccessoryList();

		if (NULL != pclTecBoxAccessoryList && pclTecBoxAccessoryList->GetCount() > 0)
		{
			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclIntegratedVessel->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;

			if( NULL != pclRuledTable )
			{
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			}

			if (CDB_TecBox::etbtTransfero == pclTecBox->GetTecBoxType() || CDB_TecBox::etbtTransferoTI == pclTecBox->GetTecBoxType())
			{
				_AddAccessoriesInfos(0, IDS_CONFSEL_PRESSMAINT_BUFFERVESSELACCTITLE, pclTecBoxAccessoryList, iGroupQty, vecBuiltInAccessories, iDistributedQty );
			}
		}
	}

	if( false == pclSelPressMaint->IsFromDirSel() )
	{
		// Prepare three accessory lists for primary and secondary vessel in case of an accessory in the primary vessel list is set as 'Distributed'.
		bool fSecondaryVesselExist = ( NULL != dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetSecondaryVesselIDPtr().MP ) ) ? true : false;
		CAccessoryList clPrimOnlyVesselAccessoryList;
		CAccessoryList clSecOnlyVesselAccessoryList;
		CAccessoryList clPrimAndSecVesselAccessoryList;

		if( NULL != pclSelPressMaint->GetVesselAccessoryList() )
		{
			CAccessoryList::AccessoryItem rAccessoryItem = pclSelPressMaint->GetVesselAccessoryList()->GetFirst();

			while( _NULL_IDPTR != rAccessoryItem.IDPtr )
			{
				if( false == rAccessoryItem.fDistributed )
				{
					if( true == fSecondaryVesselExist )
					{
						clPrimAndSecVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
					}
					else
					{
						clPrimOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
					}
				}
				else
				{
					clPrimOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );

					if( true == fSecondaryVesselExist )
					{
						clSecOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
					}
				}

				rAccessoryItem = pclSelPressMaint->GetVesselAccessoryList()->GetNext();
			}
		}

		// We have two solutions about integrated vessel. Either it's a buffer vessel as it's the case for Transfero TV.
		// Or it's a real integrated vessel that is playing the role of the primary vessel (like the one in the Simply Compresso).
		// When it's a buffer vessel, we show primary and secondary just after the buffer vessel.
		// When it's a integrated vessel, we must show after only the secondary vessel.

		if( false == bPrimaryVesselIntegrated )
		{
			// Compresso/Transfero vessel.
			CDB_Vessel *pclCompTransfVessel = dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetVesselIDPtr().MP );
			ASSERT( NULL != pclCompTransfVessel );

			if( NULL == pclCompTransfVessel )
			{
				return;
			}

			// White line.
			m_ListCtrl.AddItemText( _T("") );

			_FillPMVesselHelper( pclCompTransfVessel, pclSelPressMaint, strRemark, CDB_TecBox::ePMIntVesselType_None, false );

			// Add primary vessel accessories if exist.
			if( clPrimOnlyVesselAccessoryList.GetCount() > 0 )
			{
				// HYS-987: Built-in accessories
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclCompTransfVessel->GetAccessoriesGroupIDPtr().MP );
				std::vector<CData *> vecBuiltInAccessories;
				if( NULL != pclRuledTable )
					pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
				_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_PRIMVSSLACCTITLE, &clPrimOnlyVesselAccessoryList, iGroupQty, vecBuiltInAccessories );
			}
		}

		// If there is secondary vessel...
		if( true == fSecondaryVesselExist )
		{
			CDB_Vessel *pclCompTransfSecVessel = dynamic_cast<CDB_Vessel *>( ( CData * )pclSelPressMaint->GetSecondaryVesselIDPtr().MP );

			// Add a white line.
			m_ListCtrl.AddItemText( _T("") );

			_FillPMVesselHelper( pclCompTransfSecVessel, pclSelPressMaint, strRemark, CDB_TecBox::ePMIntVesselType_None, false );

			// Add accessories if exist.
			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclCompTransfSecVessel->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;
			if( NULL != pclRuledTable )
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			if( clSecOnlyVesselAccessoryList.GetCount() > 0 )
			{
				_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_SECVSSLACCTITLE, &clSecOnlyVesselAccessoryList, iGroupQty, vecBuiltInAccessories, pclSelPressMaint->GetSecondaryVesselNumber() );
			}

			if( clPrimAndSecVesselAccessoryList.GetCount() > 0 )
			{

				_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_PRIMSECVSSLACCTITLE, &clPrimAndSecVesselAccessoryList, iGroupQty, vecBuiltInAccessories, pclSelPressMaint->GetSecondaryVesselNumber() );
			}
		}

		// Check if there is an intermediate vessel selected.
		CDB_Vessel *pclIntermediateVessel = dynamic_cast<CDB_Vessel *>( pclSelPressMaint->GetIntermediateVesselIDPtr().MP );

		if( NULL != pclIntermediateVessel )
		{
			// White line.
			m_ListCtrl.AddItemText( _T("") );
			_FillPMVesselHelper( pclIntermediateVessel, pclSelPressMaint, strRemark );
		}

		// Fill Vento if there are some.
		if( 0 != pclSelPressMaint->GetTecBoxVentoNumber() )
		{
			// White line.
			m_ListCtrl.AddItemText( _T("") );
			_FillPMVentoHelper( pSelInfo, pclSelPressMaint, strRemark );
		}

		// Fill Pleno if there are some.
		if( 0 != pclSelPressMaint->GetTecBoxPlenoNumber() )
		{
			// White line.
			m_ListCtrl.AddItemText( _T("") );
			_FillPMPlenoHelper( pSelInfo, pclSelPressMaint, strRemark );
		}

		// Fill Pleno Refill if there are some.
		if( 0 != pclSelPressMaint->GetPlenoRefillNumber() )
		{
			// White line.
			m_ListCtrl.AddItemText( _T("") );
			_FillPMPlenoRefillHelper( pSelInfo, pclSelPressMaint, strRemark );
		}
	}
}

void CDlgConfSel::_FillPMPlenoHelper( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint, CString &strRemark )
{
	if( NULL == pSelInfo || NULL == pclSelPressMaint )
	{
		return;
	}

	CDB_Set *pWTCombination = dynamic_cast<CDB_Set *>( (CData*)pclSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP );
	int iLoopLimit = 1;

	if( NULL != pWTCombination )
	{
		iLoopLimit = 2;
	}

	for( int iLoop = 0; iLoop < iLoopLimit; iLoop++ )
	{
		CDB_TBPlenoVento *pclTecBoxPleno = NULL;

		if( 0 == iLoop )
		{
			if( NULL == pWTCombination )
			{
				pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pclSelPressMaint->GetTecBoxPlenoIDPtr().MP );
			}
			else
			{
				pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pWTCombination->GetFirstIDPtr().MP );
			}
		}
		else
		{
			pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pWTCombination->GetSecondIDPtr().MP );
		}

		if( 1 == iLoop && NULL != pclTecBoxPleno )
		{
			m_ListCtrl.AddItemText( _T("") );
		}
		int iGroupQty = pSelInfo->GetQuantity();
		int iDistributedQty = pclSelPressMaint->GetTecBoxPlenoNumber();
		if( NULL != pclTecBoxPleno )
		{
			// Add comment if exist.
			if( false == CString( pclTecBoxPleno->GetComment() ).IsEmpty() )
			{
				if( false == strRemark.IsEmpty() )
				{
					strRemark += _T("\r\n");
				}

				strRemark += pclTecBoxPleno->GetComment();
			}

			// Pleno tecbox.
			CString str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_PLENOTECBOXTITLE );
			long lRow = m_ListCtrl.AddItemText( str );
			m_ListCtrl.SetItemTextBold( lRow, 0, true );

			m_ListCtrl.AddItemText( pclTecBoxPleno->GetName() );
			_AddQuantity( lRow, iDistributedQty, iGroupQty );

			str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXPS );
			str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetPmaxmax(), true );
			m_ListCtrl.AddItemText( str );

			str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXPOWER );
			str += CString( _T(" : ") ) + WriteCUDouble( _U_ELEC_POWER, pclTecBoxPleno->GetPower(), true );
			m_ListCtrl.AddItemText( str );

			str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXSUPPLYVOLTAGE );
			str += CString( _T(" : ") ) + pclTecBoxPleno->GetPowerSupplyStr();
			m_ListCtrl.AddItemText( str );
		}

		if( NULL == pWTCombination || 1 == iLoop )
		{
			// Check if there are accessories linked to the tech-box
			CAccessoryList *pclTecBoxAccessoryList = pclSelPressMaint->GetTecBoxPlenoAccessoryList();

			if( NULL != pclTecBoxAccessoryList && pclTecBoxAccessoryList->GetCount() > 0 )
			{
				// HYS-987: Built-in accessories
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTecBoxPleno->GetAccessoriesGroupIDPtr().MP );
				std::vector<CData *> vecBuiltInAccessories;
				if( NULL != pclRuledTable )
					pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
				_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_PLENOTECBOXACCTITLE, pclTecBoxAccessoryList, iGroupQty, vecBuiltInAccessories, iDistributedQty );
			}
		}
	}
	// HYS-1121 : Add the protection module if exists
	CDB_TBPlenoVento* pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( (CData *)pclSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP );
	if( NULL != pclTecBoxPleno )
	{
		_FillPlenoProtectionHelper( pSelInfo, pclSelPressMaint, strRemark );
	}
}


void CDlgConfSel::_FillPlenoProtectionHelper( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint, CString &strRemark )
{
	if( NULL == pSelInfo || NULL == pclSelPressMaint )
	{
		return;
	}

	CDB_TBPlenoVento *pclTecBoxPleno = NULL;

	pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( (CData *)pclSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP );

	m_ListCtrl.AddItemText( _T("") );

	int iGroupQty = pSelInfo->GetQuantity();
	int iDistributedQty = pclSelPressMaint->GetTecBoxPlenoNumber();
	if( NULL != pclTecBoxPleno )
	{
		// Add comment if exist.
		if( false == CString( pclTecBoxPleno->GetComment() ).IsEmpty() )
		{
			if( false == strRemark.IsEmpty() )
			{
				strRemark += _T("\r\n");
			}

			strRemark += pclTecBoxPleno->GetComment();
		}

		// Pleno tecbox.
		CString str = TASApp.LoadLocalizedString( IDS_SELECT_PLENO_WMPROTECTION_TITLE );
		long lRow = m_ListCtrl.AddItemText( str );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		m_ListCtrl.AddItemText( pclTecBoxPleno->GetName() );
		_AddQuantity( lRow, iDistributedQty, iGroupQty );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXPS );
		str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetPmaxmax(), true );
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXPOWER );
		str += CString( _T(" : ") ) + WriteCUDouble( _U_ELEC_POWER, pclTecBoxPleno->GetPower(), true );
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXSUPPLYVOLTAGE );
		str += CString( _T(" : ") ) + pclTecBoxPleno->GetPowerSupplyStr();
		m_ListCtrl.AddItemText( str );
	}

	// Check if there are accessories linked to the tech-box
	CAccessoryList *pclTecBoxAccessoryList = pclSelPressMaint->GetTecBoxPlenoProtecAccessoryList();

	if( NULL != pclTecBoxAccessoryList && pclTecBoxAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTecBoxPleno->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
		if( NULL != pclRuledTable )
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_PLENOTECBOXACCTITLE, pclTecBoxAccessoryList, iGroupQty, vecBuiltInAccessories, iDistributedQty );
	}
}

void CDlgConfSel::_FillPMPlenoRefillHelper( CSelectedInfos* pSelInfo, CDS_SSelPMaint* pclSelPressMaint, CString& strRemark )
{
	if( NULL == pSelInfo || NULL == pclSelPressMaint )
	{
		return;
	}

	CDB_PlenoRefill *pclPlenoRefill = dynamic_cast<CDB_PlenoRefill *>( ( CData * )pclSelPressMaint->GetPlenoRefillIDPtr().MP );

	if( NULL == pclPlenoRefill )
	{
		return;
	}

	// Add comment if exist.
	if( false == CString( pclPlenoRefill->GetComment() ).IsEmpty() )
	{
		if( false == strRemark.IsEmpty() )
		{
			strRemark += _T("\r\n");
		}

		strRemark += pclPlenoRefill->GetComment();
	}

	// Pleno Refill.
	CString str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_PLENOREFILLTITLE );
	long lRow = m_ListCtrl.AddItemText( str );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	m_ListCtrl.AddItemText( pclPlenoRefill->GetName() );
	int iGroupQty = pSelInfo->GetQuantity();
	int iDistributedQty = pclSelPressMaint->GetPlenoRefillNumber();
	_AddQuantity( lRow, iDistributedQty, iGroupQty );
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_PLENOREFILLFUNCTIONS );
	str += CString( _T(" : ") ) + pclPlenoRefill->GetFunctionsStr();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_PLENOREFILLCAPACITY );
	str += CString( _T(" : ") ) + pclPlenoRefill->GetCapacityStr( true );
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_PLENOREFILLHEIGHT );
	str += CString( _T(" : ") ) + WriteCUDouble( _U_LENGTH, pclPlenoRefill->GetHeight(), true );
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_PLENOREFILLWEIGHT );
	str += CString( _T(" : ") ) + WriteCUDouble( _U_MASS, pclPlenoRefill->GetWeight(), true );
	m_ListCtrl.AddItemText( str );
	
	// Check if there are accessories linked to the Pleno refill.
	CAccessoryList *pclPlenoRefillAccessoryList = pclSelPressMaint->GetPlenoRefillAccessoryList();

	if( NULL != pclPlenoRefillAccessoryList && pclPlenoRefillAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclPlenoRefill->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
		if( NULL != pclRuledTable )
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_PLENOREFILLACCTITLE, pclPlenoRefillAccessoryList, iGroupQty, vecBuiltInAccessories, iDistributedQty );
	}
}

void CDlgConfSel::_FillPMVentoHelper( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint, CString &strRemark )
{
	if( NULL == pSelInfo || NULL == pclSelPressMaint )
	{
		return;
	}

	CDB_TBPlenoVento *pclTecBoxVento = dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pclSelPressMaint->GetTecBoxVentoIDPtr().MP );

	if( NULL == pclTecBoxVento )
	{
		return;
	}

	// Add comment if exist.
	if( false == CString( pclTecBoxVento->GetComment() ).IsEmpty() )
	{
		if( false == strRemark.IsEmpty() )
		{
			strRemark += _T("\r\n");
		}

		strRemark += pclTecBoxVento->GetComment();
	}

	// Vento tecbox.
	CString str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_VENTOTECBOXTITLE );
	long lRow = m_ListCtrl.AddItemText( str );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	m_ListCtrl.AddItemText( pclTecBoxVento->GetName() );
	int iGroupQty = pSelInfo->GetQuantity();
	int iDistributedQty = pclSelPressMaint->GetTecBoxVentoNumber();
	_AddQuantity( lRow, iDistributedQty, iGroupQty );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXPS );
	str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetPmaxmax(), true );
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXPOWER );
	str += CString( _T(" : ") ) + WriteCUDouble( _U_ELEC_POWER, pclTecBoxVento->GetPower(), true );
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_TECHBOXSUPPLYVOLTAGE );
	str += CString( _T(" : ") ) + pclTecBoxVento->GetPowerSupplyStr();
	m_ListCtrl.AddItemText( str );
	
	// Check if there are accessories linked to the tecbox.
	CAccessoryList *pclTecBoxAccessoryList = pclSelPressMaint->GetTecBoxVentoAccessoryList();

	if( NULL != pclTecBoxAccessoryList && pclTecBoxAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTecBoxVento->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;
		if( NULL != pclRuledTable )
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		_AddAccessoriesInfos( 0, IDS_CONFSEL_PRESSMAINT_VENTOTECBOXACCTITLE, pclTecBoxAccessoryList, iGroupQty, vecBuiltInAccessories, iDistributedQty );
	}
}

void CDlgConfSel::_FillActuators( CSelectedInfos *pSelInfo, CDB_Actuator *pActr, CDB_ControlValve *pCv, CString strTitle, COLORREF clColor,
		bool bFillFailSafeDRP )
{
	if( NULL == pActr )
	{
		return;
	}

	CString str = strTitle;

	if( true == str.IsEmpty() )
	{
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_ACTUATOR );
	}
	
	long lRow = m_ListCtrl.AddItemText( str );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	m_ListCtrl.AddItemText( ( LPCTSTR )pActr->GetName() );
	m_ListCtrl.SetItemTextColor( lRow, 0, clColor, clColor );

	int iGroupQty = pSelInfo->GetQuantity();
	_AddQuantity( lRow, 1, iGroupQty );

	// To use electrical characteristics if it's well an electric actuator.
	CDB_ElectroActuator *pElectroActr = dynamic_cast<CDB_ElectroActuator *>( pActr );

	if( NULL != pCv )
	{
		// Set the close off Dp if exist.
		CDB_CloseOffChar *pCloseOffChar = ( CDB_CloseOffChar * )pCv->GetCloseOffCharIDPtr().MP;

		if( pCloseOffChar != NULL && CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
		{
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_CLOSEOFFDP );
			double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pActr->GetMaxForceTorque() );

			if( dCloseOffDp > 0.0 )
			{
				str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
			}
			else
			{
				str += CString( _T(" : -") );
			}

			m_ListCtrl.AddItemText( str );
		}

		// Set the max inlet pressure if exist.
		if( pCloseOffChar != NULL && CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
		{
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_MAXINLETPRESSURE );
			double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pActr->GetMaxForceTorque() );

			if( dMaxInletPressure > 0.0 )
			{
				str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
			}
			else
			{
				str += CString( _T(" : -") );
			}

			m_ListCtrl.AddItemText( str );
		}

		if( NULL != pElectroActr )
		{
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_ACTUATING_TIME ) + CString( _T(" : ") ) + pElectroActr->GetActuatingTimesStr( pCv->GetStroke(), true );
			m_ListCtrl.AddItemText( str );
		}
	}
	else
	{
		if( pActr->GetMaxForceTorque() > 0 )
		{
			if( CDB_CloseOffChar::Linear == pActr->GetOpeningType() )
			{
				str = TASApp.LoadLocalizedString( IDS_CONFSEL_MAXFORCE );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_CONFSEL_MAXTORQUE );
			}

			str += CString( _T(" : ") ) + pActr->GetMaxForceTorqueStr( true );
			m_ListCtrl.AddItemText( str );
		}

		if( pElectroActr != NULL )
		{
			// Compute Actuating time in s/mm or s/deg.
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_ACTUATING_TIME ) + CString( _T(" : ") ) + pElectroActr->GetActuatingTimesStr( 0, true );
			m_ListCtrl.AddItemText( str );
		}
	}

	if( pElectroActr != NULL )
	{
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_IPXXAUTO );
		str += CString( _T(" : ") ) + pElectroActr->GetIPxxAuto();

		if( false == pElectroActr->GetIPxxManual().IsEmpty() )
		{
			str += _T("(" ) + pElectroActr->GetIPxxManual() + _T( ")");
		}

		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_POWSUP );
		str += CString( _T(" : ") ) + pElectroActr->GetPowerSupplyStr();
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_INPUTSIG );
		str += CString( _T(" : ") ) + pElectroActr->GetInOutSignalsStr( true );
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_OUTPUTSIG );

		if( CString( _T("") ) == pElectroActr->GetInOutSignalsStr( false ) )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pElectroActr->GetInOutSignalsStr( false );
		}

		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_RELAYTYPE );
		str += CString( _T(" : ") ) + pElectroActr->GetRelayStr();
		m_ListCtrl.AddItemText( str );

		if( true == bFillFailSafeDRP )
		{
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_FAILSAFE );

			if( pElectroActr->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
			{
				str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
			}
			else if( pElectroActr->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
			{
				str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
			}
			else
			{
				str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_NO );
			}
		
			m_ListCtrl.AddItemText( str );

			if( pElectroActr->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic 
				&& pElectroActr->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FSP ) + _T( " " );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP ) + _T( " " );
			}

			if( CString( _T("") ) == pElectroActr->GetDefaultReturnPosStr( pElectroActr->GetDefaultReturnPos() ).c_str() )
			{
				str += CString( _T("  -") );
			}
			else
			{
				str += pElectroActr->GetDefaultReturnPosStr( pElectroActr->GetDefaultReturnPos() ).c_str();
			}

			m_ListCtrl.AddItemText( str );
		}

		// HYS-726: To display max. temp. info for actuator.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

		if( DBL_MAX == pElectroActr->GetTmax() )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + WriteDouble( pElectroActr->GetTmax(), 3 );
			str += TASApp.LoadLocalizedString(IDS_UNITDEGREECELSIUS) ;
		}

		m_ListCtrl.AddItemText( str );
	}

	CDB_ThermostaticActuator *pThermoAct = dynamic_cast<CDB_ThermostaticActuator *>( pActr );

	if( pThermoAct != NULL )
	{
		// HYS-951: Min setting can be 0.
		if( pThermoAct->GetMinSetting() >= 0 && pThermoAct->GetMaxSetting() > 0 )
		{
			CString str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_SETTINGRANGE );
			CString strMin = WriteCUDouble( _U_TEMPERATURE, pThermoAct->GetMinSetting(), false );
			CString strMax =  WriteCUDouble( _U_TEMPERATURE, pThermoAct->GetMaxSetting(), true );
			str.Format( _T("%s: [%s-%s]"), str1, strMin, strMax );
			m_ListCtrl.AddItemText( str );
		}

		if( pThermoAct->GetCapillaryLength() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_CAPILLENGTH );
			str += CString( _T(": ") ) + WriteCUDouble( _U_LENGTH, pThermoAct->GetCapillaryLength(), true );
			m_ListCtrl.AddItemText( str );
		}

		if( pThermoAct->GetHysteresis() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_HYSTERESIS );
			str += CString( _T(": ") ) + WriteCUDouble( _U_DIFFTEMP, pThermoAct->GetHysteresis(), true );
			m_ListCtrl.AddItemText( str );
		}

		if( pThermoAct->GetFrostProtection() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_FROSTPROTECT );
			str += CString( _T(": ") ) + WriteCUDouble( _U_TEMPERATURE, pThermoAct->GetFrostProtection(), true );
			m_ListCtrl.AddItemText( str );
		}
		//HYS-726: To display max. temp. info for actuator
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

		if( DBL_MAX == pThermoAct->GetTmax() )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + WriteDouble( pThermoAct->GetTmax(), 3 );
			str += TASApp.LoadLocalizedString( IDS_UNITDEGREECELSIUS );
		}

		m_ListCtrl.AddItemText( str );
	}
}

long CDlgConfSel::_FillActuatorAccessories( CAccessoryList *pclActuatorAccessoryList, CDB_Actuator *pclActuator, int iGlobalQuantity, long lRow, CString strTitle, COLORREF clColor )
{
	if( NULL == pclActuatorAccessoryList || 0 == pclActuatorAccessoryList->GetCount() || NULL == pclActuator )
	{
		return lRow;
	}

	// White line.
	lRow = m_ListCtrl.AddItemText( _T("") );

	CString str = strTitle;

	if( true == strTitle.IsEmpty() )
	{
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_ACTACCESSORY );
	}

	lRow = m_ListCtrl.AddItemText( str );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );
	m_ListCtrl.SetItemTextColor( lRow, 0, clColor, clColor );

	// HYS-987: Built-in accessories
	CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclActuator->GetAccessoriesGroupIDPtr().MP );
	std::vector<CData *> vecBuiltInAccessories;
	std::vector<CData *>::iterator itBI;

	if( NULL != pclRuledTable )
	{
		pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
	}

	CAccessoryList::AccessoryItem rAccessoryItem = pclActuatorAccessoryList->GetFirst();

	while( rAccessoryItem.IDPtr.MP != NULL )
	{
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
		VERIFY( NULL != pclAccessory );

		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			break;
		}
					
		int iQuantity = ( true == rAccessoryItem.fByPair )? 2 : 1;

		str = pclAccessory->GetName() + CString( _T(" : ") ) + pclAccessory->GetComment();
		lRow = m_ListCtrl.AddItemText( str );

		// HYS-987.
		itBI = find( vecBuiltInAccessories.begin(), vecBuiltInAccessories.end(), pclAccessory );
					
		if( itBI != vecBuiltInAccessories.end() )
		{
			// Quantity of built-in accessories can't be edited.
			_AddQuantity( lRow, iQuantity, iGlobalQuantity, rAccessoryItem.lEditedQty, false );
		}
		else
		{
			_AddQuantity( lRow, iQuantity, iGlobalQuantity, rAccessoryItem.lEditedQty, true );
		}
					
		SelectedAccessories structAcc;
		structAcc.pclSelectedAccList = pclActuatorAccessoryList;
		structAcc.AccItemFromAccList.eAccessoryType = rAccessoryItem.eAccessoryType;
		structAcc.AccItemFromAccList.fByPair = rAccessoryItem.fByPair;
		structAcc.AccItemFromAccList.fDistributed = rAccessoryItem.fDistributed;
		structAcc.AccItemFromAccList.IDPtr = rAccessoryItem.IDPtr;
		structAcc.AccItemFromAccList.lEditedQty = rAccessoryItem.lEditedQty;
		m_mapSelectAccessories.insert( make_pair( lRow, structAcc ) );
		rAccessoryItem = pclActuatorAccessoryList->GetNext();
	}

	return lRow;
}

void CDlgConfSel::_AddQuantity( long lRow, unsigned int lQty, unsigned int lGroupQty, long lEditedQty, bool bToBeSaved )
{
	if ( lRow < 0 || lQty < 0 || lGroupQty < 0 )
	{
		return;
	}

	RECT rectColDesc;
	m_ListCtrl.GetItemRect( lRow, &rectColDesc, LVIR_BOUNDS );
	unsigned int *plQty = new unsigned int();
	unsigned int QtyGroup = lGroupQty;
	
	if( QtyGroup == 0 )
	{
		QtyGroup = 1;
	}
	
	*plQty = ( 0 == lQty)? 1 : lQty;
	CString str = CteEMPTY_STRING;
	
	if( -1 != lEditedQty )
	{
		if( BST_UNCHECKED == m_ButtonCheckBox.GetCheck() )
		{
			m_ButtonCheckBox.SetCheck( BST_CHECKED );
			OnClickedCheckBox();
		}
		
		str.Format( _T("%u"), (int)( lEditedQty ) );
	}
	else
	{
		str.Format( _T("%u"), (int)( ( *plQty ) * QtyGroup ) );
	}
	
	m_ListCtrl.SetItemText( lRow, 1, str );

	if( true == bToBeSaved )
	{
		CRect rect;
		m_ListCtrl.GetWindowRect( &rect );
		rect.right -= 4;
		rectColDesc.left += rect.Width() - 89;

		// HYS-1968: Use map instead of vector
		m_mapEditableQuantity.insert( std::pair<long, RECT>( lRow, rectColDesc ) );
	}

	if( m_mapQuantityRef.size() > 0 )
	{
		map<long, unsigned int*>::iterator it = m_mapQuantityRef.find( lRow );

		if( it == m_mapQuantityRef.end() )
		{
			m_mapQuantityRef.insert( make_pair( lRow, plQty ) );
		}
	}
	else
	{
		m_mapQuantityRef.insert( make_pair( lRow, plQty ) );
	}
}

void CDlgConfSel::_SetQuantityStatus( long lRowStart, long lRowEnd, bool bToBeSaved )
{
	int icount = 0;
	for( icount = lRowStart; icount < lRowEnd; icount++ )
	{
		map<long, RECT>::iterator it = m_mapEditableQuantity.begin();

		for( it = m_mapEditableQuantity.begin(); it != m_mapEditableQuantity.end(); ++it )
		{
			CRect rQty = it->second;
			if( icount == it->first && false == bToBeSaved )
			{
				m_mapEditableQuantity.erase( it );
				break;
			}
		}

		RECT rectColDesc;
		m_ListCtrl.GetItemRect( icount, &rectColDesc, LVIR_BOUNDS );

		if( true == bToBeSaved )
		{
			CRect rect;
			m_ListCtrl.GetWindowRect( &rect );
			rect.right -= 4;
			rectColDesc.left += rect.Width() - 89;

			m_mapEditableQuantity.insert( std::pair<long, RECT>( icount, rectColDesc ) );
		}
	}
}

void CDlgConfSel::_SetAllEditableQuantity( unsigned int lGroupQty )
{
	int iNbRow = m_ListCtrl.GetItemCount();

	for( map<long, unsigned int*>::iterator it = m_mapQuantityRef.begin(); it != m_mapQuantityRef.end(); ++it )
	{
		unsigned int* plQuantity = it->second;

		if( *plQuantity > 0 )
		{
			CString str = CteEMPTY_STRING; 
			str.Format( _T("%u"), (int)( ( *plQuantity ) * lGroupQty) );
			m_ListCtrl.SetItemText( it->first, 1, str );
		}
	}

	m_bApplyDefaultQty = true;
	m_bApplyAccessoriesQty = false;
}

void CDlgConfSel::_FillListCtrlForControlValve( CSelectedInfos *pSelInfo, CDS_SSelCtrl *pSelCtrl )
{
	long lRow = 0;

	CDB_ControlValve *pclControlValve = pSelCtrl->GetProductAs<CDB_ControlValve>();
	CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pSelCtrl->GetActrIDPtr().MP );

	if( NULL != pclControlValve || NULL != pclActuator )
	{
		CString str;
		CString strRemark( _T("") );

		// If product contains a comment, we add it to the remark list.
		if( true == pSelInfo->GetRemark().IsEmpty() )
		{
			if( false == CString( pclControlValve->GetComment() ).IsEmpty() )
			{
				strRemark = pclControlValve->GetComment();
			}
		}

		// With combined Dp controller, balancing and control valve, we can have shutoff valve on the return.
		if( true == pclControlValve->IsaDPCBCV() && NULL != dynamic_cast<CDS_SSelDpCBCV*>( pSelCtrl ) )
		{
			CDS_SSelDpCBCV *pclSSelDpCBCV = dynamic_cast<CDS_SSelDpCBCV*>( pSelCtrl );
			CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve*>( (CData*)pclSSelDpCBCV->GetSVIDPtr().MP );

			if( NULL != pclShutoffValve && false == CString( pclShutoffValve->GetComment() ).IsEmpty() )
			{
				if( false == strRemark.IsEmpty() )
				{
					strRemark += _T("\r\n");
				}

				strRemark += pclShutoffValve->GetComment();
			}
		}

		// Set now the remark in 'pSelInfo'.
		if( false == strRemark.IsEmpty() )
		{
			pSelInfo->SetRemark( strRemark );
		}

		_InitRefsAndRemarks( pSelInfo );

		// if selection has been done as a package...
		IDPTR idCvActSet = pSelCtrl->GetCvActrSetIDPtr();

		if( true == pSelCtrl->IsSelectedAsAPackage() && idCvActSet.MP != NULL )
		{
			// Set the title 'Control valve - Actuator set' in bold.
			pclControlValve->GetTableSet();

			switch( pclControlValve->GetCVParentTable() )
			{
				case CDB_ControlValve::ControlValveTable::CV:
					str = TASApp.LoadLocalizedString( IDS_CONFSEL_CVACTSET );
					break;

				case CDB_ControlValve::ControlValveTable::BCV:
					str = TASApp.LoadLocalizedString( IDS_CONFSEL_BCVACTSET );
					break;

				case CDB_ControlValve::ControlValveTable::PICV:
					str = TASApp.LoadLocalizedString( IDS_CONFSEL_PICVACTSET );
					break;

				case CDB_ControlValve::ControlValveTable::DPCBCV:
					str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPCBCVACTSET );
					break;
			}

			lRow = m_ListCtrl.AddItemText( str );
			m_ListCtrl.SetItemTextBold( lRow, 0, true );

			// Set the name of the set.
			CDB_Set *CvActSet = dynamic_cast<CDB_Set *>( idCvActSet.MP );
			ASSERT( CvActSet );
			str = CvActSet->GetName();
			lRow = m_ListCtrl.AddItemText( str );
			int iGroupQty = pSelCtrl->GetpSelectedInfos()->GetQuantity();
			_AddQuantity( lRow, 1, iGroupQty );
			// Add a white line.
			lRow = m_ListCtrl.AddItemText( _T("") );
		}

		int iCVAccessoryTileIDS = 0;

		// If control valve exists...
		if( pclControlValve != NULL )
		{
			CString str2;

			if( FromSSelCv == m_iFrom )
			{
				iCVAccessoryTileIDS = IDS_CONFSEL_ACCESSORY;

				if( !m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				{
					str2 = TASApp.LoadLocalizedString( IDS_CONFSEL_KVS );
				}
				else
				{
					str2 = TASApp.LoadLocalizedString( IDS_CONFSEL_CV );
				}

				str2 += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
				str = pclControlValve->GetType();
			}
			else if( FromSSelPICv == m_iFrom )
			{
				iCVAccessoryTileIDS = IDS_CONFSEL_PICV_ACCESSORY;
				str = pclControlValve->GetType();
			}
			else if( FromSSelBCv == m_iFrom )
			{
				iCVAccessoryTileIDS = IDS_CONFSEL_BVC_ACCESSORY;
				str = TASApp.LoadLocalizedString( IDS_CONFSEL_BCV );
			}
			else if( FromSSelDpCBCV == m_iFrom )
			{
				iCVAccessoryTileIDS = IDS_CONFSEL_DPCBCV_ACCESSORY;
				str = TASApp.LoadLocalizedString( IDS_CONFSEL_DPCBCV );
			}

			// Add the title corresponding to the valve.
			lRow = m_ListCtrl.AddItemText( str );
			m_ListCtrl.SetItemTextBold( lRow, 0, true );

			// Add the name of the valve.
			str = pclControlValve->GetName();
			lRow = m_ListCtrl.AddItemText( str );
			int iGroupQty = pSelCtrl->GetpSelectedInfos()->GetQuantity();
			_AddQuantity( lRow, 1, iGroupQty );

			// Add 'Kvs' for control valve.
			if( false == str2.IsEmpty() )
			{
				lRow = m_ListCtrl.AddItemText( str2 );
			}

			// Add the rangeability.
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_RANGEABILITY );
			str += CString( _T(" : ") ) + pclControlValve->GetStrRangeability().c_str();

			lRow = m_ListCtrl.AddItemText( str );

			// Add leakage.
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_CV_LEAKAGE );
			str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetLeakageRate() * 100 );
			str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_CONFSEL_CV_LEAKAGEUNIT );
			lRow = m_ListCtrl.AddItemText( str );

			// Format the string before putting it into the remark list.
			if( false == m_strComment.empty() )
			{
				pclControlValve->ApplyComment( &m_strComment );
			}
			
			int iQuantity = pSelCtrl->GetpSelectedInfos()->GetQuantity();
			
			// Add infos about accessories on the valve.
			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclControlValve->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;
			
			if( NULL != pclRuledTable )
			{
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			}

			lRow = _AddAccessoriesInfos( lRow, iCVAccessoryTileIDS, pSelCtrl->GetCvAccessoryList(), iQuantity, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_Accessory );
		}

		// If adapters exist...
		if( pSelCtrl->GetCvAccessoryList()->GetCount( CAccessoryList::AccessoryType::_AT_Adapter ) > 0 )
		{
			int iQuantity = pSelCtrl->GetpSelectedInfos()->GetQuantity();
			
			// HYS-987: Built-in accessories
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclControlValve->GetAccessoriesGroupIDPtr().MP );
			std::vector<CData *> vecBuiltInAccessories;
			
			if( NULL != pclRuledTable )
			{
				pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			}

			lRow = _AddAccessoriesInfos( lRow, IDS_CONFSEL_ADAPTER, pSelCtrl->GetCvAccessoryList(), iQuantity, vecBuiltInAccessories,
					1, CAccessoryList::AccessoryType::_AT_Adapter, ( 0 == lRow ) ? false : true );
		}

		// If actuator exist...
		if( pSelCtrl->GetActrIDPtr().MP != NULL )
		{
			// Actuator name.
			VERIFY( pclActuator );

			if( NULL == pclActuator )
			{
				return;
			}

			// Add a white line.
			if( lRow > 0 )
			{
				lRow = m_ListCtrl.AddItemText( _T("") );
			}

			_FillActuators( pSelCtrl->GetpSelectedInfos(), pclActuator, pclControlValve );

			CAccessoryList *pclActuatorAccessoryList = pSelCtrl->GetActuatorAccessoryList();

			if( pclActuatorAccessoryList != NULL && pclActuatorAccessoryList->GetCount() > 0 )
			{
				// White line.
				lRow = m_ListCtrl.AddItemText( _T("") );
				str = TASApp.LoadLocalizedString( IDS_CONFSEL_ACTACCESSORY );
				lRow = m_ListCtrl.AddItemText( str );
				m_ListCtrl.SetItemTextBold( lRow, 0, true );

				// HYS-987: Built-in accessories
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclActuator->GetAccessoriesGroupIDPtr().MP );
				std::vector<CData *> vecBuiltInAccessories;
				std::vector<CData *>::iterator itBI;

				if( NULL != pclRuledTable )
				{
					pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
				}

				CAccessoryList::AccessoryItem rAccessoryItem = pclActuatorAccessoryList->GetFirst();

				while( rAccessoryItem.IDPtr.MP != NULL )
				{
					CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
					VERIFY( NULL != pclAccessory );

					if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
					{
						break;
					}
					
					int iQuantity = ( true == rAccessoryItem.fByPair )? 2 : 1;

					str = pclAccessory->GetName() + CString( _T(" : ") ) + pclAccessory->GetComment();
					lRow = m_ListCtrl.AddItemText( str );

					// HYS-987.
					itBI = find( vecBuiltInAccessories.begin(), vecBuiltInAccessories.end(), pclAccessory );
					
					if( itBI != vecBuiltInAccessories.end() )
					{
						// Quantity of built-in accessories can't be edited.
						_AddQuantity( lRow, iQuantity, pSelCtrl->GetpSelectedInfos()->GetQuantity(), rAccessoryItem.lEditedQty, false );
					}
					else
					{
						_AddQuantity( lRow, iQuantity, pSelCtrl->GetpSelectedInfos()->GetQuantity(), rAccessoryItem.lEditedQty, true );
					}
					
					SelectedAccessories structAcc;
					structAcc.pclSelectedAccList = pclActuatorAccessoryList;
					structAcc.AccItemFromAccList.eAccessoryType = rAccessoryItem.eAccessoryType;
					structAcc.AccItemFromAccList.fByPair = rAccessoryItem.fByPair;
					structAcc.AccItemFromAccList.fDistributed = rAccessoryItem.fDistributed;
					structAcc.AccItemFromAccList.IDPtr = rAccessoryItem.IDPtr;
					structAcc.AccItemFromAccList.lEditedQty = rAccessoryItem.lEditedQty;
					m_mapSelectAccessories.insert( make_pair( lRow, structAcc ) );
					rAccessoryItem = pclActuatorAccessoryList->GetNext();
				}
			}
		}

		// If CV-Actuator set accessories exist...
		if( pSelCtrl->GetCvAccessoryList()->GetCount( CAccessoryList::AccessoryType::_AT_SetAccessory ) > 0 )
		{
			int iQuantity = pSelCtrl->GetpSelectedInfos()->GetQuantity();
			// HYS-987: Built-in accessories
			std::set<CDB_Set *> pSetTable;
			std::set<CDB_Set *>::const_iterator IterpSetTable;
			CTableSet *pTabSet = pclControlValve->GetTableSet();
			CDB_RuledTable *pclRuledTable = NULL;
			
			if( NULL != pTabSet )
			{
				if( pTabSet->FindCompatibleSet( &pSetTable, pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID ) )
				{
					IterpSetTable = pSetTable.begin();
					pclRuledTable = dynamic_cast<CDB_RuledTable *>( ( *IterpSetTable )->GetAccGroupIDPtr().MP );
				}

				std::vector<CData *> vecBuiltInAccessories;

				if( NULL != pclRuledTable )
				{
					pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
				}

				lRow = _AddAccessoriesInfos( lRow, IDS_CONFSEL_SETACCESSORY, pSelCtrl->GetCvAccessoryList(), iQuantity, vecBuiltInAccessories, 1, CAccessoryList::AccessoryType::_AT_SetAccessory );
			}
		}

		// In case of combined Dp controller, control and balancing valve we can have shutoff valve.
		if( FromSSelDpCBCV == m_iFrom && NULL != dynamic_cast<CDS_SSelDpCBCV*>( pSelCtrl ) )
		{
			CDS_SSelDpCBCV *pclSelDpCBCV = dynamic_cast<CDS_SSelDpCBCV*>( pSelCtrl );
			CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve*>( pclSelDpCBCV->GetSVIDPtr().MP );

			if( NULL != pclShutoffValve )
			{
				lRow = m_ListCtrl.AddItemText( _T("") );

				lRow = m_ListCtrl.AddItemText( pclShutoffValve->GetType() );
				m_ListCtrl.SetItemTextBold( lRow, 0, true );

				lRow = m_ListCtrl.AddItemText( pclShutoffValve->GetName() );
				int iGroupQty = pSelCtrl->GetpSelectedInfos()->GetQuantity();
				_AddQuantity( lRow, 1, iGroupQty );
				lRow = _AddTAProductInfos( lRow, pclShutoffValve );

				CAccessoryList *pclAccessoryList = pclSelDpCBCV->GetSVAccessoryList();
				int iQuantity = pclSelDpCBCV->GetpSelectedInfos()->GetQuantity();
				
				// Add accessories infos.
				int iIDS = IDS_CONFSEL_SV_ACCESSORY;
				
				// HYS-987: Built-in accessories
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclShutoffValve->GetAccessoriesGroupIDPtr().MP );
				std::vector<CData *> vecBuiltInAccessories;
				
				if( NULL != pclRuledTable )
				{
					pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
				}

				_AddAccessoriesInfos( lRow, iIDS, pclAccessoryList, iQuantity, vecBuiltInAccessories );
			}
		}
	}
}

void CDlgConfSel::_FillListCtrlForDpReliefValve( CSelectedInfos *pSelInfo, CDS_SSelDpReliefValve *pclSSelDpReliefValve )
{
	long lRow = 0;

	CString str;
	CDB_TAProduct *pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclSSelDpReliefValve->GetProductIDPtr().MP );
	
	if( NULL == pclTAProduct )
	{
		return;
	}

	// HYS-987.
	int iGroupQuantity = pSelInfo->GetQuantity();
	
	CDB_DpReliefValve *pclDpReliefValve = dynamic_cast<CDB_DpReliefValve *>( pclTAProduct );

	if( NULL == pclDpReliefValve )
	{
		return;
	}

	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclDpReliefValve->GetComment() ).IsEmpty() )
		{
			pSelInfo->SetRemark( pclDpReliefValve->GetComment() );
		}
	}

	_InitRefsAndRemarks( pSelInfo );

	lRow = m_ListCtrl.AddItemText( pclTAProduct->GetType() );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	lRow = m_ListCtrl.AddItemText( pclTAProduct->GetName() );
	
	// HYS-987.
	_AddQuantity( lRow, 1, iGroupQuantity );
	
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_BDYMAT );
	str += CString( _T(" : ") ) + pclDpReliefValve->GetBodyMaterial();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTION );
	str += CString( _T(" : ") ) + pclDpReliefValve->GetConnect();
	m_ListCtrl.AddItemText( str );

	if( false == CString( pclDpReliefValve->GetVersion() ).IsEmpty() )
	{
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_VERSION );
		str += CString( _T(" : ") ) + pclDpReliefValve->GetVersion();
		m_ListCtrl.AddItemText( str );
	}

	// Add PN.
	str = TASApp.LoadLocalizedString( IDS_PN );
	str += CString( _T(" ") ) + pclDpReliefValve->GetPN().c_str();
	m_ListCtrl.AddItemText( str );

	// Add setting range.
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_ADJUSTABLERANGE );
	str += CString( _T(" : ") ) + pclDpReliefValve->GetAdjustableRange( true );
	m_ListCtrl.AddItemText( str );

	// Add temperature range.
	FormatString( str, IDS_CONFSEL_TEMPRANGE, pclDpReliefValve->GetTempRange( true ) );
	m_ListCtrl.AddItemText( str );

	// Accessories.
	CAccessoryList *pclDpReliefValveAccessoryList = pclSSelDpReliefValve->GetAccessoryList();

	if( NULL != pclDpReliefValveAccessoryList && pclDpReliefValveAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories.
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTAProduct->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		_AddAccessoriesInfos( 0, IDS_CONFSEL_DPRELIEFVALVE_ACCESSORIES, pclDpReliefValveAccessoryList, iGroupQuantity, vecBuiltInAccessories );
	}

	// White line.
	m_ListCtrl.AddItemText( _T("") );
}

void CDlgConfSel::_FillListCtrlForTapWaterControl( CSelectedInfos *pSelInfo, CDS_SSelTapWaterControl *pclSSelTapWaterControl )
{
	long lRow = 0;

	CString str;
	CDB_TAProduct *pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclSSelTapWaterControl->GetProductIDPtr().MP );
	
	if( NULL == pclTAProduct )
	{
		return;
	}

	// HYS-987.
	int iGroupQuantity = pSelInfo->GetQuantity();
	
	CDB_TapWaterControl *pclTapWaterControl = dynamic_cast<CDB_TapWaterControl *>( pclTAProduct );

	if( NULL == pclTapWaterControl )
	{
		return;
	}

	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclTapWaterControl->GetComment() ).IsEmpty() )
		{
			pSelInfo->SetRemark( pclTapWaterControl->GetComment() );
		}
	}

	_InitRefsAndRemarks( pSelInfo );

	lRow = m_ListCtrl.AddItemText( pclTAProduct->GetType() );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	lRow = m_ListCtrl.AddItemText( pclTAProduct->GetName() );
	
	// HYS-987.
	_AddQuantity( lRow, 1, iGroupQuantity );
	
	// Add data.
	if( NULL != pclTapWaterControl )
	{
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_ADJUSTABLERANGE );
		str += CString( _T(" : ") ) + pclTapWaterControl->GetTempAdjustableRangeStr( true );
		lRow = m_ListCtrl.AddItemText( str );
	}

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_BDYMAT );
	str += CString( _T(" : ") ) + pclTapWaterControl->GetBodyMaterial();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTION );
	str += CString( _T(" : ") ) + pclTapWaterControl->GetConnect();
	m_ListCtrl.AddItemText( str );

	if( false == CString( pclTapWaterControl->GetVersion() ).IsEmpty() )
	{
		str = TASApp.LoadLocalizedString( IDS_CONFSEL_VERSION );
		str += CString( _T(" : ") ) + pclTapWaterControl->GetVersion();
		m_ListCtrl.AddItemText( str );
	}

	// Accessories.
	CAccessoryList *pclTapWaterControlAccessoryList = pclSSelTapWaterControl->GetAccessoryList();

	if( NULL != pclTapWaterControlAccessoryList && pclTapWaterControlAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories.
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTAProduct->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		_AddAccessoriesInfos( 0, IDS_CONFSEL_TAPWATERCONTROL_ACCESSORIES, pclTapWaterControlAccessoryList, iGroupQuantity, vecBuiltInAccessories );
	}

	// White line.
	m_ListCtrl.AddItemText( _T("") );
}

void CDlgConfSel::_FillListCtrlForFloorHeatingManifold( CSelectedInfos *pSelInfo, CDS_SSelFloorHeatingManifold *pclSSelFloorHeatingManifold )
{
	long lRow = 0;
	CString str;
	CDB_FloorHeatingManifold *pclFloorHeatingManifold = dynamic_cast<CDB_FloorHeatingManifold *>( pclSSelFloorHeatingManifold->GetProductIDPtr().MP );
	
	if( NULL == pclFloorHeatingManifold )
	{
		return;
	}

	// HYS-987.
	int iGroupQuantity = pSelInfo->GetQuantity();
	
	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclFloorHeatingManifold->GetComment() ).IsEmpty() )
		{
			pSelInfo->SetRemark( pclFloorHeatingManifold->GetComment() );
		}
	}

	_InitRefsAndRemarks( pSelInfo );

	lRow = m_ListCtrl.AddItemText( pclFloorHeatingManifold->GetType() );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	lRow = m_ListCtrl.AddItemText( pclFloorHeatingManifold->GetName() );
	
	// HYS-987.
	_AddQuantity( lRow, 1, iGroupQuantity );
	
	// Add data.
	str.Format( _T("%s : %i"), TASApp.LoadLocalizedString( IDS_CONFSEL_HEATINGCIRCUITS ), pclFloorHeatingManifold->GetHeatingCircuits() );
	lRow = m_ListCtrl.AddItemText( str );

	// Accessories.
	CAccessoryList *pclFloorHeatingManifoldAccessoryList = pclSSelFloorHeatingManifold->GetAccessoryList();

	if( NULL != pclFloorHeatingManifoldAccessoryList && pclFloorHeatingManifoldAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories.
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclFloorHeatingManifold->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		_AddAccessoriesInfos( 0, IDS_CONFSEL_FLOORHEATINGMANIFOLD_ACCESSORIES, pclFloorHeatingManifoldAccessoryList, iGroupQuantity, vecBuiltInAccessories );
	}

	// White line.
	m_ListCtrl.AddItemText( _T("") );
}

void CDlgConfSel::_FillListCtrlForFloorHeatingValve( CSelectedInfos *pSelInfo, CDS_SSelFloorHeatingValve *pclSSelFloorHeatingValve )
{
	long lRow = 0;
	CString str;
	CDB_FloorHeatingValve *pclFloorHeatingValve = dynamic_cast<CDB_FloorHeatingValve *>( pclSSelFloorHeatingValve->GetProductIDPtr().MP );
	
	if( NULL == pclFloorHeatingValve )
	{
		return;
	}

	// HYS-987.
	int iGroupQuantity = pSelInfo->GetQuantity();
	
	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclFloorHeatingValve->GetComment() ).IsEmpty() )
		{
			pSelInfo->SetRemark( pclFloorHeatingValve->GetComment() );
		}
	}

	_InitRefsAndRemarks( pSelInfo );

	// Remark: Here we don't take the type of the object for the name because this object has the "FHCTYPE_MANIFOLD" type.
	// We don't want that floor heating control valves are shown under one other category in the direct selection but well 
	// under the "Floor heating manifolds" type as it is in the IMI website.
	// And thus we force here the name to "Floor heating control valve".
	lRow = m_ListCtrl.AddItemText( TASApp.LoadLocalizedString( IDS_CONFSEL_FLOORHEATINGVALVES ) );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	lRow = m_ListCtrl.AddItemText( pclFloorHeatingValve->GetName() );
	
	// HYS-987.
	_AddQuantity( lRow, 1, iGroupQuantity );
	
	// Add data.

	// Comment if exist.
	if( 0 != _tcslen( pclFloorHeatingValve->GetComment() ) )
	{
		m_ListCtrl.AddItemText( pclFloorHeatingValve->GetComment() );
	}
	
	// Connection inlet.
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTIONIN );
	str += CString( _T(" : ") ) + pclFloorHeatingValve->GetConnectInlet();
	m_ListCtrl.AddItemText( str );

	// Connection outlet.
	str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTIONOUT );
	str += CString( _T(" : ") ) + pclFloorHeatingValve->GetConnectOutlet();
	m_ListCtrl.AddItemText( str );

	// With or without handheld (Only for supply pipe control valve).
	if( 0 == pclFloorHeatingValve->GetSupplyOrLockshield() )
	{
		m_ListCtrl.AddItemText( pclFloorHeatingValve->GetHasHandwheelString() );
	}

	// Accessories.
	CAccessoryList *pclFloorHeatingValveAccessoryList = pclSSelFloorHeatingValve->GetAccessoryList();

	if( NULL != pclFloorHeatingValveAccessoryList && pclFloorHeatingValveAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories.
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclFloorHeatingValve->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		_AddAccessoriesInfos( 0, IDS_CONFSEL_FLOORHEATINGVALVE_ACCESSORIES, pclFloorHeatingValveAccessoryList, iGroupQuantity, vecBuiltInAccessories );
	}

	// White line.
	m_ListCtrl.AddItemText( _T("") );
}

void CDlgConfSel::_FillListCtrlForFloorHeatingController( CSelectedInfos *pSelInfo, CDS_SSelFloorHeatingController *pclSSelFloorHeatingController )
{
	long lRow = 0;
	CString str;
	CDB_FloorHeatingController *pclFloorHeatingController = dynamic_cast<CDB_FloorHeatingController *>( pclSSelFloorHeatingController->GetProductIDPtr().MP );
	
	if( NULL == pclFloorHeatingController )
	{
		return;
	}

	// HYS-987.
	int iGroupQuantity = pSelInfo->GetQuantity();
	
	// If product contains a comment, we add it to the remark list.
	if( true == pSelInfo->GetRemark().IsEmpty() )
	{
		if( false == CString( pclFloorHeatingController->GetComment() ).IsEmpty() )
		{
			pSelInfo->SetRemark( pclFloorHeatingController->GetComment() );
		}
	}

	_InitRefsAndRemarks( pSelInfo );

	lRow = m_ListCtrl.AddItemText( pclFloorHeatingController->GetType() );
	m_ListCtrl.SetItemTextBold( lRow, 0, true );

	lRow = m_ListCtrl.AddItemText( pclFloorHeatingController->GetName() );
	
	// HYS-987.
	_AddQuantity( lRow, 1, iGroupQuantity );
	
	// Accessories.
	CAccessoryList *pclFloorHeatingControllerAccessoryList = pclSSelFloorHeatingController->GetAccessoryList();

	if( NULL != pclFloorHeatingControllerAccessoryList && pclFloorHeatingControllerAccessoryList->GetCount() > 0 )
	{
		// HYS-987: Built-in accessories.
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclFloorHeatingController->GetAccessoriesGroupIDPtr().MP );
		std::vector<CData *> vecBuiltInAccessories;

		if( NULL != pclRuledTable )
		{
			pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
		}

		_AddAccessoriesInfos( 0, IDS_CONFSEL_FLOORHEATINGCONTROLLER_ACCESSORIES, pclFloorHeatingControllerAccessoryList, iGroupQuantity, vecBuiltInAccessories );
	}

	// White line.
	m_ListCtrl.AddItemText( _T("") );
}

void CDlgConfSel::_FillPWQAccAndServices( CSelectedInfos* pSelInfo, CDS_SelPWQAccServices* pclSSelPWQAccAndSevices )
{
	long lRow = 0;
	CString str;
	CDB_PWQPressureReducer* pclPWQPressureReducer = dynamic_cast<CDB_PWQPressureReducer*>(pclSSelPWQAccAndSevices->GetProductIDPtr().MP);

	if( NULL != pclPWQPressureReducer )
	{
		// HYS-987.
		int iGroupQuantity = pSelInfo->GetQuantity();

		// If product contains a comment, we add it to the remark list.
		if( true == pSelInfo->GetRemark().IsEmpty() )
		{
			if( false == CString( pclPWQPressureReducer->GetComment() ).IsEmpty() )
			{
				pSelInfo->SetRemark( pclPWQPressureReducer->GetComment() );
			}
		}

		_InitRefsAndRemarks( pSelInfo );

		lRow = m_ListCtrl.AddItemText( pclPWQPressureReducer->GetType() );
		m_ListCtrl.SetItemTextBold( lRow, 0, true );

		lRow = m_ListCtrl.AddItemText( pclPWQPressureReducer->GetName() );

		// HYS-987.
		_AddQuantity( lRow, 1, iGroupQuantity );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_BDYMAT );
		str += CString( _T( " : " ) ) + pclPWQPressureReducer->GetBodyMaterial();
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTION );
		str += CString( _T( " : " ) ) + pclPWQPressureReducer->GetConnect();
		m_ListCtrl.AddItemText( str );

		if( false == CString( pclPWQPressureReducer->GetVersion() ).IsEmpty() )
		{
			str = TASApp.LoadLocalizedString( IDS_CONFSEL_VERSION );
			str += CString( _T( " : " ) ) + pclPWQPressureReducer->GetVersion();
			m_ListCtrl.AddItemText( str );
		}

		// Add PN.
		str = TASApp.LoadLocalizedString( IDS_PN );
		str += CString( _T( " " ) ) + pclPWQPressureReducer->GetPN().c_str();
		m_ListCtrl.AddItemText( str );

		// Add temperature range.
		FormatString( str, IDS_CONFSEL_TEMPRANGE, pclPWQPressureReducer->GetTempRange( true ) );
		m_ListCtrl.AddItemText( str );

		// Pressure.
		str = TASApp.LoadLocalizedString( IDS_PR_INLETPRESSURE );
		str += CString( _T( " " ) ) + WriteCUDouble( _U_PRESSURE, pclPWQPressureReducer->GetMaxInletPressure(), true );
		m_ListCtrl.AddItemText( str );

		str = TASApp.LoadLocalizedString( IDS_PR_OUTLETPRESSURE );
		str += CString( _T( " " ) ) + pclPWQPressureReducer->GetOutletPressureRange();
		m_ListCtrl.AddItemText( str );
	}

	// White line.
	m_ListCtrl.AddItemText( _T( "" ) );
}

void CDlgConfSel::_InitGlossaryDD( bool fClearEdRem, CString str )
{
	int i;
	CDB_DblString *pDblStr;
	m_bComboChanged = false;

	// Clear all items.
	m_ComboRemark.ResetContent();
	m_ComboRemark.LimitText( _DBLSTRING_LENGTHSTR1 - 1 );
	m_EditRemark.SetLimitText( _DBLSTRING_LENGTHSTR2 - 1 );

	// Clear remark edit.
	if( true == fClearEdRem )
	{
		m_EditRemark.SetWindowText( _T("") );
	}

	CTable *pGTab = ( CTable * )m_pUSERDB->Get( _T("GLOSSARY_TAB") ).MP;
	ASSERT( pGTab );

	// If string exist...
	if( pGTab->GetItemCount( CLASS( CDB_DblString ) ) )
	{
		for( IDPTR IDPtr = pGTab->GetFirst( CLASS( CDB_DblString ) ); NULL != *IDPtr.ID; IDPtr = pGTab->GetNext() )
		{
			pDblStr = ( CDB_DblString * )IDPtr.MP;
			i = m_ComboRemark.AddString( pDblStr->GetString1() );
			m_ComboRemark.SetItemDataPtr( i, ( void * )pDblStr );
		}
	}

	if( str != _T("") )
	{
		i = m_ComboRemark.FindStringExact( 0, str );

		if( i < 0 )
		{
			return;
		}

		// Select item.
		m_ComboRemark.SetCurSel( i );
		// Refresh remark box.
		pDblStr = ( CDB_DblString * )m_ComboRemark.GetItemDataPtr( i );
		m_EditRemark.SetWindowText( pDblStr->GetString2() );
	}
}

void CDlgConfSel::_InitRemark( CString str )
{
	// Try to find remark in all existing remarks.
	CTable *pGTab = ( CTable * )m_pUSERDB->Get( _T("GLOSSARY_TAB") ).MP;
	ASSERT( pGTab );

	// If string exist...
	if( pGTab->GetItemCount( CLASS( CDB_DblString ) ) )
	{
		for( IDPTR IDPtr = pGTab->GetFirst( CLASS( CDB_DblString ) ); NULL != *IDPtr.ID; IDPtr = pGTab->GetNext() )
		{
			CDB_DblString *pDblStr = ( CDB_DblString * )IDPtr.MP;
			CString strRem = pDblStr->GetString2();

			if( strRem == str )
			{
				_InitGlossaryDD( true, pDblStr->GetString1() );
				return;
			}
		}
	}

	// Remark not found in all existing remarks.
	m_EditRemark.SetWindowText( str );
}

void CDlgConfSel::_InitRefsAndRemarks( CSelectedInfos *pSelInfo )
{
	CString str;

	// Quantity is one by default.
	if( NULL != pSelInfo )
	{
		// Re-init First, second reference, remark and qty.
		m_Edit1stRef.SetWindowText( pSelInfo->GetReference( CSelectedInfos::eReferences::eRef1 ) );
		m_Edit2ndRef.SetWindowText( pSelInfo->GetReference( CSelectedInfos::eReferences::eRef2 ) );
		m_lQty = max( 1, pSelInfo->GetQuantity() );
		str.Format( _T("%u"), ( int )m_lQty );
		m_EditQty.SetWindowText( str );
		m_EditRemark.SetWindowText( pSelInfo->GetRemark() );
	}
}

void CDlgConfSel::_SaveRefsAndRemarks( CSelectedInfos *pSelInfo )
{
	CString str;

	// Quantity is one by default.
	if( NULL != pSelInfo )
	{
		// Re-init First, second reference, remark and qty.
		m_Edit1stRef.GetWindowText( str );
		pSelInfo->SetReference( CSelectedInfos::eReferences::eRef1, str );
		m_Edit2ndRef.GetWindowText( str );
		pSelInfo->SetReference( CSelectedInfos::eReferences::eRef2, str );
		pSelInfo->SetQuantity( max( 1, m_lQty ) );
		m_EditRemark.GetWindowText( str );
		pSelInfo->SetRemark( str );
	}
}

long CDlgConfSel::_AddAccessoriesInfos( int iRow, int iIDS, CAccessoryList *pclAccessoryList, int iGroupQuantity, std::vector<CData *> vecBuiltInAccessories,
		int iDistributedQty, CAccessoryList::AccessoryType eAccessoryType, bool bAddBlankLine, COLORREF clColor )
{
	CString strTitle = TASApp.LoadLocalizedString( iIDS );
	return _AddAccessoriesInfos( iRow, strTitle, pclAccessoryList, iGroupQuantity, vecBuiltInAccessories, iDistributedQty, eAccessoryType, bAddBlankLine, clColor );
}

long CDlgConfSel::_AddAccessoriesInfos( int iRow, CString strTitle, CAccessoryList *pclAccessoryList, int iGroupQuantity, std::vector<CData *> vecBuiltInAccessories, int iDistributedQty, 
		CAccessoryList::AccessoryType eAccessoryType, bool bAddBlankLine, COLORREF clColor )
{
	CString str;

	if( pclAccessoryList != NULL && pclAccessoryList->GetCount( eAccessoryType ) > 0 )
	{
		if( true == bAddBlankLine )
		{
			// Add a white line.
			m_ListCtrl.AddItemText( _T("") );
		}

		// Do not set title if it is empty.
		if( 0 != StringCompare( strTitle, _T( "" ) ) )
		{
			// Add title for accessories.
			iRow = m_ListCtrl.AddItemText( strTitle );
			m_ListCtrl.SetItemTextBold( iRow, 0, true );
			m_ListCtrl.SetItemTextColor( iRow, 0, clColor, clColor );
		}
		// Remark: accessories on valve, adapters and valve-actuator set accessories are all saved in the same list. 'eAccessoryType' allows to
		//         determine what type of accessory we want to show.
		CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst( eAccessoryType );

		while( rAccessoryItem.IDPtr.MP != NULL )
		{
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
			VERIFY( pclAccessory );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				break;
			}

			str = _T("");
			int iQty = iGroupQuantity;
			int iDistQty = 1;
			if( true == rAccessoryItem.fDistributed )
			{
				iDistQty = iDistributedQty;
			}

			if( true == rAccessoryItem.fByPair )
			{
				str += _T("2x ");
				iDistQty *= 2;
			}

			CString strName( _T("") );
			CString strDescription( _T("") );
			
			// Allow to fill specific name and description in regards to the situation.
			_AddAccessoriesInfosHelper_GetNameDescription( pclAccessory, strName, strDescription );

			str += strName;

			if( false == strDescription.IsEmpty() )
			{
				str += CString( _T(" : ") ) + strDescription;
			}
			
			iRow = m_ListCtrl.AddItemText( str );

			// Check if there is an error on this accessory.
			CString strError;
			_AddAccessoriesInfosHelper_GetErrorMessage( pclAccessory, strError );

			if( false == strError.IsEmpty() )
			{
				m_ListCtrl.SetItemTextColor( iRow, 0, _RED, _RED );
				m_ListCtrl.AddToolTipText( iRow, 0, strError );
			}

			// HYS-987.
			std::vector<CData *>::iterator itBI = find( vecBuiltInAccessories.begin(), vecBuiltInAccessories.end(), pclAccessory );

			if( itBI != vecBuiltInAccessories.end() )
			{
				// Quantity of built-in accessories can't be edited.
				_AddQuantity( iRow, iDistQty, iQty, rAccessoryItem.lEditedQty, false );
			}
			else
			{
				// HYS-987.
				_AddQuantity( iRow, iDistQty, iQty, rAccessoryItem.lEditedQty, true );
			}

			SelectedAccessories structAcc;
			structAcc.pclSelectedAccList = pclAccessoryList;
			structAcc.AccItemFromAccList.eAccessoryType = rAccessoryItem.eAccessoryType;
			structAcc.AccItemFromAccList.fByPair = rAccessoryItem.fByPair;
			structAcc.AccItemFromAccList.fDistributed = rAccessoryItem.fDistributed;
			structAcc.AccItemFromAccList.IDPtr = rAccessoryItem.IDPtr;
			structAcc.AccItemFromAccList.lEditedQty = rAccessoryItem.lEditedQty;
			m_mapSelectAccessories.insert( make_pair( iRow, structAcc ) );
			rAccessoryItem = pclAccessoryList->GetNext( eAccessoryType );
		}
	}

	return iRow;
}

void CDlgConfSel::_AddAccessoriesInfosHelper_GetNameDescription( CDB_Product *pclAccessory, CString &strName, CString &strDescription )
{
	strName = _T("");
	strDescription = _T("");

	if( NULL == pclAccessory )
	{
		return;
	}

	strName = pclAccessory->GetName();
	strDescription = pclAccessory->GetComment();

	// HYS-2007: manage Dp Sensor
	if( NULL != dynamic_cast<CDB_DpSensor *>( pclAccessory ) )
	{
		// Name with Dp range.
		CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclAccessory );
		strName = pclDpSensor->GetFullName();

		if( NULL != m_pSelSmartDpC )
		{
			// Add dpl if exists.
			if( true == m_pSelSmartDpC->IsCheckboxDpBranchChecked() && m_pSelSmartDpC->GetDpBranchValue() > 0.0 )
			{
				strName += _T("; ") + TASApp.LoadLocalizedString( IDS_DPL );
				strName += _T(": ") + CString( WriteCUDouble( _U_DIFFPRESS, m_pSelSmartDpC->GetDpBranchValue(), true ) );
			}
		}

		// Add temperature range.
		strName += CString( _T("; ") ) + pclDpSensor->GetTempRange( true );

		// Add description.
		// Burst pressure = %1.
		CString strBurstPressure = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true );
		FormatString( strDescription, IDS_TALINK_BURSTPRESSURE, strBurstPressure );
	}
}

void CDlgConfSel::_AddAccessoriesInfosHelper_GetErrorMessage( CDB_Product *pclAccessory, CString &strError )
{
	strError = _T("");

	if( NULL == pclAccessory )
	{
		return;
	}

	if( NULL != dynamic_cast<CDB_DpSensor *>( pclAccessory ) && NULL != m_pSelSmartDpC )
	{
		int iErrorCount = 0;
		CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclAccessory );

		if( BST_CHECKED == m_pSelSmartDpC->GetCheckboxDpBranch() && m_pSelSmartDpC->GetDpBranchValue() > 0.0 )
		{
			if( m_pSelSmartDpC->GetDpBranchValue() < pclDpSensor->GetMinMeasurableDp() )
			{
				// Dp to stabilize is below the minimum measurable pressure drop of the currenct selected Dp sensor (%1 < %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, m_pSelSmartDpC->GetDpBranchValue() );
				CString strMinMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMinMeasurableDp(), true );
				FormatString( strError, IDS_HYDROMOD_SMARTDPCDPLTOOLOW, strDpl, strMinMeasurableDpl );
				iErrorCount++;
			}
			else if( m_pSelSmartDpC->GetDpBranchValue() > pclDpSensor->GetMaxMeasurableDp() )
			{
				// Dp to stabilize is above the maximum measurable pressure drop of the current selected Dp sensor (%1 > %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, m_pSelSmartDpC->GetDpBranchValue() );
				CString strMaxMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMaxMeasurableDp(), true );
				FormatString( strError, IDS_HYDROMOD_SMARTDPCDPLTOOHIGH, strDpl, strMaxMeasurableDpl );
				iErrorCount++;
			}

			// HYS-2007: Use Burst pressure if it's above 0 value.
			if( 0 != pclDpSensor->GetBurstPressure() && m_pSelSmartDpC->GetDpBranchValue() > pclDpSensor->GetBurstPressure() )
			{
				if( 1 == iErrorCount )
				{
					strError.Insert( 0, _T("- ") );
				}
					
				if( iErrorCount >= 1 )
				{
					strError += _T("\r\n- ");
				}

				// The maximum differential pressure is above the limit of the burst pressure (%1 > %2).
				CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pSelSmartDpC->GetDpBranchValue(), true, 3, 0 );
				CString str3 = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true, 3, 0 );
				CString str4;
				FormatString( str4, IDS_PRODUCTSELECTION_ERROR_BURSTPRESSURE, str2, str3 );
				strError += str4;
				iErrorCount++;
			}
		}

		if( NULL != m_pSelSmartDpC->GetpSelectedInfos() && NULL != m_pSelSmartDpC->GetpSelectedInfos()->GetpWCData() )
		{
			double dTemp = m_pSelSmartDpC->GetpSelectedInfos()->GetpWCData()->GetTemp();

			if( dTemp < pclDpSensor->GetTmin() )
			{
				if( 1 == iErrorCount )
				{
					strError.Insert( 0, _T("- ") );
				}
					
				if( iErrorCount >= 1 )
				{
					strError += _T("\r\n- ");
				}

				// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
				CString strProductName = pclDpSensor->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
				CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmin(), true );
				CString str4;
				FormatString( str4, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
				strError += str4;
				iErrorCount++;
			}

			if( dTemp > pclDpSensor->GetTmax() )
			{
				if( 1 == iErrorCount )
				{
					strError.Insert( 0, _T("- ") );
				}
					
				if( iErrorCount >= 1 )
				{
					strError += _T("\r\n- ");
				}

				// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
				CString strProductName = pclDpSensor->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
				CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmax(), true );
				CString str4;
				FormatString( str4, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
				strError += str4;
			}
		}
	}
}

long CDlgConfSel::_AddTAProductInfos( long lRow, CDB_TAProduct *pTAP )
{
	CString str;

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_BDYMAT );
	str += CString( _T(" : ") ) + pTAP->GetBodyMaterial();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_CONNECTION );
	str += CString( _T(" : ") ) + pTAP->GetConnect();

	if( 0 == IDcmp( pTAP->GetConnTabID(), _T("COMPONENT_TAB") ) )
	{
		str += CString( _T(" -> ") ) + pTAP->GetSize();
	}

	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_VERSION );
	str += CString( _T(" : ") ) + pTAP->GetVersion();
	m_ListCtrl.AddItemText( str );

	str = TASApp.LoadLocalizedString( IDS_CONFSEL_PN );
	str += CString( _T(" ") ) + pTAP->GetPN().c_str();
	lRow = m_ListCtrl.AddItemText( str );

	return lRow;
}

void CDlgConfSel::_SetImage()
{
	if( (HGDIOBJ)NULL == m_Bmp.m_hObject )
	{
		return;
	}

	// Verify the "Gif" image has no higher dimension than the rectangle
	// If it's the case, stretch the image to fit in the rectangle.
	CRect rect;
	m_staticImg.GetClientRect( &rect );
	rect.DeflateRect( 3, 3, 3, 3 );
	CRect rectbmp = m_Bmp.GetSizeImage();

	if( rectbmp.Height() > rect.Height() || rectbmp.Width() > rect.Width() )
	{
		double dHeightDiff, dWidthDiff;
		dHeightDiff = dWidthDiff = 0;

		if( rectbmp.Height() > rect.Height() )
		{
			dHeightDiff = rectbmp.Height() - rect.Height();
		}

		if( rectbmp.Width() > rect.Width() )
		{
			dWidthDiff = rectbmp.Width() - rect.Width();
		}

		if( dHeightDiff > dWidthDiff )
		{
			m_Bmp.ResizeImage( 1 - ( dHeightDiff / (double)rectbmp.Height() ) );
		}
		else
		{
			m_Bmp.ResizeImage( 1 - ( dWidthDiff / (double)rectbmp.Width() ) );
		}
	}

	m_staticImg.SetBitmap( (HBITMAP)m_Bmp );
}
