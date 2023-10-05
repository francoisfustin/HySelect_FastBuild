#include "stdafx.h"

#include "afxctl.h"
#include <vector>
#include "TASelect.h"
#include "Global.h"
#include "Utilities.h"
#include "Database.h"
#include "Wizard.h"
#include "Dlgwizcircuit.h"
#include "Picture.h"
#include "HydroMod.h"
#include "EnBitmap.h"
#include "EnBitmapComponent.h"
#include "EnBitmapPatchWork.h"
#include "DlgWizCircuit.h"
#include "SSheetPanelCirc1.h"

CSSheetPanelCirc1::CSSheetPanelCirc1()
{
	m_iSelectedScheme = -1;
	m_lSelectedCol = 0;
	m_lSelectedRow = 0;
	// HYS-1579: Terminal unit is an enum class
	m_eTU = ePC1TU::ePC1TU_None;
}

CSSheetPanelCirc1::~CSSheetPanelCirc1( void )
{
	CleanSchArray();
}

void CSSheetPanelCirc1::Init( ePC1TU eTU, bool bShowDistributionPipe, CDS_HydroMod::ReturnType eReturnType, DWORD dwBalType, CDB_ControlProperties::eCVFUNC eCVFunc, 
		CDB_ControlProperties::CvCtrlType eCVCtrlType, int iCVTypeFamily, CDS_HydroMod *pHM )
{
	// Format spread sheet.
	ClearRange( -1, -1, -1, -1 );
	CSSheet::Init();

	// All cells are static by default and filled with _T("").
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatStaticText( -1, -1, -1, -1, _T("") );

	CMessageManager::MM_RegisterNotificationHandler( this, CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDown | CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDblClk );
	
	SetOperationMode( SS_OPMODE_READONLY );
	SetBool( SSB_ALLOWDRAGDROP, FALSE );
	SetBool( SSB_PROCESSTAB, TRUE );
	
	m_iSelectedScheme = -1;
	m_lSelectedCol = 0;
	m_lSelectedRow = 0;
	
	// HYS-1579.
	ePC1CVType eCVTypeFamily = (ePC1CVType)iCVTypeFamily;
	
	// Create an array with selectable scheme, depending of general parameters of pHM.
	 std::vector<CDB_CircuitScheme *> vecCircuitShemeList;
	
	CleanSchArray();
	
	int iDpCBestFamIndex = INT_MAX;
	
	// General parameters.
	CPrjParams *pPrjParams = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pPrjParams );
	
	CDB_CircuitScheme *pSelSch = NULL;

	if( NULL != pHM && NULL != pHM->GetpSch() )
	{
		// Add the CDB_CircuitScheme.
		pSelSch = pHM->GetpSch();
		bool bMustBeAdded = true;

		// Pump is allowed only on the root module.
		if( 0 == IDcmp( _T("PUMP"), pSelSch->GetIDPtr().ID ) || 0 == IDcmp( _T("PUMP_BV"), pSelSch->GetIDPtr().ID ) )
		{
			// Never display pump circuit for children.
			if( true == bShowDistributionPipe )
			{
				bMustBeAdded = false;
			}
		}
		
		// Terminal unit.
		if( ePC1TU::ePC1TU_TermUnit != eTU && CDB_CircuitScheme::eTERMUNIT::Compulsory == pSelSch->GetTermUnit() )
		{
			bMustBeAdded = false;
		}

		if( ePC1TU::ePC1TU_TermUnit == eTU && CDB_CircuitScheme::eTERMUNIT::NotAllowed == pSelSch->GetTermUnit() )
		{
			bMustBeAdded = false;
		}
		
		// Check control type keep NoControl or matching control.
		if( pSelSch->GetCvFunc() != CDB_ControlProperties::eCVFUNC::NoControl && pSelSch->GetCvFunc() != (eCVFunc & pSelSch->GetCvFunc()) )
		{
			bMustBeAdded = false;
		}
		// HYS-1579: Now the displayed circuit schema must have a compatible control valve type ( CVFunc )
		else if( pSelSch->GetCvFunc() == CDB_ControlProperties::eCVFUNC::NoControl && eCVFunc != CDB_ControlProperties::eCVFUNC::NoControl )
		{
			bMustBeAdded = false;
		}

		// HYS-1579: Check control valve type family: standard, adjustable, adjustable and measurable, pressure independent and smart
		// Manage  'Pressure independent' control valve type family
		if( pSelSch->GetCvFunc() != CDB_ControlProperties::eCVFUNC::NoControl
			 && ( CDB_ControlProperties::eCVFUNC::Presettable == ( eCVFunc & CDB_ControlProperties::eCVFUNC::Presettable )
				  || CDB_ControlProperties::eCVFUNC::PresetPT == ( eCVFunc & CDB_ControlProperties::eCVFUNC::PresetPT ) ) )
		{
			if( CDB_CircuitScheme::eDpCTypePICV == pSelSch->GetDpCType() && eCVTypeFamily != ePC1CVType::ePC1CV_PressInd )
			{
				bMustBeAdded = false;
			}
			if( CDB_CircuitScheme::eDpCTypePICV != pSelSch->GetDpCType() && eCVTypeFamily == ePC1CVType::ePC1CV_PressInd )
			{
				bMustBeAdded = false;
			}
		}
		
		// DpC
		// HYS-1579: No balance, Manual, dynamic balancing or electronic balancing.
		if( dwBalType != pSelSch->GetBalType() )
		{
			bMustBeAdded = false;
		}

		if( true == bMustBeAdded )
		{
			vecCircuitShemeList.push_back( pSelSch );
		}
	}

	Init_helper( &vecCircuitShemeList, eTU, bShowDistributionPipe, eReturnType, dwBalType, eCVFunc, eCVCtrlType, iCVTypeFamily, pHM );
		
	CTable *pTabCircSchCateg = (CTable *)( TASApp.GetpTADB()->Get( L"CIRCSCHCAT_TAB" ).MP );
	ASSERT( NULL != pTabCircSchCateg );

	CRank SortList( false );
	int i = 0;

	// Loop to verify for each circuit scheme with control valve if at least one valve exist or not.
	while( i < (int)vecCircuitShemeList.size() )
	{
		CDB_CircuitScheme *pCircuitScheme = vecCircuitShemeList.at( i );
		CDB_CircSchemeCateg *pCircSchCateg = (CDB_CircSchemeCateg *)( pTabCircSchCateg->Get( pCircuitScheme->GetSchemeCategID() ).MP );
		ASSERT( NULL != pCircSchCateg );
		
		// If we are in KvMode, we don't need to verify if a control valve exist to add the current circuit scheme.
		if( CPrjParams::_CVMode::KvMode == pPrjParams->GetCVMode() )
		{
			// Sort Scheme.
			_InsertCircuitScheme( &SortList, pCircSchCateg->GetOrderNr(), pCircuitScheme );
			i++;
			continue;
		}

		// If we the current circuit doesn't contain a control valve, no need to verify if a control valve exist.
		if( CDB_ControlProperties::eCVFUNC::NoControl == pCircuitScheme->GetCvFunc() )
		{
			// Sort Scheme.
			_InsertCircuitScheme( &SortList, pCircSchCateg->GetOrderNr(), pCircuitScheme );
			i++;
			continue;
		}
		
		bool bFind = false;
		SmartValveType eSmartValveType = SmartValveTypeUndefined;
		
		// Try to find one CDB_RegulatingValve with matching ctrl properties.
		std::vector<_string> vecStrTab;

		// Verify if it's a PiCv scheme, allow only PiCv valves.
		if( CDB_CircuitScheme::eDpCTypePICV == pCircuitScheme->GetDpCType() )
		{
			vecStrTab.push_back( L"PICTRLVALV_TAB" );
		}
		// HYS-1674: Add Smart control valve in PC1
		else if( SmartValveLocalization::SmartValveLocNone != pCircuitScheme->GetSmartControlValveLoc() )
		{
			eSmartValveType = SmartValveTypeControl;
			vecStrTab.push_back( L"SMARTCONTROLVALVE_TAB" );
		}
		else if( SmartValveLocalization::SmartValveLocNone != pCircuitScheme->GetSmartDpCLoc() )
		{
			// Special case for the smart differential pressure controller.
			eSmartValveType = SmartValveTypeDpC;
			vecStrTab.push_back( L"SMARTDIFFPRESS_TAB" );
		}
		else
		{
			vecStrTab.push_back( L"REGVALV_TAB" );
			vecStrTab.push_back( L"BALCTRLVALV_TAB" );
			vecStrTab.push_back( L"CTRLVALV_TAB" );
			vecStrTab.push_back( L"TRVALV_TAB" );
			vecStrTab.push_back( L"DPCBALCTRLVALV_TAB" );
		}

		for( UINT j = 0; j < vecStrTab.size() && false == bFind; j++ )
		{
			CTable *pTab = (CTable *)( TASApp.GetpTADB()->Get( vecStrTab[j].c_str() ).MP );

			for( IDPTR idptr = pTab->GetFirst(); NULL != *idptr.ID && false == bFind; idptr = pTab->GetNext() )
			{
				CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( idptr.MP );

				if( NULL == pRv )
				{
					continue;
				}

				if( false == pRv->IsSelectable( true ) )
				{
					continue;
				}

				if( eSmartValveType != SmartValveTypeUndefined )
				{
					CDB_SmartControlValve *pclSmartValve = dynamic_cast<CDB_SmartControlValve *>( pRv );

					if( NULL == pclSmartValve )
					{
						continue;
					}

					if( eSmartValveType == SmartValveTypeControl && pclSmartValve->GetSmartValveType() != SmartValveTypeControl )
					{
						continue;
					}

					if( eSmartValveType == SmartValveTypeDpC && pclSmartValve->GetSmartValveType() != SmartValveTypeDpC )
					{
						continue;
					}
				}
				
				CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pRv->GetCtrlProp() );

				if( NULL == pCtrlProp )
				{
					continue;
				}
				
				if( false == pCtrlProp->CvCtrlTypeFits( eCVCtrlType ) )
				{
					continue;
				}
				
				if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
				{
					// To exclude valves without characteristics .... KTM50
					CDB_ValveCharacteristic *pValvChar = pRv->GetValveCharacteristic();

					if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
					{
						continue;
					}
				}

				// Test Cv2W3W, CvFunc.
				bFind = pRv->IsValidCV( pCircSchCateg->Get2W3W(), pCircuitScheme->GetCvFunc() );

			}
		}
		
		// No regulating or thermostatic valve found, remove scheme.
		if( false == bFind )
		{
			vecCircuitShemeList.erase( vecCircuitShemeList.begin() + i );
			continue;
		}
		
		// Sort Scheme.
		_InsertCircuitScheme( &SortList, pCircSchCateg->GetOrderNr(), pCircuitScheme );
		i++;
	}

	const int iCols = 5;
	vecCircuitShemeList.clear();
	
	// Reinsert sorted item into the CArray.
	CString str;
	LPARAM itemdata;
	SortList.SetReadDeleteFlag( true );

	for( BOOL bContinue = SortList.GetFirst( str, itemdata ); TRUE == bContinue; bContinue = SortList.GetNext( str, itemdata ) )
	{
		CDB_CircuitScheme *pCircuitScheme = (CDB_CircuitScheme *)itemdata;
	
		sSchDesc SchDesc;
		SchDesc.pSch = pCircuitScheme;
		SchDesc.paFunc = new CArray<CAnchorPt::eFunc>;
		
		for( int j = 0; j < pCircuitScheme->GetAnchorPtListSize(); j++ )
		{
			SchDesc.paFunc->Add( pCircuitScheme->GetAnchorPtFunc( j ) );
		}
		
		m_arSchemes.Add( SchDesc );
	}

	// To have good scheme proportion (visual above all, not necessarily the true proportion of scheme), we set 140 x 229.
	CRect rectSchemeCellSize( 0, 0, 140, 229 );
	double dWidth, dHeight;
	LogUnitsToColWidth( 140, &dWidth );

	// 4 colums by scheme; nCols schemes on the panel width.
	int iMaxCols = iCols * 4 + 1;
	SetMaxCols( iMaxCols );

	for( i = 0; i < iMaxCols; i += 4 )
	{
		// Vertical separator.
		if( 0 == i )
		{
			SetColWidth( i + 1, 2 );
		}
		else
		{
			SetColWidth( i + 1, 1 );
		}
		
		// Vertical edge of selection frame.
		SetColWidth( i + 2, 0.3 );
				
		// Scheme width.
		SetColWidth( i + 3, dWidth );
		
		// Vertical edge of selection frame.
		SetColWidth( i + 4, 0.2 );
	}

	// 6 rows by line of scheme / iCols schemes by line.
	SetMaxRows( ( ( m_arSchemes.GetCount() / iCols ) + ( ( m_arSchemes.GetCount() % iCols ) !=0 ) ) * 6 );
	int iSchemeNumber = m_arSchemes.GetCount();

	for( i = 0; i < GetMaxRows(); i += 6 )
	{
		SetRowHeight( i + 1, iCols );		// Horizontal separator.
		SetRowHeight( i + 2, 1.4 );			// Horizontal edge of selection frame.
		
		// Scheme height.
		LogUnitsToRowHeight( i + 3, rectSchemeCellSize.Height(), &dHeight );
		SetRowHeight( i + 3, dHeight );
		
		SetRowHeight( i + 4, 1.4 );			// Horizontal edge of selection frame.
		SetRowHeight( i + 5, 1 );			// Thin horizontal separator.
		SetRowHeight( i + 6, 24 );			// Thin horizontal separator.
	}

	SetCellProperty( 1, 1, GetMaxCols(), GetMaxRows(), CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );

	i = 0;
	long lSelCol = 0;
	long lSelRow = 0;

	for( long lLoopRow = 3; lLoopRow < GetMaxRows() && i < iSchemeNumber; lLoopRow += 6 )
	{
		for( long lLoopColumn = 3; lLoopColumn < GetMaxCols() && i < iSchemeNumber; lLoopColumn += 4 )
		{
			SetCellProperty( lLoopColumn, lLoopRow, lLoopColumn, lLoopRow, CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, false );
			SetCellBorder( lLoopColumn, lLoopRow, lLoopColumn, lLoopRow, true, SS_BORDERTYPE_OUTLINE, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			// Span borders.
			AddCellSpanW( lLoopColumn - 1, lLoopRow - 1, 3, 1 );
			AddCellSpanW( lLoopColumn - 1, lLoopRow + 1, 3, 1 );
			CDB_CircuitScheme *pCircuitScheme = (CDB_CircuitScheme *)( m_arSchemes.GetAt( i ).pSch );

			if( NULL != pSelSch && pCircuitScheme == pSelSch && 0 == lSelCol && 0 == lSelRow )
			{
				/*
				// Schemes are matching; verify anchoring pt.
				CArray<CAnchorPt::eFunc> *paFunc = m_arSchemes.GetAt( i ).paFunc;
				bool bExist = true;

				// HYS-1579: use another index j for paFunc.
				for( int j = 0; j < paFunc->GetCount() && true == bExist; j++ )
				{
					if ( true == m_arSchemes.GetAt( i ).pSch->IsAnchorPtExist( (CAnchorPt::eFunc)i ) )
					{
						switch( paFunc->GetAt( j ) )
						{
							case CAnchorPt::BV_P:

								if ( false == m_arSchemes.GetAt( i ).pSch->IsAnchorPtOptional( CAnchorPt::BV_P ) )
								{
									bExist = ( NULL != pHM->GetpBv() ) ? true : false;
								}

								break;

							case CAnchorPt::BV_S:

								if ( false == m_arSchemes.GetAt( i ).pSch->IsAnchorPtOptional( CAnchorPt::BV_S ) )
								{
									bExist = ( NULL != pHM->GetpSecBv() ) ? true : false;
								}

								break;
						
							case CAnchorPt::PICV:
							case CAnchorPt::ControlValve:
								bExist = ( NULL != pHM->GetpCV() ) ? true : false;
								break;
						
							case CAnchorPt::DpC:
								bExist = ( NULL != pHM->GetpDpC() ) ? true : false;
								break;
						}
					}
				}	
				*/

				//if( true == bExist )
				//{
					lSelCol = lLoopColumn;
					lSelRow = lLoopRow;
				//}
			}
			
			// Define the type of cell as a picture.

			// Create the CEnBitmap.
			bool bCVModulating = false;

			if( NULL != pDlgWizCircuit )
			{
				bCVModulating = !pDlgWizCircuit->m_PanelCirc1.IsCvOnOff();
			}

			CEnBitmapPatchWork EnBmp;
			EnBmp.Reset();
			EnBmp.GetHydronicScheme( pCircuitScheme, (int)eTU, bCVModulating, m_arSchemes.GetAt( i ).paFunc, bShowDistributionPipe, eReturnType );
			
			EnBmp.ResizeImage( 0.25 );

			SetPictureCellWithHandle( (HBITMAP)EnBmp , lLoopColumn, lLoopRow, true );
			EnBmp.DeleteObject();

			// Set Picture associated text.
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
			
			SetStaticText( lLoopColumn, lLoopRow + 3, pCircuitScheme->GetSchemeCategName().c_str() );
			SetCellParam( lLoopColumn, lLoopRow, (long)i );
			i++;
		}
	}	

	m_eTU = eTU;

	if( lSelCol > 0 && lSelRow > 0 )
	{
		_OnLButtonDown( lSelCol, lSelRow );
	}
	else
	{
		_OnLButtonDown( 3, 3 );
	}
}

