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
#include "DlgInfoSSelDpCBCV.h"
#include "SSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 
CDlgInfoSSelDpCBCV::CDlgInfoSSelDpCBCV( CProductSelelectionParameters *pclProdSelParams, CWnd *pParent )
	: CDialog( CDlgInfoSSelDpCBCV::IDD, pParent )
{
	m_pclIndSelDpCBCVParams = dynamic_cast<CIndSelDpCBCVParams*>( pclProdSelParams );

	Init();
	m_bInitialized = false;
	m_pParentWnd = pParent;
}

CDlgInfoSSelDpCBCV::~CDlgInfoSSelDpCBCV()
{
}

void CDlgInfoSSelDpCBCV::Init()
{
	m_DpCBCVIDPtr = _NULL_IDPTR;
	m_STSIDPtr = _NULL_IDPTR;
	m_strSectionName = _T("DlgInfoSSelDpCBCV");
	m_bWithSTS = true;
	m_dFlow = 0.0;
	m_dRho = 0.0;
	m_dDpMin = 0.0;
	m_dDpBranch = 0.0;
	m_dDpFullyOpenedSTS = 0.0;
	m_bDpCBCVSelected = false;
	m_bSTSSelected = false;
	m_bValidDplRange = false;
	m_pDpCBCVChar = NULL;
	m_strDplRange = _T("");
	m_bSetSchemePictCalled = false;
}

void CDlgInfoSSelDpCBCV::DoDataExchange( CDataExchange *pDX )
{
	CDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CDlgInfoSSelDpCBCV, CDialog )
	ON_WM_MOVE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInfoSSelDpCBCV member functions

BOOL CDlgInfoSSelDpCBCV::Create()
{
	return CDialog::Create( CDlgInfoSSelDpCBCV::IDD );
}

void CDlgInfoSSelDpCBCV::Reset()
{
	SetSchemePict( m_dDpBranch, m_bWithSTS );
}

void CDlgInfoSSelDpCBCV::SetSchemePict( double dDpBranch, bool bWithSTS )
{
	Init();

	m_bWithSTS = bWithSTS;
	m_dDpBranch = dDpBranch;
	m_DpCBCVIDPtr = _NULL_IDPTR;
	m_STSIDPtr = _NULL_IDPTR;

	m_bSetSchemePictCalled = true;

	// Invalidate and update window to force the painting of the window.
	if( NULL != GetSafeHwnd() )
	{
		Invalidate();
		UpdateWindow();
	}
}

void CDlgInfoSSelDpCBCV::UpdateInfoDpCBCV( CDB_DpCBCValve *pclDpCBCValve, double dDpMin, bool bValidDplRange, double dFlow, double dRho )
{
	if( NULL == pclDpCBCValve )
	{
		m_DpCBCVIDPtr = _NULL_IDPTR;
		m_pDpCBCVChar = NULL;
		return;
	}

	m_DpCBCVIDPtr = pclDpCBCValve->GetIDPtr();
	m_dFlow = dFlow;
	m_dRho = dRho;
	m_dDpMin = dDpMin;
	m_bValidDplRange = bValidDplRange;

	if( NULL != pclDpCBCValve->GetDpCBCVCharacteristic() )
	{
		m_pDpCBCVChar = pclDpCBCValve->GetDpCBCVCharacteristic();
	}

	// Prepare DpL range string (for use in OnPaint).
	m_strDplRange = pclDpCBCValve->GetFormatedDplRange( dFlow, dRho, true ).c_str();

	m_bDpCBCVSelected = true;
}

void CDlgInfoSSelDpCBCV::UpdateInfoSTS( CDB_ShutoffValve *pclShutoffValve, double dDpFullyOpenedSTS )
{
	if( NULL == pclShutoffValve )
	{
		m_STSIDPtr = _NULL_IDPTR;
		return;
	}

	m_dDpFullyOpenedSTS = dDpFullyOpenedSTS;
	m_STSIDPtr = pclShutoffValve->GetIDPtr();
	m_bSTSSelected = true;
}

