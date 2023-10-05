#include "stdafx.h"
#include "afxpriv.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "TASelectDoc.h"
#include "Global.h"
#include "Utilities.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "SheetHMCalc.h"
#include "ssheetpanelcirc2.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "SSheetLDSumm.h"
#include "SSheetLogData.h"
#include "SSheetQuickMeas.h"
#include "HMPipes.h"
#include "DlgCreateModule.h"
#include "DlgHMDirSel.h"
#include "DlgLeftTabProject.h"
#include "EnBitmap.h"
#include "EnBitmapPatchWork.h"
#include "RViewSelProd.h"
#include "RViewProj.h"
#include "RViewHMCalc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRViewProj *pRViewProj = NULL;

IMPLEMENT_DYNCREATE( CRViewProj, CRScrollView )

CRViewProj::CRViewProj()
{
	ClearPrintingFlags();
	m_pSheetHMCalc = NULL;
	m_pLoggedData = NULL;
	m_pTab = NULL;
	m_pHM = NULL;
	m_pTabrecursive = NULL;
	m_pHMItemCut = NULL;
	m_HMID = _T("");
	m_SheetProjRect.SetRectEmpty();
	m_PageRect.SetRectEmpty();
	m_BmpRect.SetRectEmpty();
	m_RankList = CRank( false );
	m_fRClickOnSelectedLine = false;
	
#ifdef DEBUG_PRINTING
	m_fPrintPreviewMode = false;
	m_hDCMemory = NULL;
	m_recPrinterDP.SetRectEmpty();
	m_recScreenDP.SetRectEmpty();
	m_hBitmap = NULL;
	m_hBitmapOld = NULL;
#endif

	m_iPage = -1;
	m_fShort = false;
	m_pHMPage = NULL;
	m_mapArHMPages.RemoveAll();
	m_mapCrossRefReportedHM.RemoveAll();
	m_node = -1;
	m_page = -1;
	m_pCurHM = (LPARAM)0;
	m_TreePrintRect.SetRectEmpty();
	m_bTreePrintPreview = false;
	m_iTabX = -1;
	m_iTabY = -1;
	m_iMm = -1;

	pRViewProj = this;
}

CRViewProj::~CRViewProj()
{
	if( NULL != m_pSheetHMCalc )
	{
		delete m_pSheetHMCalc;
	}

	pRViewProj = NULL;
}

BEGIN_MESSAGE_MAP( CRViewProj, CRScrollView )
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
END_MESSAGE_MAP()

LRESULT CRViewProj::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	m_pTADS = TASApp.GetpTADS();

	// Retrieve last selected Page Setup from TADS
	m_pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != m_pTADSPageSetup );
	
	m_pHM = NULL;
	m_HMID = _T("");
	m_Xfactor = 100;
	return 0;
}

int CRViewProj::DrawHMInfo( CDC* pDC, CDS_HydroMod *pHM, int iXPos, int iYPos, int iWidth, bool fDrawNow )
{
	if( NULL == pHM )
		return iYPos;

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	CPen pen;
	pen.CreatePen( PS_SOLID, 1, RGB( 0, 0, 0 ) );
	CPen *pOldPen = pDC->SelectObject( &pen );
	CFont *pOldFont;
	if( TRUE == pDC->IsPrinting() )
		pOldFont = pDC->SelectObject( &m_PrintTextBold );
	else
		pOldFont = pDC->SelectObject( &m_TextBold );

	// Draw Upper Line
	if( true == fDrawNow )
	{
		pDC->MoveTo( CPoint( iXPos, iYPos ) );
		pDC->LineTo( CPoint( iXPos + iWidth, iYPos ) );
	}
	iYPos += ( m_iLineSpace * m_Xfactor / 100 );
		
	// Draw all Titles
	CString str; 
	int iDeltaX = iWidth / 4;

	// keep a trace of vertical position, we will use it when we draw text associated to titles
	int iYPosTemp = iYPos;
	int iXPosTemp = iXPos;

	// First Line, left text
	str = TASApp.LoadLocalizedString( IDS_PLANT );
	str += CString( _T(":" ) );
	DrawTextLine( pDC, iXPosTemp, iYPosTemp, iDeltaX, str, fDrawNow );	
	
	// right text start at the middle of the page
	str = TASApp.LoadLocalizedString( IDS_MODULEDESCRIPTION );
	str += CString( _T(":" ) );
	iXPosTemp = iXPos + 7 * iDeltaX / 4;
	iYPosTemp = DrawTextLine( pDC, iXPosTemp, iYPosTemp, iDeltaX, str, fDrawNow );	
	
	// 2nd line
	str = TASApp.LoadLocalizedString( IDS_PARENTMODULE );
	str += CString( _T(":" ) );
	iXPosTemp = iXPos;
	DrawTextLine( pDC, iXPosTemp, iYPosTemp, iDeltaX, str, fDrawNow );	
	str = TASApp.LoadLocalizedString( IDS_NBROFVALVES );
	str += CString( _T(":" ) );
	iXPosTemp = iXPos + 7 * iDeltaX / 4;
	iYPosTemp = DrawTextLine( pDC, iXPosTemp, iYPosTemp, iDeltaX, str, fDrawNow );	
	
	// 3rd line
	int iYPosTemp2 = iYPosTemp;
	bool fCBITransfer = false;	// !!! TODO : Implement DateTime in CDS_ProjectParams,
								// set this date time during transfer and take it there
	if( true == fCBITransfer )
	{
		str = TASApp.LoadLocalizedString( IDS_LASTCBITRANSFER );
		str += CString( _T(":" ) );
		iXPosTemp = iXPos;
		iYPosTemp = DrawTextLine( pDC, iXPosTemp, iYPosTemp, iDeltaX, str, fDrawNow );
	}

	double dBalIndex;
	bool fBalIndex = pHM->GetBalancingIndex( &dBalIndex );
	if( true == fBalIndex )
	{
		str = TASApp.LoadLocalizedString( IDS_BALANCINGINDEX);
		str += CString( _T(":" ) );
		iXPosTemp = iXPos + 7 * iDeltaX / 4;
		iYPosTemp2 = DrawTextLine( pDC, iXPosTemp, iYPosTemp2, iDeltaX, str, fDrawNow );
	}
	iYPosTemp = max( iYPosTemp, iYPosTemp2 );

	// Draw module info
	// Line 1 Site and module description
	if( TRUE == pDC->IsPrinting() )
		pDC->SelectObject( &m_PrintText11 );
	else
		pDC->SelectObject( &m_Text11 );
	
	str = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name ); 

	iXPosTemp = iXPos + 3 * iDeltaX / 4;
	DrawTextLine( pDC, iXPosTemp, iYPos, iDeltaX, str, fDrawNow );	
	str = pHM->GetDescription();
	if( true == str.IsEmpty() )
		str = _T("-");
	iXPosTemp = iXPos + 11 * iDeltaX / 4;
	iYPos = DrawTextLine( pDC, iXPosTemp, iYPos, iDeltaX, str, fDrawNow );	
	
	// Line 2 Parent module and nbr of valves
	if( pHM->GetLevel() > 0 )
		str = ( (CDS_HydroMod*)( pHM->GetIDPtr().PP ) )->GetHMName(); 
	else
		str = _T("-");
	iXPosTemp = iXPos + 3 * iDeltaX / 4;
	DrawTextLine( pDC, iXPosTemp, iYPos, iDeltaX, str, fDrawNow );	
	str.Format( _T("%d"), pHM->GetCount( ) ); 
	iXPosTemp = iXPos + 11 * iDeltaX / 4;
	iYPos = DrawTextLine( pDC, iXPosTemp, iYPos, iDeltaX, str, fDrawNow );	
	iYPosTemp = iYPosTemp2 = iYPos;
	
	// Line 3 Last CBI transfer and balancing index
	if( true == fCBITransfer )
	{
		CTimeUnic dtu; 
		dtu.CTime::CTime( 0 );// cfr. C++ Primer page 693
		str = dtu.Format( IDS_LOGDATETIME_FORMAT );
		iXPosTemp = iXPos + 3 * iDeltaX / 4;
		iYPosTemp = DrawTextLine( pDC, iXPosTemp, iYPosTemp, iDeltaX, str, fDrawNow );
	}

	if( true == fBalIndex )
	{
		str = WriteDouble( 100.0 * dBalIndex, 2, 1 ) + (CString)_T(" %");
		iXPosTemp = iXPos + 11 * iDeltaX / 4;
		iYPosTemp2 = DrawTextLine( pDC, iXPosTemp, iYPosTemp2, iDeltaX, str, fDrawNow );
	}
	iYPos = max( iYPosTemp, iYPosTemp2 );

	iYPos += ( m_iLineSpace * m_Xfactor / 100 );
	
	// Draw bottom Line
	if( true == fDrawNow )
	{
		pDC->MoveTo( CPoint( iXPos, iYPos ) );
		pDC->LineTo( CPoint( iXPos + iWidth, iYPos ) );
	}

	pDC->SelectObject( pOldFont );
	pDC->SelectObject( pOldPen );
	pen.DeleteObject();
	
	return iYPos;
}

/////////////////////////////////////////////////////////////////////////////
// CRViewProj diagnostics

#ifdef _DEBUG

void CRViewProj::AssertValid() const
{
	CRScrollView::AssertValid();
}

void CRViewProj::Dump( CDumpContext& dc ) const
{
	CRScrollView::Dump( dc );
}

CTASelectDoc* CRViewProj::GetDocument() // non-debug version is inline
{
	ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CTASelectDoc ) ) );
	return (CTASelectDoc*)m_pDocument;
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRViewProj message handlers

BOOL CRViewProj::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext ) 
{
	if( !CWnd::Create( lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext ) )
		return FALSE;

	return true;
}

void CRViewProj::OnPrepareDC( CDC* pDC, CPrintInfo* pInfo ) 
{
	CRScrollView::OnPrepareDC( pDC, pInfo );
}

///////////////////////////////////////////////////////////////////////////
// Printing functions

void CRViewProj::ChildTabList( CTable *pTab, int &iKey )
{
	if( false == pTab->IsClass( CLASS( CTableHM ) ) && true == ( (CDS_HydroMod *)pTab )->IsaModule() )
	{
		m_RankList.Add( _T(""), iKey, (LPARAM)pTab );
		iKey++;
	}
	
	// Sort module.
	CRank TabList;
	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{	
		if( false == IDPtr.MP->IsClass( CLASS( CTableHM ) ) && true == ( (CDS_HydroMod *)( IDPtr.MP ) )->IsaModule() )
		{
			TabList.Add( _T(""), ( (CDS_HydroMod *)( IDPtr.MP ) )->GetPos(), (LPARAM)IDPtr.MP );
		}
	}
	
	CString str;
	LPARAM lparam;
	for( BOOL fContinue = TabList.GetFirst( str, lparam ); TRUE == fContinue; fContinue = TabList.GetNext( str, lparam ) )
	{
		CTable* pTable = (CTable *)lparam;
		if( true == pTable->IsClass( CLASS( CTableHM ) ) || true == ( (CDS_HydroMod *)pTable)->IsaModule() )
			ChildTabList( (CTable *)pTable, iKey );
	}
}

void CRViewProj::ClearPrintingFlags()
{
	m_fPrintCircuit = false;
	m_fPrintRecursive = false;
	m_fPrintTreeView = false;
	m_fPrintSelProd = false;
	m_fPrintLogDataSumm = false;
	m_fPrintLogData = false;
	m_fPrintQuickMeas = false;
	m_RankList.PurgeAll();
}

void CRViewProj::PrintSelectedProduct( bool fPrint )
{
	if( true == fPrint )
		Print();
	else
	{
		ClearPrintingFlags();
		m_fPrintSelProd = true;
	}
}

void CRViewProj::PrintLogDataSumm( bool fPrint )
{
	if( true == fPrint )
		Print();
	else
	{
		ClearPrintingFlags();
		m_fPrintLogDataSumm = true;
	}
}

void CRViewProj::PrintLogData( bool fPrint )
{
	if( true == fPrint )
		Print();
	else
	{
		ClearPrintingFlags();
		m_fPrintLogData = true;
	}
}

void CRViewProj::PrintQuickMeas( CRank *pList, bool fPrint )
{
	if( true == fPrint )
		Print();
	else
	{
		ClearPrintingFlags();
		m_fPrintQuickMeas = true;
		pList->Transfer( &m_RankList );
	}
}

void CRViewProj::PrintRecursive( CRank *pList, bool fPrint )
{
	if( true == fPrint )
		Print();
	else
	{
		ClearPrintingFlags();
		m_fPrintRecursive = true;
		pList->Transfer( &m_RankList );
	}
}