void CSSheetPanelCirc1::Init_helper( std::vector<CDB_CircuitScheme *> *pvecCircuitShemeList, ePC1TU eTU, bool bShowDistributionPipe, CDS_HydroMod::ReturnType eReturnType, 
		DWORD dwBalType, CDB_ControlProperties::eCVFUNC eCVType, CDB_ControlProperties::CvCtrlType eCVCtrlType, int iCVTypeFamily, CDS_HydroMod *pHM )
{
	if( NULL == pvecCircuitShemeList )
	{
		ASSERT_RETURN;
	}

	CTable *pTabSch = (CTable *)( TASApp.GetpTADB()->Get( _T("CIRCSCHEME_TAB") ).MP );
	ASSERT( NULL != pTabSch ); 

	// Loop on all existing circuit scheme defined in the 'CIRCSCHEME_TAB' table.
	for( IDPTR IDPtrSch = pTabSch->GetFirst(); _T('\0') != *IDPtrSch.ID; IDPtrSch = pTabSch->GetNext() )
	{
		CDB_CircuitScheme *pCircuitScheme = dynamic_cast<CDB_CircuitScheme *>( IDPtrSch.MP );

		if( NULL == pCircuitScheme )
		{
			continue;
		}

		// Already added
		if ( NULL != pHM && pCircuitScheme == pHM->GetpSch() )
		{
			continue;
		}

		// Pump is allowed only on the root module.
		if ( ( 0 == IDcmp( _T("PUMP"), IDPtrSch.ID ) ) || ( 0 == IDcmp( _T("PUMP_BV"), IDPtrSch.ID ) ) )
		{
			// Never display Pump circuit for children.
			if (true == bShowDistributionPipe)
			{
				continue;
			}
		}
		
		// HYS-1579: No balance, Manual, dynamic balancing or electronic balancing.
		if( dwBalType != pCircuitScheme->GetBalType() )
		{
			continue;
		}
		
		// Terminal Unit.
		if( ePC1TU::ePC1TU_TermUnit != eTU && CDB_CircuitScheme::eTERMUNIT::Compulsory == pCircuitScheme->GetTermUnit() )
		{
			continue;
		}

		if( ePC1TU::ePC1TU_TermUnit == eTU && CDB_CircuitScheme::eTERMUNIT::NotAllowed == pCircuitScheme->GetTermUnit() )
		{
			continue;
		}
		
		// Check control valve type keep NoControl or matching control.
		if( pCircuitScheme->GetCvFunc() != CDB_ControlProperties::eCVFUNC::NoControl && pCircuitScheme->GetCvFunc() != ( eCVType & pCircuitScheme->GetCvFunc() ) )
		{
			continue;
		}
		// HYS-1579: Now the displayed circuit schema must have a compatible control valve type ( CVFunc )
		else if( pCircuitScheme->GetCvFunc() == CDB_ControlProperties::eCVFUNC::NoControl && eCVType != CDB_ControlProperties::eCVFUNC::NoControl )
		{
			continue;
		}

		// HYS-1519: Check control valve type family: standard, adjustable, adjustable and measurable, pressure independent and smart
		// Manage 'Pressure independent'
		if( pCircuitScheme->GetCvFunc() != CDB_ControlProperties::eCVFUNC::NoControl 
			&& ( CDB_ControlProperties::eCVFUNC::Presettable == ( eCVType & CDB_ControlProperties::eCVFUNC::Presettable )
			    || CDB_ControlProperties::eCVFUNC::PresetPT == ( eCVType & CDB_ControlProperties::eCVFUNC::PresetPT ) ) )
		{
			if( CDB_CircuitScheme::eDpCTypePICV == pCircuitScheme->GetDpCType() && iCVTypeFamily != (int)ePC1CVType::ePC1CV_PressInd )
			{
				continue;
			}
			if( CDB_CircuitScheme::eDpCTypePICV != pCircuitScheme->GetDpCType() && iCVTypeFamily == (int)ePC1CVType::ePC1CV_PressInd )
			{
				continue;
			}
		}

		// HYS-1763: In case of DoBack() with a smart control valve in PC2 we should keep the location in PC2 (pHM->GetpSch()).
		if( NULL != pHM && NULL != pHM->GetpSch() && SmartValveLocalization::SmartValveLocNone != pHM->GetpSch()->GetSmartControlValveLoc()
				&& SmartValveLocalization::SmartValveLocNone != pCircuitScheme->GetSmartControlValveLoc() 
				&& pHM->GetpSch()->GetSmartControlValveLoc() != pCircuitScheme->GetSmartControlValveLoc() )
		{
			continue;
		}
		else if( NULL == pHM || NULL == pHM->GetpSch() || SmartValveLocalization::SmartValveLocNone == pHM->GetpSch()->GetSmartControlValveLoc() )
		{
			// In case of DoBack() with no smart control valve in PC2 we should keep the location in technical parameters.
			// Same remark for PC1 init from contextual menu.
			CDS_TechnicalParameter* pclTechParam = TASApp.GetpTADS()->GetpTechParams();
			int iDefaultSmartCVLocation = -1;

			if( NULL != pclTechParam )
			{
				iDefaultSmartCVLocation = pclTechParam->GetDefaultSmartCVLoc();
			}

			if( SmartValveLocalization::SmartValveLocReturn == pCircuitScheme->GetSmartControlValveLoc()
					&& SmartValveLocalization::SmartValveLocReturn != iDefaultSmartCVLocation )
			{
				continue;
			}

			if( SmartValveLocalization::SmartValveLocSupply == pCircuitScheme->GetSmartControlValveLoc()
					&& SmartValveLocalization::SmartValveLocSupply != iDefaultSmartCVLocation )
			{
				continue;
			}
		}

		// HYS-1956: In case of DoBack() with a smart differential pressure controller in PC2 we should keep the location in PC2 (pHM->GetpSch()).
		if( NULL != pHM && NULL != pHM->GetpSch() && SmartValveLocalization::SmartValveLocNone != pHM->GetpSch()->GetSmartDpCLoc()
				&& SmartValveLocalization::SmartValveLocNone != pCircuitScheme->GetSmartDpCLoc() 
				&& pHM->GetpSch()->GetSmartDpCLoc() != pCircuitScheme->GetSmartDpCLoc() )
		{
			continue;
		}
		else if( NULL == pHM || NULL == pHM->GetpSch() || SmartValveLocalization::SmartValveLocNone == pHM->GetpSch()->GetSmartDpCLoc() )
		{
			// In case of DoBack() with no smart differential pressure controller in PC2 we should keep the location in technical parameters.
			// Same remark for PC1 init from contextual menu.
			CDS_TechnicalParameter *pclTechParam = TASApp.GetpTADS()->GetpTechParams();
			int iDefaultSmartDpCLocation = -1;

			if( NULL != pclTechParam )
			{
				iDefaultSmartDpCLocation = pclTechParam->GetDefaultSmartCVLoc();
			}

			if( SmartValveLocalization::SmartValveLocReturn == pCircuitScheme->GetSmartDpCLoc()
					&& SmartValveLocalization::SmartValveLocReturn != iDefaultSmartDpCLocation )
			{
				continue;
			}

			if( SmartValveLocalization::SmartValveLocSupply == pCircuitScheme->GetSmartDpCLoc()
					&& SmartValveLocalization::SmartValveLocSupply != iDefaultSmartDpCLocation )
			{
				continue;
			}
		}

		// For DpC stabilization display only one schema with correct Mv position.
		bool bAlreadyExist = false;

		for( int k = 0; k < (int)pvecCircuitShemeList->size() && false == bAlreadyExist; k++ )
		{
			CDB_CircuitScheme *pPreviousCircSch = pvecCircuitShemeList->at( k );

			if( NULL != pPreviousCircSch )
			{
				if( CDB_CircuitScheme::eBALTYPE::DPC == pCircuitScheme->GetBalType() && CDB_CircuitScheme::eDpCTypeDPC == pCircuitScheme->GetDpCType() )
				{
					// HYS-1579: Make the difference between DPCType DpCBcv and Dpc, not the same schema. When clicking on previous button
					if( pCircuitScheme->GetDpCType() == pPreviousCircSch->GetDpCType()
							&& pCircuitScheme->GetpSchCateg() == pPreviousCircSch->GetpSchCateg()
							&& pCircuitScheme->GetCvFunc() == pPreviousCircSch->GetCvFunc()
							&& pCircuitScheme->GetDpStab() == pPreviousCircSch->GetDpStab()
							&& pCircuitScheme->GetCvLoc() == pPreviousCircSch->GetCvLoc()
							&& pCircuitScheme->GetMvLoc() != pPreviousCircSch->GetMvLoc() )
					{
						bAlreadyExist = true;
					}
				}
				else if( CDB_CircuitScheme::eBALTYPE::ELECTRONIC == pCircuitScheme->GetBalType() && CDB_CircuitScheme::eDpCTypeSMARTDP == pCircuitScheme->GetDpCType() )
				{
					// HYS-1955: For the moment we have only 2 schemas with a TA-Smart Dp on the supply and the return.
					// We want to show only one of both.
					if( CDB_CircuitScheme::eDpCTypeSMARTDP == pPreviousCircSch->GetDpCType() )
					{
						bAlreadyExist = true;
					}
				}
			}
		}

		if( false == bAlreadyExist )
		{
			// Add the CDB_CircuitScheme.
			pvecCircuitShemeList->push_back( pCircuitScheme );
		}
	}
}

