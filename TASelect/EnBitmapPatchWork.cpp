#include "StdAfx.h"

#include "TASelect.h"
#include "HydroMod.h"
#include "EnBitmap.h"
#include "EnBitmapPatchWork.h"

#define _USE_MATH_DEFINES
#include "math.h"
#include <map>

const COLORREF WHITE = RGB( 255, 255, 255 );
const COLORREF BLACK = RGB( 0, 0, 0 );

void CEnBitmapPatchWork::AddAnchor( CAnchorPt *pAnchoringPoint, int iAnchor )
{
	if( NULL == pAnchoringPoint )
	{
		return;
	}
	
	for( int i = 0; i < iAnchor; i++)
	{
		m_ArrayAnchor.Add( &pAnchoringPoint[i] );
	}
}

CEnBitmapPatchWork::CEnBitmapPatchWork(CEnBitmapCompMngr *pComponentManager)
{
	Reset();
	SetComponentManager( pComponentManager);
}

CEnBitmapPatchWork::~CEnBitmapPatchWork(void)
{
	Reset();
}

void CEnBitmapPatchWork::Reset()
{
	m_strName = _T("");			// Scheme's name
	m_ulBackground = 0;			// Background ID
	CleanArrayAnchor();		    // list of anchor point
	m_pComponentManager = NULL; // ptr to a manager of all components
	SetBackgroundColor( WHITE);
	POSITION pos = m_GroupMap.GetStartPosition();
	int iGroup;
	CPtrArray *pArrayPt = NULL;
	while( NULL != pos )
	{
		m_GroupMap.GetNextAssoc( pos, iGroup, pArrayPt );
		CleanPtList(pArrayPt);
		delete(pArrayPt);
		m_GroupMap.RemoveKey(iGroup);
	}
	m_iMvLoc = 0;
	m_iDpCLoc = 0;
}

void CEnBitmapPatchWork::SetComponentManager( const CEnBitmapCompMngr *pComponentManager )
{
	m_pComponentManager = (CEnBitmapCompMngr *)pComponentManager;
}

////////////////////////////////////////////////////////////////
/// Define the Background color of the picture
/// use with the DoubleBuffer
////////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::SetBackgroundColor( COLORREF color)
{
	m_BackgroundColor = color;
}

////////////////////////////////////////////////////////////////
/// Get Double Buffer
////////////////////////////////////////////////////////////////
CDC* CEnBitmapPatchWork::GetDoubleBufferDC( CDC* pDC)
{
	// Create compatible DC
	CDC *pMemDC = new CDC();

	if( NULL == pMemDC)
	{
		return 0;
	}

	pMemDC->CreateCompatibleDC( pDC);

	CWnd *pWnd = pDC->GetWindow();

	if( NULL == pWnd )
	{
		return 0;
	}

	CRect rc;
	pWnd->GetClientRect( &rc );

	CBitmap BigBmp;
	BigBmp.CreateCompatibleBitmap( pDC, rc.Width(), rc.Height() );
	pMemDC->SelectObject( BigBmp );

	// Draw the Background Color
	CBrush brush( m_BackgroundColor );
	pMemDC->FillRect( &rc, &brush );

	return pMemDC;
}

void CEnBitmapPatchWork::CopyDC( CDC* pDCSrc, CDC* pDCDst, pair<int,int> *ptOrigin )
{
	if( NULL == pDCSrc || NULL == pDCDst )
	{
		return;
	}

	CWnd *pWnd = pDCDst->GetWindow();

	if( NULL == pWnd )
	{
		return;
	}

	pair<int,int> pt;

	if( NULL != ptOrigin)
	{
		pt = *ptOrigin;
	}

	CRect rc;
	pWnd->GetClientRect( &rc );

	pDCDst->BitBlt( pt.first, pt.second, rc.Width(), rc.Height(), pDCSrc, 0, 0, SRCCOPY ); 
}

//////////////////////////////////////////////////////////////
/// Append Top and Bottom Images (Terminal unit, distribution pipes
/// This operation should be done AFTER the DrawPatchWork
/// Anchoring points are not moved by this operation
//////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::Append( CEnBitmap *pBmp, bool bTop )
{
	CRect SizeRect = GetSizeImage();
	CRect BmpRect = pBmp->GetSizeImage();
	CSize NewSize = CSize(SizeRect.Width(), SizeRect.Height() + BmpRect.Height());
	// Offset
	CSize offset, BmpPos;
	if (bTop)
	{
		offset = CSize(0,BmpRect.Height());
		BmpPos = CSize(0,0);
	}
	else
	{
		offset = CSize(0,0);
		BmpPos = CSize(0,SizeRect.Height());
	}
	// Enlarge base bitmap and translate the Body 
	ShiftImage(NewSize, offset);
	// Paste component
	RGBX TransparentCmptColor(pBmp->GetPixel(0,0));
	PasteCEnBitmap(BmpPos.cx,BmpPos.cy,pBmp,&TransparentCmptColor);
}

CDB_CircuitScheme *CEnBitmapPatchWork::FindSSelDpCBCVHydronicScheme( eDpStab eDpStabilized, ShutoffValveLoc eShutoffValveLocation )
{
	CDB_CircuitScheme *pCircuitScheme = NULL;

	// Determine the circuit scheme.
	CTable *pCircTab = (CTable *)( TASApp.GetpTADB()->Get( _T("DPCBCVSCHEME_TAB") ).MP );
	ASSERT( NULL != pCircTab );
	
	if( NULL == pCircTab )
	{
		return NULL;
	}
	
	// Do a loop on all scheme.
	for( IDPTR CircIDPtr = pCircTab->GetFirst(); _T('\0') != *CircIDPtr.ID && NULL == pCircuitScheme; CircIDPtr = pCircTab->GetNext() )
	{
		// Load variables.
		CDB_CircuitScheme *pSch = (CDB_CircuitScheme*)( CircIDPtr.MP );
		
		if( CDB_CircuitScheme::eBALTYPE::DPC != pSch->GetBalType() )
		{
			continue;
		}

		if( eDpStabilized != pSch->GetDpStab() )
		{
			continue;
		}

		if( eShutoffValveLocation != pSch->GetShutoffValveLoc() )
		{
			continue;
		}

		pCircuitScheme = pSch;
	}

	return pCircuitScheme;
}

void CEnBitmapPatchWork::GetSSelDpCBCVHydronicScheme( CDB_CircuitScheme *pCircuitScheme, IDPTR DpCBCVIDPtr, IDPTR SVIDPtr )
{
	// Verify the scheme is defined.
	ASSERT( NULL != pCircuitScheme );

	if( NULL == pCircuitScheme )
	{
		return;
	}

	PrepareDefaultComponents( pCircuitScheme, true );

	// Create a manager to store all components.
	CEnBitmapCompMngr CompoMng;

	// Create all needed components.
	CArray<CEnBitmapComponentFile *>arCompoFile;

	for( int i = 0; i < pCircuitScheme->GetAnchorPtListSize(); i++ )
	{
		// Verify if component should be added.
		bool bFound = false;

		for( int j = 0; j < pCircuitScheme->GetAnchorPtListSize() && false == bFound; j++ )
		{
			bFound = ( pCircuitScheme->GetAnchorPtFunc( i ) == pCircuitScheme->GetAnchorPtFunc( j ) );
		}

		if( false == bFound )
		{
			continue;
		}

		// Retrieve selected TA Product.
		IDPTR IDPtr = _NULL_IDPTR;
		CString strComponent = m_arstrDefaultComponents.GetAt( pCircuitScheme->GetAnchorPtFunc( i ) );

		switch( pCircuitScheme->GetAnchorPtFunc( i ) )
		{
			case CAnchorPt::ShutoffValve:

				if( NULL != SVIDPtr.MP )
				{
					IDPtr = SVIDPtr;
				}
				else
				{
					// Container doesn't exist.
					strComponent = L"TA_SHV";
				}

			break;

			case CAnchorPt::DPCBCV:
				
				if( NULL != DpCBCVIDPtr.MP )
				{
					IDPtr = DpCBCVIDPtr;
				}
				else
				{
					// Container doesn't exist
					strComponent = L"TA_DPCBCV_M";
				}

				break;
		}

		// Overwrite component with the valve image.
		if( _T('\0') != *IDPtr.ID )
		{
			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}

			strComponent = pTAP->GetProdPicID();
		}

		if( true == strComponent.IsEmpty() )
		{
			continue;
		}

		CEnBitmapComponentFile *pCompo = new CEnBitmapComponentFile;
		pCompo->FillExistingComponentFile( TASApp.GetpProdPic( (LPCTSTR)strComponent ), pCircuitScheme->GetAnchorPtFunc( i ) );
		CompoMng.AddComponent( pCompo );
		arCompoFile.Add( pCompo );
	}

	// Link all components with the background.
	SetComponentManager( &CompoMng );

	PrepareHydronicScheme( pCircuitScheme );

	// Clean component array.
	for( int i = 0; i < arCompoFile.GetCount(); i++ )
	{
		delete arCompoFile.GetAt( i );
	}
}

////////////////////////////////////////////////////////////////
/// Draw all the scheme
////////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::Add_TU_DistrPipes( CString TU, COLORREF colTU, CString Pipes, COLORREF colBot, int iOffset )
{
	CEnBitmap EnBmp;

	if( false == TU.IsEmpty() )
	{
		// Add terminal unit.
		CEnBitmap TermUnit;
		TermUnit.LoadImage( TASApp.GetDynCircSch( TU )->GetBackgroundResourceID(), _T("GIF"), TASApp.GetHMProdPics() );
		
		if( iOffset > 0 )
		{
			CRect Bmprect = TermUnit.GetSizeImage();
			TermUnit.ShiftImage( CSize( Bmprect.Width() + 2 * iOffset, Bmprect.Height() ), CSize( iOffset, 0 ) );
		}

		Append( &TermUnit, true );
		ReplaceColor( (COLORREF)_TAH_COLORTOREPLACE_1, (COLORREF)colTU );
	}
	
	if( false == Pipes.IsEmpty() )
	{
		// Add distribution pipe.
		CEnBitmap DistibPipes;
		DistibPipes.LoadImage( TASApp.GetDynCircSch( Pipes )->GetBackgroundResourceID(), _T("GIF"), TASApp.GetHMProdPics() );
		
		if( iOffset > 0 )
		{
			CRect Bmprect = DistibPipes.GetSizeImage();
			DistibPipes.ShiftImage( CSize( Bmprect.Width() + 2 * iOffset, Bmprect.Height() ), CSize( iOffset, 0 ) );
		}
		
		Append( &DistibPipes, false );
		ReplaceColor( (COLORREF)_TAH_COLORTOREPLACE_1, (COLORREF)colBot );
	}
}

CDynCircSch *CEnBitmapPatchWork::LoadBackGround( CDB_CircuitScheme *pCircuitScheme )
{
	// Load the background bitmap.
	CDynCircSch *pDynamicCircuitScheme = TASApp.GetDynCircSch( pCircuitScheme->GetDynCircuitID().c_str() );
	LoadImage( pDynamicCircuitScheme->GetBackgroundResourceID(), _T("GIF"), TASApp.GetHMProdPics() );

	BITMAP rBitmap;
	GetBitmap( &rBitmap );

	COLORREF crBck = GetPixel( 0, 0);
	SetBackgroundColor( crBck );
	return pDynamicCircuitScheme;
}