void CRViewProj::PrintModule( CTable * pTab, bool fRecursive )
{	
	ClearPrintingFlags();
	m_fPrintRecursive = fRecursive;
	if( false == m_fPrintRecursive )
	{
		if( false == pTab->IsClass( CLASS( CTableHM ) ) && true == ( (CDS_HydroMod *)pTab )->IsaModule() )
		{
			m_RankList.Add( _T(""), 1, (LPARAM)pTab );
			Print();
		}
	}
	else
	{
		int iKey = 1;
		ChildTabList( pTab, iKey );
		Print();
	}
}

void CRViewProj::PrintTreeView( CTable *pTab, bool fPrint, bool fShort )
{
	ClearPrintingFlags();
	m_fShort = fShort;
	m_pTab = pTab;
	m_fPrintTreeView = true;
	if( true == fPrint )
		OnFilePrint();
}

CString CRViewProj::FormatPipeAccessoryStr( CPipes *pPipe )
{
	CString strText( TASApp.LoadLocalizedString( IDS_RVPROJ_NOSINGULARITY ) );

	// Description.
	CString strDescription = pPipe->GetSingularitiesDescriptionShort( true, true );
	if( false == strDescription.IsEmpty() )
		strText = strDescription;

	// Input data.
	CString strInputData = pPipe->GetSingularitiesInputDataShort();
	if( false == strInputData.IsEmpty() )
	{
		if( false == strText.IsEmpty() )
			strText += CString( _T("; ") ) + strInputData;
		else
			strText = strInputData;
	}

	// Dp.
	CString strDp = WriteCUDouble( _U_DIFFPRESS, pPipe->GetSingularityTotalDp( true ), true );
	if( false == strDp.IsEmpty() )
	{
		if( false == strText.IsEmpty() )
			strText += CString( _T("; ") ) + TASApp.LoadLocalizedString( IDS_RVPROJ_DP ) + strDp;
		else
			strText = TASApp.LoadLocalizedString( IDS_RVPROJ_DP ) + strDp;
	}

	return strText;
}

// Replace GetTextExtend that return a fake result
CSize CRViewProj::GetTextSize( CDC *pDC, CString str )
{
	CRect TxtRect( 0, 0, 0, 0 );
	// Cannot use GetTextExtent because string could contains \r\n character that is not considered by TextExtent 
	pDC->DrawText( str, &TxtRect, DT_CALCRECT | DT_VCENTER | DT_LEFT | DT_NOPREFIX );
	return ( CSize( TxtRect.Width(), TxtRect.Height() ) );
}