CSSheetPanelCirc1::ePC1CVType CSSheetPanelCirc1::ConvertCvFunc2PC1CVType( CDB_ControlProperties::eCVFUNC eCVType )
{
	CSSheetPanelCirc1::ePC1CVType eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_NoControl;

	if( CDB_ControlProperties::eCVFUNC::ControlOnly == eCVType )
	{
		eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_Standard;
	}
	else if( CDB_ControlProperties::eCVFUNC::Presettable == eCVType )
	{
		eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_Adjustable;
	}
	else if( CDB_ControlProperties::eCVFUNC::PresetPT == eCVType )
	{
		eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_AdjustableMeas;
	}
	else if( ( CDB_ControlProperties::eCVFUNC::Presettable | CDB_ControlProperties::eCVFUNC::PresetPT ) == eCVType )
	{
		eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_PressInd;
	}
	else if( CDB_ControlProperties::eCVFUNC::Electronic == eCVType )
	{
		eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_Smart;
	}

	return eCVTypeFamily;
}

CDB_CircuitScheme *CSSheetPanelCirc1::GetSelectedSch()
{
	if( m_iSelectedScheme < 0 )
	{
		return NULL;
	}

	return m_arSchemes.GetAt( m_iSelectedScheme ).pSch;
}

bool CSSheetPanelCirc1::IsAnchorPtExistInSelectedSCH( CAnchorPt::eFunc func )
{
	if( m_iSelectedScheme < 0 )
	{
		return false;
	}
	
	CArray<CAnchorPt::eFunc> *paFunc = m_arSchemes.GetAt( m_iSelectedScheme ).paFunc;
	
	if( NULL == paFunc )
	{
		return false;
	}
	
	for( int i = 0; i < paFunc->GetCount(); i++ )
	{
		if( paFunc->GetAt( i ) == func )
		{
			return true;
		}
	}

	return false;
}