void CEnBitmapPatchWork::LoadAnchoringPoints( CDB_CircuitScheme *pCircuitScheme, CDynCircSch *pDynamicCircuitScheme, bool bMarkOptionalComponent )
{
	// Add anchoring points on the background.
	for( unsigned int i = 0; i < pDynamicCircuitScheme->GetNbrAnchoringPt(); i++ )
	{
		CAnchorPt *pAnchorPt = pDynamicCircuitScheme->GetAnchoringPt( i );
		
		if( false == pAnchorPt->IsVirtualPoint() )
		{
			COLORREF ptCol = GetPixel( pAnchorPt->GetPoint().first, pAnchorPt->GetPoint().second );
			pCircuitScheme->SetAnchorPtColor( pAnchorPt->GetFunction(), ptCol );
		}

		AddAnchor( pAnchorPt );
	}

	// Draw schema.
	DrawPatchwork( pCircuitScheme->GetMvLoc(), pCircuitScheme->GetDpCLoc() );

	for( unsigned int i = 0; i < pDynamicCircuitScheme->GetNbrAnchoringPt(); i++ )
	{
		CAnchorPt *pAnchorPt = pDynamicCircuitScheme->GetAnchoringPt( i );
		if ( true == bMarkOptionalComponent )
		{
			// Draw "#" for optional components
			if ( true == pCircuitScheme->IsAnchorPtOptional( pAnchorPt->GetFunction() ) )
			{
				// Add "*"
				CFont Font;	
				Font.CreateFont(-24,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Comic Sans MS"));
				CRect rect = GetSizeImage();
				CString str = _T("#");
				CDC dc; 
				dc.CreateCompatibleDC(NULL); 
				CSize cs = dc.GetTextExtent(str);
				CEnBitmap::DrawText(pAnchorPt->GetPoint().first+pAnchorPt->GetPairCompoSize().first/2, pAnchorPt->GetPoint().second-pAnchorPt->GetPairCompoSize().second/2,-90, str, RGB(0,0,0),&Font);

				pAnchorPt->GetPoint();
				Font.DeleteObject();
			}
		}
	}
}

void CEnBitmapPatchWork::PrepareHydronicScheme( CDB_CircuitScheme *pCircuitScheme, bool fMarkOptionalComponent /*=false*/ )
{
	CDynCircSch* pDynamicCircuitScheme = LoadBackGround( pCircuitScheme );
	LoadAnchoringPoints( pCircuitScheme, pDynamicCircuitScheme, fMarkOptionalComponent );
/*	// Load the background bitmap.
	CDynCircSch* pDynamicCircuitScheme = TASApp.GetDynCircSch( pCircuitScheme->GetDynCircuitID().c_str() );
	LoadImage( pDynamicCircuitScheme->GetBackgroundResourceID(), _T("GIF"), TASApp.GetHMProdPics() );
	
	COLORREF crBck = GetPixel( 0, 0);
	SetBackgroundColor( crBck );
	
	// Add anchoring points on the background.
	for( unsigned int i = 0; i < pDynamicCircuitScheme->GetNbrAnchoringPt(); i++ )
		AddAnchor( pDynamicCircuitScheme->GetAnchoringPt( i ) );

	// Draw schema.
	DrawPatchwork( pCircuitScheme->GetMvLoc(), pCircuitScheme->GetDpCLoc() );

	for( unsigned int i = 0; i < pDynamicCircuitScheme->GetNbrAnchoringPt(); i++ )
	{
		CAnchorPt *pAnchorPt = pDynamicCircuitScheme->GetAnchoringPt( i );
		if ( true == fMarkOptionalComponent )
		{
			// Draw "#" for optional components
			if ( true == pCircuitScheme->IsAnchorPtOptional( pAnchorPt->GetFunction() ) )
			{
				// Add "*"
				CFont Font;	
				Font.CreateFont(-24,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Comic Sans MS"));
				CRect rect = GetSizeImage();
				CString str = _T("#");
				CDC dc; 
				dc.CreateCompatibleDC(NULL); 
				CSize cs = dc.GetTextExtent(str);
				CEnBitmap::DrawText(pAnchorPt->GetPoint().first+pAnchorPt->GetPairCompoSize().first/2, pAnchorPt->GetPoint().second-pAnchorPt->GetPairCompoSize().second/2,-90, str, RGB(0,0,0),&Font);

				pAnchorPt->GetPoint();
				Font.DeleteObject();
			}
		}
		COLORREF ptCol = GetPixel( pAnchorPt->GetPoint().first, pAnchorPt->GetPoint().second );
		pCircuitScheme->SetAnchorPtColor(pAnchorPt->GetFunction(), ptCol);
	}
*/
}

void CEnBitmapPatchWork::PrepareDefaultComponents( CDB_CircuitScheme *pCircuitScheme, bool fCVModulating )
{
	CDB_CircSchemeCateg *pCircuitCategory  = static_cast<CDB_CircSchemeCateg *>( pCircuitScheme->GetSchemeCategIDPtr().MP );
	ASSERT( NULL != pCircuitCategory );

	if( NULL == pCircuitCategory )
	{
		return;
	}

	// Remark: to see the list of all symbols, go in 'dllmain.cpp' file in the 'HydronicPics' project.

	// List of default symbols.
	m_arstrDefaultComponents.SetSize( CAnchorPt::eFuncLast );
	m_arstrDefaultComponents.SetAt( CAnchorPt::eFuncNU, L"" );
	m_arstrDefaultComponents.SetAt( CAnchorPt::BV_P, L"TA_BV" );
	m_arstrDefaultComponents.SetAt( CAnchorPt::BV_Byp, L"TA_BV" );
	m_arstrDefaultComponents.SetAt( CAnchorPt::BV_S, L"TA_BV" );
	
	// Set the control valve ID in regards to the fact it is a 2 or 3 ways circuit.
	CString strCvID = ( true == pCircuitCategory->Is3W() ) ? L"TA_3WCV" : L"TA_2WCV";

	// Append '_M' if control valve is proportional or '_01' in other cases.
	strCvID += ( true == fCVModulating ) ? L"_M" : L"_01";
	
	// If we can preset the control valve...
	if( CDB_ControlProperties::Presettable == pCircuitScheme->GetCvFunc() )
	{
		strCvID += L"_P";
	}
	
	// If there is measuring points on the control valve...
	if( CDB_ControlProperties::PresetPT == pCircuitScheme->GetCvFunc() )
	{
		strCvID += L"_PPT";
	}

	m_arstrDefaultComponents.SetAt( CAnchorPt::ControlValve, (LPCTSTR)strCvID );

	// Same comments as for the control valve.
	CString PICV = L"TA_PICV";
	PICV += ( true == fCVModulating ) ? L"_M" : L"_01";

	if( CDB_ControlProperties::Presettable == pCircuitScheme->GetCvFunc() )
	{
		PICV += L"_P";
	}

	if( CDB_ControlProperties::PresetPT == pCircuitScheme->GetCvFunc() )
	{
		PICV += L"_PPT";
	}

	m_arstrDefaultComponents.SetAt( CAnchorPt::PICV, (LPCTSTR)PICV );

	// For DpCBCV we allow to show the control either in proportional mode or in On/Off mode.
	CString strDpCBCV = L"TA_DPCBCV";
	strDpCBCV += ( true == fCVModulating ) ? L"_M" : L"_01";

	m_arstrDefaultComponents.SetAt( CAnchorPt::DPCBCV, (LPCTSTR)strDpCBCV );

	m_arstrDefaultComponents.SetAt( CAnchorPt::DpC, L"TA_DPC" );
	m_arstrDefaultComponents.SetAt( CAnchorPt::PUMP, L"TA_VSP" );
	m_arstrDefaultComponents.SetAt( CAnchorPt::ShutoffValve, L"TA_SHV" );

	m_arstrDefaultComponents.SetAt( CAnchorPt::SmartControlValve, L"TA_SMART_MV" );
	m_arstrDefaultComponents.SetAt( CAnchorPt::TempSensor, L"TA_TEMP_SENSOR" );

	m_arstrDefaultComponents.SetAt( CAnchorPt::SmartDpC, L"TA_SMART_DP" );
	m_arstrDefaultComponents.SetAt( CAnchorPt::DpSensor, L"TA_LINK" );
}

void CEnBitmapPatchWork::GetHydronicScheme( CDS_HydroMod *pHM )
{
	ASSERT( NULL != pHM );
	
	if( NULL == pHM )
	{
		return;
	}
	
	CDB_CircuitScheme *pCircuitScheme = pHM->GetpSch();
	ASSERT( NULL != pCircuitScheme );

	if( NULL == pCircuitScheme )
	{
		return;
	}

	// List of default symbol.
	bool bCvModulating = true;

	if( NULL != pHM->GetpCV() )
	{
		bCvModulating = ( CDB_ControlProperties::CvCtrlType::eCvProportional == pHM->GetpCV()->GetCtrlType() ) ? true : false;
	}

	PrepareDefaultComponents( pCircuitScheme, bCvModulating );

	// Create a manager to store all components.
	CEnBitmapCompMngr CompoMng;
	
	// Create all needed components.
	CArray<CEnBitmapComponentFile *>arCompoFile;
	CDS_Hm2W *pHm2W = dynamic_cast<CDS_Hm2W *>( pHM );
	CDS_Hm2WInj *pHm2WInj = dynamic_cast<CDS_Hm2WInj *>( pHM );
	CDS_Hm3W *pHm3W = dynamic_cast<CDS_Hm3W *>( pHM );
	
	for( int i = 0; i < pCircuitScheme->GetAnchorPtListSize(); i++ )
	{
		// Verify if component is not yet already added.
		bool bFound = false;

		for( int j = 0; j < pCircuitScheme->GetAnchorPtListSize() && false == bFound; j++ )
		{
			bFound = ( pCircuitScheme->GetAnchorPtFunc( i ) == pCircuitScheme->GetAnchorPtFunc( j ) ) ? true : false;
		}
		
		if( false == bFound )
		{
			continue;
		}

		// Retrieve selected TA Product.
		IDPTR IDPtr = _NULL_IDPTR;
		CString strComponent = m_arstrDefaultComponents.GetAt( pCircuitScheme->GetAnchorPtFunc( i ) );

		switch( pCircuitScheme->GetAnchorPtFunc( i ) )
		{
			case CAnchorPt::BV_P:

				if( NULL != pHM->GetpBv() )
				{
					IDPtr = pHM->GetpBv()->GetIDPtr();
				}
				else
				{
					strComponent = L"";					// Container doesn't exist
				}

				break;

			case CAnchorPt::BV_Byp:
				
				if( NULL != pHm2WInj && eDpStab::DpStabOnBVBypass == pHm2WInj->GetpSch()->GetDpStab() )
				{
					// HYS-1930: auto-adapting variable flow decoupling circuit.
					if( NULL != pHm2WInj->GetpBypBv() )
					{
						IDPtr = pHm2WInj->GetpBypBv()->GetIDPtr();
					}
				}
				else if( NULL != pHm3W )
				{
					// Circuit should be 3w, BypValve must exist and is used.

					if( CDS_HydroMod::eUseProduct::Never ==  pHm3W->GetUseBypBv() )
					{
						strComponent = L"";
					}
					else
					{
						if( NULL != pHM->GetpBypBv() )
						{
							IDPtr = pHM->GetpBypBv()->GetIDPtr();
						}
						
						// Bypass valve doesn't exist and we are in Auto mode clear the TA symbol too.
						if( CDS_HydroMod::eUseProduct::Auto == pHm3W->GetUseBypBv() && !*IDPtr.ID )
						{
							strComponent = L"";
						}
					}	
				}
				else
				{
					strComponent = L"";
				}

				break;

			case CAnchorPt::BV_S:

				if( NULL != pHM->GetpSecBv() )
				{
					IDPtr = pHM->GetpSecBv()->GetIDPtr();
				}
				else
				{
					strComponent = L"";						// Container doesn't exist
				}

				break;

			case CAnchorPt::PICV:
			case CAnchorPt::ControlValve:

				if( NULL != pHM->GetpCV() )
				{
					IDPtr = pHM->GetpCV()->GetCvIDPtr();
				}
				else
				{
					strComponent = L"";						// Container doesn't exist
				}

				break;

			case CAnchorPt::DpC:

				if( NULL != pHM->GetpDpC() )
				{
					IDPtr = pHM->GetpDpC()->GetIDPtr();
				}
				else
				{
					strComponent = L"";						// Container doesn't exist
				}

				break;

			case CAnchorPt::DPCBCV:

				if( NULL != pHM->GetpCV() )
				{
					IDPtr = pHM->GetpCV()->GetCvIDPtr();
				}
				else
				{
					strComponent = L"";						// Container doesn't exist
				}

				break;

			case CAnchorPt::ShutoffValve:

				// Circuit should be 2w.
				if( NULL != pHm2W )
				{
					// No auto mode for shutoff valve.
					if( CDS_HydroMod::eUseProduct::Never == pHm2W->GetUseShutoffValve() ||
						CDS_HydroMod::eUseProduct::Auto == pHm2W->GetUseShutoffValve() )
					{
						strComponent = L"";
					}
					else
					{
						if( NULL != pHM->GetpShutoffValve( pHm2W->GetShutoffValveLocation() ) )
						{
							IDPtr = pHM->GetpShutoffValve( pHm2W->GetShutoffValveLocation() )->GetIDPtr();
						}
					}	
				}
				else
				{
					strComponent = L"";
				}

				break;

			case CAnchorPt::SmartControlValve:

				if( NULL != pHM->GetpSmartControlValve() )
				{
					IDPtr = pHM->GetpSmartControlValve()->GetIDPtr();
				}
				else
				{
					strComponent = L"";						// Container doesn't exist
				}

				break;

			case CAnchorPt::SmartDpC:

				if( NULL != pHM->GetpSmartDpC() )
				{
					IDPtr = pHM->GetpSmartDpC()->GetIDPtr();
				}
				else
				{
					strComponent = L"";						// Container doesn't exist
				}

				break;

			case CAnchorPt::eFuncNU:
			case CAnchorPt::TempSensor:
			case CAnchorPt::PUMP:
				break;

			case CAnchorPt::DpSensor:

				if( NULL != pHM->GetpSmartDpC() )
				{
					if( eBool3::eb3False == pHM->GetpSmartDpC()->IsSelectedAsaPackage() )
					{
						IDPtr = pHM->GetpSmartDpC()->GetDpSensorIDPtr();
					}
					else
					{
						IDPtr = pHM->GetpSmartDpC()->GetDpSensorIDPtrInDpSensorSet();
					}
				}
				else
				{
					strComponent = L"";						// Container doesn't exist
				}

				break;
			
			default:
				ASSERT( 0 );
		}
		
		// Overwrite component with the valve image.
		if( _T('\0') != *IDPtr.ID )
		{
			CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( IDPtr.MP );
			ASSERT( NULL != pclProduct );

			if( NULL == pclProduct )
			{
				continue;
			}

			CString str = pclProduct->GetProdPicID();

			if ( false == str.IsEmpty() )
			{
				// HYS-1433: Take only picture and not symbol!
				if( TASApp.GetpProdPic( (LPCTSTR)str )->GetProdPicType() == CProdPic::eProdPicType::Pic )
				{
					strComponent = str;
				}
			}
		}
		
		if( true == strComponent.IsEmpty() )
		{
			continue;
		}
		
		CEnBitmapComponentFile *pCompo = new CEnBitmapComponentFile;
		pCompo->FillExistingComponentFile( TASApp.GetpProdPic( (LPCTSTR)strComponent ), pCircuitScheme->GetAnchorPtFunc( i ) );
		CompoMng.AddComponent( pCompo );
		arCompoFile.Add( pCompo );
	}
	
	SetComponentManager( &CompoMng );
	CDynCircSch *pDynSch = LoadBackGround( pCircuitScheme );
	
	CString strTUSchemeID;
	CString strPipesSchemeID;

	COLORREF colBot = _TAH_ORANGE;
	COLORREF colTop = _TAH_ORANGE;

	// Terminal unit.
	if( 0 == IDcmp( _T("SCHCATEG_PDG"), pCircuitScheme->GetSchemeCategID() ) )
	{
		strTUSchemeID = _T("SCH_PIPESTOPPENDING");
	}
	else
	{
		if( true == pHM->IsaModule() )
		{
			strTUSchemeID = _T("SCH_PIPESTOPBOTARROWS");
			colTop = _TAH_BERYLLIUM_XLIGHT;

			if( false == pCircuitScheme->GetpSchCateg()->IsSecondarySideExist() )
			{
				colBot = _TAH_BERYLLIUM_XLIGHT;
			}
		}
		else
		{
			strTUSchemeID = _T("SCH_TERMINALUNIT");

			if( true == pCircuitScheme->GetpSchCateg()->IsSecondarySideExist() )
			{
				colTop = _TAH_GREEN_MED;
			}
		}
	}

	// Middle pipe for 3way injection.
	COLORREF colMid = _TAH_ORANGE;

	if( 0 == IDcmp( _T("SCHCATEG_3WINJ_MIX"), pCircuitScheme->GetSchemeCategID() ) )
	{
		// Special treatment for 3W mixing circuit
		colTop = _TAH_ORANGE;
	}
	else if( 0 == IDcmp( _T("SCHCATEG_3WINJ_DB"), pCircuitScheme->GetSchemeCategID() ) )
	{
		if( CvLocation::CvLocSecSide == pCircuitScheme->GetCvLoc() )
		{
			colMid = colTop;
		}
	}

	// Distribution pipes.

	if( 0 == pHM->GetLevel() )
	{
		strPipesSchemeID = _T("SCH_PIPESTOPBOTARROWS");
	}
	else
	{
		strPipesSchemeID = ( CDS_HydroMod::ReturnType::Direct == pHM->GetReturnType() ) ? _T("SCH_DISTRIBPIPES") : _T("SCH_DISTRIBPIPES_REV");
	}

	// Replace the lila color.
	ReplaceColor( _TAH_COLORTOREPLACE_2 , colMid );
	
	if( false == strTUSchemeID.IsEmpty() )
	{
		// Replace the violet color.
		ReplaceColor( _TAH_COLORTOREPLACE_1, colTop );
	}
	
	if( false == strPipesSchemeID.IsEmpty() )
	{
		// Replace the violet color.
		ReplaceColor( _TAH_COLORTOREPLACE_1, colBot );
	}
	
	// Link all components with the background.
	LoadAnchoringPoints(pCircuitScheme, pDynSch, false);

	Add_TU_DistrPipes( strTUSchemeID, colTop, strPipesSchemeID, colBot );
	
	// Clean component array.
	for( int i = 0; i < arCompoFile.GetCount(); i++ )
	{
		delete arCompoFile.GetAt( i );
	}
}

void CEnBitmapPatchWork::GetHydronicScheme( CString strCircuitSchemeID, std::map<CAnchorPt::eFunc, CString> *pmapProductList, bool bControlProportional, bool bTerminalUnit, int iLevel, CDS_HydroMod::ReturnType eReturnType )
{
	CDB_CircuitScheme *pCircuitScheme = dynamic_cast<CDB_CircuitScheme *>( TASApp.GetpTADB()->Get( strCircuitSchemeID ).MP );

	if( NULL == pCircuitScheme )
	{
		ASSERT_RETURN;
	}

	PrepareDefaultComponents( pCircuitScheme, bControlProportional );

	// Create a manager to store all components.
	CEnBitmapCompMngr CompoMng;

	// Create all needed components.
	CArray<CEnBitmapComponentFile *>arCompoFile;

	for( int i = 0; i < pCircuitScheme->GetAnchorPtListSize(); i++ )
	{
		// Verify if component is not yet already added.
		bool bFound = false;

		for( int j = 0; j < pCircuitScheme->GetAnchorPtListSize() && false == bFound; j++ )
		{
			bFound = ( pCircuitScheme->GetAnchorPtFunc( i ) == pCircuitScheme->GetAnchorPtFunc( j ) ) ? true : false;
		}

		if( false == bFound )
		{
			continue;
		}

		// Retrieve selected TA Product.
		CString strComponent = m_arstrDefaultComponents.GetAt( pCircuitScheme->GetAnchorPtFunc( i ) );

		if( pmapProductList->count( pCircuitScheme->GetAnchorPtFunc( i ) ) > 0 )
		{
			IDPTR IDPtr = TASApp.GetpTADB()->Get( pmapProductList->at( pCircuitScheme->GetAnchorPtFunc( i ) ) );

			if( _NULL_IDPTR == IDPtr )
			{
				ASSERT_CONTINUE;
			}

			// Overwrite component with the valve image.
			if( _T('\0') != *IDPtr.ID )
			{
				CDB_Product *pTAP = dynamic_cast<CDB_Product *>( IDPtr.MP );

				if( NULL == pTAP )
				{
					ASSERT_CONTINUE;
				}

				CString str = pTAP->GetProdPicID();

				if( false == str.IsEmpty() )
				{
					// HYS-1433: Take only picture and not symbol!
					CProdPic *pclProdPic = TASApp.GetpProdPic( (LPCTSTR)str );
				
					if( TASApp.GetpProdPic( (LPCTSTR)str )->GetProdPicType() == CProdPic::eProdPicType::Pic )
					{
						strComponent = str;
					}
				}
			}
		}

		if( true == strComponent.IsEmpty() )
		{
			continue;
		}

		CEnBitmapComponentFile *pCompo = new CEnBitmapComponentFile;
		pCompo->FillExistingComponentFile( TASApp.GetpProdPic( (LPCTSTR)strComponent ), pCircuitScheme->GetAnchorPtFunc( i ) );
		CompoMng.AddComponent( pCompo );
		arCompoFile.Add( pCompo );
	}

	SetComponentManager( &CompoMng );
	CDynCircSch *pDynSch = LoadBackGround( pCircuitScheme );

	CString strTUSchemeID;
	CString strPipesSchemeID;

	COLORREF colBot = _TAH_ORANGE;
	COLORREF colTop = _TAH_ORANGE;

	// Terminal unit.
	if( 0 == IDcmp( _T("SCHCATEG_PDG"), pCircuitScheme->GetSchemeCategID() ) )
	{
		strTUSchemeID = _T("SCH_PIPESTOPPENDING");
	}
	else
	{
		if( false == bTerminalUnit )
		{
			strTUSchemeID = _T("SCH_PIPESTOPBOTARROWS");
			colTop = _TAH_BERYLLIUM_XLIGHT;

			if (false == pCircuitScheme->GetpSchCateg()->IsSecondarySideExist() )
			{
				colBot = _TAH_BERYLLIUM_XLIGHT;
			}
		}
		else
		{
			strTUSchemeID = _T("SCH_TERMINALUNIT");

			if (true == pCircuitScheme->GetpSchCateg()->IsSecondarySideExist() )
			{
				colTop = _TAH_GREEN_MED;
			}
		}
	}

	// Middle pipe for 3way injection or auto-adapting variable flow decoupling circuit.
	COLORREF colMid = _TAH_ORANGE;

	if( 0 == IDcmp( _T("SCHCATEG_3WINJ_MIX"), pCircuitScheme->GetSchemeCategID() ) )
	{
		// Special treatment for 3W mixing circuit
		colTop = _TAH_ORANGE;
	}
	else if( 0 == IDcmp( _T("SCHCATEG_3WINJ_DB"), pCircuitScheme->GetSchemeCategID() ) )
	{
		if( CvLocation::CvLocSecSide == pCircuitScheme->GetCvLoc() )
		{
			colMid = colTop;
		}
	}
	else if( 0 == IDcmp( _T("SCHCATEG_2W_AUTOADAPT"), pCircuitScheme->GetSchemeCategID() ) )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		colMid = _TAH_ORANGE;
	}

	// Distribution pipes.

	if( 0 == iLevel )
	{
		strPipesSchemeID = _T("SCH_PIPESTOPBOTARROWS");
	}
	else
	{
		strPipesSchemeID = ( CDS_HydroMod::ReturnType::Direct == eReturnType ) ? _T("SCH_DISTRIBPIPES") : _T("SCH_DISTRIBPIPES_REV");
	}

	// Replace the lila color.
	ReplaceColor(_TAH_COLORTOREPLACE_2 , colMid );

	if( false == strTUSchemeID.IsEmpty() )
	{
		// Replace the violet color.
		ReplaceColor( _TAH_COLORTOREPLACE_1, colTop );
	}

	if( false == strPipesSchemeID.IsEmpty() )
	{
		// Replace the violet color.
		ReplaceColor( _TAH_COLORTOREPLACE_1, colBot );
	}

	// Link all components with the background.
	LoadAnchoringPoints(pCircuitScheme, pDynSch, false);

	Add_TU_DistrPipes( strTUSchemeID, colTop, strPipesSchemeID, colBot );

	// Clean component array.
	for( int i = 0; i < arCompoFile.GetCount(); i++ )
	{
		delete arCompoFile.GetAt( i );
	}
}