// Draw one HydroMod, return a sized rect at CurXY position 
CRect CRViewProj::DrawHMNode( CDC *pDC, CDS_HydroMod *pHM, CPoint CurXY, bool bDraw )
{
	CString str, str1;
	CTADatabase *pTADB = (CTADatabase *)TASApp.GetpTADB();
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();	

	// Size needed for HM node will be stored into rect
	CRect rect( 0, 0, 0, 0 );
	rect.MoveToXY( CurXY );
	
	// Current position
	CPoint CurPos = CurXY;

	CRect TxtRect( 0, 0, 0, 0 );
	CRect SchSize( 0, 0, 0, 0 );
	CSize sizeText;

	CFont clText, clTextBold;
	_CreateFont( pDC, clText, 8, FW_NORMAL, CString( _T("Arial") ) );
	_CreateFont( pDC, clTextBold, 8, FW_BOLD, CString( _T("Arial") ) );
	CFont *pOldFont = pDC->SelectObject( &clText );
	
	COLORREF OldTextColor = pDC->SetTextColor( m_StringColor );
	int iOldBkMode = pDC->SetBkMode( TRANSPARENT );

	// Distribution pipe are located m_iTabY before HM Name
	for (int k=0; k<2; k++)
	{
		// Distribution Supply Pipe
		CPipes *pDistrPipe = pHM->GetpDistrSupplyPipe();
		
		// Distribution return Pipe
		if (k==1)
			pDistrPipe = pHM->GetpDistrReturnPipe();
			
		if (NULL == pDistrPipe)
			continue;
		if( false == m_fShort && true == bDraw )
		{
			CDB_Pipe *pPipe = (CDB_Pipe *)( pDistrPipe->GetIDPtr().MP );
			str.Empty();
			if( NULL != pPipe && 0.0 != pDistrPipe->GetLength() )
			{
				str = (k==0) ? TASApp.LoadLocalizedString( IDS_RVPROJ_SUPPLYPIPE ) : TASApp.LoadLocalizedString( IDS_RVPROJ_RETURNPIPE );
				str += CString( _T(" ") ) + pPipe->GetName();
				str += CString( _T("; ") ) + WriteCUDouble( _U_LENGTH, pDistrPipe->GetLength(), true ) + _T("; ");
				str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DP );
				str += str1 + WriteCUDouble( _U_DIFFPRESS, pDistrPipe->GetPipeDp(), true ) + _T("; ");
				str1 = TASApp.LoadLocalizedString( IDS_HMCALC_LINDP );
				str += str1 + _T("=") + WriteCUDouble( _U_LINPRESSDROP, pDistrPipe->GetLinDp(), true );
				str += CString( _T(" ( ") ) + pDistrPipe->GetPipeSeries()->GetName() + _T(" )");
			}

			if( pDistrPipe->GetSingularityTotalDp( true ) > 0 )
			{
				if( false == str.IsEmpty() )
					str += CString( _T("\r\n") );

				str += TASApp.LoadLocalizedString( IDS_RVPROJ_SINGULARITY ) + _T(" ") + FormatPipeAccessoryStr( pDistrPipe );
			}

			if( false == str.IsEmpty() )
			{
				sizeText = GetTextSize( pDC, str );

				CurPos.y = CurPos.y -  m_iTabY/2 - sizeText.cy + k*(sizeText.cy);

				TxtRect.MoveToXY( CurPos );
				TxtRect.bottom += sizeText.cy;
				TxtRect.right = m_TreePrintRect.right;
				TxtRect.left -= Getmm2pt( pDC, 2, false );
				pDC->DrawText( str, &TxtRect, DT_VCENTER | DT_LEFT | DT_NOPREFIX );
			}
		}
		CurPos = CurXY;
	}

	// HM Name
	pDC->SelectObject( &clTextBold );
	
	// Size for HM name rectangle is fixed to size of bitmap
	sizeText = GetTextSize(pDC, _T("MMMMMMMMMMM") );
	sizeText.cx = Getmm2pt( pDC, 38, false );
	if( true == bDraw )
	{
		CBrush BGBrush;
		BGBrush.CreateSolidBrush( m_TitleBGColor );
		CBrush *pOldBrush = pDC->SelectObject( &BGBrush );
		pDC->SetTextColor( m_TitleColor );
		CRect rect( CurPos.x, CurPos.y, CurPos.x + sizeText.cx, CurPos.y + sizeText.cy );

		// If we must insert image between module name...
		if( true == m_fShort )
		{
			CRect rectBmp( 0, 0, Getmm2pt( pDC, 3, false ), Getmm2pt( pDC, 3, true ) );
			
			int iBmp = ( true == pHM->IsaModule() ) ? GIF_HMTREE : GIF_CIRCUIT;
			DrawBmp( pDC, CurPos.x, CurPos.y, CString( _T("") ), iBmp, true, &rectBmp, true );

			// Move cursor X from 4 position to right (3 for bmp and 1 for space between bmp and module name)
			CurPos.x += Getmm2pt( pDC, 4, false );
			rect = CRect( CurPos.x, CurPos.y, CurPos.x + sizeText.cx, CurPos.y + sizeText.cy );
		}
		pDC->FillRect( rect, &BGBrush );
		pDC->TextOut( CurPos.x + ( sizeText.cx - GetTextSize( pDC, pHM->GetHMName()).cx ) / 2, CurPos.y, pHM->GetHMName() );
		pDC->SelectObject( pOldBrush );
		BGBrush.DeleteObject();
	}

	// HM Description
	CurPos.x += sizeText.cx + Getmm2pt( pDC, 3, false );
	rect.bottom += sizeText.cy;
	rect.right += sizeText.cx;
	pDC->SelectObject( &clText );
	pDC->SetTextColor( m_StringColor );
	TxtRect = CRect( 0, 0, 0, 0 );
	TxtRect.MoveToXY( CurPos );
	TxtRect.bottom += sizeText.cy;
	TxtRect.right = m_TreePrintRect.right;
	if( true == bDraw )
		pDC->DrawText( pHM->GetDescription(), &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );

	if( true == m_fShort )
	{
		pDC->SelectObject( pOldFont );
		pDC->SetTextColor( OldTextColor );
		pDC->SetBkMode( iOldBkMode );
		clText.DeleteObject();
		clTextBold.DeleteObject();
		return rect;
	}

	// Picture
	// Size for scheme stamp is fixed to 28*22mm
	SchSize.right = Getmm2pt( pDC, 38, false );
	SchSize.bottom = Getmm2pt( pDC, 22, true );
	CurPos.x = CurXY.x;
	CurPos.y = rect.bottom;
	pHM->ResetSchemeIDPtr();
	CDB_CircuitScheme *pSch = (CDB_CircuitScheme *)( pHM->GetSchemeIDPtr().MP );
	
	// Set the CEnBitamp size
	CRect PicRect( 0, 0, 0, 0 );
	PicRect.MoveToXY( CurPos );
	PicRect.right += SchSize.Width() - 1;
	PicRect.bottom += SchSize.Height();

	if( true == bDraw )
	{
		// Create the CEnBitmap
		CEnBitmapPatchWork EnBmp;
		EnBmp.GetHydronicScheme( pHM );
		EnBmp.ResizeImage( 0.25 );
		
		// Rotate and draw
		EnBmp.RotateImage( 90 );
		EnBmp.DrawOnDC( pDC, PicRect );
		EnBmp.DeleteObject();
	}

	rect.top = rect.bottom;
	rect.bottom = PicRect.bottom;
	rect.right = PicRect.right;
	
	if( true == bDraw )
	{
		// Draw a rectangle around picture
		CBrush BlackBrush;
		BlackBrush.CreateSolidBrush( _BLACK );
		pDC->FrameRect( &rect, &BlackBrush );
		BlackBrush.DeleteObject();
	}

	// Draw devices characteristics.
	// Terminal unit.
	TxtRect.left = PicRect.right + Getmm2pt( pDC, 3, false );
	TxtRect.right = m_TreePrintRect.right;
	TxtRect.top = PicRect.top;
	TxtRect.bottom = PicRect.top + sizeText.cy;
	
	// Flow and Hmin.
	if( true == pHM->IsHMCalcMode() )
	{
		str1 = TASApp.LoadLocalizedString( IDS_HMCALC_Q );
		str = str1 + _T("=") + WriteCUDouble( _U_FLOW, pHM->GetQ(), true );
		
		// HYS-1277 : Add possibilty to display Power and DT if QType = PdT.
		if( ( NULL != pHM->GetpTermUnit() ) && ( false == pHM->IsaModule() )
				&& ( CTermUnit::_QType::PdT == pHM->GetpTermUnit()->GetQType() ) )
		{
			str1 = TASApp.LoadLocalizedString( IDS_HMCALC_P );
			str += CString( _T(" (" ) ) + str1 + _T( "=") + WriteCUDouble( _U_TH_POWER, pHM->GetpTermUnit()->GetP(), true );

			str1 = TASApp.LoadLocalizedString( IDS_HMCALC_DT );
			str += CString( _T("; " ) ) + str1 + _T( "=" ) + WriteCUDouble( _U_DIFFTEMP, pHM->GetpTermUnit()->GetDT(), true ) + CString( _T( ")") );
		}
		// HYS-1882 : Add possibilty to display DT if QType = Q.
		else if( ( NULL != pHM->GetpTermUnit() ) && ( false == pHM->IsaModule() )
				&& ( true == pHM->IsDTFieldDisplayedInFlowMode() ) )
		{
			str1 = TASApp.LoadLocalizedString( IDS_HMCALC_DT );
			str += CString( _T( " (" ) ) + str1 + _T( "=" ) + WriteCUDouble( _U_DIFFTEMP, pHM->GetpTermUnit()->GetDTFlowMode( true ), true ) + CString( _T( ")" ) );
		}

		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_HMIN );
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetHminForUserDisplay(), true );
		sizeText =  GetTextSize( pDC, str );
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );

		if( true == bDraw )
		{
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
		}
	}
	
	// Terminal unit.
	if( NULL != pHM->GetpTermUnit() && false == pHM->IsaModule() )
	{
		TxtRect.top = TxtRect.bottom;
		TxtRect.bottom += sizeText.cy; 
		str = TASApp.LoadLocalizedString( IDS_RVPROJ_TU );
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DP );
		str += _T(" ") + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp(), true );
		
		if( CDS_HydroMod::eDpType::QDpref == pHM->GetpTermUnit()->GetDpType() )
		{
			str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DPREF );
			double dDpref, dQref;
			CDS_HydroMod::eDpType DpType;
			pHM->GetpTermUnit()->GetDp( &DpType, &dDpref, &dQref );
			str += CString( _T(" (") ) + str1 + WriteCUDouble( _U_DIFFPRESS, dDpref, true );
			str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_QREF );
			str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_FLOW, dQref, true ) + CString( _T(")" ) );
		}
		// HYS-1277: Add details for DpType Kv and Cv
		else if( CDS_HydroMod::eDpType::Kv == pHM->GetpTermUnit()->GetDpType() )
		{
			CString strKv = TASApp.LoadLocalizedString( IDS_KV );
			strKv += _T("=");
			double dKvValue;
			pHM->GetpTermUnit()->GetDp( NULL, &dKvValue, NULL );
			str += CString( _T(" (") ) + strKv + WriteCUDouble( _C_KVCVCOEFF, dKvValue ) + CString( _T(")" ) );
		}		
		else if( CDS_HydroMod::eDpType::Cv == pHM->GetpTermUnit()->GetDpType() )
		{
			CString strCv = TASApp.LoadLocalizedString( IDS_CV );
			strCv += _T("=");
			double dCvValue;
			pHM->GetpTermUnit()->GetDp( NULL, &dCvValue, NULL );
			str += CString( _T(" (") ) + strCv + WriteCUDouble( _C_KVCVCOEFF, dCvValue ) + CString( _T(")" ) );
		}
		sizeText =  GetTextSize( pDC, str );
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );
		if( true == bDraw )
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
	}

	// Circuit Pipe only for circuit and not for module
	if( false == pHM->IsaModule() && NULL != pHM->GetpCircuitPrimaryPipe())
	{
		CDB_Pipe *pPipe = (CDB_Pipe *)pHM->GetpCircuitPrimaryPipe()->GetIDPtr().MP;
		str.Empty();
		if( NULL != pPipe && 0.0 != pHM->GetpCircuitPrimaryPipe()->GetLength() )
		{
			TxtRect.top = TxtRect.bottom;
			str = TASApp.LoadLocalizedString( IDS_RVPROJ_PIPE );

			CString str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DP );
			str += CString( _T(" ") ) + pPipe->GetName();
			str += CString( _T("; ") ) + WriteCUDouble( _U_LENGTH, pHM->GetpCircuitPrimaryPipe()->GetLength(), true );

			CString str2 = _T("; ") + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpCircuitPrimaryPipe()->GetPipeDp(), true ) + _T("; ");
			str += str2;
			str1 = TASApp.LoadLocalizedString( IDS_HMCALC_LINDP );
			str2 = str1 + CString( _T("=") ) + WriteCUDouble( _U_LINPRESSDROP, pHM->GetpCircuitPrimaryPipe()->GetLinDp(), true );
			str += str2	+ CString( L" " ) + CString( L"( " ) + pHM->GetpCircuitPrimaryPipe()->GetPipeSeries()->GetName() + CString( L" )" );
		}

		if( false == pHM->IsaModule() && pHM->GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true ) > 0 )
		{
			if( false == str.IsEmpty() )
				str += _T("\r\n");
			else
				TxtRect.top = TxtRect.bottom;

			str += TASApp.LoadLocalizedString( IDS_RVPROJ_SINGULARITY ) + _T(" ") + FormatPipeAccessoryStr( pHM->GetpCircuitPrimaryPipe() );
		}

		if( false == str.IsEmpty() )
		{
			sizeText =  GetTextSize( pDC, str );
			if( true == m_bTreePrintPreview )
				sizeText.cx -= str.GetLength();			// Pixel quantization !!! should work in twips in place of pixel !
			rect.right = max( sizeText.cx + TxtRect.left, rect.right );
			TxtRect.right = m_TreePrintRect.right;
			TxtRect.bottom +=sizeText.cy;

			if( true == bDraw )
			{
				pDC->DrawText( str, &TxtRect, DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
			}
		}
	}

	int iYOffsetForHAvailable = 0;

	// Balancing valve.
	if( true == pHM->IsBvExist( true ) )
	{
		if( true == pHM->IsDpCExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_RVPROJ_MV );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_RVPROJ_BV );
		}

		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DP );
		str += CString( _T(" ") ) + ( (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP ) )->GetName();
		str += _T("; ") + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpBv()->GetDp(), true );
		str1 = TASApp.LoadLocalizedString( IDS_HMCALC_BVSETTING );
		str += _T("; ") + str1 + _T("=") + pHM->GetpBv()->GetSettingStr( true );

		TxtRect.top = TxtRect.bottom;
		sizeText =  GetTextSize( pDC, str );
		TxtRect.bottom += sizeText.cy;
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );

		if( true == bDraw )
		{
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
		}
	}

	// DpC.
	if( true == pHM->IsDpCExist( true ) )
	{
		str = TASApp.LoadLocalizedString( IDS_RVPROJ_DPC );
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DPL );
		str += CString( _T(" ") ) + ( (CDB_TAProduct *)( pHM->GetpDpC()->GetIDPtr().MP ) )->GetName();
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpDpC()->GetDpL(), true );
		TxtRect.top = TxtRect.bottom;
		sizeText =  GetTextSize( pDC, str );
		TxtRect.bottom += sizeText.cy; 
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );

		if( true == bDraw )
		{
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
		}
	}

	// Control Valve (standard, BCV and PICV. DpCBCV are done after).
	if( true == pHM->IsCvExist( true ) && eb3False == pHM->GetpCV()->IsDpCBCV() )
	{
		str = TASApp.LoadLocalizedString( IDS_RVPROJ_CV );	
		
		if( true == pHM->GetpCV()->IsTaCV() && NULL != pHM->GetpCV()->GetCvIDPtr().MP && true == pHM->GetpCV()->IsPresettable() )
		{
			str1 = TASApp.LoadLocalizedString( IDS_HMCALC_BVSETTING );
			str += CString( _T(" ") ) + ( (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP ) )->GetName();
			str += CString( _T("; ") ) + str1 + _T("=") + pHM->GetpCV()->GetSettingStr( false );
		}
		else
		{
			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				str1 = TASApp.LoadLocalizedString( IDS_KVS );
			else
				str1 = TASApp.LoadLocalizedString( IDS_CV );
			str += str1 + CString( _T("=") ) + WriteCUDouble( _C_KVCVCOEFF, pHM->GetpCV()->GetKvs() );
		}
		
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DP );
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpCV()->GetDp(), true );
		
		if( false == pHM->GetpCV()->IsOn_Off() && eb3False == pHM->GetpCV()->IsPICV() )
		{
			str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_AUTH );
			str += CString( _T("; ") ) + str1 + (CString)WriteDouble( pHM->GetpCV()->GetAuth(), 2, 0 ) +  CString( _T("/") ) +  (CString)WriteDouble( pHM->GetpCV()->GetAuth(true), 2, 0 );
		}

		TxtRect.top = TxtRect.bottom;
		sizeText =  GetTextSize( pDC, str );
		TxtRect.bottom += sizeText.cy; 
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );

		if( true == bDraw )
		{
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
		}

		if( NULL != pHM->GetpCV()->GetActrIDPtr().MP )
		{
			CDB_ElectroActuator *pActr = dynamic_cast<CDB_ElectroActuator *> (pHM->GetpCV()->GetActrIDPtr().MP);

			if ( NULL != pActr )
			{
				str = _T("");
				str = TASApp.LoadLocalizedString( IDS_RVPROJ_ACTUATOR );
				str +=  CString( _T(" ") ) + pActr->GetName() + _T("; ") + pActr->GetRelayStr();

				TxtRect.top = TxtRect.bottom;
				sizeText =  GetTextSize( pDC, str );
				TxtRect.bottom += sizeText.cy; 
				rect.right = max( sizeText.cx + TxtRect.left, rect.right );

				if( true == bDraw )
				{
					pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
				}
			}
		}

	}

	// Bypass balancing valve.
	if( true == pHM->IsBvBypExist( true ) )
	{
		str = TASApp.LoadLocalizedString( IDS_RVPROJ_BVB );
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DP );
		str += CString( _T(" ") ) + ( (CDB_TAProduct *)( pHM->GetpBypBv()->GetIDPtr().MP ) )->GetName();
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpBypBv()->GetDp(), true );
		str1 = TASApp.LoadLocalizedString( IDS_HMCALC_BVSETTING );
		str += CString( _T("; ") ) + str1 + CString( _T("=") ) + pHM->GetpBypBv()->GetSettingStr( true );
		TxtRect.top = TxtRect.bottom;
		sizeText =  GetTextSize( pDC, str );
		TxtRect.bottom += sizeText.cy; 
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );

		if( true == bDraw )
		{
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
		}
	}

	// Combined Dp controller, control and balancing valve.
	if( true == pHM->IsCvExist( true ) && eb3True == pHM->GetpCV()->IsDpCBCV() )
	{
		str = TASApp.LoadLocalizedString( IDS_RVPROJ_DPCBCV );
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DP );
		str += CString( _T(" ") ) + ( (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP ) )->GetName();
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpCV()->GetDp(), true );
		str1 = TASApp.LoadLocalizedString( IDS_HMCALC_BVSETTING );
		str += CString( _T("; ") ) + str1 + CString( _T("=") ) + pHM->GetpCV()->GetSettingStr( true );
		TxtRect.top = TxtRect.bottom;
		sizeText =  GetTextSize( pDC, str );
		TxtRect.bottom += sizeText.cy; 
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );

		if( true == bDraw )
		{
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
		}
	}

	// Shut-off valve.
	if( true == pHM->IsShutoffValveExist( true ) && NULL != pHM->GetpSch() )
	{
		CDS_HydroMod::eHMObj HMObj = CDS_HydroMod::eNone;
		
		// HYS-1722: pHM instead of m_pHM
		if( ShutoffValveLoc::ShutoffValveLocSupply == pHM->GetpSch()->GetShutoffValveLoc() )
		{
			HMObj = CDS_HydroMod::eShutoffValveSupply;
		}
		else if( ShutoffValveLoc::ShutoffValveLocReturn == pHM->GetpSch()->GetShutoffValveLoc() )
		{
			HMObj = CDS_HydroMod::eShutoffValveReturn;
		}

		if( CDS_HydroMod::eNone != HMObj )
		{
			str = TASApp.LoadLocalizedString( IDS_RVPROJ_SV );
			str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DP );
			str += CString( _T(" ") ) + ( (CDB_TAProduct *)( pHM->GetpShutoffValve( HMObj )->GetIDPtr().MP ) )->GetName();
			str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpShutoffValve( HMObj )->GetDp(), true );
			TxtRect.top = TxtRect.bottom;
			sizeText =  GetTextSize( pDC, str );
			TxtRect.bottom += sizeText.cy; 
			rect.right = max( sizeText.cx + TxtRect.left, rect.right );

			if( true == bDraw )
			{
				pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
			}
		}
	}

	// HYS-1713: Smart control valve.
	if( true == pHM->IsSmartControlValveExist( true ) && NULL != pHM->GetpSch() )
	{
		str = TASApp.LoadLocalizedString( IDS_RVPROJ_SMCV );

		// HYS-1894: It's well the Dp min.
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_SMCVDPMIN );
		str += CString( _T(" ") ) + ( (CDB_TAProduct *)( pHM->GetpSmartControlValve()->GetIDPtr().MP ) )->GetName();
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpSmartControlValve()->GetDpMin(), true );
		
		// Flow max.
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_SMCVFLOWMAX );
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_FLOW, pHM->GetpSmartControlValve()->GetQ(), true );
		
		// Power max.
		if( SmartValveControlMode::SCVCM_Power == pHM->GetpSmartControlValve()->GetControlMode() )
		{
			str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_SMCVPOWMAX );
			str += CString( _T( "; " ) ) + str1 + WriteCUDouble( _U_TH_POWER, pHM->GetpSmartControlValve()->GetPowerMax(), true );
		}
		
		TxtRect.top = TxtRect.bottom;
		sizeText =  GetTextSize( pDC, str );
		TxtRect.bottom += sizeText.cy; 
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );

		if( true == bDraw )
		{
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
		}

		if( SmartValveLocalization::SmartValveLocSupply == pHM->GetpSmartControlValve()->GetLocalization() )
		{
			iYOffsetForHAvailable += 100;
		}
	}

	// HYS-1963: Smart differential pressure controller
	if( true == pHM->IsSmartDpCExist( true ) && NULL != pHM->GetpSch() )
	{
		str = TASApp.LoadLocalizedString( IDS_RVPROJ_SMARTDPC );
		CDB_DpSensor *pclDpSensor = NULL;

		// Name.
		str += CString( _T(" ") );
		str += ( (CDB_TAProduct *)( pHM->GetpSmartDpC()->GetIDPtr().MP ) )->GetName();

		if( false == pHM->GetpSmartDpC()->IsSelectedAsaPackage() )
		{
			pclDpSensor = pHM->GetpSmartDpC()->GetpDpSensor();
		}
		else
		{
			pclDpSensor = pHM->GetpSmartDpC()->GetDpSensorInDpSensorSet();
		}

		// Dp.
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_SMARTDPCDP );
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpSmartDpC()->GetDp(), true );

		// Dp min.
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_SMARTDPCDPMIN );
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpSmartDpC()->GetDpMin(), true );
		
		// Kvs.
		str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_SMARTDPCKVS );
		str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpSmartDpC()->GetpSmartDpC()->GetKvs() , true );

		TxtRect.top = TxtRect.bottom;
		sizeText =  GetTextSize( pDC, str );
		TxtRect.bottom += sizeText.cy; 
		rect.right = max( sizeText.cx + TxtRect.left, rect.right );

		if( true == bDraw )
		{
			pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
		}

		// Now show info for Dp sensor.
		if( NULL != pclDpSensor )
		{
			str = TASApp.LoadLocalizedString( IDS_RVPROJ_DPSENSOR );

			// Name.
			str += CString( _T(" ") );
			str += pclDpSensor->GetFullName();

			// Dpl.
			str1 = TASApp.LoadLocalizedString( IDS_RVPROJ_DPSENSORDPL );
			str += CString( _T("; ") ) + str1 + WriteCUDouble( _U_DIFFPRESS, pHM->GetpSmartDpC()->GetDpToStabilize(), true );

			// Burst pressure.
			str1 = TASApp.LoadLocalizedString( IDS_BURSTPRESSURE );
			str += CString( _T("; ") ) + str1 + _T(": ") + WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetPmaxmax(), true );

			TxtRect.top = TxtRect.bottom;
			sizeText =  GetTextSize( pDC, str );
			TxtRect.bottom += sizeText.cy; 
			rect.right = max( sizeText.cx + TxtRect.left, rect.right );

			if( true == bDraw )
			{
				pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
			}
		}

		if( SmartValveLocalization::SmartValveLocSupply == pHM->GetpSmartDpC()->GetLocalization() )
		{
			iYOffsetForHAvailable += 150;
		}
	}

	// HAvailable.
	if( true == bDraw )
	{
		str = TASApp.LoadLocalizedString( IDS_RVPROJ_HAVAIL );
		str1 = str + WriteCUDouble( _U_DIFFPRESS, pHM->GetHAvail(), true );
		sizeText =  GetTextSize( pDC, str1 );
		TxtRect.top = rect.top + rect.Height() / 2 - sizeText.cy + iYOffsetForHAvailable;
		TxtRect.bottom = TxtRect.top + sizeText.cy;
		TxtRect.left = rect.left + Getmm2pt( pDC, 3, false );
		TxtRect.right = TxtRect.left + sizeText.cx;
		pDC->DrawText( str1, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX );
	}

	// Index ?
	if( true == pHM->IsCircuitIndex() && true == bDraw )
	{
		CBrush BGBrush;
		BGBrush.CreateSolidBrush( _INDEXCOLOR );
		CBrush *pOldBrush = pDC->SelectObject( &BGBrush );
		CPen Pen;
		Pen.CreatePen( PS_SOLID, 1, _INDEXCOLOR );
		CPen *pOldPen = pDC->SelectObject( &Pen );
		
		CPoint pts[4];
		pts[0].x = rect.left - Getmm2pt( pDC, 2, false );
		pts[0].y = rect.bottom - Getmm2pt( pDC, 5, true );
		pts[1].x = rect.left - Getmm2pt( pDC, 3, false );
		pts[1].y = rect.bottom - Getmm2pt( pDC, 6, true );
		pts[2].x = pts[0].x;
		pts[2].y = rect.bottom - Getmm2pt( pDC, 7, true );
		pts[3].x = rect.left - Getmm2pt( pDC, 1, false );
		pts[3].y = pts[1].y;
		
		pDC->Polygon( pts, 4 );
		pDC->SelectObject( pOldBrush );
		BGBrush.DeleteObject();
		pDC->SelectObject( pOldPen );
		Pen.DeleteObject();
	}
	
	pDC->SelectObject( pOldFont );
	pDC->SetTextColor( OldTextColor );
	pDC->SetBkMode( iOldBkMode );
	clText.DeleteObject();
	clTextBold.DeleteObject();

	return rect;
}