/*
CArray<CAnchorPt::eFunc> * CSSheetPanelCirc1::GetAnchorPtsForSelectedScheme()
{
	return ( m_arSchemes.GetAt( m_iSelectedScheme ).paFunc );
}
*/

BEGIN_MESSAGE_MAP( CSSheetPanelCirc1, CSSheet )
	ON_MESSAGE( WM_USER_MESSAGEDATA, MM_OnMessageReceive )
END_MESSAGE_MAP()

LRESULT CSSheetPanelCirc1::MM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
{
	CMessageManager::CMessage* pclMessage = CMessageManager::MM_ReadMessage( wParam );

	if( NULL == pclMessage )
	{
		return 0;
	}
	
	if( CMessageManager::ClassID::CID_CSSheet != (CMessageManager::ClassID)pclMessage->GetClassID() )
	{
		delete pclMessage;
		return 0;
	}

	switch( (CMessageManager::SSheetMessageType)pclMessage->GetMessageType() )
	{
		case CMessageManager::SSheetMessageType::SSheetMST_MouseEvent:
			{
				CMessageManager::CSSheetMEMsg* pclSShetMEMsg = dynamic_cast<CMessageManager::CSSheetMEMsg*>( pclMessage );

				if( NULL == pclSShetMEMsg )
				{
					delete pclMessage;
					return 0;
				}

				SSheetMEParams* prParams = pclSShetMEMsg->GetParams();

				if( NULL == prParams->m_hSheet || prParams->m_hSheet != GetSafeHwnd() )
				{
					delete pclMessage;
					return 0;
				}

				switch( prParams->m_eMouseEventType )
				{
					case CMessageManager::SSheetMET::MET_LButtonDown:
						_OnLButtonDown( prParams->m_lColumn, prParams->m_lRow );
						break;

					case CMessageManager::SSheetMET::MET_LButtonDblClk:
						_OnLButtonDblClk( prParams->m_lColumn, prParams->m_lRow );
						break;
				}
			}
			break;
	}
	delete pclMessage;
	return 0;
}