void CEnBitmapPatchWork::GetHydronicScheme( CDB_CircuitScheme *pCircuitScheme, int iUseTermUnit, bool fCVModulating, CArray<CAnchorPt::eFunc> *paAnchorPt, bool fDistributionPipe, CDS_HydroMod::ReturnType eReturnType )
{
	if( NULL == pCircuitScheme )
	{
		ASSERT_RETURN;
	}

	// HYS-1579: We have an enum class for Terminal unit values.
	ePatchWorkTerminalUnit eUseTermUnit = (ePatchWorkTerminalUnit)iUseTermUnit;

	// If we are working with symbol keep previous state and force TASymbol.
	CProdPic::eProdPicType ProdPicType = TASApp.GetProdPicType();
	
	// Switch to TA symbol if pictures are pre-selected.
	if(CProdPic::Pic == ProdPicType )
	{
		TASApp.SetProdPicType( CProdPic::PicTASymb );
	}

	// List of default symbol.
	PrepareDefaultComponents( pCircuitScheme, fCVModulating );

	// Create a manager to store all components.
	CEnBitmapCompMngr CompoMng;
	
	// Create all needed components.
	CArray<CEnBitmapComponentFile *>arCompoFile;

	for( int i = 0; i < pCircuitScheme->GetAnchorPtListSize(); i++ )
	{
		if( pCircuitScheme->GetAnchorPtFunc( i ) >= m_arstrDefaultComponents.GetCount() )
		{
			ASSERT_CONTINUE;	// Missing entry into m_DefaultComponents? Skip it.
		}
		
		if( true == m_arstrDefaultComponents.GetAt( pCircuitScheme->GetAnchorPtFunc( i ) ).IsEmpty() )
		{
			continue;	// Take care of eFuncNU
		}

		// Verify if component should be added.
		bool bFound = false;

		// If an array of anchoring point functions is passed as argument, we verify if at least one anchoring point in this 
		// array has the same function of the anchoring point of the current circuit scheme.
		if( NULL != paAnchorPt )
		{
			for( int j = 0; j < paAnchorPt->GetCount() && false == bFound; j++ )
			{
				bFound = ( paAnchorPt->GetAt( j ) == pCircuitScheme->GetAnchorPtFunc( i ) ) ? true : false;
			}

			if( false == bFound )
			{
				continue;
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ??? What is this code ???
		bFound = false;

		for( int j = 0; j < pCircuitScheme->GetAnchorPtListSize() && false == bFound; j++ )
		{
			bFound = ( pCircuitScheme->GetAnchorPtFunc( i ) == pCircuitScheme->GetAnchorPtFunc( j ) ) ? true : false;
		}

		if( false == bFound )
		{
			continue;
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		CEnBitmapComponentFile *pCompo = new CEnBitmapComponentFile;
		pCompo->FillExistingComponentFile( TASApp.GetpProdPic( m_arstrDefaultComponents.GetAt( pCircuitScheme->GetAnchorPtFunc( i ) ) ), pCircuitScheme->GetAnchorPtFunc( i ) );
		CompoMng.AddComponent( pCompo );
		arCompoFile.Add( pCompo );
	}
	
	// Link all components with the background .
	SetComponentManager( &CompoMng );
	CDynCircSch *pDynSch = LoadBackGround( pCircuitScheme );
	
	CString strTUSchemeID;
	CString strPipesSchemeID;

	COLORREF colBot = _TAH_ORANGE;
	COLORREF colTop = _TAH_ORANGE;

	// Terminal unit.
	if( 0 == IDcmp( _T("SCHCATEG_PDG"), pCircuitScheme->GetSchemeCategID() ) )
	{
		strTUSchemeID = _T("SCH_PIPESTOPPENDING");
	}
	else
	{
		// HYS-1579: We have an enum class for Terminal unit values
		if ( ePatchWorkTerminalUnit::eTermUnit != eUseTermUnit )
		{
			strTUSchemeID = _T("SCH_PIPESTOPBOTARROWS");
			colTop = _TAH_BERYLLIUM_XLIGHT;

			if (false == pCircuitScheme->GetpSchCateg()->IsSecondarySideExist())
			{
				colBot = _TAH_BERYLLIUM_XLIGHT;
			}
		}
		else
		{
			strTUSchemeID = _T("SCH_TERMINALUNIT");

			if( true == pCircuitScheme->GetpSchCateg()->IsSecondarySideExist() )
			{
				colTop = _TAH_GREEN_MED;
			}
		}
	}

	// Middle pipe for 3way injection or auto adaptive variable flow with decoupling bypass.
	COLORREF colMid = _TAH_ORANGE;

	if( 0 == IDcmp( _T("SCHCATEG_3WINJ_MIX"), pCircuitScheme->GetSchemeCategID() ) )
	{
		// Special treatment for 3W mixing circuit
		colTop = _TAH_ORANGE;
	}
	else if( 0 == IDcmp( _T("SCHCATEG_3WINJ_DB"), pCircuitScheme->GetSchemeCategID() ) )
	{
		if( CvLocation::CvLocSecSide == pCircuitScheme->GetCvLoc() )
		{
			colMid = colTop;
		}
	}
	else if( 0 == IDcmp( _T("SCHCATEG_2W_AUTOADAPT"), pCircuitScheme->GetSchemeCategID() ) )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		colMid = _TAH_ORANGE;
	}

	// Distribution pipes.
	if( false == fDistributionPipe )
	{
		strPipesSchemeID = _T("SCH_PIPESTOPBOTARROWS");
	}
	else
	{
		strPipesSchemeID = ( CDS_HydroMod::ReturnType::Direct == eReturnType ) ? _T("SCH_DISTRIBPIPES") : _T("SCH_DISTRIBPIPES_REV");
	}
	
	ReplaceColor( _TAH_COLORTOREPLACE_2 , colMid );

	if (false == strTUSchemeID.IsEmpty())
	{
		ReplaceColor( _TAH_COLORTOREPLACE_1, colTop );
	}
	if (false == strPipesSchemeID.IsEmpty())
	{
		ReplaceColor( _TAH_COLORTOREPLACE_1, colBot );
	}

	LoadAnchoringPoints(pCircuitScheme, pDynSch, true);

	Add_TU_DistrPipes( strTUSchemeID, colTop, strPipesSchemeID, colBot );
	
	// Clean component array.
	for( int i = 0; i < arCompoFile.GetCount(); i++ )
		delete arCompoFile.GetAt( i );
	
	// Restore previous ProdPic.
	TASApp.SetProdPicType( ProdPicType );
}

CDB_CircuitScheme *CEnBitmapPatchWork::FindSSelDpCHydronicScheme( eMvLoc MvLoc, eDpCLoc DpCLoc, eDpStab DpStab, bool bForSet )
{
	CDB_CircuitScheme *pCircSch = NULL;
	
	// Determine the circuit scheme.
	CTable *pCircTab = (CTable *)( TASApp.GetpTADB()->Get( _T("DPCSCHEME_TAB") ).MP );

	if( NULL == pCircTab )
	{
		ASSERTA_RETURN( NULL );
	}

	// Do a loop on all schemes.
	CDB_CircuitScheme *pAlternativeScheme = NULL;

	for( IDPTR CircIDPtr = pCircTab->GetFirst(); _T('\0') != *CircIDPtr.ID; CircIDPtr = pCircTab->GetNext() )
	{
		// Load variables.
		CDB_CircuitScheme *pSch = (CDB_CircuitScheme *)( CircIDPtr.MP );

		if( CDB_CircuitScheme::eBALTYPE::DPC != pSch->GetBalType() )
		{
			continue;
		}

		if( DpStab != pSch->GetDpStab() )
		{
			continue; // DpStab 0 branch, 1 control valve
		}

		if( MvLoc != pSch->GetMvLoc() )
		{
			continue;
		}

		if( DpCLoc != pSch->GetDpCLoc() )
		{
			continue;
		}
		
		if( eDpStab::DpStabOnBranch == DpStab )
		{
			// Stabilize on branch.
			// Do not continue with wrong scheme.
			if( CDB_ControlProperties::eCVFUNC::NoControl != pSch->GetCvFunc() )
			{
				continue;
			}

			pCircSch = pSch;
		}
		else if (eDpStab::DpStabOnCV == DpStab)
		{
			// Stabilize on control valve.
			// Do not continue with wrong scheme.
			if( CDB_ControlProperties::eCVFUNC::NoControl == pSch->GetCvFunc() )
			{
				continue;
			}

			pCircSch = pSch;
		}

		// HYS-1828: It can happen that we don't have schematic for selection by set. In this case, we keep the best schematic without set.
		if( bForSet != pSch->IsForASet() )
		{
			pAlternativeScheme = pCircSch;
			continue;
		}
		
		return pCircSch;
	}
	
	return pAlternativeScheme;
}

CDB_CircuitScheme *CEnBitmapPatchWork::FindSSelDpCHydronicScheme(int index, CDS_SSelDpC **pSelDpC )
{
	CTable *pTab = (CTable *)( TASApp.GetpTADS()->Get( _T("DPCONTR_TAB") ).MP );
	
	if( NULL == pTab )
	{
		ASSERTA_RETURN( NULL );
	}

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDS_SSelDpC ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Verify ID of DpC.
		*pSelDpC = (CDS_SSelDpC *)IDPtr.MP;
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( (*pSelDpC)->GetDpCIDPtr().MP );
		
		if( NULL == pDpC )
		{
			continue;
		}
		
		// Verify it's the right SelDpC.
		if( (*pSelDpC)->GetSchemeIndex() != index )
		{
			continue;
		}
		
		return FindSSelDpCHydronicScheme( (*pSelDpC)->GetMvLoc(), pDpC->GetDpCLoc(), (*pSelDpC)->GetDpStab(), (*pSelDpC)->IsSelectedAsAPackage() );
	}
	return NULL;
}

void CEnBitmapPatchWork::AddSSelDpCArrows(CDB_CircuitScheme *pCircSch)
{
	// Draw Arrows
	CRect rect = GetSizeImage();
	CString strHMin = TASApp.LoadLocalizedString(IDS_HMIN);
	CString strDPL = TASApp.LoadLocalizedString(IDS_DPL);
	CString strDPB = L"dPB";

	CFont Font;	
	Font.CreateFont(-24,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Comic Sans MS"));
	if (pCircSch->GetDpStab() == eDpStab::DpStabOnBranch)
	{
		CPoint start,stop;
		// Fix y position
		const int TopY = rect.Height()-50;
		const int BottomY = 80;

		start.y = stop.y = TopY;
		start.x = stop.x = -1;
		// for X position retrieve DpC and Bv anchoring points 
		const CPtrArray *ArrayAnchor = GetArrayAnchor();

		CAnchorPt *pAnchor;
		for(int i = 0; i < ArrayAnchor->GetSize(); i++)
		{
			pAnchor = (CAnchorPt*)ArrayAnchor->GetAt(i);
			if (!pAnchor) continue;
			// Skip anchoring points as Spline, they are used to draw capillary 
			if (pAnchor->GetConnectType() >= CAnchorPt::Spline) continue;
			if (pAnchor->GetFunction() == CAnchorPt::eFunc::DpC)
				start.x = pAnchor->GetPoint().first;
			if (pAnchor->GetFunction() == CAnchorPt::eFunc::BV_P)
				stop.x = pAnchor->GetPoint().first;
			if (start.x>-1 && stop.x>-1) break;
		}
		// Draw Hmin
		DrawArrow(start, stop);
		// calculation of the text extent.
		CDC dc; 
		dc.CreateCompatibleDC(NULL); 
		CSize cs = dc.GetTextExtent(strHMin);
		CEnBitmap::DrawText(rect.Width()/2-cs.cy/2, TopY-5,-90, strHMin, RGB(0,0,0),&Font);

		start.y = stop.y = BottomY;
		if (pCircSch->GetMvLoc() == eMvLoc::MvLocSecondary)
		{
			int y = rect.Height()/2;
			start.y = stop.y = y;
		}

		DrawArrow(start, stop);
		cs = dc.GetTextExtent(strDPL);
		CEnBitmap::DrawText(rect.Width()/2-cs.cy/2, start.y-5,-90, strDPL, RGB(0,0,0),&Font);
		COLORREF colBot = _TAH_ORANGE;
		COLORREF colTop = _TAH_ORANGE;
		Add_TU_DistrPipes(_T("SCH_PIPESTOPBOTARROWS"),colTop, _T("SCH_PIPESTOPBOTARROWS"), colBot);
	}
	else if (pCircSch->GetDpStab() == eDpStab::DpStabOnCV)
	{
		CPoint start,stop;
		// Fix X position
		const int BottomX = rect.Width()-100;
		const int TopX = 100;
		int cv_y = -1;

		start.y = stop.y = -1;
		start.x = stop.x = BottomX;
		// for Y position retrieve DpC and Bv anchoring points 
		const CPtrArray *ArrayAnchor = GetArrayAnchor();

		CAnchorPt *pAnchor;
		for(int i = 0; i < ArrayAnchor->GetSize(); i++)
		{
			pAnchor = (CAnchorPt*)ArrayAnchor->GetAt(i);
			if (!pAnchor) continue;
			// Skip anchoring points as Spline, they are used to draw capilary 
			if (pAnchor->GetConnectType() >= CAnchorPt::Spline) continue;
			if (pAnchor->GetFunction() == CAnchorPt::eFunc::DpC)
				stop.y = pAnchor->GetPoint().second;
			if (pAnchor->GetFunction() == CAnchorPt::eFunc::BV_P)
				start.y = pAnchor->GetPoint().second;
			if (pAnchor->GetFunction() == CAnchorPt::eFunc::ControlValve)
				cv_y = pAnchor->GetPoint().second;
			if (start.y>-1 && stop.y>-1 && cv_y>-1) break;
		}
		// Draw HMin arrow
		start.y += 75;
		stop.y-=75;
		DrawArrow(start, stop);

		// calculation of the text extent.
		CDC dc; 
		dc.CreateCompatibleDC(NULL); 
		CSize cs = dc.GetTextExtent(strHMin);
		CEnBitmap::DrawText(start.x, start.y-(start.y-stop.y-cs.cx)/2,-90, strHMin, RGB(0,0,0),&Font);

		// Draw DPL arrow
		start.x = stop.x = TopX;
		if (pCircSch->GetMvLoc() == eMvLoc::MvLocSecondary)
		{
			//start.y	-= 75; //BV
			stop.y = cv_y-75;
		}
		else
		{
			start.y = cv_y - 75;
			stop.y = cv_y + 75;
		}
		DrawArrow(start, stop);
		cs = dc.GetTextExtent(strDPL);
		CEnBitmap::DrawText(start.x-(2*cs.cy+5),start.y-(start.y-stop.y-cs.cx)/2,-90, strDPL, RGB(0,0,0),&Font);
		COLORREF colBot = _TAH_ORANGE;
		COLORREF colTop = _TAH_ORANGE;
		Add_TU_DistrPipes(_T("SCH_ONEPIPETOPBOTARROW"),colTop, _T("SCH_ONEPIPETOPBOTARROW"), colBot);
	}
	else ASSERT(0);
	Font.DeleteObject(); 

}

void CEnBitmapPatchWork::GetSSelDpCHydronicScheme(CDB_CircuitScheme *pCircSch, IDPTR ValveIDPtr, IDPTR DpCIDPtr, eDpCLoc DpCLoc /*= DpCLocDownStream*/)
{
	// Verify the Scheme is defined
	ASSERT (pCircSch);
	if (!pCircSch) return;

	PrepareDefaultComponents(pCircSch,true);

	// Create a manager to store all components
	CEnBitmapCompMngr CompoMng;
	// Create all needed components
	CArray<CEnBitmapComponentFile *>arCompoFile;
	for (int i=0; i<pCircSch->GetAnchorPtListSize(); i++)
	{
		// Verify if component should be added
		bool bFound = false;
		for (int j=0; j<pCircSch->GetAnchorPtListSize()&&!bFound;j++)
			bFound = pCircSch->GetAnchorPtFunc(i) == pCircSch->GetAnchorPtFunc(j);
		if (!bFound) continue;

		// Retrieve selected TA Product
		IDPTR IDPtr = _NULL_IDPTR;
		CString strComponent = m_arstrDefaultComponents.GetAt(pCircSch->GetAnchorPtFunc(i));
		switch (pCircSch->GetAnchorPtFunc(i))
		{
			case CAnchorPt::BV_P:
			case CAnchorPt::BV_S:
				if (ValveIDPtr.MP)
					IDPtr = ValveIDPtr;
				else strComponent = L"TA_BV";					// Container doesn't exist
			break;
			case CAnchorPt::DpC:
				if (DpCIDPtr.MP)
					IDPtr = DpCIDPtr;
				else
				{
					if (eDpCLoc::DpCLocDownStream==DpCLoc)
						strComponent = L"TA_DPC";					// Container doesn't exist
					else
						strComponent = L"TA_DPCF";					// Container doesn't exist
				}
			break;
		}
		// Overwrite component with the valve image.
		if (*IDPtr.ID)
		{
			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>(IDPtr.MP);
			ASSERT(pTAP);
			if (!pTAP) continue;
			strComponent = pTAP->GetProdPicID();
		}
		if (strComponent.IsEmpty())
			continue;
		CEnBitmapComponentFile *pCompo = new CEnBitmapComponentFile;
		pCompo->FillExistingComponentFile(TASApp.GetpProdPic((LPCTSTR)strComponent),pCircSch->GetAnchorPtFunc(i));
		CompoMng.AddComponent(pCompo);
		arCompoFile.Add(pCompo);
	}
	// Link all components with the background 
	SetComponentManager(&CompoMng);

	PrepareHydronicScheme(pCircSch);

	// Clean component array
	for (int i=0; i<arCompoFile.GetCount(); i++)
		delete(arCompoFile.GetAt(i));

}

void CEnBitmapPatchWork::DrawPatchwork( int iMvLoc, int iDpCLoc )
{
	CDC MemDc;
	m_iMvLoc = iMvLoc;
	m_iDpCLoc = iDpCLoc;
	MemDc.CreateCompatibleDC( NULL );
	CBitmap *pOldBmp = MemDc.SelectObject( this );

	if( NULL == pOldBmp )
	{
		return;
	}

	DrawAnchor( &MemDc );
}

void CEnBitmapPatchWork::DrawScheme( CDC* pDC, pair<int, int> *ptOrigin )
{
	if( NULL == pDC )
	{ 
		return;
	}

	CDC *pMemDC = GetDoubleBufferDC( pDC);

	if( NULL == pMemDC)
	{
		return;
	}

	DrawBackground( pMemDC, GetBackground() );
	DrawAnchor( pMemDC );
	
	CopyDC( pMemDC, pDC, ptOrigin );

	delete pMemDC;
}

////////////////////////////////////////////////////////////////
/// Draw the background
////////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::DrawBackground( CDC* pDC, ULONG ulBcKID )
{
	if( NULL == pDC || NULL == m_pComponentManager )
	{
		return;
	}

	CEnBitmapComponent *pComponent = m_pComponentManager->GetComponent( ulBcKID );

	if( NULL == pComponent )
	{
		return;
	}

	// Retrieve image..
	CEnBitmap *pBitmap = pComponent->GetImage();
	
	if( NULL == pBitmap )
	{
		return;
	}

	// Draw image.
	RotateDC( pDC, pBitmap);

	delete pBitmap;
}

void CEnBitmapPatchWork::DrawAnchor( CDC *pDC )
{
	const CPtrArray *pArrayAnchor = GetArrayAnchor();

	if( NULL == pDC || NULL == pArrayAnchor || NULL == m_pComponentManager )
	{
		return;
	}

	// First step draw all components according anchoring points.
	CAnchorPt *pAnchor;

	for( int i = 0; i < pArrayAnchor->GetSize(); i++ )
	{
		pAnchor = (CAnchorPt *)pArrayAnchor->GetAt( i );

		if( NULL == pAnchor )
		{
			continue;
		}
		
		// Skip anchoring points as Spline, they are used to draw capillary.
		if( pAnchor->GetConnectType() >= CAnchorPt::Spline && false == pAnchor->IsVirtualPoint() )
		{
			continue;
		}
		
		DrawComponent( pDC, pAnchor );
	}

	// Add spline points.
	for( int i = 0; i < pArrayAnchor->GetSize(); i++ )
	{
		pAnchor = (CAnchorPt *)pArrayAnchor->GetAt( i );

		if( pAnchor->GetConnectType() != CAnchorPt::Spline )
		{
			continue;
		}
		
		ULONG ulKey = m_pComponentManager->BuildKey( pAnchor->GetFunction(), pAnchor->GetStructure() );
		CEnBitmapComponent *pComponent = m_pComponentManager->GetComponent( ulKey );

		AddPtSpline( pDC, pAnchor, pComponent );
	}

	POSITION pos = m_GroupMap.GetStartPosition();
	int iGroup;
	CPtrArray *pArrayPt = NULL;

	while( pos != NULL )
	{
		m_GroupMap.GetNextAssoc( pos, iGroup, pArrayPt );
		DrawSpline( pDC, iGroup );
	}
}

_Pt *CEnBitmapPatchWork::GetCapilPt( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor )
{
	if( CAnchorPt::ePressSign::Pplus == pAnchor->GetPressureSign() || CAnchorPt::ePressSign::Pminus == pAnchor->GetPressureSign() )
	{
		return pComponent->GetptCapil( pAnchor->GetPressureSign() );
	}

	if( CAnchorPt::eFunc::BV_P == pAnchor->GetFunction() || CAnchorPt::eFunc::BV_Byp == pAnchor->GetFunction() )
	{
		//   - MvLoc in primary:
		//		 DpCLoc Upstream    -> we will use the 1st capillary point.
		//		 DpCLoc Downstream  -> we will use the 2nd capillary point.
		// 
		//   - MvLoc in secondary:
		//		 DpCLoc Upstream    -> we will use the 2nd capillary point.
		//		 DpCLoc Downstream  -> we will use the 1st capillary point.
		//
		// By convention:
		//
		//  For MV	Primary capillary point is UPSTREAM.			// For DPC	Primary capillary point is P+.
		//			Secondary capillary point is DOWNSTREAM.		//			Secondary capillary point is P-.

		bool bMvSec = ( m_iMvLoc == eMvLoc::MvLocSecondary );
		bool bDpCUpStream = ( m_iDpCLoc == eDpCLoc::DpCLocUpStream );
		bool bUse2ndCapilPt = (!bMvSec && !bDpCUpStream) || (bMvSec && bDpCUpStream);

		if( true == bUse2ndCapilPt && NULL != pComponent->GetpPt( ecpCapilSec ) )
		{
			return pComponent->GetpPt( ecpCapilSec );
		}

		return pComponent->GetpPt(ecpCapilPrim);
	}
	
	return NULL;
}

double CEnBitmapPatchWork::GetCapilAngle( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor )
{
	if( CAnchorPt::ePressSign::Pplus == pAnchor->GetPressureSign() || CAnchorPt::ePressSign::Pminus == pAnchor->GetPressureSign() )
	{
		return pComponent->GetAngleCapil(pAnchor->GetPressureSign());
	}

	if( CAnchorPt::eFunc::BV_P == pAnchor->GetFunction() || CAnchorPt::eFunc::BV_Byp == pAnchor->GetFunction() )
	{
		bool bMvSec = m_iMvLoc == eMvLoc::MvLocSecondary;
		bool bDpCUpStream = m_iDpCLoc == eDpCLoc::DpCLocUpStream;
		bool bUse2ndCapilPt = (!bMvSec && !bDpCUpStream) || (bMvSec && bDpCUpStream);

		if( true == bUse2ndCapilPt && pComponent->GetAngle( ecpCapilSec ) > 0.0 ) 
		{
			return pComponent->GetAngle( ecpCapilSec );
		}

		return pComponent->GetAngle( ecpCapilPrim );
	}
	
	return 0.0;
}

double CEnBitmapPatchWork::GetTempSensorCableAngle( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor )
{
	double dAngle = 0.0;

	if( CAnchorPt::TC_Yes == pAnchor->GetTempConnection() )
	{
		dAngle = pComponent->GetAngle( ecpTempSensor );
	}

	return dAngle;
}

double CEnBitmapPatchWork::GetInputOutputCableAngle( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor )
{
	double dAngle = 0.0;

	if( CAnchorPt::IOC_Yes == pAnchor->GetIOConnection() )
	{
		if( NULL != pComponent->GetpPt( eConnectionPoints::ecpInputSignal ) )
		{
			dAngle = pComponent->GetAngle( ecpInputSignal );
		}
		else if( NULL != pComponent->GetpPt( eConnectionPoints::ecpOutputSignal ) )
		{
			dAngle = pComponent->GetAngle( ecpOutputSignal );
		}
		else
		{
			// Not normal to not have at least one input or output signal point.
			ASSERT( 0 );
		}
	}

	return dAngle;
}

// For the moment this feature is only available for temperature sensor cable !!
int CEnBitmapPatchWork::GetCtrlPointDistance( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor )
{
	int iCtrlPointDistance = 0;

	if( true == pAnchor->IsTempConnectionDefined() )
	{
		iCtrlPointDistance = pComponent->GetCtrlPointDistance( ecpTempSensor );
	}
	else if( true == pAnchor->IsIOConnectionDefined() )
	{
		if( NULL != pComponent->GetpPt( eConnectionPoints::ecpInputSignal ) )
		{
			iCtrlPointDistance = pComponent->GetCtrlPointDistance( eConnectionPoints::ecpInputSignal );
		}
		else if( NULL != pComponent->GetpPt( eConnectionPoints::ecpOutputSignal ) )
		{
			iCtrlPointDistance = pComponent->GetCtrlPointDistance( eConnectionPoints::ecpOutputSignal );
		}
		else
		{
			// Must not happen !
			ASSERT( 0 );
		}
	}
	else if( true == pAnchor->IsPressureSignDefined() )
	{
		CAnchorPt::ePressSign ePressureSign = pAnchor->GetPressureSign();
		eConnectionPoints ConnectionPoint = ( CAnchorPt::ePressSign::Pplus == ePressureSign ) ? eConnectionPoints::ecpCapilPrim : eConnectionPoints::ecpCapilSec;
		iCtrlPointDistance = pComponent->GetCtrlPointDistance( ConnectionPoint );
	}

	return iCtrlPointDistance;
}

bool CEnBitmapPatchWork::MvShouldBeReturned(CEnBitmapComponent *pComponent, CAnchorPt* pAnchor)
{
	// Specific treatment for valve with only one fixed capillary point as STAD.
	//
	//   - MvLoc in primary:
	//		 DpCLoc Upstream    -> we will use the 1st capillary point.
	//		 DpCLoc Downstream  -> we will use the 2nd capillary point.
	// 
	//   - MvLoc in secondary:
	//		 DpCLoc Upstream    -> we will use the 2nd capillary point.
	//		 DpCLoc Downstream  -> we will use the 1st capillary point.
	//
	// By convention:
	//
	//  For MV	Primary capillary point is UPSTREAM.			// For DPC	Primary capillary point is P+.
	//			Secondary capillary point is DOWNSTREAM.		//			Secondary capillary point is P-.

	bool bValveShouldbeReturned = false;

	if( ( CAnchorPt::eFunc::BV_P == pAnchor->GetFunction() || CAnchorPt::eFunc::BV_Byp == pAnchor->GetFunction() )
			&& CAnchorPt::PNU != pAnchor->GetPressureSign() )
	{
		bool bMvSec = ( m_iMvLoc == eMvLoc::MvLocSecondary ) ? true : false;
		bool bDpCUpStream = ( m_iDpCLoc == eDpCLoc::DpCLocUpStream ) ? true : false;
		bool bUse2ndCapilPt = ( !bMvSec && !bDpCUpStream ) || ( bMvSec && bDpCUpStream );
		
		bool b1stCapilExist = pComponent->GetpPt( eConnectionPoints::ecpCapilPrim ) ? true : false;
		bool b2ndCapilExist = pComponent->GetpPt( eConnectionPoints::ecpCapilSec ) ? true : false;
		
		bValveShouldbeReturned = ( !b1stCapilExist && b2ndCapilExist && !bUse2ndCapilPt ) || ( b1stCapilExist && !b2ndCapilExist && bUse2ndCapilPt );
	}
	return bValveShouldbeReturned;
}

////////////////////////////////////////////////////////////////
/// Draw required component at anchoring point pAnchor
////////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::DrawComponent( CDC *pDC, CAnchorPt *pAnchor )
{
	if( NULL == pDC || NULL == pAnchor || NULL == m_pComponentManager )
	{
		return;
	}

	ULONG ulKey = m_pComponentManager->BuildKey( pAnchor->GetFunction(), pAnchor->GetStructure() );
	CEnBitmapComponent *pComponent = m_pComponentManager->GetComponent( ulKey );

	// If we have a component that is a virtual point, there is nothing to draw.
	if( false == pAnchor->IsVirtualPoint() )
	{
		if( NULL == pComponent )
		{
			return;
		}

		// Get image.
		CEnBitmap *pEnBmpComponent = pComponent->GetImage();

		if( NULL == pEnBmpComponent )
		{
			return;
		}
	
		// Get image dimension.
		BITMAP bm;
		pEnBmpComponent->GetBitmap( &bm );

		// Retrieve the anchor coordinate (link to the background).
		pair<int, int> ptAnch = pAnchor->GetPoint();

		// Retrieve component coordinate (link to the component).
		pair<int, int> ptComp = pComponent->GetCoord( pAnchor->GetConnectType() );

		if( true == MvShouldBeReturned( pComponent, pAnchor ) )
		{
			if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
			{
				// Vertical flow, thus horizontal flip.
				pEnBmpComponent->FlipImage( FALSE, TRUE );
				ptComp.second = bm.bmHeight - 1 - ptComp.second;
			}
		
			if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
			{
				// Horizontal flow, thus vertical flip.
				pEnBmpComponent->FlipImage( TRUE );
				ptComp.first = bm.bmWidth - 1 - ptComp.first;
			}
		}	
	
		// Apply flip if ClockWise, done on the pEnBitmap before DC rotation.
		if( true == pAnchor->IsClockwise() )
		{
			if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
			{
				// 90° is North, 270° is South.
				// Vertical flow, thus horizontal flip.
				pEnBmpComponent->FlipImage( TRUE );
				ptComp.first = bm.bmWidth - 1 - ptComp.first;
			}
		
			if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
			{
				// 0° is East, 180° is West.
				// Horizontal flow, thus vertical flip.
				pEnBmpComponent->FlipImage( FALSE, TRUE);
				ptComp.second = bm.bmHeight - 1 - ptComp.second;
			}
		}

		int iAngleOffset = ( pAnchor->GetDirection() + 90 ) % 360 ;
	
		if( iAngleOffset == 90 || iAngleOffset == 180 )
		{
			// 90° is North, 180° is West.
			pAnchor->SetPairCompoSize( pair<int, int>( bm.bmHeight, bm.bmWidth ) );
		}
		else
		{
			// 0° is East, 270° is South.
			pAnchor->SetPairCompoSize( pair<int, int>( bm.bmWidth, bm.bmHeight ) );
		}


		// Rotate to the right degree with transparency.
		// pAnchor->GetDirection()+90 because the default south (270°) direction of components must result in no rotation.
		RotateDC( pDC, pEnBmpComponent, iAngleOffset, &ptAnch, &ptComp );

		delete pEnBmpComponent;
	}

	// Add spline point for differential pressure controller, for the temperature sensor, for the input/output signal cable
	// or for a virtual point.
	if( CAnchorPt::PNU != pAnchor->GetPressureSign() 
			|| CAnchorPt::TC_Yes == pAnchor->GetTempConnection()
			|| CAnchorPt::IOC_Yes == pAnchor->GetIOConnection()
			|| true == pAnchor->IsVirtualPoint() )
	{
		AddPtSpline( pDC, pAnchor, pComponent );
	}
}

//=============================================================================
// Draw Image with Rotation and Translation if needed
//=============================================================================
void CEnBitmapPatchWork::RotateDC( CDC *pDCDest, CEnBitmap *pEnBitmap, int degree, pair<int,int> *pAnchor, pair<int,int> *ptRotCenter, COLORREF backgroundColor)
{
	XFORM xFormOld, xFormRot, xFormTran, xForm;

	if( NULL == pDCDest || NULL == pEnBitmap )
	{
		return;
	}
	
	if( -1 == backgroundColor )
	{																		// Retrieve Background color if defined
		backgroundColor = pEnBitmap->GetPixel( 0, 0 );
	}

	//-----------------------------------------------
	// Create the DC which contains bitmap.
	BITMAP bm;
	pEnBitmap->GetBitmap( &bm );											// Retrieve image size

	CDC dc;
	dc.CreateCompatibleDC( pDCDest );										// Create compatible DC
	dc.SelectObject( pEnBitmap );											// Load image in this DC

	//-----------------------------------------------
	// Init the Rotation center.
	pair<int,int> ptRot( 0,0 );
	if( ptRotCenter != 0 )
	{
		ptRot = *ptRotCenter;
	}

	//-----------------------------------------------
	// Modify World coordinates.
	pDCDest->GetWorldTransform( &xFormOld );
	int nOldGraphicMode = pDCDest->SetGraphicsMode( GM_ADVANCED );

	double deg = -degree % 360;
	double radian = deg*M_PI/180;
	double sine = sin( radian);
	double cosine = cos( radian);

	// Rotation matrix.
	xFormRot.eM11 = (float)cosine;
	xFormRot.eM12 = (float)sine;
	xFormRot.eM21 = (float)-sine;
	xFormRot.eM22 = (float)cosine;
	xFormRot.eDx = 0;
	xFormRot.eDy = 0;
	
	// Translation matrix.
	xFormTran.eM11 = 1;
	xFormTran.eM12 = 0;
	xFormTran.eM21 = 0;
	xFormTran.eM22 = 1;
	xFormTran.eDx = (pAnchor)?(float)pAnchor->first:0;
	xFormTran.eDy = (pAnchor)?(float)pAnchor->second:0;

	// Combine translation with rotation.
	CombineTransform( &xForm, &xFormRot, &xFormTran);
	
	// Apply this to the world.
	pDCDest->SetWorldTransform( &xForm);

	// Copy image to the Destination DC.
	pDCDest->TransparentBlt( -ptRot.first,-ptRot.second,bm.bmWidth,bm.bmHeight,&dc, 0,0,bm.bmWidth,bm.bmHeight, backgroundColor );

	//-----------------------------------------------
	// Restore old world coordinates.
	pDCDest->SetWorldTransform( &xFormOld);
	pDCDest->SetGraphicsMode( nOldGraphicMode);
}

///////////////////////////////////////////////////////////////////////
/// Add points for Spline  number of points should be a multiple of 3 + 1--> 4, 7, 10
/// if pComponent exist, two points will be added, connection point and ctrl point
///////////////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::AddPtSpline( CDC *pDC, CAnchorPt *pAnchor, CEnBitmapComponent *pComponent )
{
	if( NULL == pDC || NULL == pAnchor )
	{
		return;
	}
	
	pair<int, int> ptVirtual;
	pair<int, int> *pt = NULL;
	CPtrArray *pArrayPt = NULL;				// list of points for drawing a spline.

	if( FALSE == m_GroupMap.Lookup( pAnchor->GetGroup(), pArrayPt ) )
	{
		pArrayPt = new CPtrArray();
		m_GroupMap[pAnchor->GetGroup()] = pArrayPt;

		// The first point set if we are working with Dp capillary, temperature sensor cable or input/output signal cable.
		int *pSplineType = new int();

		if( NULL == pSplineType )
		{
			ASSERT_RETURN;
		}

		if( CAnchorPt::TC_None != pAnchor->GetTempConnection() )
		{
			*pSplineType = ST_TempSensor;
		}
		else if( CAnchorPt::IOC_None != pAnchor->GetIOConnection() )
		{
			*pSplineType = ST_InputOutput;
		}
		else
		{
			*pSplineType = ST_Capillary;
		}

		pArrayPt->Add( pSplineType );
	}

	// This method is called twice for each component and for each group defined.
	// So we can have group 0 and 2 calls to link a BV with a DpC for example.
	// See after for more comments.
	bool bByPass = false;

	if( NULL != pComponent )
	{
		if( true == pAnchor->IsVirtualPoint() )
		{
			// If no connection point is defined in the 'CAnchorPt' object, we have the virtual point already in the hydraulic circuit coordinate system.
			if( false == pAnchor->IsConnectionDefined() )
			{
				ptVirtual = pAnchor->GetPoint();
				pt = new pair<int, int>( ptVirtual.first, ptVirtual.second );
				bByPass = true;
			}
		}
		
		if( false == bByPass )
		{
			if( CAnchorPt::TC_None != pAnchor->GetTempConnection() )
			{
				pt = FindTempSensorCablePt( pAnchor, pComponent );
			}
			else if( CAnchorPt::IOC_None != pAnchor->GetIOConnection() )
			{
				pt = FindInputOutputCablePt( pAnchor, pComponent );
			}
			else
			{
				pt = FindCapillPt( pAnchor, pComponent );
			}
		}

		// In this stage, 'pt' contains the coordinate of the connection point in the hydraulic circuit system coordinate.

		if( NULL == pt )
		{
			return;
		}
		
		// Add first capillary point.
		// Remark: if '1' it means that is the first call (point 0 is the spline type) for the spline belonging to this group. So the current
		// anchoring point from the current compoment is the first point of the spline.
		if( 1 == pArrayPt->GetCount() )
		{
			pArrayPt->Add( pt );
		}

		// Add a control point.
		// If it's the first call for the group, it will be the second point of the spline.
		// If it's the second call for the group, it will be the third point of the spline.
		pair<int, int> *pPtCtrl = CalculateCtrlPoint( pt, pAnchor, pComponent );
		pArrayPt->Add( pPtCtrl );

		// Add last capillary point.
		// Remark: if '4' it means that there is already the spline definition (0), the beginning point (1), the first (2) and second point control (3).
		// So, the 'pt' is the last point of the spline.
		if( 4 == pArrayPt->GetCount() )
		{
			pArrayPt->Add( pt );
		}

#if _DEBUG	
		// Debugging stuff display green circle for capillary connection point; red circle for spline ctrl pt.
		CPen Pen;
		Pen.CreatePen( PS_SOLID, 1, _RED );
		CPen *pOldPen = pDC->SelectObject( &Pen );
		pDC->Ellipse( pPtCtrl->first - 5, pPtCtrl->second - 5, pPtCtrl->first + 5, pPtCtrl->second + 5 );
		pDC->SelectObject( pOldPen );
		Pen.DeleteObject();
#endif
	}
	else 
	{
		pt = new pair<int, int>( pAnchor->GetPoint() );	
		pArrayPt->Add( pt );
	}

#if _DEBUG	
	// Debugging stuff display green circle for capillary connection point; red circle for spline ctrl pt.
	CPen Pen;
	Pen.CreatePen( PS_SOLID,1,_GREEN );
	CPen *pOldPen = pDC->SelectObject( &Pen );
	pDC->Ellipse( pt->first - 5, pt->second - 5, pt->first + 5, pt->second + 5 );
	pDC->SelectObject( pOldPen );
	Pen.DeleteObject();
#endif
}

///////////////////////////////////////////////////////////////////////
/// Draw a Spline when enough points (4) are written into ArrayPt
/// This function creates a Bezier Spline from 4 points or more (should be a multiple of 3 + 1--> 4, 7, 10).
///                                                   
///             ptCtrlStart                                
///                 *                             
///               __                             
///             /    \                    *  ptEnd            
///           /        \                /                
/// ptStart *            \           /                 
///                        \      /                     
///                          ---                         
///                               * ptCtrlEnd                    
/// 
///////////////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::DrawSpline( CDC *pDC, int Group )
{
	if( NULL == pDC )
	{
		return;
	}

	CPtrArray *pArrayPt = NULL;			// list of points for drawing a spline

	if( FALSE == m_GroupMap.Lookup( Group, pArrayPt ) )
	{
		return;
	}

	// The first element in the array is the spline type.
	int *piSplineType = (int *)pArrayPt->GetAt( 0 );
	SplineType eSplineType = (SplineType)*piSplineType;
	pArrayPt->RemoveAt( 0 );
	delete piSplineType;

	// Convert CPtrArray to CPoint array.
	CPoint *ptList = new CPoint[pArrayPt->GetCount()];

	// Points should be ordered.
	multimap<double, CPoint *> mmap;

	for( int i = 0; i < pArrayPt->GetCount(); i++ )
	{
		CPoint *pPt = (CPoint *) pArrayPt->GetAt( i );

		// First 4 points are fixed represent starting, ending and angles of capillary connection on the component 
		if( i < 2 )
		{
			ptList[i] = *pPt;
		}
		else if( 2 == i || 3 == i )
		{
			int index = pArrayPt->GetCount() - 4 + i;
			ptList[index] = *pPt;
		}
		else
		{
			// Try to ordering spline points.
			double dPhi = atan2( (double)pPt->y, (double)pPt->x );
			double dR = sqrt( pow( (double)pPt->y, 2 ) + pow( (double)pPt->x, 2 ) );
			dPhi *= dR;
			mmap.insert( pair<double, CPoint *>( dPhi, pPt ) );
		}
	}
	
	// Insert spline points in the middle of fixed point.
	if( mmap.size() > 0 )
	{
		CPoint *pPt = (CPoint *)pArrayPt->GetAt(1);
		double dPt1 = atan2( (double)pPt->y, (double)pPt->x) * sqrt(pow( (double)pPt->y, 2 ) + pow( (double)pPt->x, 2 ) );
		pPt = (CPoint*)pArrayPt->GetAt( 2 );
		double dPt2 = atan2( (double)pPt->y, (double)pPt->x) * sqrt( pow( (double)pPt->y, 2 ) + pow( (double)pPt->x, 2 ) );
		int i = 2;
		
		// We go from PT1 to PT2.
		if( dPt1 > dPt2 )
		{
			// Greatest to smallest.
			multimap<double, CPoint *>::reverse_iterator rit;

			for( rit = mmap.rbegin(); rit != mmap.rend(); rit++, i++ )
			{
				ptList[i] = *rit->second;
			}
		}
		else
		{
			// Smallest to greatest.
			multimap<double, CPoint *>::iterator it;

			for( it = mmap.begin(); it != mmap.end(); it++,i++ )
			{
				ptList[i] = *it->second;
			}
		}
	}
	
	CPen Pen;
	LOGBRUSH logBrush;
	logBrush.lbStyle = BS_SOLID;
	logBrush.lbColor = RGB( 160, 160, 160 );

	if( SplineType::ST_Capillary == eSplineType )
	{
		Pen.CreatePen( PS_DASH | PS_GEOMETRIC | PS_ENDCAP_ROUND, 4, &logBrush );
	}
	else
	{
		Pen.CreatePen( PS_DOT | PS_GEOMETRIC | PS_ENDCAP_ROUND, 4, &logBrush );
	}

	CPen *pOldPen = pDC->SelectObject( &Pen );

	pDC->PolyBezier( ptList, (int) pArrayPt->GetCount() ); 

	pDC->SelectObject( pOldPen );
	Pen.DeleteObject();

	delete [] ptList;

	CleanPtList( pArrayPt );
	delete pArrayPt;
	m_GroupMap.RemoveKey( Group );
}

///////////////////////////////////////////////////////////////////////
/// Calculate the position of the ctrl for the capillary point
///////////////////////////////////////////////////////////////////////
pair<int, int> *CEnBitmapPatchWork::CalculateCtrlPoint( pair<int, int> *pPtCurve, CAnchorPt *pAnchor, CEnBitmapComponent *pComponent )
{
	pair<int, int> *pPtCtrl = NULL;

	if( NULL == pPtCurve || NULL == pAnchor || NULL == pComponent )
	{
		return pPtCtrl;
	}
	
	// pPtCurve is the point in the current DC where we can connect the capillary pipe.

	bool bByPass = false;
	int iUnitPx = 0;

	if( true == pAnchor->IsVirtualPoint() )
	{
		// In case of the anchoring point is virtual we have two cases:
		//		1) If a connection point is defined in the 'pAnchor' then we retrieve this point -> PAY ATTENTION: this point
		//         is in the component coordinate system.
		//		2) If no connection point is defined, take the point in the 'pAnchor' -> PAY ATTENTION: this point is in the
		//         hydraulic circuit coordinate system.

		if( false == pAnchor->IsConnectionDefined() )
		{
			// Case 2.
			iUnitPx = pAnchor->GetCtrlPointDistance();
			bByPass = true;
		}
	}
	
	if( false == bByPass )
	{
		iUnitPx = GetCtrlPointDistance( pComponent, pAnchor );
	}

	if( 0 == iUnitPx )
	{
		CRect rect = pComponent->GetSizeImage();
		iUnitPx = max( rect.Width(), rect.Height() );
	}
	
	// Retrieve the degree for the capillary curve.
	bByPass = false;
	double dAngle = 0.0;

	if( true == pAnchor->IsVirtualPoint() )
	{
		// See note above.

		if( false == pAnchor->IsConnectionDefined() )
		{
			dAngle = pAnchor->GetAngle();
			bByPass = true;
		}
	}
	
	if( false == bByPass )
	{
		if( CAnchorPt::TC_None != pAnchor->GetTempConnection() )
		{
			dAngle = GetTempSensorCableAngle( pComponent, pAnchor );
		}
		else if( CAnchorPt::IOC_None != pAnchor->GetIOConnection() )
		{
			dAngle = GetInputOutputCableAngle( pComponent, pAnchor );
		}
		else
		{
			dAngle = GetCapilAngle( pComponent, pAnchor );
		}

		/*
		if( false == pAnchor->IsClockwise() )
		{
			dAngle += 180.0;

			if( dAngle > 360.0 )
			{
				dAngle -= 360.0;
			}
		}
		*/
	}

	/*
	int iAnchorDir = pAnchor->GetDirection();

	if( true == MvShouldBeReturned( pComponent, pAnchor ) )
	{
		// Valve has been returned.
		iAnchorDir += 180;
	}

	while( iAnchorDir > 360 )
	{
		iAnchorDir -= 360;
	}

	// Modify capillary orientation in function of anchoring point.
	// CW: clockwise; CCW counter clockwise.
	bool bFlipV = false;
	bool bFlipH = false;

	if( 270 == iAnchorDir )	// South 
	{
		if( false == pAnchor->IsClockwise() )
		{
			bFlipV = false;
			bFlipH = false;
		}
		else
		{
			bFlipV = true;
			bFlipH = false;
		}
	}
	else if( 90 == iAnchorDir )	// North
	{
		if(  false == pAnchor->IsClockwise() )
		{
			bFlipV = true;
			bFlipH = true;
		}
		else
		{
			bFlipV = false;
			bFlipH = true;
		}
	}
	else if( 0 == iAnchorDir )	// East
	{
		if( false == pAnchor->IsClockwise() )
		{
			bFlipV = false;
			bFlipH = true;
		}
		else
		{
			bFlipV = false;
			bFlipH = false;
		}
	}
	else if( 180 == iAnchorDir)	// West
	{
		if( false == pAnchor->IsClockwise() )
		{
			bFlipV = true;
			bFlipH = false;
		}
		else
		{
			bFlipV = true;
			bFlipH = true;
		}
	}

	double dRrad = dAngle * M_PI / 180.0;

	if( true == bFlipV )
	{
		bool bSign = sin( dRrad ) >= 0;
		double dCos = -1 * cos( dRrad );
		dRrad = acos( dCos );

		// Validity field for acos is 0 to PI
		// Correct angle with sin sign
		if( false == bSign )
		{
			dRrad *=-1;
		}
	}

	if( true == bFlipH )
	{
		bool bSign = cos( dRrad ) >= 0;
		double dSin = -1 * sin( dRrad );
		dRrad = asin( dSin );

		// Validity field for asin is -PI/2 to PI/2
		// Correct angle with cos sign
		if( false == bSign )
		{
			dRrad = M_PI - dRrad;
		}
	}

	int x = (int)( pPtCurve->first + cos( dRrad ) * iUnitPx );
	int y = (int)( pPtCurve->second - sin( dRrad ) * iUnitPx );
	*/

	// Here we check if we need to invert input and output.
	if( true == MvShouldBeReturned( pComponent, pAnchor ) )
	{
		// 90° is North, 270° is South.
		if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
		{
			// Remark: if the valve is vertical, to invert input and output we need to apply a vertical flip (Mirror on a horizontal line).
			dAngle = 360.0 - dAngle;
		}
		
		// 0° is East, 180° is West.
		if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
		{
			// Remark: if the valve is horizontal, to invert input and output we need to apply a horizontal flip (Mirror on a vertical line).
			dAngle = 180.0 - dAngle;
		}
	}
	
	dAngle = (int)dAngle % 360;

	// Here we check if we need to invert handwheel.
	if( true == pAnchor->IsClockwise() )
	{
		// 90° is North, 270° is South.
		if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
		{
			// Remark: if the valve is vertical, to invert handwheel position we need to apply a horizontal flip (Mirror on a vertical line).
			dAngle = 180.0 - dAngle;
		}
		
		// 0° is East, 180° is West.
		if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
		{
			
			// Remark: if the valve is horizontal, to invert handwheel position we need to apply a vertical flip (Mirror on a horizontal line).
			dAngle = 360.0 - dAngle;
		}
	}

	dAngle = (int)dAngle % 360;

	int iAngleOffset = ( pAnchor->GetDirection() + 90 ) % 360;
	dAngle += iAngleOffset;

	double dRrad = dAngle * M_PI / 180.0;
	int x = (int)( pPtCurve->first + cos( dRrad ) * iUnitPx );
	int y = (int)( pPtCurve->second - sin( dRrad ) * iUnitPx );

	pPtCtrl = new pair<int, int>( x, y );

	return pPtCtrl;
}

///////////////////////////////////////////////////////////////////////
/// Delete all points contained in the PtArray
///////////////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::CleanPtList(CPtrArray *pPtArray)
{
	if( 0 == pPtArray)
	{
		return;
	}

	CPoint* pPt;

	for(int i = 0; i < pPtArray->GetCount(); i++)
	{
		pPt = (CPoint*) pPtArray->GetAt(i);
		if( 0 == pPt)
		{
			continue;
		}

		delete pPt;
	}

	pPtArray->RemoveAll();
}

pair<int, int> *CEnBitmapPatchWork::FindCapillPt( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent )
{
	pair<int, int> *ptCapill = NULL;

	if( NULL == pAnchor || NULL == pComponent || NULL == GetCapilPt( pComponent, pAnchor ) )
	{
		return ptCapill;
	}
	
	// Get image.
	CEnBitmap *pBitmap = pComponent->GetImage();

	if( NULL == pBitmap )
	{
		return ptCapill;
	}

	// Get image dimension.
	BITMAP bm;
	pBitmap->GetBitmap( &bm );
	
	// Get reference point of the component.
	pair<int, int> ptRef = pComponent->GetCoord( pAnchor->GetConnectType() );

	// Get capillary point.
	ptCapill = new pair<int, int>();
	*ptCapill = *GetCapilPt( pComponent, pAnchor );

	if( true == MvShouldBeReturned( pComponent, pAnchor ) )
	{
		if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
		{
			// Vertical flow, thus horizontal flip.
			ptCapill->second = bm.bmHeight - 1 - ptCapill->second;
			ptRef.second = bm.bmHeight - 1 - ptRef.second;
		}

		if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
		{
			// Horizontal flow, thus vertical flip.
			ptCapill->first = bm.bmWidth - 1 - ptCapill->first;
			ptRef.first = bm.bmWidth - 1 - ptRef.first;
		}
	}	

	// Apply flip if ClockWise, done on the pEnBitmap before DC rotation.
	if( true == pAnchor->IsClockwise() )
	{
		if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
		{
			// Vertical flow, thus horizontal flip.
			ptCapill->first = bm.bmWidth - 1 - ptCapill->first;
			ptRef.first = bm.bmWidth - 1 - ptRef.first;

		}

		if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
		{
			// Horizontal flow, thus vertical flip.
			ptCapill->second = bm.bmHeight - 1 - ptCapill->second;
			ptRef.second = bm.bmHeight - 1 - ptRef.second;
		}
	}

	//------------------------------------------
	// Retrieve angle in degree
	// +90 because the default south (270°) direction of components must result in no rotation
	int degree = ( pAnchor->GetDirection() + 90 ) % 360;

	double deg = -degree % 360;
	double radian = deg * M_PI / 180;
	double sine = sin( radian );
	double cosine = cos( radian );

	pair<int,int> Pt = *ptCapill;
	ptCapill->first = (long)(Pt.first * cosine - Pt.second * sine + 0.5 );
	ptCapill->second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );
	
	Pt = ptRef;
	ptRef.first = (long)( Pt.first * cosine - Pt.second * sine + 0.5 );
	ptRef.second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );

	// Map Coord in function of the image position.
	MapImagePointToDC( pAnchor, pComponent, &ptRef, *ptCapill );

	delete pBitmap;

	return ptCapill;
}