int CRViewProj::HMNodePrinting(CDC* pDC, int iPage)
{
	CDS_HydroMod *pPrevHM = NULL;
	sHMPage prevHM;
	int iXmin = INT_MAX;
	for( int i = 0; i < m_pHMPage->ArHM.GetCount(); i++ )
	{
		sHMPage currHM = m_pHMPage->ArHM.GetAt( i );
		iXmin = min( iXmin, currHM.XY.x );
		CRect rect = DrawHMNode( pDC, currHM.pHM, currHM.XY, true );
		CPoint ConnXY = currHM.XY;
		ConnXY.Offset( -m_iTabX, -m_iTabY );
		
		//////////////////////////////////////////////////////////////////////////////////////
		// Process dash line to mark reported modules
		if( true == currHM.bContentReported )
		{
			CPoint DashLineXY( rect.left + m_iTabX, rect.bottom );
			CPen dashpen;
			dashpen.CreatePen( PS_DOT, 1, _BLACK );
			CPen *pOldPen = pDC->SelectObject( &dashpen );
			
			// Vertical line
			pDC->MoveTo( DashLineXY );
			DashLineXY.y += m_iTabY / 3;
			pDC->LineTo( DashLineXY );
			
			// Horizontal line
			DashLineXY.x += m_iTabX;
			pDC->LineTo( DashLineXY );
			pDC->SelectObject( pOldPen );
			dashpen.DeleteObject();
			
			if( currHM.iRepPage > 0 )
			{
				CFont clText;
				_CreateFont( pDC, clText, 8, FW_NORMAL, CString( _T("Arial") ) );
				CFont *pOldFont = pDC->SelectObject( &clText );
				CRect TxtRect( DashLineXY.x, DashLineXY.y, DashLineXY.x, DashLineXY.y );
				CString str, str1;
				str = TASApp.LoadLocalizedString( IDS_PRINT_PAGENUMBER );
				str1.Format( _T(" %d"), currHM.iRepPage );
				str += str1;
				CSize TxtSize = GetTextSize( pDC, str );
				TxtRect.right += TxtSize.cx;
				TxtRect.top -= TxtSize.cy / 2;
				TxtRect.bottom += TxtSize.cy / 2;
				pDC->DrawText( str, &TxtRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT  | DT_NOPREFIX );
				pDC->SelectObject( pOldFont );					// Restore GDI
				clText.DeleteObject();
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// Check Next sibling (same father) for drawing vertical lines down to bottom of sheet
		if (false == currHM.bLastSibling )
		{
			int j = i + 1;
			bool bFound = false;
			while( j < m_pHMPage->ArHM.GetCount() && false == bFound )
			{
				sHMPage nextHM = m_pHMPage->ArHM.GetAt( j );
				// Same father and located on the same page, vertical line will be draw by is brother
				if( nextHM.pHM->GetIDPtr().PP == currHM.pHM->GetIDPtr().PP )
					bFound = true;
				j++;
			}
			
			if( false == bFound )
			{
				// Current module has a brother and it's not located on the same page --> draw vertical lines
				CPoint LineXY = ConnXY;
					// Vertical line
				pDC->MoveTo( LineXY );
				LineXY.y = m_TreePrintRect.bottom;
				pDC->LineTo( LineXY );
			}
		}
			/////////////////////////////
		// Process distribution pipes
		if( NULL == pPrevHM )
		{
		}
		else if( pPrevHM->GetIDPtr().PP == currHM.pHM->GetIDPtr().PP )
		{
			// Same parent or first circuit in this page
			// in this case we have to go up to the middle of previous circuit
			ConnXY.y = prevHM.XY.y + rect.Height() / 2;
		}
		else if( pPrevHM == currHM.pHM->GetIDPtr().PP )
		{// Children of previous HM
		}
		else									
		{											
			// Return to the previous level; try to found on the same page his parent or previous module with the same parent
			// if it doesn't exist draw to the top
			ConnXY.y = m_TreePrintRect.top;		
			// Try to previous brother
			if( i > 0 )
			{
				bool bFound = false;
				for( int j = i - 1; j >= 0 && false == bFound; j-- )
				{
					sHMPage prevHM = m_pHMPage->ArHM.GetAt( j );
					
					//brother?
					if( currHM.pHM->GetIDPtr().PP == prevHM.pHM->GetIDPtr().PP )
					{
						bFound = true;
						//draw connection
						// Assumption all picture have the same size!
						ConnXY.y = prevHM.XY.y + rect.Height() / 2;
					}
					else if( currHM.pHM->GetIDPtr().PP == prevHM.pHM->GetIDPtr().MP )
					{
						bFound = true;					
						//father?
						// Assumption all picture have the same size!
						ConnXY.y = prevHM.XY.y + rect.Height();
					}
				}
			}
		}	
		// Vertical line
		pDC->MoveTo( ConnXY );
		ConnXY.y = rect.top + rect.Height() / 2;
		pDC->LineTo( ConnXY );

		// Horizontal line
		ConnXY.x = rect.left;
		pDC->LineTo( ConnXY );
		pPrevHM = currHM.pHM;
		prevHM = currHM;
	}
	
	/////////////////////////////////////////////
	// All module of this page has been proceed
	// draw vertical lines (distribution pipes) on the left if they are needed for next pages
	if( iPage <= m_mapArHMPages.GetCount() )
	{
		// iXmin is the minimum X position for module drawn on this page
		iXmin = ( iXmin - ( m_TreePrintRect.left + m_iTabX ) ) / ( 2 * m_iTabX );
		
		while( iXmin > 0 && iXmin < 20 )
		{
			if( m_pHMPage->ArIndent.GetAt( iXmin ) )
			{
				// Assumption all picture have the same size!
				CPoint ConnXY;
				ConnXY.y = m_TreePrintRect.top;			
				ConnXY.x = m_TreePrintRect.left + ( iXmin * 2 * m_iTabX );
				
				// Vertical line
				pDC->MoveTo( ConnXY );
				ConnXY.y = m_TreePrintRect.bottom;
				pDC->LineTo( ConnXY );
			}
			--iXmin;
		}
	}
	return 0;
}

// pCurHM is used to retrieve correct position in table after a page incrementation
int CRViewProj::PrepareHMNodePrinting( CDC* pDC, CTable *pTab, int &iPage, CPoint &CurXY, CArray<CTable *> *pArReportedHM, bool fFirst )
{
	CRank SortList;
	CDS_HydroMod *pHM = NULL;
	int iCount = 0;
	
	// Sort children
	for( IDPTR HMIDPtr = pTab->GetFirst(); NULL != *HMIDPtr.ID; HMIDPtr = pTab->GetNext( HMIDPtr.MP ) )
	{
		pHM = (CDS_HydroMod *)HMIDPtr.MP;
		SortList.Add( _T(""), pHM->GetPos(), (LPARAM)pHM );
		++iCount;
	}

	// Add parent module at the first position for the main module
	if( true == fFirst )
	{
		if( false == pTab->IsClass( CLASS( CTableHM ) ) )
		{
			SortList.AddHead( _T(""), 0, (LPARAM)pTab );
		}
	}
	
	CString str;
	LPARAM itemdata;
	// Draw Sorted children
	for( BOOL fContinue = SortList.GetFirst( str, itemdata ); TRUE == fContinue; fContinue = SortList.GetNext( str, itemdata ) )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)itemdata;
		CRect rect( 0, 0, 0, 0 );
		rect = DrawHMNode( pDC, pHM, CurXY, false );
		
		// Verify  Y position with Printing Zone
		if( ( rect.bottom + m_iTabY / 3 ) > m_TreePrintRect.bottom )
		{
			// Store previous page
			//mapArHMPages::CPair* p = m_mapArHMPages.PLookup(iPage);
			//ASSERT(!p);
			m_mapArHMPages.SetAtGrow( iPage, m_pHMPage );
			CArray<bool> arIndent;
			arIndent.Append( m_pHMPage->ArIndent );

			// New page needed
			++iPage;
			m_pHMPage = new ArHMPage;
			
			// Initialize Indent array with the previous page content
			m_pHMPage->ArIndent.Append( arIndent );
			
			// Reset Y position
			CurXY.y = m_TreePrintRect.top + m_iTabY;
			rect = DrawHMNode( pDC, pHM, CurXY, false );
		}
		// pHM will be drawn on this page
		sHMPage HMPage;
		HMPage.pHM = pHM;
		HMPage.XY = CurXY;
		HMPage.bLastSibling = !( pHM->GetPos() < iCount );
		HMPage.bContentReported = false;
		HMPage.iRepPage = 0;
		
		// Establish indent for this module (allow to draw distribution pipe crossing a complete page)
		int iIndent = ( CurXY.x - m_TreePrintRect.left - m_iTabX ) / ( 2 * m_iTabX );
//		ASSERT( iIndent >= 0 && iIndent < 10 );
		if( m_pHMPage->ArIndent.GetCount() < ( iIndent + 1 ) )
			m_pHMPage->ArIndent.SetSize( iIndent + 1 );
		m_pHMPage->ArIndent.SetAt( iIndent, !HMPage.bLastSibling );
		
		// Position for next Node
		CurXY.y = rect.bottom + m_iTabY;

		// Add main module and skip recurse function for the main module to avoid repetition
		if( true == fFirst && pHM == pTab )
		{
			HMPage.bLastSibling = true;
			m_pHMPage->ArIndent.SetAt( iIndent, !HMPage.bLastSibling );
			m_pHMPage->ArHM.Add( HMPage );
			CurXY.x += 2*m_iTabX;		
		}
		else // Verify Children and recurse if needed 
		{
			if( NULL != pHM->GetFirst().MP )
			{
				CPoint ChildXY = CurXY;
				ChildXY.x += 2 * m_iTabX;
				// Compute needed horizontal space for one level more
				// if there is not enough horizontal space, report this module
				// to the end
				bool fChildrenCanbeDrawn = true;
				for( IDPTR HMChildIDPtr = pHM->GetFirst(); NULL != *HMChildIDPtr.ID && true == fChildrenCanbeDrawn; HMChildIDPtr = pHM->GetNext() )
				{
					CDS_HydroMod *pChilHM = (CDS_HydroMod *)HMChildIDPtr.MP;
					CRect rect( 0, 0, 0, 0 );
					rect = DrawHMNode( pDC, pChilHM, ChildXY, false );
					if( rect.right > m_TreePrintRect.right )
						fChildrenCanbeDrawn = false;
				}
			
				if( false == fChildrenCanbeDrawn )
				{
					// All children will be reported 
					pArReportedHM->Add( pHM );
					CurXY.y += m_iTabY / 3;
					HMPage.bContentReported = true;
					m_mapCrossRefReportedHM.SetAt( pHM, iPage );
					
					// Add current module
					m_pHMPage->ArHM.Add( HMPage );
				}
				else
				{
					// Add current module before processing children
					m_pHMPage->ArHM.Add( HMPage );
					
					// Children could be drawn on the current page and next pages
					CPoint ChildXY = CurXY;
					ChildXY.x += 2 * m_iTabX;
					
					// Recurse inside this module
					PrepareHMNodePrinting( pDC, pHM, iPage, ChildXY, pArReportedHM, false );
					CurXY.y = ChildXY.y;
				}
			}
			else	// No children
			{
				// Simply proceed for the current circuit
				m_pHMPage->ArHM.Add( HMPage );
			}
		}
	}

	// Add latest added page when we finish
	if( true == fFirst )
	{
		m_mapArHMPages.SetAtGrow( iPage, m_pHMPage );
	}
	return 0;
}

