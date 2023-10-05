#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "Utilities.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"

#include "HydroMod.h"
#include "EnBitmap.h"
#include "EnBitmapPatchWork.h"
#include "DlgInfoSSelDpC.h"
#include "SSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgInfoSSelDpC::CDlgInfoSSelDpC( CProductSelelectionParameters *pclProdSelParams, CWnd *pParent )
	: CDialog( CDlgInfoSSelDpC::IDD, pParent )
{
	m_pclIndSelDpCParams = dynamic_cast<CIndSelDpCParams*>( pclProdSelParams );

	Init();
	m_bInitialized = false;
	m_pParentWnd = pParent;
}

void CDlgInfoSSelDpC::Init()
{
	m_DpCIDPtr = _NULL_IDPTR;
	m_ValveIDPtr = _NULL_IDPTR;
	m_strSectionName = _T("DlgInfoSSelDpC");
	m_eDpStab = eDpStab::DpStabOnBranch;
	m_eMvLoc = eMvLoc::MvLocPrimary;
	m_eDpCLoc = eDpCLoc::DpCLocDownStream;
	m_bKvDpOK = false;
	m_dDpBranch = 0.0;
	m_dKvCv = 0.0;
	m_bForSet = false;
	m_dDpComputedOnMv = 0.0;
	m_dDpFullyOpenedMv = 0.0;
	m_dSettingMv = 0.0;
	m_bDpCSelected = false;
	m_bMvSelected = false;
	m_dDpMin = 0.0;
	m_bValidDplRange = false;
	m_pDpCChar = NULL;
	m_dBPDpC = 0.0;
	m_bAbsoluteBP = false;
	m_strDplRange = _T("");
	m_bSetSchemePictCalled = false;
}

void CDlgInfoSSelDpC::DoDataExchange( CDataExchange *pDX )
{
	CDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CDlgInfoSSelDpC, CDialog )
	ON_WM_MOVE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInfoSSelDpC member functions

BOOL CDlgInfoSSelDpC::Create()
{
	return CDialog::Create( CDlgInfoSSelDpC::IDD );
}

void CDlgInfoSSelDpC::Reset()
{
	SetSchemePict( m_eDpStab, m_eMvLoc, m_eDpCLoc, m_bKvDpOK, m_dDpBranch, m_dKvCv, m_bForSet );
}

void CDlgInfoSSelDpC::SetSchemePict( eDpStab DpStab, eMvLoc MvLoc, eDpCLoc DpCLoc, bool fKvDpOK, double dDpBranch, double dKvCv, bool bForSet )
{
	Init();

	m_eDpStab = (eDpStab)DpStab;
	m_eMvLoc = MvLoc;
	m_eDpCLoc = DpCLoc;
	m_bKvDpOK = fKvDpOK;
	m_dDpBranch = dDpBranch;
	m_dKvCv = dKvCv;
	m_bForSet = bForSet;
	m_DpCIDPtr = _NULL_IDPTR;
	m_ValveIDPtr = _NULL_IDPTR;

	m_bSetSchemePictCalled = true;

	// Invalidate and update window to force the painting of the window
	if (NULL != GetSafeHwnd())
	{
		Invalidate();
		UpdateWindow();
	}
}

void CDlgInfoSSelDpC::UpdateInfoMV( CDB_TAProduct* pDev, double dDpComputedOnMv, double dDpFullyOpenedMv, double dSettingMv )
{
	m_dDpComputedOnMv = dDpComputedOnMv;
	m_dDpFullyOpenedMv = dDpFullyOpenedMv;
	m_dSettingMv = dSettingMv;
	m_ValveIDPtr = pDev->GetIDPtr();
	m_bMvSelected = true;
}

void CDlgInfoSSelDpC::UpdateInfoDpC( CDB_DpController* pDev, double dDpMin, bool fValidDplRange )
{
	if( NULL == pDev )
	{
		m_pDpCChar = NULL;
		return;
	}
	
	m_DpCIDPtr = pDev->GetIDPtr();
	m_dDpMin = dDpMin;
	m_bValidDplRange = fValidDplRange;

	if( NULL != pDev->GetDpCCharacteristic() )
	{
		m_dBPDpC = pDev->GetDpCCharacteristic()->GetProportionalBand( m_dDpMin );
		m_bAbsoluteBP = pDev->GetDpCCharacteristic()->IsProportionalBandAbsolute();
		m_pDpCChar = pDev->GetDpCCharacteristic();
	}

	// Prepare DpL range string (for use in OnPaint).
	m_strDplRange = pDev->GetFormatedDplRange(true).c_str();

	m_bDpCSelected = true;
}