pair<int, int> *CEnBitmapPatchWork::FindTempSensorCablePt( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent )
{
	pair<int, int> *ptTempSensorCable = NULL;

	if( NULL == pAnchor || NULL == pComponent || NULL == pComponent->GetpPt( ecpTempSensor ) )
	{
		return ptTempSensorCable;
	}
	
	// Get image.
	CEnBitmap *pBitmap = pComponent->GetImage();

	if( NULL == pBitmap )
	{
		return ptTempSensorCable;
	}

	// Get image dimension.
	BITMAP bm;
	pBitmap->GetBitmap( &bm );
	
	// Get reference point of component (point that will match the anchoring point).
	pair<int, int> ptRef = pComponent->GetCoord( pAnchor->GetConnectType() );

	// Get temperature sensor point.
	ptTempSensorCable = NULL;
	
	if( NULL != pComponent->GetpPt( ecpTempSensor ) )
	{
		std::pair<short, short> *pShort = pComponent->GetpPt( ecpTempSensor );
		ptTempSensorCable = new pair<int, int>();
		ptTempSensorCable->first = pShort->first;
		ptTempSensorCable->second = pShort->second;
	}
	else
	{
		return NULL;
	}

	// Apply flip if clockWise, done on the pEnBitmap before DC rotation.
	if( true == pAnchor->IsClockwise() )
	{
		if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
		{
			// Vertical flow, thus vertical flip.
			ptTempSensorCable->first = bm.bmWidth - 1 - ptTempSensorCable->first;
			ptRef.first = bm.bmWidth - 1 - ptRef.first;

		}

		if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
		{
			// Horizontal flow, thus horizontal flip.
			ptTempSensorCable->second = bm.bmHeight - 1 - ptTempSensorCable->second;
			ptRef.second = bm.bmHeight - 1 - ptRef.second;
		}
	}

	//------------------------------------------
	// Retrieve angle in degree
	// +90 because the default south (270°) direction of components must result in no rotation
	int degree = pAnchor->GetDirection() + 90;

	double deg = -degree % 360;
	double radian = deg * M_PI / 180;
	double sine = sin( radian );
	double cosine = cos( radian );
	pair<int,int> Pt = *ptTempSensorCable;
	ptTempSensorCable->first = (long)(Pt.first * cosine - Pt.second * sine + 0.5 );
	ptTempSensorCable->second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );
	Pt = ptRef;
	ptRef.first = (long)( Pt.first * cosine - Pt.second * sine + 0.5 );
	ptRef.second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );

	// Map Coord in function of the image position.
	MapImagePointToDC( pAnchor, pComponent, &ptRef, *ptTempSensorCable );

	delete pBitmap;

	return ptTempSensorCable;
}