// Dedicated OnPreparePrinting functions
BOOL CRViewProj::OnPreparePrinting( CPrintInfo* pInfo )
{
		pInfo->m_pPD->m_pd.Flags |= PD_NOSELECTION;

	BOOL fReturn;
	if( true == m_fPrintTreeView )
		fReturn = OPP_TreeView( pInfo );
	else if( true == m_fPrintSelProd )
		fReturn = OPP_SelectedProduct( pInfo );
	else if( true == m_fPrintLogDataSumm )
		fReturn = OPP_LogDataSumm( pInfo );
	else if( true == m_fPrintLogData )
		fReturn = OPP_LogData( pInfo );
	else if( true == m_fPrintQuickMeas )
		fReturn = OPP_QuickMeas( pInfo );
	else if( true == m_fPrintCircuit )
		fReturn = OPP_Circuit( pInfo );
	else
		fReturn = OPP_HMCalc( pInfo );
	
	if( false == fReturn )
		return false;
	
	return CRScrollView::OnPreparePrinting( pInfo );
}

BOOL CRViewProj::OPP_TreeView( CPrintInfo* pInfo )
{
	return TRUE;
}

BOOL CRViewProj::OPP_HMCalc( CPrintInfo* pInfo )
{
	CRect rect( 0, 0, 50, 50 );

	if( NULL != m_pSheetHMCalc )
	{
		delete m_pSheetHMCalc;
	}
	
	m_pSheetHMCalc = new CSheetHMCalc();
	ASSERT( NULL != m_pSheetHMCalc );

	if( NULL != m_pSheetHMCalc )
	{
		if( FALSE == m_pSheetHMCalc->Create( WS_CHILD ^ !WS_VISIBLE, rect, this, IDC_TMPSSHEET ) ) 
		{
			return FALSE;
		}
	}

	CSheetHMCalc *pclSSheetHMCalc = NULL;

	if( NULL != pRViewHMCalc )
	{
		pclSSheetHMCalc = pRViewHMCalc->GetSheetHMCalc();
	}

	if( NULL != pclSSheetHMCalc )
	{
		mapMapShortLong mapAllColumnWidth;
		pclSSheetHMCalc->GetAllColumnWidth( mapAllColumnWidth );
		m_pSheetHMCalc->SetAllColumnWidth( mapAllColumnWidth );
	}

	return TRUE;
}

BOOL CRViewProj::OPP_Circuit( CPrintInfo* pInfo )
{
	CRect rect( 0, 0, 1000, 1000 );
	if( m_SheetPC2.GetSafeHwnd() != NULL )
		m_SheetPC2.DestroyWindow();
	
	if( FALSE == m_SheetPC2.Create( WS_CHILD ^ !WS_VISIBLE, rect, this, IDC_TMPSSHEET ) )
		return FALSE;
	return TRUE;
}

BOOL CRViewProj::OPP_SelectedProduct( CPrintInfo* pInfo )
{
	return TRUE;
}

BOOL CRViewProj::OPP_LogDataSumm( CPrintInfo* pInfo )
{
	CRect rect( 0, 0, 50, 50 );
	if( m_SheetLDSumm.GetSafeHwnd() )
		m_SheetLDSumm.DestroyWindow();
	
	if( !m_SheetLDSumm.Create( WS_CHILD ^ !WS_VISIBLE, rect, this, IDC_TMPSSHEET ) )
		return FALSE;
	return TRUE;
}

BOOL CRViewProj::OPP_LogData( CPrintInfo* pInfo )
{
	CRect rect( 0, 0, 50, 50 );
	if( m_SheetLogData.GetSafeHwnd() )
		m_SheetLogData.DestroyWindow();
	
	if( !m_SheetLogData.Create( WS_CHILD ^ !WS_VISIBLE, rect, this, IDC_TMPSSHEET ) ) 
		return FALSE;
	return TRUE;
}

BOOL CRViewProj::OPP_QuickMeas( CPrintInfo* pInfo )
{
	CRect rect( 0, 0, 50, 50 );
	if( m_SheetQuickMeas.GetSafeHwnd() )
		m_SheetQuickMeas.DestroyWindow();
	
	if( !m_SheetQuickMeas.Create( WS_CHILD ^ !WS_VISIBLE, rect, this, IDC_TMPSSHEET ) ) 
		return FALSE;
	return TRUE;
}

// Dedicated OnBeginPrinting functions
void CRViewProj::OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo ) 
{
	pDC->SetMapMode( MM_TEXT );  

#ifdef DEBUG_PRINTING
	// Check here if pDC is a normal CDC or a CPreviewDC.
	if( pDC->IsKindOf( RUNTIME_CLASS( CPreviewDC ) ) )
	{
		m_fPrintPreviewMode = true;
		m_hDCMemory = CreateCompatibleDC( NULL ); // 1920 x 1080 for example

		m_recPrinterDP = CRect( 0, 0, pDC->GetDeviceCaps( HORZRES ), pDC->GetDeviceCaps( VERTRES ) );
		m_recScreenDP = m_recPrinterDP;
		CPreviewDC *pPreviewDC = (CPreviewDC *)pDC;
		pPreviewDC->PrinterDPtoScreenDP( &m_recScreenDP.TopLeft() );
		pPreviewDC->PrinterDPtoScreenDP( &m_recScreenDP.BottomRight() );

		m_hBitmap = CreateCompatibleBitmap( pDC->GetSafeHdc(), m_recScreenDP.right - m_recScreenDP.left, m_recScreenDP.bottom - m_recScreenDP.top );
		m_hBitmapOld = (HBITMAP)SelectObject( m_hDCMemory, m_hBitmap );

		pDC = CDC::FromHandle( m_hDCMemory );
	}
	else
		m_fPrintPreviewMode = false;
#endif

	CRScrollView::OnBeginPrinting( pDC, pInfo );

	if( true == m_fPrintTreeView ) 
		OBP_TreeView( pDC, pInfo);
	else if( true == m_fPrintSelProd )
		OBP_SelectedProduct( pDC, pInfo );
	else if( true == m_fPrintLogDataSumm )
		OBP_LogDataSumm( pDC, pInfo );
	else if( true == m_fPrintLogData )
		OBP_LogData( pDC, pInfo );
	else if( true == m_fPrintQuickMeas )
		OBP_QuickMeas( pDC, pInfo );
	else if( true == m_fPrintCircuit )
		OBP_Circuit( pDC, pInfo );
	else
		OBP_HMCalc( pDC, pInfo );
}