bool CDlgInfoSSelDpC::PrepareImage( CDC &dc, CEnBitmapPatchWork *pEnBmp )
{
	if( NULL == pEnBmp )
	{
		return false;
	}
	
	CDB_CircuitScheme *pCircSch = pEnBmp->FindSSelDpCHydronicScheme( (eMvLoc)m_eMvLoc, m_eDpCLoc, (eDpStab)m_eDpStab, m_bForSet );

	if( NULL == pCircSch )
	{
		return false;
	}

	pEnBmp->GetSSelDpCHydronicScheme( pCircSch, m_ValveIDPtr, m_DpCIDPtr, m_eDpCLoc );

	//********************************************
	// Draw info strings superposed on the bitmap
	// Bitmap will be rotated and resized at the end!
	// so all x,y offset should be applied on the vertical scheme
	//********************************************
	CFont Font;	

	Font.CreateFont( -28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Arial") );

	CFont *pOldFont = dc.SelectObject( &Font );

	// Set the text for the Dp units.
	TCHAR tcPressUnitName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcPressUnitName );

	CDS_TechnicalParameter *pclTechParams = NULL;

	if( NULL != m_pclIndSelDpCParams )
	{
		// We come from a product selection.
		pclTechParams = m_pclIndSelDpCParams->m_pTADS->GetpTechParams();
	}
	else
	{
		// We come from the result page.
		pclTechParams = TASApp.GetpTADS()->GetpTechParams();
	}

	// Hmin calc.
	double dHMin;
	
	if( 0 != pclTechParams->GetDpCHminFullyOpenMv() && eMvLoc::MvLocPrimary == m_eMvLoc )		// Hmin calculated with Dp 
	{
		dHMin = m_dDpFullyOpenedMv + m_dDpBranch + m_dDpMin;			// for fully open Mv (Nordic thing)
	}
	else	// Standard way	
	{
		dHMin = m_dDpComputedOnMv + m_dDpBranch + m_dDpMin;
	}

	const int iMainXOffset = 50;
	CRect Bmprect = pEnBmp->GetSizeImage();
	pEnBmp->ShiftImage( CSize( Bmprect.Width() + 2 * iMainXOffset, Bmprect.Height() ), CSize( iMainXOffset, 0 ) );
	Bmprect = pEnBmp->GetSizeImage();
		
	CSize cs( 0, 0 ); 
	
	//===================
	// Dp stab on branch
	//===================
	if( eDpStab::DpStabOnBranch == m_eDpStab )
	{
		CPoint ptStart, ptStop;
		
		// Fix y position.
		const int iTopY = 50;
		const int iBottomY = Bmprect.Height()-50;

		ptStart.y = ptStop.y = iBottomY;
		ptStop.x = ptStart.x = -1;
		
		// for X position retrieve DpC and Bv anchoring points.
		CAnchorPt *pAnchorDpC, *pAnchorBV;
		pAnchorDpC = pEnBmp->FindAnchoringPt( CAnchorPt::eFunc::DpC );
		pAnchorBV = pEnBmp->FindAnchoringPt( CAnchorPt::eFunc::BV_P );

		if( NULL != pAnchorDpC && NULL != pAnchorBV )
		{
			ptStop.x = pAnchorDpC->GetPoint().first + iMainXOffset;
			ptStart.x = pAnchorBV->GetPoint().first + iMainXOffset;

			CString str, tmpstr;
			
			if( true == m_bMvSelected && true == m_bDpCSelected && true == m_bValidDplRange && true == m_bKvDpOK )
			{
				str = TASApp.LoadLocalizedString( IDS_HMIN );
				str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, dHMin, true );
				
				// Draw Hmin.
				pEnBmp->DrawArrow( ptStart, ptStop );
				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y, 0, str, RGB( 0, 0, 0), &Font );
			}

			// Draw DpL.
			ptStart.y = iTopY;
			ptStop.y = iTopY;
			CPoint xyTxt( 0, 0 );
			str = TASApp.LoadLocalizedString( IDS_DPL );
			tmpstr = TASApp.LoadLocalizedString( IDS_DPBRANCH );

			if( eMvLoc::MvLocPrimary == m_eMvLoc )
			{
				pEnBmp->DrawArrow( ptStart, ptStop );
				str += _T("=") + tmpstr;
				CSize cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y, 0, str, RGB( 0, 0 ,0 ), &Font );
				
				if( true == m_bKvDpOK )
				{
					str = _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS,m_dDpBranch, true );
				}
				else if( true == m_bDpCSelected )
				{
					str = _T("= ") + m_strDplRange;
				}
				else
				{
					str.Empty();
				}
				
				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y + cs.cy, 0, str, RGB( 0, 0, 0), &Font );
			}
			else
			{
				// Dp Branch arrow.
				pEnBmp->DrawArrow( ptStart, ptStop );
				CSize cs = dc.GetTextExtent( tmpstr );
				pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y, 0, tmpstr, RGB( 0, 0, 0 ), &Font );

				// DpL arrow.
				ptStart.y = iTopY + Bmprect.Height() / 2;
				ptStop.y = ptStart.y;
				pEnBmp->DrawArrow( ptStart, ptStop );
				
				if( false == m_bKvDpOK )
				{
					if( true == m_bDpCSelected )
					{
						str += _T("= ") + m_strDplRange;
					}
					else
					{
						str.Empty();
					}
				}
				else if( true == m_bMvSelected )
				{
					str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpBranch + m_dDpComputedOnMv, true );
				}
				else
				{
					str.Empty();
				}

				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y, 0, str, RGB( 0, 0, 0 ), &Font );
			}

			// When the Dp Controller is Downstream text is draw below.
			bool fAbBel = ( eDpCLoc::DpCLocDownStream == m_eDpCLoc );
			
			// Measuring valve Dp.
			if( true == m_bMvSelected )
			{
				CPoint ptMV( 0, 0 );
				ptMV.x = pAnchorBV->GetPoint().first + iMainXOffset;
				ptMV.y = pAnchorBV->GetPoint().second;
				str = TASApp.LoadLocalizedString( IDS_DP );
				str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpComputedOnMv, true );

				if( 0 != pclTechParams->GetDpCHminFullyOpenMv() )
				{
					tmpstr = TASApp.LoadLocalizedString( IDS_SHEETHDR_DPFO );
					str += _T("     ") + tmpstr + _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpFullyOpenedMv, true );
				}

				cs = dc.GetTextExtent( str );
				int iOffset = ( true == fAbBel ) ? -3 * cs.cy : 2 * cs.cy;
				pEnBmp->DrawText( ptMV.x + iOffset, ptMV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

 				str = _T("-");
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( m_ValveIDPtr.MP );

				if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
				{
					str = pTAP->GetValveCharacteristic()->GetSettingString( m_dSettingMv, true );
				}

				cs = dc.GetTextExtent( str );
				iOffset = ( true == fAbBel ) ? -2 * cs.cy : 3 * cs.cy;
				pEnBmp->DrawText( ptMV.x + iOffset, ptMV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
			}

			// Dpmin.
			if( true == m_bDpCSelected )
			{
				CPoint ptDpC( 0, 0 );
				ptDpC.x = pAnchorDpC->GetPoint().first + iMainXOffset;
				ptDpC.y = pAnchorDpC->GetPoint().second;
				
				if( true == m_bValidDplRange )
				{
					str = TASApp.LoadLocalizedString( IDS_DPMIN );
					str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpMin, true );

					cs = dc.GetTextExtent( str );
					int iOffset = ( true == fAbBel ) ? 2 * cs.cy : -3 * cs.cy;
					pEnBmp->DrawText( ptDpC.x + iOffset, ptDpC.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

					if( true == m_bKvDpOK && true == m_bMvSelected )
					{
						str = _T("-");
						CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( m_DpCIDPtr.MP );

						if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
						{
							double dOpening = 0.0;

							if( eMvLoc::MvLocPrimary == m_eMvLoc )
							{
								dOpening = m_pDpCChar->GetOpening( m_dDpBranch );
							}
							else
							{
								dOpening = m_pDpCChar->GetOpening( m_dDpBranch + m_dDpComputedOnMv);
							}

							str = pTAP->GetValveCharacteristic()->GetSettingString( dOpening, true );
						}

						cs = dc.GetTextExtent( str );
						iOffset = ( true == fAbBel ) ? 3 * cs.cy: -2 * cs.cy;
						pEnBmp->DrawText( ptDpC.x + iOffset, ptDpC.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
					}
				}
			}
		}

		COLORREF colBot = _TAH_ORANGE;
		COLORREF colTop = _TAH_ORANGE;
		pEnBmp->Add_TU_DistrPipes(_T("SCH_PIPESTOPBOTARROWS"),colTop, _T("SCH_PIPESTOPBOTARROWS"), colBot, iMainXOffset );
	}
	else
	{
		//==========================
		// Dp stab on control valve
		//==========================
		CPoint ptStart, ptStop;
		
		// Fix y position.
		const int iTopY = 50;
		const int iBottomY = Bmprect.Height()-50;

		ptStart.y = ptStop.y = iBottomY;
		ptStop.x = ptStart.x = -1;
		const int xOffset = 75;
		const int yOffset = ( m_eDpCLoc == eDpCLoc::DpCLocDownStream ) ? 75 : -75;
		CString str = _T("T");
		cs = dc.GetTextExtent( str );
		CArray <int> iPos;
		iPos.SetSize( 5 );

		for( int i = 0; i < iPos.GetSize(); i++ )
		{
			iPos[i] = cs.cy * ( i + 3 );
		}

		// for X position retrieve DpC and Bv anchoring points .
		CAnchorPt *pAnchorDpC, *pAnchorBV, *pAnchorCV;
		pAnchorDpC = pEnBmp->FindAnchoringPt( CAnchorPt::eFunc::DpC );
		pAnchorBV = pEnBmp->FindAnchoringPt( CAnchorPt::eFunc::BV_P );
		pAnchorCV = pEnBmp->FindAnchoringPt( CAnchorPt::eFunc::ControlValve );

		if( NULL != pAnchorDpC && NULL != pAnchorBV &&  NULL != pAnchorCV )
		{
			// Set the KvCv string to Kv or Cv
			CString strKvCv;

			if( pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				strKvCv = TASApp.LoadLocalizedString( IDS_CV );
			}
			else
			{
				strKvCv = TASApp.LoadLocalizedString( IDS_KV );
			}

			CPoint ptMV( pAnchorBV->GetPoint().first + iMainXOffset, pAnchorBV->GetPoint().second );
			CPoint ptCV( pAnchorCV->GetPoint().first + iMainXOffset, pAnchorCV->GetPoint().second );
			CPoint ptDpC( pAnchorDpC->GetPoint().first + iMainXOffset, pAnchorDpC->GetPoint().second );
			CString str;

			if( true == m_bMvSelected )
			{
				if( 0 != pclTechParams->GetDpCHminFullyOpenMv() && eMvLoc::MvLocPrimary == m_eMvLoc )
				{
					str = TASApp.LoadLocalizedString( IDS_SHEETHDR_DPFO );
					str += _T(" =");

					str += (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpFullyOpenedMv, true );
					cs = dc.GetTextExtent( str );
					pEnBmp->DrawText( ptMV.x - iPos[4], ptMV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
				}

				str = TASApp.LoadLocalizedString( IDS_DP );
				str += _T("=");
				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( ptMV.x - iPos[3], ptMV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

				str = (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpComputedOnMv, true );
				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( ptMV.x - iPos[2], ptMV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

				str = _T("-");
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( m_ValveIDPtr.MP );

				if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
				{
					str = pTAP->GetValveCharacteristic()->GetSettingString( m_dSettingMv, true );
				}

				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( ptMV.x - iPos[1], ptMV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
			}

			// Kv exist, show DP(L) above the control valve
			if( true == m_bKvDpOK )
			{
				// DP(L) above the control valve.
				str = TASApp.LoadLocalizedString( ( eMvLoc::MvLocPrimary == m_eMvLoc ) ? IDS_DPL : IDS_DP );
				str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpBranch , true );
				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( ptCV.x - iPos[3], ptCV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
				
				// Kv value.
				str = strKvCv + _T("=") + (CString)WriteCUDouble( _C_KVCVCOEFF, m_dKvCv ); 
				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( ptCV.x - iPos[2], ptCV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

				// Authority calculated based on the SP obtained with the hypothesis H=Hmin.
				if( true == m_bMvSelected && true == m_bDpCSelected && true == m_bValidDplRange && true == m_bKvDpOK )
				{
					str = TASApp.LoadLocalizedString( IDS_AUTHOR );
					double dAuth = 0.0;

					if( eMvLoc::MvLocPrimary == m_eMvLoc )
					{
						if( true == m_bAbsoluteBP )
						{
							dAuth = m_dDpBranch / ( m_dDpBranch + m_dBPDpC );
						}
						else
						{
							dAuth = 1.0 / ( 1.0 + m_dBPDpC );
						}
					}
					else
					{
						if( true == m_bAbsoluteBP )
						{
							dAuth = m_dDpBranch / ( m_dDpBranch + m_dDpComputedOnMv + m_dBPDpC );
						}
						else
						{
							dAuth = m_dDpBranch / ( ( 1.0 + m_dBPDpC ) * ( m_dDpBranch + m_dDpComputedOnMv ) );
						}
					}
					
					str += _T(">") + (CString)WriteDouble( dAuth , 2, 2 );
					cs = dc.GetTextExtent( str );
					pEnBmp->DrawText( ptCV.x - iPos[1], ptCV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

					// Hmin Arrow between Mv and DpC.
					ptStart.x = ptMV.x + iMainXOffset + xOffset;
					ptStart.x += (eMvLoc::MvLocPrimary == m_eMvLoc) ? 0 : xOffset;
					ptStart.y = ptMV.y + yOffset;
					ptStop.x = ptStart.x;
					ptStop.y = ptDpC.y - yOffset;
					pEnBmp->DrawArrow( ptStart, ptStop );
					
					str = TASApp.LoadLocalizedString( IDS_HMIN );
					str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, dHMin, true );
					int iMidArrow = ( ptStart.y > ptStop.y ) ? ptStart.y - ( ptStart.y - ptStop.y ) / 2 : ptStop.y - ( ptStop.y - ptStart.y ) / 2;
					pEnBmp->DrawText( ptStart.x + 2, iMidArrow + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
				}
			}
			else if( true == m_bDpCSelected)
			{ 
				// Kv doesn't exist.
				if( eMvLoc::MvLocPrimary == m_eMvLoc )
				{
					// DpL = DpRange.
					str = TASApp.LoadLocalizedString( IDS_DPL );
					str += _T("=");
					cs = dc.GetTextExtent( str );
					pEnBmp->DrawText( ptCV.x - iPos[3], ptCV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

					str = m_strDplRange;
					cs = dc.GetTextExtent( str );
					pEnBmp->DrawText( ptCV.x - iPos[2], ptCV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
				}
			}

			if( true == m_bDpCSelected )
			{
				if( eMvLoc::MvLocSecondary == m_eMvLoc)
				{	
					// DpL arrow between MV and CV.
					ptStart.x = ptMV.x + iMainXOffset + xOffset;
					ptStart.y = ptMV.y + yOffset;
					ptStop.x = ptStart.x;
					ptStop.y = ptCV.y - yOffset;
					pEnBmp->DrawArrow( ptStart, ptStop );
					str = TASApp.LoadLocalizedString( IDS_DPL );
					str += _T("=") + m_strDplRange;
					int iMidArrow = ( ptStart.y > ptStop.y ) ? ptStart.y - ( ptStart.y - ptStop.y ) / 2 : ptStop.y - ( ptStop.y - ptStart.y ) / 2;
					pEnBmp->DrawText( ptStart.x + 2, iMidArrow+cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
				}
				
				if( true == m_bValidDplRange )
				{
					str = TASApp.LoadLocalizedString( IDS_DPMIN );
					str += _T("=");
					cs = dc.GetTextExtent( str );
					pEnBmp->DrawText( ptDpC.x - iPos[3], ptDpC.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

					str = (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpMin ,true );
					cs = dc.GetTextExtent( str );
					pEnBmp->DrawText(ptDpC.x - iPos[2], ptDpC.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );

					if( true == m_bKvDpOK )
					{
						double dSetting = 0.0;

						if( eMvLoc::MvLocPrimary == m_eMvLoc )
						{
							dSetting = m_pDpCChar->GetOpening( m_dDpBranch );
						}
						else
						{
							if( 0 == m_dDpComputedOnMv && m_dDpBranch < m_pDpCChar->GetDplmin() )
							{
								// STAD has not yet been chosen. Thus, we don't know what is the Dp to stabilize (Dp branch (or DpCv) + DpMv). 
								// But, we know that we must choose at least a minimal pressure drop that DpC can stabilize.
								// If 'm_dDpBranch' is above Dpl min, no problem, we can call 'GetOpening' with 'm_dDpBranch'.
								// But if 'm_dDpBranch' is below Dpl min, in debug mode we will have an ASSERT. Thus, to avoid that, let's assume
								// that opening must be computed for the 'm_dDpBranch' + dDpMv to reach Dplmin.
								dSetting = m_pDpCChar->GetOpening( m_pDpCChar->GetDplmin() );
							}
							else
							{
								dSetting = m_pDpCChar->GetOpening( m_dDpBranch + m_dDpComputedOnMv);
							}
						}

						str = m_pDpCChar->GetSettingString( dSetting, true );
						
						cs = dc.GetTextExtent( str );
						pEnBmp->DrawText( ptDpC.x - iPos[1], ptDpC.y + cs.cx / 2, 270, str, RGB( 0, 0, 0 ), &Font );
					}
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_DPLRANGE );
					cs = dc.GetTextExtent( str );
					pEnBmp->DrawText(ptDpC.x - iPos[3], ptDpC.y + cs.cx / 2, 270, str, RGB( 255, 0, 0 ), &Font );
				}
			}
		}

		COLORREF colBot = _TAH_ORANGE;
		COLORREF colTop = _TAH_ORANGE;
		pEnBmp->Add_TU_DistrPipes( _T("SCH_ONEPIPETOPBOTARROW"), colTop, _T("SCH_ONEPIPETOPBOTARROW"), colBot, iMainXOffset );
	}
	
	dc.SelectObject( pOldFont );									// Restore GDI
	Font.DeleteObject(); 

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInfoSSelDpC message handlers

BOOL CDlgInfoSSelDpC::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGINFOSSELB_CAPTION );
	SetWindowText( str );
	str.Empty();

	// Set the window position to the last stored position in registry.
	// If window position is not yet stored in the registry, the window is centered by default in the mainframe rect.
	CRect apprect, rect;
	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
	GetWindowRect( &rect );
	int x = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );

	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// If the window is placed on a screen that is currently deactivated,
	// center the application into the main active screen
	HMONITOR hMonitor = NULL;
	GetWindowRect( &rect );
    hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );

	if( INVALID_HANDLE_VALUE == hMonitor )
	{
		CenterWindow();
	}

	m_bInitialized = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgInfoSSelDpC::OnDestroy() 
{
	CDialog::OnDestroy();
	m_bInitialized = false;
	m_bDpCSelected = false;
	m_bMvSelected = false;
}

void CDlgInfoSSelDpC::OnOK()		// not really a message handler
{
	if( NULL != m_pParentWnd )
	{
		// modeless case -- do not call base class OnOK
		UpdateData( TRUE );
		m_pParentWnd->PostMessage( WM_USER_DESTROYDIALOGINFOSSELDPC, IDOK );
	}
	else
	{
		CDialog::OnOK(); // modal case
	}
}

void CDlgInfoSSelDpC::OnCancel()	// not really a message handler
{
	if( NULL != m_pParentWnd )
	{
		// modeless case -- do not call base class OnCancel
		m_pParentWnd->PostMessage( WM_USER_DESTROYDIALOGINFOSSELDPC, IDCANCEL );
	}
	else
	{
		CDialog::OnCancel(); // modal case
	}
}

void CDlgInfoSSelDpC::OnMove( int x, int y )
{
	CDialog::OnMove( x, y );
	
	// modeless case -- stores window position in registry
	if( NULL != m_pParentWnd && true == m_bInitialized )
	{
		CRect rect;
		GetWindowRect( &rect );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("ScreenYPos"), y );
	}
}

void CDlgInfoSSelDpC::OnPaint() 
{
	if( false == m_bSetSchemePictCalled )
	{
		return;
	}

	CPaintDC dc( this ); // device context for painting

	// Create the CEnBitmap.
	CEnBitmapPatchWork EnBmp;
	
	if( false == PrepareImage( dc, &EnBmp ) )
	{
		EnBmp.DeleteObject();
		return;
	}

	EnBmp.RotateImage( 90 );
	
	// Get client area of the dialog.
	CRect dcRect;
	GetClientRect( &dcRect );
	EnBmp.ResizeImage( CSize( dcRect.Width(), dcRect.Height() ) );
	EnBmp.DrawOnDC( &dc, dcRect, false) ;

	// Do not call CDialog::OnPaint() for painting messages
}