pair<int, int> *CEnBitmapPatchWork::FindInputOutputCablePt( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent )
{
	pair<int, int> *ptInputOutputSignalCable = NULL;

	if( NULL == pAnchor || NULL == pComponent 
			|| ( NULL == pComponent->GetpPt( ecpInputSignal ) && NULL == pComponent->GetpPt( ecpOutputSignal ) ) )
	{
		return ptInputOutputSignalCable;
	}

	// Get image.
	CEnBitmap *pBitmap = pComponent->GetImage();

	if( NULL == pBitmap )
	{
		return ptInputOutputSignalCable;
	}

	// Get image dimension.
	BITMAP bm;
	pBitmap->GetBitmap( &bm );

	// Get reference point of component (point that will match the anchoring point).
	pair<int, int> ptRef = pComponent->GetCoord( pAnchor->GetConnectType() );

	// Get input signal cable point.
	std::pair<short, short> *pShort = pComponent->GetpPt( ecpInputSignal );

	if( NULL == pShort )
	{
		// Get output signal cable point.
		pShort = pComponent->GetpPt( ecpOutputSignal );
	}

	if( NULL == pShort )
	{
		return ptInputOutputSignalCable;
	}

	ptInputOutputSignalCable = new pair<int, int>();
	ptInputOutputSignalCable->first = pShort->first;
	ptInputOutputSignalCable->second = pShort->second;

	// Apply flip if clockWise, done on the pEnBitmap before DC rotation.
	if( true == pAnchor->IsClockwise() )
	{
		if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
		{
			// Vertical flow, thus vertical flip.
			ptInputOutputSignalCable->first = bm.bmWidth - 1 - ptInputOutputSignalCable->first;
			ptRef.first = bm.bmWidth - 1 - ptRef.first;

		}

		if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
		{
			// Horizontal flow, thus horizontal flip.
			ptInputOutputSignalCable->second = bm.bmHeight - 1 - ptInputOutputSignalCable->second;
			ptRef.second = bm.bmHeight - 1 - ptRef.second;
		}
	}

	//------------------------------------------
	// Retrieve angle in degree
	// +90 because the default south (270°) direction of components must result in no rotation
	int degree = pAnchor->GetDirection() + 90;

	double deg = -degree % 360;
	double radian = deg * M_PI / 180;
	double sine = sin( radian );
	double cosine = cos( radian );
	pair<int,int> Pt = *ptInputOutputSignalCable;
	ptInputOutputSignalCable->first = (long)(Pt.first * cosine - Pt.second * sine + 0.5 );
	ptInputOutputSignalCable->second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );
	Pt = ptRef;
	ptRef.first = (long)( Pt.first * cosine - Pt.second * sine + 0.5 );
	ptRef.second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );

	// Map Coord in function of the image position.
	MapImagePointToDC( pAnchor, pComponent, &ptRef, *ptInputOutputSignalCable );

	delete pBitmap;

	return ptInputOutputSignalCable;
}