void CRViewProj::OBP_TreeView( CDC* pDC, CPrintInfo* pInfo )
{
	pInfo->m_rectDraw.top = 0;
	pInfo->m_rectDraw.left = 0;
	pInfo->m_rectDraw.right = pDC->GetDeviceCaps( HORZRES );
	pInfo->m_rectDraw.bottom = pDC->GetDeviceCaps( VERTRES );
	m_bTreePrintPreview = pInfo->m_bPreview?true:false;
	// m_TreePrintRect space available on sheet 
	m_TreePrintRect = pInfo->m_rectDraw;
	
	// Compute Left and right Margin
	int iMargin;
	
	// Margin is fixed to 10 mm
	iMargin = Getmm2pt( pDC, 10 );
	m_iTabX = Getmm2pt( pDC, 4 );
	if( true == m_fShort )
		m_iTabY = Getmm2pt( pDC, 4 );
	else
		m_iTabY=Getmm2pt( pDC, 25 );
	
	m_iMm = (int)Getmm2pt( pDC, 1 );

	m_TreePrintRect.right -= iMargin;
	m_TreePrintRect.left += iMargin;
	m_TreePrintRect.bottom -= iMargin;
	m_TreePrintRect.top += iMargin;
	CFont Text9;
	_CreateFont( pDC, Text9, 9, FW_NORMAL, CString( _T("Arial") ) );

	CFont *pOldFont = pDC->SelectObject( &Text9 );
	m_iPage = 1;
	m_Pagearray.RemoveAll();
	m_mapArHMPages.RemoveAll();
	m_mapCrossRefReportedHM.RemoveAll();
	
	CTable *pTab = NULL;

	if( NULL != m_pTab )
	{
		pTab = m_pTab;
	}
	else
	{
		pTab = m_pTADS->GetpHydroModTable();
		ASSERT( NULL != pTab );
	}

	CPoint CurXY( m_TreePrintRect.left + m_iTabX, m_TreePrintRect.top + m_iTabY );
	
	_Pages spage;
	spage.fnewpage = true;
	spage.lpTab = (LPARAM)pTab;
	spage.lpHM = NULL;
	spage.bDelayed = false;
	m_Pagearray.Add( spage );

	m_PrintRect = pInfo->m_rectDraw;

	// Compute space needed for footer and update TreePrintRect
	DrawFooter( pDC, m_TreePrintRect.left, 0, 1, false );
	m_TreePrintRect.bottom = m_PrintRect.bottom;
	if( true == m_pTADSPageSetup->GetPrintPrjInfo() )
	{
		m_Pagearray.Add( spage );
		m_iPage++;
	}

	// always Draw Header and footer
	int iYPos = 0;
	m_PrintRect = pInfo->m_rectDraw;
	iYPos = DrawHeader( pDC, m_TreePrintRect.left, iYPos, m_iPage, false );
	iYPos = DrawShortPrjInfo( pDC, m_TreePrintRect.left, iYPos, false );
	m_TreePrintRect.top = iYPos + m_iTabY;
			
	int iPage = m_iPage-1;
	CArray<CTable *> ArReportedHM;
	CArray<CTable *> ArTmpReportedHM;
	ArReportedHM.Add(pTab);
	m_mapArHMPages.SetSize(100,100);
	for (int i=0;i<100; i++)
		m_mapArHMPages[i] = NULL;
	for (int i=0; i<ArReportedHM.GetCount(); i++)
	{
		iPage++;
		// Allocate a new structure for new page
		m_pHMPage = new ArHMPage;	
		// Reset position
		CurXY.x = m_TreePrintRect.left+m_iTabX;
		CurXY.y = m_TreePrintRect.top+m_iTabY;
		// Get pointer on the first Module
		pTab = ArReportedHM.GetAt(i);
		// Update page number for reported module
		if (m_mapCrossRefReportedHM.GetCount())
		{
			// retrieve page number 
			int page = 0;
			if (m_mapCrossRefReportedHM.Lookup(pTab, page))
			{
				ArHMPage *pHMPage = m_mapArHMPages.ElementAt(page);
				if (pHMPage)
				{
					bool bfound = false;
					for (int j=0; j<pHMPage/*->value*/->ArHM.GetCount() && false == bfound;j++)
					{
						if (pHMPage/*->value*/->ArHM.GetAt(j).pHM == pTab)
						{
							// Found, update page number
							ArHMPage *pArHMPage = pHMPage;//->value;
							sHMPage &ArHM = pArHMPage->ArHM.ElementAt(j);
							ArHM.iRepPage = iPage;
							bfound = true;
						}
					}
				}
			}
		}
		// Scan all included module (recursive function)
		PrepareHMNodePrinting(pDC, pTab, iPage,CurXY, &ArTmpReportedHM, true);
		// Some modules have been reported, add them into the loop
		ArReportedHM.Append(ArTmpReportedHM);
		ArTmpReportedHM.RemoveAll();
	} 
	m_iPage = iPage;
	m_mapArHMPages.SetSize(iPage+1);
	pDC->SelectObject( pOldFont );
	Text9.DeleteObject();
	pInfo->SetMaxPage( m_iPage );
	m_pCurHM = NULL;
//	m_iPage = 1;
}

void CRViewProj::OBP_HMCalc(CDC* pDC, CPrintInfo* pInfo)
{
	Init( pDC );

	LPARAM lparam;
	CString str;
	// Draw each module; fill 
	int pagenum=1;
	m_Pagearray.RemoveAll();


	_Pages Page;

	// Draw footer, same space needed on each page 
	DrawFooter( pDC, m_iLeftMargin, 0, 1, false );

	// Keep a copy of drawing page size
	CRect rcSave( m_PrintRect );
	if( true == m_pTADSPageSetup->GetPrintPrjInfo() )
	{
		memset( &Page, 0, sizeof( Page ) );
		Page.fnewpage = true;
		m_Pagearray.Add( Page );
		pagenum++;
	}

	// Loop on HydroMod
	for( BOOL fContinue = m_RankList.GetFirst( str, lparam ); TRUE == fContinue; fContinue = m_RankList.GetNext( str, lparam ) )
	{
		// Space available by sheet
		m_PrintRect = rcSave;
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)lparam );
		if( NULL == pHM )
			continue;		
		
		// Verify if at least one measurement exist
		bool fMeasMustBeDisplayed = false;
		if( true == m_pSheetHMCalc->IsMeasurementExist( pHM ) || false == pMainFrame->IsHMCalcChecked() )
			fMeasMustBeDisplayed = true;
		
		// One page by HM and by sheet
		Page.fnewpage = true;
		Page.lpHM = lparam;
		Page.lpTab = 0;
		
		//Determine space needed for header and footer. 
		int iYPos = 0;
		iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, 1, false );
		iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

		iYPos = DrawShortPrjInfo( pDC, m_PrintRect.left, iYPos, false );
		iYPos = DrawTitle( pDC, m_iLeftMargin, iYPos, pHM->GetHMName(), m_TitleBGColor, false );
		iYPos += ( m_iLineSpace * m_Xfactor ) / 100;
		iYPos = CDrawSet::DrawHMInfo( pDC, pHM, m_iLeftMargin, iYPos, m_PrintRect.Width(), false, false, true );
		iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

		// Update size available for spread sheet
		m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );
		m_pSheetHMCalc->Init( pHM, true );

		// Remarks: For balancing, all modules must have measurements. Thus, if first module is balanced, it's ok
		bool fDisplayTADiagnostic = false;
		CDS_HydroMod::CMeasData *pMeasData = pHM->GetpMeasData( 0 );
		if( NULL != pMeasData && false == pMainFrame->IsHMCalcChecked() )
			fDisplayTADiagnostic = ( pMeasData->GetFlagBalanced() == CDS_HydroModX::eTABalanceMethod::etabmTABalPlus );
								  

		// Loop on sheets by HydroMod
		for( int iLoopSheet = 1; iLoopSheet <= CSheetHMCalc::SheetDescription::SLast; iLoopSheet++ )
		{
			if( false == pMainFrame->IsHMCalcChecked() && ( CSheetHMCalc::SheetDescription::SDistribution == iLoopSheet || CSheetHMCalc::SheetDescription::SCircuit == iLoopSheet ) )
				continue;

			if( CSheetHMCalc::SheetDescription::SCircuitInj == iLoopSheet && FALSE == m_pSheetHMCalc->GetSheetVisible( CSheetHMCalc::SheetDescription::SCircuitInj ) )
				continue;
			
			if( CSheetHMCalc::SheetDescription::SMeasurement == iLoopSheet && false == fMeasMustBeDisplayed )
				continue;

			if( CSheetHMCalc::SheetDescription::STADiagnostic == iLoopSheet && false == fDisplayTADiagnostic )
				continue;

			m_pSheetHMCalc->SetSheet( (CSheetHMCalc::SheetDescription)iLoopSheet );
			Page.Sheet = (LPARAM)iLoopSheet;
			long lPageCount = 0;
			
			// Determine number of page needed for current module
			m_pSheetHMCalc->OwnerPrintPageCount( pDC, m_PrintRect, lPageCount );
			
			// One entry by page needed
			for( long lLoopPage = 1; lLoopPage <= lPageCount; lLoopPage++ )
			{
				Page.PageIndex = lLoopPage;
				m_Pagearray.Add( Page );
				pagenum++;
			}
		}
	}

	pInfo->SetMaxPage( pagenum - 1 );
	m_iPage = 1;
	return;
}

void CRViewProj::OBP_Circuit( CDC* pDC, CPrintInfo* pInfo )
{
	Init( pDC );
	LPARAM lparam;
	CString str;

	// Draw each module; fill 
	int iPageNumber = 1;
	m_Pagearray.RemoveAll();


	_Pages Page;

	// Draw footer, same space needed on each page 
	DrawFooter( pDC, m_iLeftMargin, 0, 1, false );

	// Keep a copy of drawing page size
	CRect rcSave( m_PrintRect );

	// One page more if first page contains Project informations
	if( true == m_pTADSPageSetup->GetPrintPrjInfo() )
	{
		memset( &Page, 0, sizeof( Page ) );
		Page.fnewpage = true;
		Page.lpHM = 0;
		Page.lpTab = 0;
		Page.Sheet = 1;
		Page.PageIndex = 1;
		m_Pagearray.Add( Page );
		iPageNumber++;
	}

	// Loop on HydroMod
	for( BOOL fContinue = m_RankList.GetFirst( str, lparam ); TRUE == fContinue; fContinue = m_RankList.GetNext( str, lparam ) )
	{
		if( NULL == lparam )
			continue;
		
		// One page by HM Circuit
		memset( &Page, 0, sizeof( Page ) );
		Page.fnewpage = true;
		Page.lpHM = lparam;
		Page.lpTab = 0;
		Page.Sheet = 1;
		Page.PageIndex = 1;
		m_Pagearray.Add( Page );
		iPageNumber++;
	}

	pInfo->SetMaxPage( iPageNumber - 1 );
	m_iPage = 1;
	return;
}

void CRViewProj::OBP_SelectedProduct( CDC* pDC, CPrintInfo* pInfo )
{
	CContainerForPrint *pclContainerToPrint = NULL;

	if( NULL != pRViewSelProd )
	{
		pclContainerToPrint = pRViewSelProd->GetContainerToPrint();
	}

	ASSERT( NULL != pclContainerToPrint );
	
	if( NULL == pclContainerToPrint )
	{
		return;
	}

	Init( pDC );

	CString str;
	
	// Draw each module; fill 
	int iPageNumber = 1;
	m_Pagearray.RemoveAll();

	_Pages Page;

	// Draw footer, same space needed on each page 
	DrawFooter( pDC, m_iLeftMargin, 0, 1, false );
	
	// Keep a copy of drawing page size
	CRect rcSave( m_PrintRect );
	if( true == m_pTADSPageSetup->GetPrintPrjInfo() )
	{
		memset( &Page, 0, sizeof( Page ) );
		Page.fnewpage = true;
		m_Pagearray.Add( Page );
		iPageNumber++;
	}
	
	// Space available by sheet
	m_PrintRect = rcSave;
	
	// One page by HM and by sheet
	Page.fnewpage = true;
	Page.lpHM = NULL;//lparam;
	Page.lpTab = 0;
	
	// Determine space needed for header and footer. 
	int iYPos = 0;
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, 1, false );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

	iYPos = DrawShortPrjInfo( pDC, m_PrintRect.left, iYPos, false );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

	// Update size available for spread sheet.
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );
	CRect rectPrintArea( m_PrintRect );

	// Fill pages.
	BeginWaitCursor();
	bool fReturn = pclContainerToPrint->BeginPrinting();
	EndWaitCursor();
	if( false == fReturn )
		return;
	
	CPreviewDC* pPreviewDC = (CPreviewDC*)pDC;
		
	CDC* pScreenDC = GetDC();
	double dScreenHorzPixelByInch = (double)pScreenDC->GetDeviceCaps( LOGPIXELSX );
	double dScreenVertPixelByInch = (double)pScreenDC->GetDeviceCaps( LOGPIXELSY );
	double dPrinterHorzPixelByInch = (double)pPreviewDC->GetDeviceCaps( LOGPIXELSX );
	double dPrinterVertPixelByInchs = (double)pPreviewDC->GetDeviceCaps( LOGPIXELSY );
	ReleaseDC( pScreenDC );

	// Convert printable area size into screen area size.
	rectPrintArea.left = (LONG)( (double)rectPrintArea.left / dPrinterHorzPixelByInch * dScreenHorzPixelByInch );
	rectPrintArea.top = (LONG)( (double)rectPrintArea.top / dPrinterVertPixelByInchs * dScreenVertPixelByInch );
	rectPrintArea.right = (LONG)( (double)rectPrintArea.right / dPrinterHorzPixelByInch * dScreenHorzPixelByInch );
	rectPrintArea.bottom = (LONG)( (double)rectPrintArea.bottom / dPrinterVertPixelByInchs * dScreenVertPixelByInch );
		
	// In SS_PRINT.c at line 934, spread add by default 1/16" pixel to the left margin if this one is set to 0.
	// And when we will call 'PrintPreview', the job will be done in a bitmap with left, top coordinates set to 0. We must thus take in count
	// these margins.
	double dMinOffsetFactor = 16.0;
	rectPrintArea.right = (LONG)( (double)rectPrintArea.right - ( dScreenHorzPixelByInch / dMinOffsetFactor ) );
	rectPrintArea.bottom = (LONG)( (double)rectPrintArea.bottom - ( dScreenVertPixelByInch / dMinOffsetFactor ) );

	CDB_PageSetup *pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != pTADSPageSetup );

	// Loop on sheets
	for( int iLoopSheet = CDB_PageSetup::enCheck::FIRSTFIELD; iLoopSheet < CDB_PageSetup::enCheck::LASTFIELD; iLoopSheet++ )
	{
		CDB_PageSetup::enCheck eLoopSheet = (CDB_PageSetup::enCheck)iLoopSheet;

		if( false == pTADSPageSetup->GetCheck( eLoopSheet ) )
		{
			continue;
		}

		if( false == pclContainerToPrint->HasSomethingToPrint( eLoopSheet ) )
		{
			continue;
		}
		
		// Take current size of sheet in pixel (in the screen resolution, 96ppi).
		CRect rcSpreadPixel = pclContainerToPrint->GetSheetSizeInPixels( eLoopSheet );
		double dZoomFactor = (double)rectPrintArea.Width() / (double)rcSpreadPixel.Width();

		pclContainerToPrint->SetPageBreak( eLoopSheet, pDC, rectPrintArea, dZoomFactor );
		Page.Sheet = (LPARAM)iLoopSheet;
		long lPageCount = 0;
		
		// Determine number of page needed for current sheet.
		pclContainerToPrint->PrintPageCount( eLoopSheet, pDC, m_PrintRect, lPageCount );
		
		// One entry by page needed.
		for( long lLoopPage = 1; lLoopPage <= lPageCount; lLoopPage++ )
		{
			Page.PageIndex = lLoopPage;
			m_Pagearray.Add( Page );
			iPageNumber++;
		}
	}
	
	pInfo->SetMaxPage( iPageNumber - 1 );
	m_iPage = 1;
	return;
}