bool CDlgInfoSSelDpCBCV::PrepareImage( CDC &dc, CEnBitmapPatchWork *pEnBmp )
{
	if( NULL == pEnBmp )
	{
		return false;
	}

	// For the moment with TA-COMPACT-DP we have only the possibility to have STS on the downstream.
	ShutoffValveLoc eShutoffValveLocation = ( true == m_bWithSTS ) ? ShutoffValveLocReturn : ShutoffValveLocNone;
	CDB_CircuitScheme *pCircuitScheme = pEnBmp->FindSSelDpCBCVHydronicScheme( DpStabOnBranch, eShutoffValveLocation );
	
	if( NULL == pCircuitScheme )
	{
		return false;
	}

	pEnBmp->GetSSelDpCBCVHydronicScheme( pCircuitScheme, m_DpCBCVIDPtr, m_STSIDPtr );

	//********************************************
	// Draw info strings superposed on the bitmap
	// Bitmap will be rotated and resized at the end!
	// so all x,y offset should be applied on the vertical scheme
	//********************************************
	CFont Font;	

	Font.CreateFont( -28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial") );

	CFont *pOldFont = dc.SelectObject( &Font );

	// Set the text for the Dp units.
	TCHAR tcPressUnitName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcPressUnitName );
	
	// Hmin calc.
	double dHMin = 0.0;
	CDB_DpCBCValve *pclDpCBCValve = dynamic_cast<CDB_DpCBCValve*>( m_DpCBCVIDPtr.MP ) ;
	CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve*>( m_STSIDPtr.MP );

	if( NULL != pclDpCBCValve && m_dFlow > 0.0 )
	{
		dHMin = pclDpCBCValve->GetHMin( m_dFlow, m_dRho );
	}

	// We add Dp STS fully open if exist
	dHMin += m_dDpFullyOpenedSTS;
	
	const int iMainXOffset = 50;
	CRect Bmprect = pEnBmp->GetSizeImage();

	pEnBmp->ShiftImage( CSize( Bmprect.Width() + 2 * iMainXOffset, Bmprect.Height() ), CSize( iMainXOffset, 0 ) );
	Bmprect = pEnBmp->GetSizeImage();
		
	CSize cs(0,0); 
	CPoint ptStart, ptStop;
	
	// Fix y position.
	const int TopY = 50;
	const int BottomY = Bmprect.Height() - 50;

	ptStart.y = ptStop.y = BottomY;
	ptStop.x = ptStart.x = -1;
	
	// for X position retrieve DpCBCV and ShutoffValve anchoring points.
	CAnchorPt *pAnchorDpCBCV = pEnBmp->FindAnchoringPt( CAnchorPt::eFunc::DPCBCV );
	CAnchorPt *pAnchorSV = pEnBmp->FindAnchoringPt( CAnchorPt::eFunc::ShutoffValve );

	if( NULL != pAnchorDpCBCV && NULL != pAnchorSV )
	{
		ptStop.x = pAnchorDpCBCV->GetPoint().first + iMainXOffset;
		ptStart.x = pAnchorSV->GetPoint().first + iMainXOffset;

		CString str, tmpstr;

		if( true == m_bDpCBCVSelected && true == m_bValidDplRange )
		{
			str = TASApp.LoadLocalizedString( IDS_HMIN );
			str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, dHMin, true);
			
			// Draw Hmin.
			pEnBmp->DrawArrow( ptStart, ptStop );
			cs = dc.GetTextExtent( str );
			pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y, 0, str, RGB( 0, 0, 0 ), &Font );
		}

		// Draw DpL.
		ptStart.y = TopY;
		ptStop.y = TopY;
		CPoint xyTxt( 0, 0);
		pEnBmp->DrawArrow( ptStart, ptStop );
		CSize cs;

		str = TASApp.LoadLocalizedString( IDS_DPL );

		if( true == m_bDpCBCVSelected )
		{
			if( m_dDpBranch > 0.0 )
			{
				str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpBranch, true );
				
				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y, 0, str, RGB( 0, 0, 0), &Font );
			}
			else
			{
				str += _T("=") + TASApp.LoadLocalizedString( IDS_DPBRANCH );

				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y, 0, str, RGB( 0, 0, 0), &Font );

				str = m_strDplRange;
				cs = dc.GetTextExtent( str );
				pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y + cs.cy, 0, str, RGB( 0, 0, 0), &Font );
			}
		}
		else
		{
			str += _T("=") + TASApp.LoadLocalizedString( IDS_DPBRANCH );

			cs = dc.GetTextExtent( str );
			pEnBmp->DrawText( Bmprect.Width() / 2 - cs.cx / 2, ptStart.y, 0, str, RGB( 0, 0, 0), &Font );
		}

		// Shutoff valve Dp.
		if( true == m_bSTSSelected )
		{
			CPoint ptSTS( 0, 0 );
			ptSTS.x = pAnchorSV->GetPoint().first + iMainXOffset;
			ptSTS.y = pAnchorSV->GetPoint().second;
			str = TASApp.LoadLocalizedString( IDS_DP );
			str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpFullyOpenedSTS, true );

			cs = dc.GetTextExtent( str );
			int offset = 3 * cs.cy;
			pEnBmp->DrawText( ptSTS.x + offset, ptSTS.y + cs.cx / 2, 270, str, RGB( 0, 0, 0), &Font );
		}

		// Dpmin
		if( true == m_bDpCBCVSelected && m_dDpMin > 0.0 )
		{
			CPoint ptDpCBCV( 0, 0 );
			ptDpCBCV.x = pAnchorDpCBCV->GetPoint().first + iMainXOffset;
			ptDpCBCV.y = pAnchorDpCBCV->GetPoint().second;

			if( true == m_bValidDplRange )
			{
				str = TASApp.LoadLocalizedString( IDS_DPMIN );
				str += _T("=") + (CString)WriteCUDouble( _U_DIFFPRESS, m_dDpMin, true );

				cs = dc.GetTextExtent( str );
				int offset = ( -4 * cs.cy ) - 10;
				pEnBmp->DrawText( ptDpCBCV.x + offset, ptDpCBCV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0), &Font );

				if( true == m_bSTSSelected )
				{
					double dSetting = m_pDpCBCVChar->GetSetting( m_dDpMin, m_dFlow, m_dRho );
					str = m_pDpCBCVChar->GetSettingString( dSetting, true );
					cs = dc.GetTextExtent( str );
					offset = ( -3 * cs.cy ) - 10;
					pEnBmp->DrawText( ptDpCBCV.x + offset, ptDpCBCV.y + cs.cx / 2, 270, str, RGB( 0, 0, 0), &Font );
				}
			}
		}
	}

	COLORREF colBot = _TAH_ORANGE;
	COLORREF colTop = _TAH_ORANGE;
	pEnBmp->Add_TU_DistrPipes( _T("SCH_PIPESTOPBOTARROWS"), colTop, _T("SCH_PIPESTOPBOTARROWS"), colBot, iMainXOffset );

	dc.SelectObject( pOldFont );
	Font.DeleteObject(); 

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInfoSSelDpCBCV message handlers