void CEnBitmapPatchWork::ConvertComponentCoordinateSystemToBitmapCoordinateSystem( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent, pair<int, int> *ptPoint )
{
	if( NULL == pAnchor || NULL == pComponent || NULL == ptPoint )
	{
		return;
	}

	// Get image.
	CEnBitmap *pBitmap = pComponent->GetImage();

	if( NULL == pBitmap )
	{
		return;
	}

	// Get image dimension.
	BITMAP bm;
	pBitmap->GetBitmap( &bm );

	// Get reference point of component (point that will match the anchoring point).
	pair<int, int> ptRef = pComponent->GetCoord( pAnchor->GetConnectType() );

	pair<int, int> *ptVirtualPoint = new pair<int, int>();
	ptVirtualPoint->first = ptPoint->first;
	ptVirtualPoint->second = ptPoint->second;

	// Apply flip if clockWise, done on the pEnBitmap before DC rotation.
	if( true == pAnchor->IsClockwise() )
	{
		if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
		{
			// Vertical flow, thus vertical flip.
			ptVirtualPoint->first = bm.bmWidth - 1 - ptVirtualPoint->first;
			ptRef.first = bm.bmWidth - 1 - ptRef.first;

		}

		if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
		{
			// Horizontal flow, thus horizontal flip.
			ptVirtualPoint->second = bm.bmHeight - 1 - ptVirtualPoint->second;
			ptRef.second = bm.bmHeight - 1 - ptRef.second;
		}
	}

	//------------------------------------------
	// Retrieve angle in degree
	// +90 because the default south (270°) direction of components must result in no rotation
	int degree = pAnchor->GetDirection() + 90;

	double deg = -degree % 360;
	double radian = deg * M_PI / 180;
	double sine = sin( radian );
	double cosine = cos( radian );
	pair<int,int> Pt = *ptVirtualPoint;
	ptVirtualPoint->first = (long)(Pt.first * cosine - Pt.second * sine + 0.5 );
	ptVirtualPoint->second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );
	Pt = ptRef;
	ptRef.first = (long)( Pt.first * cosine - Pt.second * sine + 0.5 );
	ptRef.second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );

	// Map Coord in function of the image position.
	MapImagePointToDC( pAnchor, pComponent, &ptRef, *ptVirtualPoint );

	delete pBitmap;

	ptPoint->first = ptVirtualPoint->first;
	ptPoint->second = ptVirtualPoint->second;

	delete ptVirtualPoint;
	
	return;
}