void CRViewProj::OBP_LogDataSumm( CDC* pDC, CPrintInfo* pInfo )
{
	Init( pDC );

	// Variables
	CString str;
	int iPageNumber = 1;
	m_Pagearray.RemoveAll();


	_Pages Page;

	// Draw footer, same space needed on each page 
	DrawFooter( pDC, m_iLeftMargin, 0, 1, false );
	
	// Keep a copy of drawing page size
	CRect rcSave( m_PrintRect );
	
	// Space available by sheet
	m_PrintRect = rcSave;
	
	// One page by HM and by sheet
	Page.fnewpage = true;
	Page.lpHM = NULL;//lparam;
	Page.lpTab = 0;
	
	// Determine space needed for header and footer. 
	int iYPos = 0;
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, 1, false );
	
	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );
	
	// Fill pages
	m_SheetLDSumm.SetRedraw( true );
	m_SheetLDSumm.ResizeColumns( (m_PrintRect.Width() * 130 ) / RIGHTVIEWWIDTH );

	Page.Sheet = 1;
	long lPageCount = 0;
	
	// Determine number of page needed for current sheet
	m_SheetLDSumm.OwnerPrintPageCount( pDC, m_PrintRect, lPageCount ); 
	
	// One entry by page needed
	for( long lLoopPage = 1; lLoopPage <= lPageCount; lLoopPage++ )
	{
		Page.PageIndex = lLoopPage;
		m_Pagearray.Add( Page );
		iPageNumber++;
	}
	pInfo->SetMaxPage( iPageNumber - 1 );
	m_iPage = 1;
	return;
}

void CRViewProj::OBP_LogData( CDC* pDC, CPrintInfo* pInfo )
{
	Init( pDC );

	// Variables
	CString str;
	int iPageNumber = 1;
	m_Pagearray.RemoveAll();


	_Pages Page;

	// Draw footer, same space needed on each page 
	DrawFooter( pDC, m_iLeftMargin, 0, 1, false );
	
	// Keep a copy of drawing page size
	CRect rcSave( m_PrintRect );
	
	// Space available by sheet
	m_PrintRect = rcSave;
	
	// One page by HM and by sheet
	Page.fnewpage = true;
	Page.lpHM = NULL;//lparam;
	Page.lpTab = 0;
	
	//Determine space needed for header and footer. 
	int iYPos = 0;
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, 1, false );
	
	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );
	
	// Fill pages
	m_SheetLogData.SetRedraw(GetCurrentLog(), true );

	Page.Sheet = 1;
	long lPageCount = 0;
	
	// Determine number of page needed for current sheet
	m_SheetLogData.OwnerPrintPageCount( pDC, m_PrintRect, lPageCount ); 
	
	// One entry by page needed
	for( long lLoopPage = 1; lLoopPage <= lPageCount; lLoopPage++ )
	{
		Page.PageIndex = lLoopPage;
		m_Pagearray.Add( Page );
		iPageNumber++;
	}
	pInfo->SetMaxPage( iPageNumber - 1 );
	m_iPage = 1;
	return;
}

void CRViewProj::OBP_QuickMeas( CDC* pDC, CPrintInfo* pInfo )
{
	Init( pDC );

	// Variables
	CString str;
	int iPageNumber = 1;
	m_Pagearray.RemoveAll();


	_Pages Page;

	// Draw footer, same space needed on each page 
	DrawFooter( pDC, m_iLeftMargin, 0, 1, false );
	
	// Keep a copy of drawing page size
	CRect rcSave( m_PrintRect );
	
	// Space available by sheet
	m_PrintRect = rcSave;
	
	// One page by HM and by sheet
	Page.fnewpage = true;
	Page.lpHM = NULL;//lparam;
	Page.lpTab = 0;
	
	//Determine space needed for header and footer. 
	int iYPos = 0;
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, 1, false );
	
	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );
	
	// Fill pages
	m_SheetQuickMeas.SetRedraw( false, true, &m_RankList );
	m_SheetQuickMeas.ResizeColumns( ( m_PrintRect.Width() * 130 ) / RIGHTVIEWWIDTH );

	Page.Sheet = 1;
	long lPageCount = 0;
	
	// Determine number of page needed for current sheet
	m_SheetQuickMeas.OwnerPrintPageCount( pDC, m_PrintRect, lPageCount ); 
	
	// One entry by page needed
	for( long lLoopPage = 1; lLoopPage <= lPageCount; lLoopPage++ )
	{
		Page.PageIndex = lLoopPage;
		m_Pagearray.Add( Page );
		iPageNumber++;
	}
	pInfo->SetMaxPage( iPageNumber - 1 );
	m_iPage = 1;
	return;
}

bool CRViewProj::PrintPreview( CDC *pDC, CSSheet *pSSheet, CRect rcSpread, CRViewProj::_Pages *pPage, bool fAllSheetInOnePage )
{
	bool fReturn = false;

	CDC MemDC;
	HDC hMemoryDC;

	if( TRUE == pDC->IsKindOf( RUNTIME_CLASS( CPreviewDC ) ) && ( hMemoryDC = ::CreateCompatibleDC( ::GetDC( NULL ) ) ) )
	{
		// If DC is in fact a Preview DC, thus we need to create a compatible DC from screen. To get handle of screen device context,
		// simply call ::GetDC with NULL parameter.

		CPreviewDC *pPreviewDC = (CPreviewDC *)pDC;
		CDC *pMemDC = CDC::FromHandle( hMemoryDC );

		// Take current size of sheet in pixel (in the screen resolution, 96ppi).
		CRect rcSpreadPixel = pSSheet->GetSheetSizeInPixels( false );

		CDC* pScreenDC = GetDC();
		double dScreenHorzPixelsByInch = (double)pScreenDC->GetDeviceCaps( LOGPIXELSX );
		double dScreenVertPixelsByInch = (double)pScreenDC->GetDeviceCaps( LOGPIXELSY );
		double dPrinterHorzPixelsByInch = (double)pPreviewDC->GetDeviceCaps( LOGPIXELSX );
		double dPrinterVertPixelsByInch = (double)pPreviewDC->GetDeviceCaps( LOGPIXELSY );
		ReleaseDC( pScreenDC );

		// Convert printable area size into screen area size.
		CRect rcFill( rcSpread );
		OffsetRect( rcFill, -rcFill.left, -rcFill.top );
		rcFill.right = (LONG)( (double)rcFill.right / dPrinterHorzPixelsByInch * dScreenHorzPixelsByInch );
		rcFill.bottom = (LONG)( (double)rcFill.bottom / dPrinterVertPixelsByInch * dScreenVertPixelsByInch );

		// Create a compatible bitmap (in regards to current preview device context) to have same colors and so on.
		// For a complete description of why we use 'rcFill' to create bitmap, see 'CSSheet::OwnerPrintDraw' method.
		HBITMAP hBitmap, hBitmapOld;
		hBitmap = CreateCompatibleBitmap( pDC->GetSafeHdc(), rcFill.Width(), rcFill.Height() );
		hBitmapOld = (HBITMAP)::SelectObject( hMemoryDC, hBitmap );

		// Fill with white color to be sure to not have black border around spreadsheet.
		CBrush Brush( RGB( 255,255,255 ) );
		pMemDC->FillRect( &rcFill, &Brush );

		// Remark: for 'fAllSheetInOnePage' argument, see 'OwnerPrintPreviewDraw' method in 'SSheet.h' file for comments.
		pSSheet->OwnerPrintPreviewDraw( pMemDC, rcFill, (short)pPage->PageIndex, fAllSheetInOnePage );

		// For debug purpose.
		// pSSheet->_SaveBitmap( pMemDC->GetSafeHdc(), hBitmap );

		if( RC_STRETCHBLT == ( pDC->GetDeviceCaps( RASTERCAPS)  & RC_STRETCHBLT ) )
			pDC->StretchBlt( rcSpread.left, rcSpread.top, rcSpread.Width(), rcSpread.Height(), pMemDC, 0, 0, rcFill.Width(), rcFill.Height(), SRCCOPY );
		else if( pDC->GetDeviceCaps( RASTERCAPS ) & RC_BITBLT )
			pDC->BitBlt( rcSpread.left, rcSpread.top, rcSpread.Width(), rcSpread.Height(), pMemDC, 0, 0, SRCCOPY );

		SelectObject( pMemDC->GetSafeHdc(), hBitmapOld );
		DeleteObject( hBitmap );
		DeleteDC( pMemDC->GetSafeHdc() );

		fReturn = true;
	}
	return fReturn;
}

// Dedicated OnPrint functions
void CRViewProj::OnPrint( CDC* pDC, CPrintInfo* pInfo ) 
{
	CDC* pReplaceDC = pDC;
	
#ifdef DEBUG_PRINTING
	if( true == m_fPrintPreviewMode )
	{
		pReplaceDC = CDC::FromHandle( m_hDCMemory );
		CBrush Brush( RGB( 255,255,255 ) );
		pReplaceDC->FillRect( &m_recScreenDP, &Brush );
	}
#endif

	if( true == m_fPrintTreeView )
		OP_TreeView( pReplaceDC, pInfo );
	else if( true == m_fPrintSelProd )
		OP_SelectedProduct( pReplaceDC, pInfo );
	else if( true == m_fPrintLogDataSumm)
		OP_LogDataSumm( pReplaceDC, pInfo );
	else if( true == m_fPrintLogData)
		OP_LogData( pReplaceDC, pInfo );
	else if( true == m_fPrintQuickMeas)
		OP_QuickMeas( pReplaceDC, pInfo );
	else if( true == m_fPrintCircuit)
		OP_Circuit( pReplaceDC, pInfo );
	else
		OP_HMCalc( pReplaceDC, pInfo );

#ifdef DEBUG_PRINTING
	if( true == m_fPrintPreviewMode )
	{
		if( pDC->GetDeviceCaps( RASTERCAPS)  & RC_STRETCHBLT )
			pDC->StretchBlt( 0, 0, pDC->GetDeviceCaps( HORZRES ), pDC->GetDeviceCaps( VERTRES ), pReplaceDC, 0, 0, m_recScreenDP.Width(), m_recScreenDP.Height(), SRCCOPY );
		else if( pDC->GetDeviceCaps( RASTERCAPS ) & RC_BITBLT )
			pDC->BitBlt( 0, 0, pDC->GetDeviceCaps( HORZRES ), pDC->GetDeviceCaps( VERTRES ), pReplaceDC, 0, 0, SRCCOPY );
	}
#endif
}