void CSSheetPanelCirc1::CleanSchArray()
{
	for( int i = 0; i < m_arSchemes.GetCount(); i++ )
	{

		if( m_arSchemes.GetAt( i ).paFunc )
		{
			delete( m_arSchemes.GetAt( i ).paFunc );
		}
	}

	m_arSchemes.RemoveAll();
}

void CSSheetPanelCirc1::_OnLButtonDown( long lLButtonDownCol, long lLButtonDownRow )
{
	if( lLButtonDownCol <= 0 || lLButtonDownRow <= 0 )
		return;

	// Selection already done
	if( lLButtonDownCol == m_lSelectedCol && lLButtonDownRow == m_lSelectedRow )
		return;

	m_iSelectedScheme = (int)GetCellParam( lLButtonDownCol, lLButtonDownRow );
	
	SetBackColor( lLButtonDownCol - 1, lLButtonDownRow, _SELECTEDBLUE );
	SetBackColor( lLButtonDownCol + 1, lLButtonDownRow, _SELECTEDBLUE );
	SetBackColor( lLButtonDownCol - 1, lLButtonDownRow - 1, _SELECTEDBLUE );
	SetBackColor( lLButtonDownCol - 1, lLButtonDownRow + 1, _SELECTEDBLUE );
	
	// HighLight Title
	SetBackColor( lLButtonDownCol, lLButtonDownRow + 3, _SELECTEDBLUE );
	SetForeColor( lLButtonDownCol, lLButtonDownRow + 3, _WHITE );
	
	// Unselect previous selection if exist.
	if( m_lSelectedCol && m_lSelectedRow )
	{
		SetBackColor( m_lSelectedCol - 1, m_lSelectedRow, _WHITE );
		SetBackColor( m_lSelectedCol + 1, m_lSelectedRow, _WHITE );
		SetBackColor( m_lSelectedCol - 1, m_lSelectedRow - 1, _WHITE );
		SetBackColor( m_lSelectedCol - 1, m_lSelectedRow + 1, _WHITE );

		SetBackColor( m_lSelectedCol, m_lSelectedRow + 3, _WHITE );
		SetForeColor( m_lSelectedCol, m_lSelectedRow + 3, _BLACK );
	}
	m_lSelectedCol = lLButtonDownCol;
	m_lSelectedRow = lLButtonDownRow;
	( (CPanelCirc1 *)this->GetParent() )->EnableButtonNext();
}