void CEnBitmapPatchWork::ConvertHydraulicCircuitCoordinateSystemToBitmapCoordinateSystem( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent, pair<int, int> *ptPoint )
{
	if( NULL == pAnchor || NULL == pComponent || NULL == ptPoint )
	{
		return;
	}

	// Get image.
	CEnBitmap *pBitmap = pComponent->GetImage();

	if( NULL == pBitmap )
	{
		return;
	}

	// Get image dimension.
	BITMAP bm;
	pBitmap->GetBitmap( &bm );

	// Get reference point of component (point that will match the anchoring point).
	pair<int, int> ptRef = pComponent->GetCoord( pAnchor->GetConnectType() );

	pair<int, int> *ptVirtualPoint = new pair<int, int>();
	ptVirtualPoint->first = ptPoint->first;
	ptVirtualPoint->second = ptPoint->second;

	// Apply flip if clockWise, done on the pEnBitmap before DC rotation.
	if( true == pAnchor->IsClockwise() )
	{
		if( 90 == pAnchor->GetDirection() || 270 == pAnchor->GetDirection() )
		{
			// Vertical flow, thus vertical flip.
			ptVirtualPoint->first = bm.bmWidth - 1 - ptVirtualPoint->first;
			ptRef.first = bm.bmWidth - 1 - ptRef.first;

		}

		if( 0 == pAnchor->GetDirection() || 180 == pAnchor->GetDirection() )
		{
			// Horizontal flow, thus horizontal flip.
			ptVirtualPoint->second = bm.bmHeight - 1 - ptVirtualPoint->second;
			ptRef.second = bm.bmHeight - 1 - ptRef.second;
		}
	}

	//------------------------------------------
	// Retrieve angle in degree
	// +90 because the default south (270°) direction of components must result in no rotation
	int degree = pAnchor->GetDirection() + 90;

	double deg = -degree % 360;
	double radian = deg * M_PI / 180;
	double sine = sin( radian );
	double cosine = cos( radian );
	pair<int,int> Pt = *ptVirtualPoint;
	ptVirtualPoint->first = (long)(Pt.first * cosine - Pt.second * sine + 0.5 );
	ptVirtualPoint->second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );
	Pt = ptRef;
	ptRef.first = (long)( Pt.first * cosine - Pt.second * sine + 0.5 );
	ptRef.second = (long)( Pt.first * sine + Pt.second * cosine + 0.5 );

	// Map Coord in function of the image position.
	MapImagePointToDC( pAnchor, pComponent, &ptRef, *ptVirtualPoint );

	delete pBitmap;

	ptPoint->first = ptVirtualPoint->first;
	ptPoint->second = ptVirtualPoint->second;

	delete ptVirtualPoint;

	return;
}