void CRViewProj::OP_TreeView( CDC* pDC, CPrintInfo* pInfo )
{
	CFont Text9;
	_CreateFont( pDC, Text9, 9, FW_NORMAL, CString( _T("Verdana") ) );
	CFont *pOldFont = pDC->SelectObject( &Text9 );
	m_PrintRect = pInfo->m_rectDraw;
	int iMargin = Getmm2pt( pDC, 10, false );
	// Margin is fixed to 10 mm
	m_PrintRect.right -= iMargin;
	m_PrintRect.left += iMargin;

	int iYPos = 0;
	bool fFirst = false;
	if( true == m_pTADSPageSetup->GetPrintPrjInfo() )
		fFirst = ( pInfo->m_nCurPage == 2 );
	else
		fFirst = ( pInfo->m_nCurPage == 1 );

	m_iLeftMargin = iMargin;
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos,pInfo->m_nCurPage, true );
	if( 1 == pInfo->m_nCurPage && true == m_pTADSPageSetup->GetPrintPrjInfo() )
	{
		iYPos = DrawPrjInfo( pDC, m_TreePrintRect.left, iYPos, true );
		DrawFooter( pDC, m_iLeftMargin, 0, pInfo->m_nCurPage, true );
		return;
	}	
	iYPos = DrawShortPrjInfo( pDC, m_TreePrintRect.left, iYPos, true );
	m_TreePrintRect.top = iYPos + m_iTabY;


	DrawFooter( pDC, m_iLeftMargin, 0, pInfo->m_nCurPage, true );

	int iPage = max( pInfo->m_nCurPage, 1 );
	m_pHMPage = m_mapArHMPages.ElementAt( iPage );
	
	
	if( m_pHMPage )
		HMNodePrinting( pDC, iPage);
	
	pDC->SelectObject( pOldFont );
	Text9.DeleteObject();

	if( pInfo->m_pPD->GetToPage() == pInfo->m_nCurPage )
		pInfo->m_bContinuePrinting = false;
}

void CRViewProj::OP_HMCalc( CDC* pDC, CPrintInfo* pInfo )
{
	_Pages Page;
	int iCurrentPage = ( pInfo->m_nCurPage > 0 ) ? pInfo->m_nCurPage - 1 : 0;

	Page = m_Pagearray.GetAt( iCurrentPage );
	CDS_HydroMod *pHM = (CDS_HydroMod *)Page.lpHM;
	
	// Update working variables
	Init( pDC );
	
	// Draw header, project info and footer
	int iYPos = 0;
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos,iCurrentPage + 1, true );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;
	if( iCurrentPage == 0 && true == m_pTADSPageSetup->GetPrintPrjInfo() )
	{
		iYPos = DrawPrjInfo( pDC, m_PrintRect.left, iYPos, true );
		DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );
		return;
	}	
	
	if( NULL == pHM )
		return;
	
	iYPos = DrawShortPrjInfo( pDC, m_PrintRect.left, iYPos, true );
	iYPos = DrawTitle( pDC, m_iLeftMargin, iYPos, pHM->GetHMName(), m_TitleBGColor );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

	m_pSheetHMCalc->Init( pHM, true );
	m_pSheetHMCalc->SetSheet( (CSheetHMCalc::SheetDescription)Page.Sheet );
	m_pSheetHMCalc->GetSSPrintFormat().fDrawColors = TRUE;


	// Modify the header if it is the Measurement sheet
	bool fShowHeaderMeas= false;
	if( Page.Sheet == m_pSheetHMCalc->GetSheetNumber( CSheetHMCalc::SheetDescription::SMeasurement ) )
		fShowHeaderMeas = true;
	
	iYPos = CDrawSet::DrawHMInfo( pDC, pHM, m_iLeftMargin, iYPos, m_PrintRect.Width(), true, fShowHeaderMeas, true );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

	DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );
	
	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );

	CRect rcSpread( m_PrintRect );
	if( false == PrintPreview ( pDC, m_pSheetHMCalc, rcSpread, &Page ) )
		m_pSheetHMCalc->OwnerPrintDraw( pDC, rcSpread, (short)Page.PageIndex );
}

void CRViewProj::OP_Circuit( CDC* pDC, CPrintInfo* pInfo )
{
	_Pages Page;
	int iCurrentPage = ( pInfo->m_nCurPage > 0 ) ? pInfo->m_nCurPage - 1 : 0;

	Page = m_Pagearray.GetAt( iCurrentPage );
	CDS_HydroMod *pHM = (CDS_HydroMod *)Page.lpHM;
	
	// Update working variables
	Init( pDC );
	int iYPos = 0;
	
	// Draw header, project info and footer
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, iCurrentPage + 1, true );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;
	if( 0 == iCurrentPage && m_pTADSPageSetup->GetPrintPrjInfo() )
	{
		iYPos = DrawPrjInfo( pDC, m_PrintRect.left, iYPos, true );
		DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );
		return;
	}	

	if( NULL == pHM )
		return;
	
	iYPos = DrawShortPrjInfo( pDC, m_PrintRect.left, iYPos, true );
	iYPos = DrawTitle( pDC, m_iLeftMargin, iYPos, pHM->GetHMName(),m_TitleBGColor);
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;
	iYPos = CDrawSet::DrawHMInfo( pDC, pHM, m_iLeftMargin, iYPos, m_PrintRect.Width(), true, false, true );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;
	DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );
	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );

	m_SheetPC2.Init( pHM, CPoint( -1, -1 ), true );

	CRect rcSpread( m_PrintRect );
	// Remark: for last argument, see 'OwnerPrintPreviewDraw' method in 'SSheet.h' file for comments.
	if( false == PrintPreview( pDC, &m_SheetPC2, rcSpread, &Page, true ) )
		m_SheetPC2.OwnerPrintDraw( pDC, rcSpread, (short)Page.PageIndex, true );
}

void CRViewProj::OP_SelectedProduct( CDC* pDC, CPrintInfo* pInfo )
{
	CContainerForPrint *pclContainerToPrint = NULL;

	if( NULL != pRViewSelProd )
	{
		pclContainerToPrint = pRViewSelProd->GetContainerToPrint();
	}

	ASSERT( NULL != pclContainerToPrint );

	if( NULL == pclContainerToPrint )
	{
		return;
	}

	_Pages Page;
	int iCurrentPage = ( pInfo->m_nCurPage > 0 ) ? pInfo->m_nCurPage - 1 : 0;

	Page = m_Pagearray.GetAt( iCurrentPage );

	// Update working variables
	Init( pDC );
	int iYPos = 0;
	
	// Draw header, project info and footer
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, iCurrentPage + 1, true );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100 ;
	if( 0 == iCurrentPage && m_pTADSPageSetup->GetPrintPrjInfo() )
	{
		iYPos = DrawPrjInfo( pDC, m_PrintRect.left, iYPos, true );
		DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );
		return;
	}	

	CSSheet* pclSheetToPrint = pclContainerToPrint->GetSheetToPrint( (CDB_PageSetup::enCheck)Page.Sheet );
	ASSERT( NULL != pclSheetToPrint );
	if( NULL == pclSheetToPrint )
		return;

	iYPos = DrawShortPrjInfo( pDC, m_PrintRect.left, iYPos, true );
	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

	iYPos += ( m_iLineSpace * m_Xfactor ) / 100;

	DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );
	
	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );
	
	pclSheetToPrint->SetSheet( (short)Page.Sheet + 1 );

	CRect rcSpread( m_PrintRect );
	if( false == PrintPreview( pDC, pclSheetToPrint, rcSpread, &Page ) )
		pclSheetToPrint->OwnerPrintDraw( pDC, rcSpread, (short)Page.PageIndex );
}

void CRViewProj::OP_LogDataSumm( CDC* pDC, CPrintInfo* pInfo )
{
	_Pages Page;
	int iCurrentPage = ( pInfo->m_nCurPage > 0 ) ? pInfo->m_nCurPage - 1 : 0;

	Page = m_Pagearray.GetAt( iCurrentPage );
	
	// Update working variables
	Init( pDC );
	int iYPos = 0;
	
	// Draw header
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, iCurrentPage + 1, true );
	
	DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );

	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );

	m_SheetLDSumm.SetSheet( (short)Page.Sheet );

	CRect rcSpread( m_PrintRect );
	if( false == PrintPreview ( pDC, &m_SheetLDSumm, rcSpread, &Page ) )
		m_SheetLDSumm.OwnerPrintDraw( pDC, rcSpread, (short)Page.PageIndex );
}

void CRViewProj::OP_LogData( CDC* pDC, CPrintInfo* pInfo )
{
	_Pages Page;
	int iCurrentPage = ( pInfo->m_nCurPage > 0 ) ? pInfo->m_nCurPage - 1 : 0;

	Page = m_Pagearray.GetAt( iCurrentPage );
	
	// Update working variables
	Init( pDC );
	int iYPos = 0;
	
	// Draw header
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, iCurrentPage + 1, true );
	
	DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );

	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );

	CRect rcSpread( m_PrintRect );
	if( false == PrintPreview ( pDC, &m_SheetLogData, rcSpread, &Page ) )
		m_SheetLogData.OwnerPrintDraw( pDC, rcSpread, (short)Page.PageIndex );
}

void CRViewProj::OP_QuickMeas( CDC* pDC, CPrintInfo* pInfo )
{
	_Pages Page;
	int iCurrentPage = ( pInfo->m_nCurPage > 0 ) ? pInfo->m_nCurPage - 1 : 0;

	Page = m_Pagearray.GetAt( iCurrentPage );
	
	// Update working variables
	Init( pDC );
	int iYPos = 0;
	
	// Draw header
	iYPos = DrawHeader( pDC, m_iLeftMargin, iYPos, iCurrentPage + 1, true );
	
	DrawFooter( pDC, m_iLeftMargin, 0, iCurrentPage + 1, true );

	// Update size available for spread sheet
	m_PrintRect.top = iYPos + Getmm2pt( pDC, 5, true );

	CRect rcSpread( m_PrintRect );
	if( false == PrintPreview ( pDC, &m_SheetQuickMeas, rcSpread, &Page ) )
		m_SheetQuickMeas.OwnerPrintDraw( pDC, rcSpread, (short)Page.PageIndex );
}

// Dedicated OnEndPrinting functions
void CRViewProj::OnEndPrinting( CDC* pDC, CPrintInfo* pInfo )
{
#ifdef DEBUG_PRINTING
	if( true == m_fPrintPreviewMode )
	{
		CDC* pMemDC = CDC::FromHandle( m_hDCMemory );
		pDC = pMemDC;
	}
#endif

	if( true == m_fPrintTreeView )
		OEP_TreeView( pDC, pInfo );
	else if( true == m_fPrintSelProd )
		OEP_SelectedProduct( pDC, pInfo );
	else if( true == m_fPrintLogDataSumm )
		OEP_LogDataSumm( pDC, pInfo );
	else if( true == m_fPrintLogData )
		OEP_LogData( pDC, pInfo );
	else if( true == m_fPrintLogData )
		OEP_QuickMeas( pDC, pInfo );
	else if( true == m_fPrintCircuit )
		OEP_Circuit( pDC, pInfo );
	else
		OEP_HMCalc( pDC, pInfo );
	
	// Take correct screen size
	CRect PageRect;
	GetClientRect( &PageRect);
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
		m_PrintRect.right = RIGHTVIEWWIDTH;
	if( m_PrintRect.Width() < m_SheetProjRect.Width() )
		m_PrintRect.right = m_SheetProjRect.right;

	CRScrollView::OnEndPrinting( pDC, pInfo );

#ifdef DEBUG_PRINTING
	if( true == m_fPrintPreviewMode )
	{
		SelectObject( m_hDCMemory, m_hBitmapOld );
		DeleteObject( m_hBitmap );
		DeleteDC( m_hDCMemory );
	}
#endif

	Invalidate();
}

void CRViewProj::OEP_TreeView( CDC* pDC, CPrintInfo* pInfo )
{
	// Clean allocated memory
	for( int i = 0; i < m_mapArHMPages.GetCount(); i++ )
	{
		if( m_mapArHMPages[i] )
			delete m_mapArHMPages[i];
	}
	return;
}

void CRViewProj::OEP_HMCalc( CDC* pDC, CPrintInfo* pInfo )
{
	// Destroy temporary sheet.
	if( NULL != m_pSheetHMCalc )
	{
		delete m_pSheetHMCalc;
		m_pSheetHMCalc = NULL;
	}
}

void CRViewProj::OEP_Circuit( CDC* pDC, CPrintInfo* pInfo )
{
	// Destroy temporary sheet
	m_SheetPC2.DestroyWindow();
}

void CRViewProj::OEP_SelectedProduct( CDC* pDC, CPrintInfo* pInfo )
{
}

void CRViewProj::OEP_LogDataSumm( CDC* pDC, CPrintInfo* pInfo )
{
	// Destroy temporary sheet
	m_SheetLDSumm.DestroyWindow();
}

void CRViewProj::OEP_LogData( CDC* pDC, CPrintInfo* pInfo )
{
	// Destroy temporary sheet
	m_SheetLogData.DestroyWindow();
}

void CRViewProj::OEP_QuickMeas( CDC* pDC, CPrintInfo* pInfo )
{
	// Destroy temporary sheet
	m_SheetQuickMeas.DestroyWindow();
}