void CSSheetPanelCirc1::_OnLButtonDblClk( long lLButtonDlbClkCol, long lLButtonDlbClkRow )
{
	_OnLButtonDown( lLButtonDlbClkCol, lLButtonDlbClkRow );
	
	// TODO call click on Next button
	( (CPanelCirc1 *)this->GetParent() )->DisableButtons( CWizardManager::WizButNext );
	( (CPanelCirc1 *)this->GetParent() )->GoWizNext();
	( (CPanelCirc1 *)this->GetParent() )->DoNext();
}

void CSSheetPanelCirc1::_InsertCircuitScheme( CRank *pclRank, int iCircSchemeCategoryOrder, CDB_CircuitScheme *pclCircuitScheme )
{
	if( 0 == pclRank->GetCount() )
	{
		pclRank->Add( L"", iCircSchemeCategoryOrder * 100, (LPARAM)pclCircuitScheme );
	}
	else
	{
		struct _Data
		{
			double m_dKey;
			CString strID;
		};
		std::map<int, _Data> mapPos;

		CString str;
		LPARAM itemdata;
		int iPos = 0;

		for( BOOL bContinue = pclRank->GetFirst( str, itemdata ); TRUE == bContinue; bContinue = pclRank->GetNext( str, itemdata ) )
		{
			CDB_CircuitScheme *pclLoopCircuitScheme = (CDB_CircuitScheme *)itemdata;
			
			CString str( _T("") );
			double dKey = 0.0;
			LPARAM lpItemData = itemdata;

			if( FALSE == pclRank->GetaCopy( str, dKey, lpItemData ) )
			{
				ASSERT_RETURN;
			}

			_Data ItemData;
			ItemData.m_dKey = dKey;
			ItemData.strID = pclLoopCircuitScheme->GetIDPtr().ID;

			mapPos.insert( std::pair<int, _Data>( iPos, ItemData ) );
			iPos++;
		}

		int iStartCateg = INT_MIN;
		int iEndCateg = INT_MAX;
		int iInsertPos = -1;
		CString strCircuitShemeIDToInsert = pclCircuitScheme->GetIDPtr().ID;

		for( auto &iter : mapPos )
		{
			int iCurrentCateg = (int)(floor)( iter.second.m_dKey / 100.0 );

			if( iCircSchemeCategoryOrder == iCurrentCateg )
			{
				if( INT_MIN == iStartCateg )
				{
					iStartCateg = iter.first;
				}
				else
				{
					iEndCateg = iter.first;
				}

				if( -1 == iInsertPos )
				{
					if( strCircuitShemeIDToInsert.Compare( iter.second.strID ) < 0 )
					{
						iInsertPos = iter.first;
					}
				}
			}
		}

		if( INT_MIN == iStartCateg )
		{
			// Category not yet exist.
			pclRank->Add( L"", iCircSchemeCategoryOrder * 100, (LPARAM)pclCircuitScheme );
		}
		else if( -1 == iInsertPos )
		{
			// In this case, the circuit scheme must be added at the end of the category.
			int iLastPosForCateg = ( INT_MAX != iEndCateg ) ? iEndCateg : iStartCateg;
			double dKey = mapPos[iLastPosForCateg].m_dKey + 1.0;
			pclRank->Add( L"", dKey, (LPARAM)pclCircuitScheme );
		}
		else
		{
			// In this case, the circuit scheme must be inserted before the insert position.
			// We need for that to increase node that are following.
			int iLoopStart = ( iEndCateg != INT_MAX ) ? iEndCateg : iStartCateg;

			for( int iLoopItem = iLoopStart; iLoopItem >= iLoopStart; iLoopItem-- )
			{
				CString str( _T("") );
				double dKey = mapPos[iLoopItem].m_dKey;
				LPARAM lpItemData = NULL;

				if( FALSE == pclRank->GetaCopy( str, dKey, lpItemData ) )
				{
					ASSERT_RETURN;
				}

				pclRank->Delete( _T(""), lpItemData );

				mapPos[iLoopItem].m_dKey += 1.0;

				pclRank->Add( L"", mapPos[iLoopItem].m_dKey, (LPARAM)lpItemData );
			}

			// We can insert the object.
			pclRank->Add( L"", mapPos[iInsertPos].m_dKey, (LPARAM)pclCircuitScheme );
		}
	}
}