////////////////////////////////////////////////////////////////////////
/// This function converts a CPoint defined in a image to coordinates 
/// in function of the image position on the current DC
////////////////////////////////////////////////////////////////////////
void CEnBitmapPatchWork::MapImagePointToDC(CAnchorPt* pAnchor, CEnBitmapComponent *pComponent, pair<int,int> *pPtAnchor, pair<int,int> &pt)
{
	if( !pAnchor || !pComponent || !pPtAnchor)
	{
		return;
	}

	//-------------------------------------------
	// Substract Anchor inside Image from Anchor Pt
	pair<int,int> ptAnchor = pAnchor->GetPoint();
	ptAnchor.first -= pPtAnchor->first;
	ptAnchor.second -= pPtAnchor->second;

	//-------------------------------------------
	// Now these coordinates are ready to use on the current DC
	pt.first += ptAnchor.first;
	pt.second += ptAnchor.second;
}

//========================================================================
// Map pPt coordinates to a new space with pAnchor as origin
//========================================================================
BOOL CEnBitmapPatchWork::MapPointToAnchor( pair<int,int> *pAnchor, pair<int,int> *pPt)
{
	if( pAnchor == 0 || pPt == 0)
	{
		return FALSE;
	}

	pPt->first = pPt->first - pAnchor->first;
	pPt->second = pAnchor->second - pPt->second;

	return TRUE;
}

//=========================================================================
// Map pPt coordinates back from pAnchor space coordinates
// Default means [0,0] = Left, Top of the screen
//=========================================================================
BOOL CEnBitmapPatchWork::MapPointToDefault( pair<int,int> *pAnchor, pair<int,int> *pPt)
{
	if( pAnchor == 0 || pPt == 0)
	{
		return FALSE;
	}

	pPt->first = pAnchor->first + pPt->first;
	pPt->second = pAnchor->second - pPt->second;

	return TRUE;
}

double CEnBitmapPatchWork::GetRadian(pair<int,int> *pPt)
{
	if( pPt == 0)
	{
		return 0;
	}
	
	//------------------------------------------
	// Retrieve Angle in radian    
	return atan2( (double)pPt->second, (double)pPt->first);
}

CAnchorPt *CEnBitmapPatchWork::FindAnchoringPt( CAnchorPt::eFunc func )
{
	CAnchorPt *pAnchor = NULL;
	
	for( int i = 0; i < m_ArrayAnchor.GetSize(); i++ )
	{
		pAnchor = (CAnchorPt *)m_ArrayAnchor.GetAt( i );
		
		if( NULL == pAnchor )
		{
			continue;
		}

		// Skip anchoring points as Spline, they are used to draw capillary .
		if( pAnchor->GetConnectType() >= CAnchorPt::Spline )
		{
			continue;
		}

		if( pAnchor->GetFunction() == func )
		{
			return pAnchor;
		}
	}
	
	return NULL;
}