BOOL CDlgInfoSSelDpCBCV::OnInitDialog() 
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
	// center the application into the main active screen.
	HMONITOR hMonitor = NULL;
	GetWindowRect( &rect );
    hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );

	if( NULL == hMonitor )
	{
		CenterWindow();
	}

	m_bInitialized = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgInfoSSelDpCBCV::OnDestroy() 
{
	CDialog::OnDestroy();
	m_bInitialized = false;
	m_bDpCBCVSelected = false;
	m_bSTSSelected = false;
}

void CDlgInfoSSelDpCBCV::OnOK()		// not really a message handler
{
	if( NULL != m_pParentWnd )
	{
		// modeless case -- do not call base class OnOK
		UpdateData( TRUE );
		m_pParentWnd->PostMessage( WM_USER_DESTROYDIALOGINFOSSELDPCBCV, IDOK );
	}
	else
	{
		CDialog::OnOK(); // modal case
	}
}

void CDlgInfoSSelDpCBCV::OnCancel()	// not really a message handler
{
	if( m_pParentWnd != NULL )
	{
		// modeless case -- do not call base class OnCancel
		m_pParentWnd->PostMessage( WM_USER_DESTROYDIALOGINFOSSELDPCBCV, IDCANCEL );
	}
	else
	{
		CDialog::OnCancel(); // modal case
	}
}

void CDlgInfoSSelDpCBCV::OnMove( int x, int y )
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

void CDlgInfoSSelDpCBCV::OnPaint() 
{
	if( false == m_bSetSchemePictCalled )
	{
		return;
	}

	CPaintDC dc( this );

	// Create the 'CEnBitmap'.
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
	EnBmp.DrawOnDC( &dc, dcRect, false );
}
