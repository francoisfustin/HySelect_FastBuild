#include "stdafx.h"
#include "TASelect.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include "Utilities.h"
#include "hydromod.h"
#include "HMPipes.h"
#include "HMPump.h"
#include "EnBitmap.h"
#include "drawset.h"


CDrawSet::CDrawSet(void)
{
	m_TitleFont.CreateFont(-22,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Verdana"));		
	m_SubTitleFont.CreateFont(-18,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Verdana"));
	m_Text10.CreateFont(-10,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_Text11.CreateFont(-11,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_TextBold.CreateFont(-11,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));

	m_PrintTitleFont.CreateFont(-22,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Verdana"));		
	m_PrintSubTitleFont.CreateFont(-18,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Verdana"));
	m_PrintText10.CreateFont(-10,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_PrintText11.CreateFont(-11,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));
	m_PrintTextBold.CreateFont(-11,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Arial"));

	
	m_BrushWindow.CreateSolidBrush(::GetSysColor(COLOR_WINDOW));
	m_BrushSelect.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHTTEXT));

	m_pTADS = TASApp.GetpTADS();

	m_TitleBGColor=_TAH_TITLE_MAIN;
	m_SubTitleBGColor=_TAH_TITLE_SECOND;
	m_TitleColor=_TAH_WHITE;		// White
	m_SubTitleColor=_TAH_BLACK;			// Black
	m_StringColor=_TAH_BLACK;
	
	m_iLeftMargin=3;
	m_iTopMargin=0;
	m_iLineSpace=10;
	m_iVerticalSpace=20;
	m_Xfactor=100;
	m_CurPrintIndex = 0;
	m_bContinuePrinting = false;
	m_fRedraw = false;
	m_CurrentPage = 0;
	m_CurPrintPos = 0;
	m_bPrintThisPage = false;
	m_pTADSPageSetup = NULL;
}

CDrawSet::~CDrawSet(void)
{
	m_TitleFont.DeleteObject();
	m_SubTitleFont.DeleteObject();
	m_Text10.DeleteObject();
	m_Text11.DeleteObject();
	m_TextBold.DeleteObject();

	m_PrintTitleFont.DeleteObject();
	m_PrintSubTitleFont.DeleteObject();
	m_PrintText10.DeleteObject();
	m_PrintText11.DeleteObject();
	m_PrintTextBold.DeleteObject();

	m_BrushWindow.DeleteObject();
	m_BrushSelect.DeleteObject();
}

void CDrawSet::Init(CDC* pDC)
{
	m_PrintRect.left = 0;
	m_PrintRect.top = 0;

	// HORZRES gives horizontal resolution in pixels
	// HORZSIZE gives horizontal resolution in mm
	m_PrintRect.right = pDC->GetDeviceCaps( HORZRES );
	m_PrintRect.bottom = pDC->GetDeviceCaps( VERTRES );
	
	double dRatioPixelByMeter = ( pDC->GetDeviceCaps( HORZRES ) * 1000 ) / pDC->GetDeviceCaps( HORZSIZE );

	// Margin defined in pixels for 10mm
	m_iLeftMargin = (int)( dRatioPixelByMeter / 100.0 );

	m_PrintRect.right -= m_iLeftMargin;
	m_PrintRect.left += m_iLeftMargin;

	// Right view in TASelect has a maximum fixed resolution of RIGHTVIEWWIDTH pixels (at now 900 pixels).
	// We call CDrawSet methods for 2 cases: one for printing preview and second for printing.
	//
	// In printing preview mode we have the screen resolution which generally has a width greater than height.
	// If we want that all sheet is shown, we must apply a ratio (height/right view width).
	//
	// In printing mode we have the device resolution which generally has a height greater that width (unless if we
	// are in landscape mode). In this case we must apply a ratio (width/right view width) to be sure that all
	// display is printed.
	if( m_PrintRect.Width() < m_PrintRect.Height() )
		m_Xfactor = ( m_PrintRect.Width() * 80 ) / RIGHTVIEWWIDTH;
	else
		m_Xfactor = ( m_PrintRect.Height() * 80 ) / RIGHTVIEWWIDTH;
//	m_Xfactor = 100;
}

// Draw a bitmap at x,y coordinates.
// If pRect exist adjust bitmap size to the pRect Size
// If DllName exist, try to load with BmpID from this DLL
// If DllName doesn't exist, try to load in program resources with BmpID in BMP format
// If fIsGIF is set to true, try to load in program resources with BmpID and GIF format
// return coordinates in pRect
void CDrawSet::DrawBmp( CDC* pDC, int x, int y, CString DllName, int BmpID, bool fIsGIF, CRect *pRect, bool DrawNow /* true */)
{
	CBitmap bmp, *pbmp = NULL;
	CEnBitmap EnBmp;

	// Load bmp from DLL if name exist
	HMODULE hmod = NULL;
	HBITMAP hbmp = NULL;
	if( false == DllName.IsEmpty() )
	{
		hmod = TASApp.GetDLLHandle( (LPCTSTR)DllName );
		EnBmp.LoadImage( BmpID, _T("GIF"), hmod );
		hbmp = HBITMAP( EnBmp );
	}

	if( NULL == hbmp)
	{
		// Load internal Bitmap
		if( false == fIsGIF )
		{
			bmp.LoadBitmap( BmpID );
			pbmp = &bmp;
		}
		else
		{
			EnBmp.LoadImage( BmpID, L"GIF" );
			hbmp = HBITMAP( EnBmp );
			pbmp = bmp.FromHandle( hbmp );
		}
	}
	else
		pbmp = bmp.FromHandle( hbmp );

	// Retrieve size of Bitmap
	BITMAP BM;
	pbmp->GetObject( sizeof(BM), &BM );
	
	// If pRect is empty fill with bmp size
	if( TRUE == pRect->IsRectEmpty() )
		pRect->SetRect( x, y, x + ( BM.bmWidth * m_Xfactor ) / 100, y + ( BM.bmHeight * m_Xfactor ) / 100 );	
	else									// else adjust pRect with x and y
		pRect->SetRect( x, y, x + pRect->Width(), y + pRect->Height() );

	if( true == DrawNow )
	{
		CBitmap BigBmp;
		CDC MemDcbb, MemDc;
		MemDc.CreateCompatibleDC( NULL );
		CBitmap *pOldBmp = MemDc.SelectObject( pbmp );
		
		//	StretchBlt is not supported by all device
		//  Create a big Bitmap in memory, stretch the bitmap in this memory space 
		//	and  use BitBlt to print this new Big bitmap
		//
		BigBmp.CreateCompatibleBitmap( pDC, pRect->Width(), pRect->Height() );
		MemDcbb.CreateCompatibleDC( NULL );
		MemDcbb.SelectObject( &BigBmp );
		MemDcbb.StretchBlt( 0, 0, pRect->Width(), pRect->Height(), &MemDc, 0, 0, BM.bmWidth, BM.bmHeight, SRCCOPY );

		if( pDC->GetDeviceCaps( RASTERCAPS ) & RC_STRETCHBLT )
			pDC->StretchBlt( x, y, pRect->Width(), pRect->Height(), &MemDc, 0, 0, BM.bmWidth, BM.bmHeight, SRCCOPY );
		else if( pDC->GetDeviceCaps( RASTERCAPS ) & RC_BITBLT )
			pDC->BitBlt( x, y, pRect->Width(), pRect->Height(), &MemDcbb, 0, 0, SRCCOPY );

		MemDc.SelectObject( pOldBmp );
		
	}
	pbmp->DeleteObject();
	EnBmp.DeleteObject();
}
CRect CDrawSet::DrawTALogo(CDC* pDC, bool bDrawNow)
{
	int LogoID = IDB_LOGOTA_PRINT;
	int LogoWidth = Getmm2pt(pDC,36,false);

	// Load Bitmap
	CBitmap bmp;
	bmp.LoadBitmap( LogoID );
	// Retrieve bitmap size
	BITMAP BM;
	bmp.GetObject( sizeof( BM ), &BM );
	// Compute Height based on width
	int LogoHeight = (int)((double) LogoWidth/ (double)BM.bmWidth * (double)BM.bmHeight + 0.5);

	int iPhysOffsetX = pDC->GetDeviceCaps(PHYSICALOFFSETX);
	int iPhysOffsetY = pDC->GetDeviceCaps(PHYSICALOFFSETY);
	int iPhysWidth = pDC->GetDeviceCaps(PHYSICALWIDTH);
	int iPhysHeight = pDC->GetDeviceCaps(PHYSICALHEIGHT);

	int Spacefromrightborder = iPhysWidth - Getmm2pt(pDC,10,false);

	// Create rectangle position for the bmp
	CRect rect;
	rect.top = max(0,Getmm2pt(pDC,10,false)-iPhysOffsetY);			//10mm from top -> to get 15mm !!! for goto txt 
	rect.right = iPhysWidth - Getmm2pt(pDC,15,false);				//pDC->GetDeviceCaps(HORZRES ) - Getmm2pt(pDC,10,false);			//10mm from right
	rect.left = rect.right-LogoWidth;
	rect.bottom = rect.top + LogoHeight;

	DrawBmp( pDC, rect.left, rect.top, _T(""), LogoID, false, &rect, bDrawNow );

	return rect;
}

CRect CDrawSet::DrawCustoLogo(CDC* pDC, bool bDrawNow)
{
	int LogoWidth = Getmm2pt(pDC,36,false);

	// Load Bitmap
	CEnBitmap bmp;
	CDS_UserRef *pUserRef =  (CDS_UserRef*)TASApp.GetpUserDB()->Get( _T("USER_REF") ).MP;

	if( 0 == StringCompare( pUserRef->GetString( CDS_UserRef::Path ), CteEMPTY_STRING ) )
	{
		return CRect(0, 0, 0, 0);
	}

	HBITMAP hBitMap = bmp.LoadImageFile( pUserRef->GetString( CDS_UserRef::Path ), -1, 90, 70 );
	bool ret = bmp.Attach( hBitMap );

	// Retrieve bitmap size
	BITMAP BM;
	bmp.GetObject( sizeof( BM ), &BM );
	// Compute Height based on width
	int LogoHeight = (int)((double) LogoWidth/ (double)BM.bmWidth * (double)BM.bmHeight + 0.5);

	int iPhysOffsetX = pDC->GetDeviceCaps(PHYSICALOFFSETX);
	int iPhysOffsetY = pDC->GetDeviceCaps(PHYSICALOFFSETY);
	int iPhysWidth = pDC->GetDeviceCaps(PHYSICALWIDTH);
	int iPhysHeight = pDC->GetDeviceCaps(PHYSICALHEIGHT);

	// Create rectangle position for the bmp
	CRect rect;
	rect.top = max(0,Getmm2pt(pDC,10,false)-iPhysOffsetY);			//10mm from top -> to get 15mm !!! for goto txt 
	rect.right = Getmm2pt( pDC, 10, false ) + LogoWidth;				//pDC->GetDeviceCaps(HORZRES ) - Getmm2pt(pDC,10,false);			//10mm from right
	rect.left = rect.right-LogoWidth;
	rect.bottom = rect.top + LogoHeight;

	if( bDrawNow )
	{
		CBitmap BigBmp;
		CDC MemDcbb, MemDc;
		MemDc.CreateCompatibleDC( NULL );
		CBitmap *pOldBmp = MemDc.SelectObject( &bmp );

		//	StretchBlt is not supported by all device
		//  Create a big Bitmap in memory, stretch the bitmap in this memory space 
		//	and  use BitBlt to print this new Big bitmap
		//
		BigBmp.CreateCompatibleBitmap( pDC, rect.Width(), rect.Height() );
		MemDcbb.CreateCompatibleDC( NULL );
		MemDcbb.SelectObject( &BigBmp );
		MemDcbb.StretchBlt( 0, 0, rect.Width(), rect.Height(), &MemDc, 0, 0, BM.bmWidth, BM.bmHeight, SRCCOPY );

		if( pDC->GetDeviceCaps( RASTERCAPS ) & RC_STRETCHBLT )
			pDC->StretchBlt( rect.left, rect.top, rect.Width(), rect.Height(), &MemDc, 0, 0, BM.bmWidth, BM.bmHeight, SRCCOPY );
		else if( pDC->GetDeviceCaps( RASTERCAPS ) & RC_BITBLT )
			pDC->BitBlt( rect.left, rect.top, rect.Width(), rect.Height(), &MemDcbb, 0, 0, SRCCOPY );

		MemDc.SelectObject( pOldBmp );
	}

	return rect;
}

int CDrawSet::DrawHeader( CDC* pDC, int x, int y, int page, bool DrawNow )
{
	CRect rect( 0, 0, 0, 0 );
	int pageWidth, botBmp;
	CString HeaderTxt, strPage;
	CFont HeaderFont;
	LOGFONT	lf;
	int TextAlignment;
	COLORREF headercolor;

	pageWidth = m_PrintRect.Width();								// used to compute free space between logo and page number

	if( ( 1 == page )													// First Page 
		&& ( true == m_pTADSPageSetup->GetFirstPageDifferent() ) )
	{
		// meter to inch to point
		int margin = (int)( m_pTADSPageSetup->GetMargin( epsFIRSTHEADER ) * 100 / 2.54 * 72 + .5 );
		y += ( margin * m_Xfactor ) / 100;
		botBmp = y;
		m_PrintRect.top += ( margin * m_Xfactor ) / 100;
		TextAlignment = m_pTADSPageSetup->GetAlignment( epsFIRSTHEADER );

		// Logo Exist ?
		if( m_pTADSPageSetup->GetFirstLogo() )
		{
			CRect bmpRect = DrawTALogo(pDC,DrawNow);
			botBmp = bmpRect.bottom;
			y = botBmp+ ( m_iVerticalSpace * m_Xfactor ) / 100;
		}

		// HYS-1090: Customer Logo Exist ?
		if( m_pTADSPageSetup->GetFirstCustoLogo() )
		{
			CRect bmpRect = DrawCustoLogo(pDC,DrawNow);
			if( false == bmpRect.IsRectNull() )
			{
				botBmp = bmpRect.bottom;
				y = botBmp + ( m_iVerticalSpace * m_Xfactor ) / 100;
			}
		}
		
		headercolor = m_pTADSPageSetup->GetFontColor( epsFIRSTHEADER );
		HeaderTxt = m_pTADSPageSetup->GetText( epsFIRSTHEADER );
		m_pTADSPageSetup->GetLogFont( epsFIRSTHEADER, &lf );

		// Get equivalent font in regards to current device context resolution
		_CreateFont( pDC, HeaderFont, lf.lfHeight, lf.lfWeight, CString( lf.lfFaceName) );
	}
	else															// Other Pages
	{
		// meter to inch to point
		int margin = (int)(m_pTADSPageSetup->GetMargin( epsHEADER ) * 100 / 2.54 * 72 + .5);
		y += ( margin * m_Xfactor ) / 100;
		botBmp = y;		
		m_PrintRect.top += ( margin * m_Xfactor ) / 100;
		TextAlignment = m_pTADSPageSetup->GetAlignment( epsHEADER );
		if( m_pTADSPageSetup->GetLogo() )
		{
			CRect bmpRect = DrawTALogo(pDC,DrawNow);
			botBmp = bmpRect.bottom;
			// Update free width
			y = botBmp+ ( m_iVerticalSpace * m_Xfactor ) / 100;
		}
 		// HYS-1090:
		if( m_pTADSPageSetup->GetCustoLogo() )
		{
			CRect bmpRect = DrawCustoLogo(pDC,DrawNow);
			if( false == bmpRect.IsRectNull() )
			{
				botBmp = bmpRect.bottom;
				// Update free width
				y = botBmp + ( m_iVerticalSpace * m_Xfactor ) / 100;
			}
		}

		headercolor = m_pTADSPageSetup->GetFontColor( epsHEADER );
		HeaderTxt = m_pTADSPageSetup->GetText( epsHEADER );
		
		m_pTADSPageSetup->GetLogFont( epsHEADER, &lf );

		// Get equivalent font in regards to current device context resolution
		_CreateFont( pDC, HeaderFont, lf.lfHeight, lf.lfWeight, CString( lf.lfFaceName) );
	}


	CFont *pOldFont = pDC->SelectObject(&HeaderFont);
	COLORREF OldTextColor = pDC->SetTextColor(headercolor);
	int OldBkMode = pDC->SetBkMode(TRANSPARENT);
	pageWidth -= ( m_iVerticalSpace * m_Xfactor ) / 100;

	// Compute text height
	rect.SetRect( m_PrintRect.left, y/*m_PrintRect.top*/, 0, 0 );
	pDC->DrawText( HeaderTxt, &rect, DT_LEFT | DT_CALCRECT | DT_NOPREFIX );
	
	// Rect is now updated with height and width of text
	CRect DrawRect( rect );
	DrawRect.bottom = DrawRect.top + rect.Height();
	switch( TextAlignment )
	{
		// Left
		case 0:
			DrawRect.left = x;
			DrawRect.right = DrawRect.right + pageWidth;
			break;

		// Center
		case 1:
			DrawRect.left = x + ( pageWidth - rect.Width() ) / 2;
			DrawRect.right = DrawRect.left + rect.Width();
			break;

		// Right
		case 2:
			DrawRect.left = x + pageWidth - rect.Width();
			DrawRect.right = DrawRect.left + rect.Width();
			break;
	}
	if( true == DrawNow )
		pDC->DrawText( HeaderTxt, DrawRect, DT_LEFT | DT_NOPREFIX );

	// Restore GDI
	pDC->SelectObject( pOldFont );
	pDC->SetTextColor( OldTextColor );
	pDC->SetBkMode( OldBkMode );
	HeaderFont.DeleteObject();

	// Take the most important Height
	if( botBmp > DrawRect.bottom )
		m_PrintRect.top = botBmp;
	else
		m_PrintRect.top = DrawRect.bottom;

	return m_PrintRect.top;
}

CRect CDrawSet::_DrawTagline( CDC* pDC, bool fDrawNow )
{
	int LogoID = IDB_IMI_TAGLINE;
	int LogoWidth = Getmm2pt(pDC, 36, false);

	// Load Bitmap
	CBitmap bmp;
	bmp.LoadBitmap(LogoID);
	// Retrieve bitmap size
	BITMAP BM;
	bmp.GetObject(sizeof(BM), &BM);
	// Compute Height based on width
	int LogoHeight = (int)((double)LogoWidth / (double)BM.bmWidth * (double)BM.bmHeight + 0.5);

	int iPhysOffsetX = pDC->GetDeviceCaps(PHYSICALOFFSETX);
	int iPhysOffsetY = pDC->GetDeviceCaps(PHYSICALOFFSETY);
	int iPhysWidth = pDC->GetDeviceCaps(PHYSICALWIDTH);
	int iPhysHeight = pDC->GetDeviceCaps(PHYSICALHEIGHT);

	int Spacefromrightborder = iPhysWidth - Getmm2pt(pDC, 10, false);

	// Create rectangle position for the bmp
	CRect rect;
	rect.bottom = min(iPhysHeight, iPhysHeight - Getmm2pt(pDC, 10, false) - iPhysOffsetY);			//10mm from bottom -> to get 15mm !!! for goto txt 
	rect.top = rect.bottom - LogoHeight;
	rect.right = iPhysWidth - Getmm2pt(pDC, 15, false);				//pDC->GetDeviceCaps(HORZRES ) - Getmm2pt(pDC,10,false);			//10mm from right
	rect.left = rect.right - LogoWidth;
	
	DrawBmp(pDC, rect.left, rect.top, _T(""), LogoID, false, &rect, fDrawNow);

	return rect;
}

int CDrawSet::DrawFooter(CDC* pDC, int x, int y,int page, bool DrawNow )
{
	CRect rect( 0, 0, 0, 0);
	int pageWidth, topBmp;	
	CString FooterTxt, strPage;
	CFont FooterFont;
	LOGFONT	lf;
	int TextAlignment;
	COLORREF FooterColor;

	// Used to compute free space between page beginning and logo.
	pageWidth = m_PrintRect.Width() - x;

	// First Page. 
	if( 1 == page && true == m_pTADSPageSetup->GetFirstPageDifferent() )
	{
		m_PrintRect.bottom = _DrawTagline(pDC, DrawNow && m_pTADSPageSetup->GetFirstLogo()).top;
		// FOOTER MARGIN

		// Meter to inch to point.
		int margin = (int)( m_pTADSPageSetup->GetMargin( epsFIRSTFOOTER ) * 100 / 2.54 * 72 + .5 );
		m_PrintRect.bottom -= ( margin * m_Xfactor ) / 100;

		// Draw Logo and select font for footer text.
		topBmp = m_PrintRect.bottom;
		TextAlignment = m_pTADSPageSetup->GetAlignment( epsFIRSTFOOTER );
		FooterColor = m_pTADSPageSetup->GetFontColor( epsFIRSTFOOTER );
		FooterTxt = m_pTADSPageSetup->GetText( epsFIRSTFOOTER );
		m_pTADSPageSetup->GetLogFont( epsFIRSTFOOTER, &lf );
		
		// Get equivalent font in regards to current device context resolution
		_CreateFont( pDC, FooterFont, lf.lfHeight, lf.lfWeight, CString( lf.lfFaceName) );
	}
	else
	{
		m_PrintRect.bottom = _DrawTagline(pDC, DrawNow && m_pTADSPageSetup->GetLogo()).top;
		// Other Pages

		// FOOTER MARGIN

		// Meter to inch to point
		int margin = (int)( m_pTADSPageSetup->GetMargin( epsFOOTER ) * 100 / 2.54 * 72 + .5);
		m_PrintRect.bottom -= ( margin * m_Xfactor ) / 100;

		// Draw Logo and select font for footer text
		topBmp = m_PrintRect.bottom;
		TextAlignment = m_pTADSPageSetup->GetAlignment( epsFOOTER );
		FooterColor = m_pTADSPageSetup->GetFontColor( epsFOOTER );
		FooterTxt = m_pTADSPageSetup->GetText( epsFOOTER );
		m_pTADSPageSetup->GetLogFont( epsFOOTER, &lf );
		
		// Get equivalent font in regards to current device context resolution
		_CreateFont( pDC, FooterFont, lf.lfHeight, lf.lfWeight, CString( lf.lfFaceName) );
	}

	// Save GDI, print page number
	CFont clText10;
	_CreateFont(pDC, clText10, 10, FW_NORMAL, CString(_T("Arial")));
	CFont *pOldFont = pDC->SelectObject(&clText10);

	COLORREF OldTextColor = pDC->SetTextColor(m_StringColor);
	int OldBkMode = pDC->SetBkMode(TRANSPARENT);

	// Print The page number
	strPage.FormatMessage(_T("%1!d!"), page);

	// Update page width
	CSize sizeText = pDC->GetTextExtent(strPage);
	pageWidth -= sizeText.cx;

	// Write page number
	if (true == DrawNow)
		pDC->TextOut(x, m_PrintRect.bottom + Getmm2pt(pDC, 15, true), strPage);

	clText10.DeleteObject();

	// Select header font
	pDC->SelectObject( &FooterFont );
	pDC->SetTextColor( FooterColor );
	pDC->SetBkMode( TRANSPARENT );

	// Compute text height
	rect.SetRect( m_PrintRect.left, m_PrintRect.bottom, 0, 0 );
	pDC->DrawText( FooterTxt, &rect, DT_LEFT | DT_CALCRECT | DT_NOPREFIX );

	// Rect is now updated with height and width of text.
	CRect DrawRect( rect );
	DrawRect.top = rect.top - rect.Height();	
	DrawRect.bottom = DrawRect.top + rect.Height();
	switch( TextAlignment )
	{
		// Left
		case 0:
			break;

		// Center
		case 1:
			DrawRect.left = x + ( pageWidth - rect.Width() ) / 2;
			DrawRect.right = DrawRect.left + rect.Width();
			break;
		
		// Right
		case 2:
			DrawRect.left = x + pageWidth - rect.Width();
			DrawRect.right = DrawRect.left + rect.Width();
			break;
	}
	
	if( true == DrawNow )
		pDC->DrawText( FooterTxt, DrawRect, DT_LEFT | DT_NOPREFIX );

	// Restore GDI
	pDC->SelectObject( pOldFont );
	pDC->SetTextColor( OldTextColor );
	pDC->SetBkMode( OldBkMode );
	FooterFont.DeleteObject();

	// Take the most important Height
	if( topBmp < DrawRect.top )
		m_PrintRect.bottom = topBmp;
	else
		m_PrintRect.bottom = DrawRect.top;
	
	// Keep a space of 5mm
	m_PrintRect.bottom -= Getmm2pt( pDC, 5, true );

	return y;
}

int CDrawSet::Getmm2pt( CDC* pDC, double mm, bool vertical/*true*/)
{
	if( true == vertical )
	{
		// Vertical resolution pt/mm
		int iVertRes = ( pDC->GetDeviceCaps( VERTRES ) * 100 ) / pDC->GetDeviceCaps( VERTSIZE );
		return ( (int)(( iVertRes * mm ) / 100) );
	}
	else
	{
		// Horizontal resolution pt/mm
		int iHorzRes = ( pDC->GetDeviceCaps( HORZRES ) * 100 ) / pDC->GetDeviceCaps( HORZSIZE );
		return ( (int)(( iHorzRes * mm) / 100) );
	}
}

// Draw Title 'title' on the pDC at position x,y
//	Use : text font is m_TitleFont; text color is m_TitleColor
//		  rectangle drawn throughout the page width 
//        with background color m_TitleBGColor
//	Return bottom y coordinate or -1 if there is not enough space
int CDrawSet::DrawTitle( CDC* pDC, int x, int y, CString title, COLORREF BGcolor, bool DrawNow /* true */)
{
	// 2 Lines Before
	y += ( 2 * m_iLineSpace * m_Xfactor ) / 100;
	
	CFont clText;
	_CreateFont( pDC, clText, 20, FW_BOLD, CString( _T("Verdana") ) );

	CFont *pOldFont;
	pOldFont = pDC->SelectObject( &clText );
	COLORREF OldTextColor = pDC->SetTextColor( m_TitleColor );
	int OldBkMode = pDC->SetBkMode( TRANSPARENT );

	CBrush BGBrush;
	BGBrush.CreateSolidBrush( BGcolor );
	CSize sizeText = pDC->GetTextExtent( title );

	// For printing check page break
	if( TRUE == pDC->IsPrinting() )
	{
		if( ( sizeText.cy + y ) > m_PrintRect.bottom )
		{
			// Restore GDI
			pDC->SelectObject( pOldFont );
			pDC->SetTextColor( OldTextColor );
			pDC->SetBkMode( OldBkMode );
			BGBrush.DeleteObject();	
			clText.DeleteObject();
			return -1;
		}
	}

	if( true == DrawNow )
	{
		CRect rect( x, y, x + m_PrintRect.Width(), y + sizeText.cy );
		pDC->FillRect( rect, &BGBrush );
		pDC->TextOut( x + ( m_PrintRect.Width() - sizeText.cx ) / 2, y, title );
	}

	// Restore GDI
	pDC->SelectObject( pOldFont );
	pDC->SetTextColor( OldTextColor );
	pDC->SetBkMode( OldBkMode );
	BGBrush.DeleteObject();
	clText.DeleteObject();

	return ( y + sizeText.cy );
}

//Draw SubTitle 'title' on the pDC at position x,y
//	Use : text font is m_SubTitleFont; text color is m_SubTitleColor
//		  rectangle drawn on page width/2 
//        with background color m_SubTitleBGColor
//	Return bottom y coordinate
int CDrawSet::DrawSubTitle( CDC* pDC, int x, int y, CString title, bool DrawNow /* true */)
{
	// 2 Lines Before
	y += ( 2 * m_iLineSpace * m_Xfactor) / 100;

	CFont clText18;
	_CreateFont( pDC, clText18, 18, FW_NORMAL, CString( _T("Verdana") ) );

	CFont *pOldFont = pDC->SelectObject( &clText18 );

	COLORREF OldTextColor = pDC->SetTextColor( m_SubTitleColor );
	int OldBkMode = pDC->SetBkMode( TRANSPARENT );
	
	CBrush BGBrush;
	BGBrush.CreateSolidBrush( m_SubTitleBGColor );
	CPen pen;
	pen.CreatePen( PS_SOLID, 1, m_SubTitleColor );
	
	CSize sizeText = pDC->GetTextExtent( title );

	// For printing check page break
	if( TRUE == pDC->IsPrinting() )
	{
		if( ( sizeText.cy + y ) > m_PrintRect.bottom ) 
		{
			// Restore GDI
			pDC->SelectObject( pOldFont );
			pDC->SetTextColor( OldTextColor );
			pDC->SetBkMode( OldBkMode );
			BGBrush.DeleteObject();
			pen.DeleteObject();
			clText18.DeleteObject();
			return -1;
		}
	}

	CPen *pOldPen;
	pOldPen = pDC->SelectObject( &pen );

	if( true == DrawNow )
	{
		CRect rect( x, y, x + m_PrintRect.Width() / 4 * 3, y + sizeText.cy );
		pDC->FillRect( rect, &BGBrush );
		pDC->TextOut( x + ( m_PrintRect.Width() / 4 * 3 - sizeText.cx ) / 2, y, title );
	
		pDC->MoveTo( CPoint( x, y ) );
		pDC->LineTo( CPoint( x + m_PrintRect.Width() / 4 * 3, y ) );
		pDC->MoveTo( CPoint( x, y + sizeText.cy ) );
		pDC->LineTo( CPoint( x + m_PrintRect.Width() / 4 * 3, y + sizeText.cy ) );
	}

	// Restore GDI
	pDC->SelectObject( pOldPen );
	pDC->SelectObject( pOldFont );
	pDC->SetTextColor( OldTextColor );
	pDC->SetBkMode( OldBkMode );
	
	BGBrush.DeleteObject();
	pen.DeleteObject();
	clText18.DeleteObject();

	return ( y + sizeText.cy );
}

int CDrawSet::DrawTextLine( CDC* pDC, int &x, int y, int maxx, CString str, bool DrawNow /* true */)
{
	if( true == str.IsEmpty() )
		return y;
	
	CRect rect( x, y, maxx, 0 );
	
	CRect rectCpy( x, y, maxx, 0 );
	// HYS-1090: Test if the string can be cut by using the spaces
	pDC->DrawText( str, &rectCpy, DT_LEFT | DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK );
	if( rectCpy.right > maxx )
	{
		// The string does not have any space to allow DT_WORDBREAK to break the line. So we use elipsis that 
		// work find if there is not an other line after. If there is, juste cut the line
		pDC->DrawText( str, &rect, DT_LEFT | DT_CALCRECT | DT_NOPREFIX | DT_END_ELLIPSIS );

		if( true == DrawNow )
			pDC->DrawText( str, &rect, DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS );
	}
	else
	{
		rect.CopyRect( rectCpy );
		if( true == DrawNow )
			pDC->DrawText( str, &rect, DT_LEFT | DT_NOPREFIX | DT_WORDBREAK );
	}
	y += rect.Height();
	x = rect.right;
	return y;
}

// Draw Project information return the position 
int CDrawSet::DrawPrjInfo( CDC* pDC, int x, int y, bool DrawNow /* true */)
{
	CDS_UserRef *pUserRef = m_pTADS->GetpUserRef();
	ASSERT( NULL != pUserRef );

	CDS_ProjectRef *pPrjRef = (CDS_ProjectRef *)m_pTADS->Get( _T("PROJECT_REF") ).MP;
	ASSERT( NULL != pPrjRef );

	// Create font in regards to device capabilities
	CFont clText10, clText10Bold;
	_CreateFont( pDC, clText10, 10, FW_NORMAL, CString( _T("Arial") ) );
	_CreateFont( pDC, clText10Bold, 10, FW_BOLD, CString( _T("Arial") ) );
	
	// Check if something will be displayed
	// flag is a combination of 0x1 (date is printed), 0x10 ('Project' block of project is displayed) 
	// and 0x100 (for the 'Selection by' block) 
	//
	int flag = 0;
	if( true == m_pTADSPageSetup->GetShowDate() )
		flag = 1;
	if ( m_pTADSPageSetup->GetShowComment() ||	m_pTADSPageSetup->GetShowCustRef() || 
	     m_pTADSPageSetup->GetShowOtherInfo() || m_pTADSPageSetup->GetShowPrjName() || 
	     m_pTADSPageSetup->GetShowUserRef() )
	{
		for( int i = 0; i < pPrjRef->GetNumofLines(); i++ )
		{
			if( *pPrjRef->GetString( i ) )
				flag |= 0x10;
		}

		for( int i = 0; i < pUserRef->GetNumofLines(); i++ )
		{
			if( *pUserRef->GetString( i ) )
				flag |= 0x100;
		}
	}

	if( 0 != flag )
		y += ( 2 * m_iLineSpace * m_Xfactor) / 100 ;				// 2 Lines Before
	else
		return y;

	CFont *pOldFont = pDC->SelectObject( &clText10 );
	COLORREF OldTextColor = pDC->SetTextColor( m_StringColor );
	int OldBkMode = pDC->SetBkMode( TRANSPARENT );

	CPen pen;
	pen.CreatePen( PS_SOLID, 1, m_StringColor );
	CPen *pOldPen = pDC->SelectObject( &pen );

	CString str;
	int left = 0;
	int iTab1 = m_PrintRect.left + Getmm2pt( pDC, 5, false );
	int iTab2 = m_PrintRect.left + Getmm2pt( pDC, 10, false );

	int	maxx = m_PrintRect.Width();
	if( true == m_pTADSPageSetup->GetShowDate() )
	{
		CFont clText11, clText11Bold;
		_CreateFont( pDC, clText11, 11, FW_NORMAL, CString( _T("Arial") ) );
		_CreateFont( pDC, clText11Bold, 11, FW_BOLD, CString( _T("Arial") ) );

		// Print "Date:"
		str = TASApp.LoadLocalizedString( IDS_PRINT_DATE );
		str += _T(" ");
		CFont *pMemFont = pDC->SelectObject( &clText11Bold );
		left = m_PrintRect.left;
		DrawTextLine( pDC, left, y, maxx, str, DrawNow );
		
		// Print time
		pDC->SelectObject( &clText11 );
		COleDateTime tdate = COleDateTime::GetCurrentTime();
		str = tdate.Format( LOCALE_NOUSEROVERRIDE | VAR_DATEVALUEONLY );
		y = DrawTextLine( pDC, left, y, maxx, str, DrawNow );

		pDC->SelectObject( pMemFont );
		clText11.DeleteObject();
		clText11Bold.DeleteObject();
	}

	// Something to draw !
	if( ( flag & 0x110) != 0 )
	{
		int posafterdate = y;
		int ytmp = y;

		// Project information block
		left = iTab1;
		if( m_pTADSPageSetup->GetShowPrjName() && *pPrjRef->GetString( CDS_ProjectRef::Name ) )
		{
			// Draw project name (big title)
			str = TASApp.LoadLocalizedString( IDS_PRINT_PROJECT );
			str += _T(" ");
			str += pPrjRef->GetString( CDS_ProjectRef::Name );
			ytmp = DrawTitle( pDC, m_PrintRect.left, ytmp, str, m_TitleBGColor, DrawNow );
			ytmp += Getmm2pt( pDC, 10 );
			
			// Add separation only if there is at least customer references or comments to show
			if( true == DrawNow &&
				true == m_pTADSPageSetup->GetShowCustRef() ||
				( true == m_pTADSPageSetup->GetShowComment() && *pPrjRef->GetString( CDS_ProjectRef::Comment ) ) )
			{
				pDC->MoveTo( CPoint( x, ytmp ) );
				pDC->LineTo( CPoint( x + m_PrintRect.Width(), ytmp ) );
			}

			// Draw project name
			pDC->SelectObject( &clText10Bold );
			ytmp += ( m_iLineSpace * m_Xfactor) / 100;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
		}
		else
		{
			// _T("A") because we just want title height
			ytmp = DrawTitle( pDC, m_PrintRect.left, ytmp, CString( _T("A") ), m_TitleBGColor, false );
			ytmp += Getmm2pt( pDC, 10 );

			// Add separation only if there is at least customer references or comments to show
			if( true == DrawNow &&
				true == m_pTADSPageSetup->GetShowCustRef() ||
				( true == m_pTADSPageSetup->GetShowComment() && *pPrjRef->GetString( CDS_ProjectRef::Comment ) ) )
			{
				pDC->MoveTo( CPoint( x, ytmp ) );
				pDC->LineTo( CPoint( x + m_PrintRect.Width(), ytmp ) );
				ytmp += ( m_iLineSpace * m_Xfactor) / 100;
			}
		}

		// Draw each field
		pDC->SelectObject( &clText10 );
		if( true == m_pTADSPageSetup->GetShowCustRef() )
		{
			// Customer's company
			str = pPrjRef->GetString( CDS_ProjectRef::CustCompany );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );

			// Customer's name and reference
			str = pPrjRef->GetString( CDS_ProjectRef::CustName );
			if( false == str.IsEmpty() && *pPrjRef->GetString( CDS_ProjectRef::CustName ) )
				str += ';';
			str += pPrjRef->GetString( CDS_ProjectRef::CustRef );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );

			// Customer's address
			str = pPrjRef->GetString( CDS_ProjectRef::CustAddress );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
	
			// Zip, town and country
			str = _T("");
			str = pPrjRef->GetString( CDS_ProjectRef::CustZip );
			if( false == str.IsEmpty() )
				str += CString(' ');
			str += pPrjRef->GetString( CDS_ProjectRef::CustTown );
			if( *( pPrjRef->GetString( CDS_ProjectRef::CustCountry ) ) )	// Not an empty string
			{
				if( false == str.IsEmpty() )
					str += CString( ' ' );
				str += CString( '(' ) + pPrjRef->GetString( CDS_ProjectRef::CustCountry ) + CString( ')' );
			}
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );

			// Tel, Fax
			str = _T("");
			CString str2;
			str = pPrjRef->GetString( CDS_ProjectRef::CustTel );
			if( false == str.IsEmpty() ) 
			{
				str2 = TASApp.LoadLocalizedString( IDS_PRINT_TEL );
				str = str2 + str;
			}
			if( *( pPrjRef->GetString( CDS_ProjectRef::CustFax ) ) )	// Not an empty string
			{
				if( false == str.IsEmpty() )
					str += CString(';');
				str2 = TASApp.LoadLocalizedString( IDS_PRINT_FAX );
				str += ( str2 + pPrjRef->GetString( CDS_ProjectRef::CustFax ) );
			}
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );

			// e-mail
			str = pPrjRef->GetString( CDS_ProjectRef::CustEmail );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
		}

		if( true == m_pTADSPageSetup->GetShowComment() && *pPrjRef->GetString( CDS_ProjectRef::Comment ) )
		{
			if( ytmp != posafterdate )
				ytmp += ( m_iLineSpace * m_Xfactor ) / 100;
			str = TASApp.LoadLocalizedString( IDS_PRINT_COMMENT );
			pDC->SelectObject( &clText10Bold );
			left = iTab1;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
	
			pDC->SelectObject( &clText10 );
			str = pPrjRef->GetString( CDS_ProjectRef::Comment );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
		}

		if( ( ( flag & 0x100 ) != 0 ) && ( true == m_pTADSPageSetup->GetShowUserRef() ) )
		{
			ytmp += ( m_iLineSpace * m_Xfactor ) / 100;
			if( true == DrawNow )
			{
				pDC->MoveTo( CPoint( x, ytmp ) );
				pDC->LineTo( CPoint( x + m_PrintRect.Width(), ytmp ) );
			}
			ytmp += ( m_iLineSpace * m_Xfactor ) / 100;

			// Draw title "Selection By:"
			str = TASApp.LoadLocalizedString( IDS_PRINT_SELECTBY );
			pDC->SelectObject( &clText10Bold );
			left = iTab1;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
			
			// Draw each field
			pDC->SelectObject( &clText10 );
			
			// Company
			str = pUserRef->GetString( CDS_UserRef::Company );
			if( false == str.IsEmpty() )
				left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
			
			// Name
			str = pUserRef->GetString( CDS_UserRef::Name );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
			
			// Address
			str = pUserRef->GetString( CDS_UserRef::Address );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
			
			// Zip, town and country
			str = _T("");
			str = pUserRef->GetString( CDS_UserRef::Zip );
			if( false == str.IsEmpty() )
				str += CString(' ');
			str += pUserRef->GetString( CDS_UserRef::Town );
			if( *( pUserRef->GetString( CDS_UserRef::Country ) ) )	// Not an empty string
			{
				if( false == str.IsEmpty() )
					str += CString(' ');
				str += CString( '(' ) + pUserRef->GetString( CDS_UserRef::Country ) + CString( ')' );
			}
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );

			// Tel, Fax
			str = _T("");
			CString str2;
			str = pUserRef->GetString( CDS_UserRef::Tel );
			if( false == str.IsEmpty() )
			{
				str2 = TASApp.LoadLocalizedString( IDS_PRINT_TEL );
				str = str2 + str;
			}
			if( *( pUserRef->GetString( CDS_UserRef::Fax ) ) )	// Not an empty string
			{
				if( false == str.IsEmpty() )
					str += CString( ';' );
				str2 = TASApp.LoadLocalizedString( IDS_PRINT_FAX );
				str += ( str2 + pUserRef->GetString( CDS_UserRef::Fax ) );
			}
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );

			// e-mail
			str = pUserRef->GetString( CDS_UserRef::Email );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
		}

		// Other informations
		if( true == m_pTADSPageSetup->GetShowOtherInfo() && *(pUserRef->GetString( CDS_UserRef::Other ) ) )
		{
			// If we have not show user information, we must add here the separation line
			if( false == m_pTADSPageSetup->GetShowUserRef() )
			{
				ytmp += ( m_iLineSpace * m_Xfactor ) / 100;
				if( true == DrawNow )
				{
					pDC->MoveTo( CPoint( x, ytmp ) );
					pDC->LineTo( CPoint( x + m_PrintRect.Width(), ytmp ) );
				}
			}

			ytmp += ( m_iLineSpace * m_Xfactor ) / 100;
			str = TASApp.LoadLocalizedString( IDS_DLGREF_STATICFOTHER );
			str += _T(":");
			pDC->SelectObject( &clText10Bold );
			left = iTab1;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );

			pDC->SelectObject( &clText10 );
			str = pUserRef->GetString( CDS_UserRef::Other );
			left = iTab2;
			ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
		}
		
		if( ytmp > y )
			y = ytmp;

		// Draw End line
		y += ( m_iLineSpace * m_Xfactor ) / 100;
		if( true == DrawNow )
		{
			pDC->MoveTo( CPoint( x, y ) );
			pDC->LineTo( CPoint( x + m_PrintRect.Width(), y ) );
		}
	}

	// Restore GDI
	pDC->SelectObject( pOldPen );
	pDC->SelectObject( pOldFont );
	pDC->SetTextColor( OldTextColor );
	pDC->SetBkMode( OldBkMode );
	pen.DeleteObject();
	clText10.DeleteObject();
	clText10Bold.DeleteObject();
	return y;
}

// Just date and project name
int CDrawSet::DrawShortPrjInfo( CDC* pDC, int x, int y, bool DrawNow /* true */)
{
	CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();
	ASSERT( NULL != pPrjRef );

	// Check if something will be displayed.
	int flag = 0;
	
	if( true == m_pTADSPageSetup->GetShowDate() || true == m_pTADSPageSetup->GetShowPrjName() )
	{
		flag = 1;
	}

	// 2 lines before.
	if( 1 == flag )
	{
		y += ( 2 * m_iLineSpace * m_Xfactor ) / 100;
	}
	else
	{
		return y;
	}
	
	CFont clText11, clText11Bold;
	_CreateFont( pDC, clText11, 11, FW_NORMAL, CString( _T("Arial") ) );
	_CreateFont( pDC, clText11Bold, 11, FW_BOLD, CString( _T("Arial") ) );

	CFont *pOldFont;
	pOldFont = pDC->SelectObject( &clText11 );

	COLORREF OldTextColor = pDC->SetTextColor( m_StringColor );
	int OldBkMode = pDC->SetBkMode( TRANSPARENT );
	
	CPen pen;
	pen.CreatePen( PS_SOLID, 1, m_StringColor );
	CPen *pOldPen = pDC->SelectObject( &pen );

	// Draw Date:
	CString str;
	int left = 0;
	
	// Left part of sheet.
	int maxx = m_PrintRect.Width() / 2 + m_PrintRect.left - ( m_iVerticalSpace * m_Xfactor ) / 100;
	int ytmp = y;
	
	if( true == m_pTADSPageSetup->GetShowDate() )
	{
		// Print "Date:"
		pDC->SelectObject( &clText11Bold );
		str = TASApp.LoadLocalizedString( IDS_PRINT_DATE );
		str += _T(" ");
		left = m_PrintRect.left;
		DrawTextLine( pDC,left, y, maxx, str, DrawNow );
		
		pDC->SelectObject( &clText11 );
		COleDateTime tdate = COleDateTime::GetCurrentTime();
		str = tdate.Format( LOCALE_NOUSEROVERRIDE | VAR_DATEVALUEONLY );
		y = DrawTextLine( pDC, left, y, maxx, str, DrawNow );
	}

	int middlepage = m_PrintRect.left + m_PrintRect.Width() / 2;
	maxx = m_PrintRect.right;
	
	// Right column	
	if( true == m_pTADSPageSetup->GetShowPrjName() && *pPrjRef->GetString(CDS_ProjectRef::Name) )
	{
		pDC->SelectObject( &clText11Bold );
		str = TASApp.LoadLocalizedString( IDS_PRINT_PROJECT );
		str += _T(" ");
		str += pPrjRef->GetString( CDS_ProjectRef::Name );

		left = middlepage - pDC->GetTextExtent( str ).cx / 2;
		ytmp = DrawTextLine( pDC, left, ytmp, maxx, str, DrawNow );
	}

	if( ytmp > y )
	{
		y = ytmp;
	}

	// Restore GDI
	pDC->SelectObject( pOldPen );
	pDC->SelectObject( pOldFont );
	pDC->SetTextColor( OldTextColor );
	pDC->SetBkMode( OldBkMode );
	pen.DeleteObject();
	clText11.DeleteObject();
	clText11Bold.DeleteObject();

	return y;
}

// Draw all log Informations 
int CDrawSet::DrawLogInfo( CDC* pDC, CLog *pLD , int x, int y, int width/*=RIGHTVIEWWIDTH*/, bool DrawNow /*=true*/) 
{
	if( NULL == pLD )
		return y;
	
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	
	CPen pen;
	pen.CreatePen( PS_SOLID, 1, RGB( 0, 0, 0) );
	CPen *pOldPen;
	pOldPen = pDC->SelectObject(&pen);
	
	CFont clText11, clText11Bold;
	_CreateFont( pDC, clText11, 11, FW_NORMAL, CString( _T("Arial") ) );
	_CreateFont( pDC, clText11Bold, 11, FW_BOLD, CString( _T("Arial") ) );
	CFont *pOldFont = pDC->SelectObject( &clText11Bold );

	// Draw Upper Line
	if (DrawNow)			
	{
		pDC->MoveTo( CPoint( x, y ) );
		pDC->LineTo( CPoint( x + width, y ) );
	}
	y += m_iLineSpace * m_Xfactor / 100;
		
	// Draw all Titles
	CString str;
	int deltaX = width / 4;

	// keep a trace of vertical position, we will use it when we draw text associated to titles
	int iYLine1 = y;
	int xtmp = x;
	
	// Because we use bold font for title, we must remember y position for each line to write after
	// the value in regards to each title.

	// First Line, left text
	str = TASApp.LoadLocalizedString( IDS_PLANT );
	str += CString( _T(":") );
	DrawTextLine( pDC, xtmp, iYLine1, xtmp + deltaX, str, DrawNow );
	
	// Right text start at the middle of the page
	str = TASApp.LoadLocalizedString( IDS_STARTINGTIME );
	str += CString( _T(":") );
	xtmp = x + 7 * deltaX / 4;
	int iYLine2 = DrawTextLine( pDC, xtmp, iYLine1, xtmp + deltaX, str, DrawNow );
	
	// 2nd line
	str = TASApp.LoadLocalizedString( IDS_MODULE );
	str += CString( _T(":") );
	xtmp = x;
	DrawTextLine( pDC, xtmp, iYLine2, xtmp + deltaX, str, DrawNow );

	str = TASApp.LoadLocalizedString( IDS_ENDINGTIME );
	str += CString( _T(":") );
	xtmp = x + 7 * deltaX / 4;
	int iYLine3 = DrawTextLine( pDC, xtmp, iYLine2, xtmp + deltaX, str, DrawNow );

	// 3th line
	str = TASApp.LoadLocalizedString( IDS_VALVEINDEX );
	str += CString( _T(":") );
	xtmp = x;
	DrawTextLine( pDC, xtmp, iYLine3, xtmp + deltaX, str, DrawNow );

	str = TASApp.LoadLocalizedString( IDS_NBROFPOINTS );
	str += CString( _T(":") );
	xtmp = x + 7 * deltaX / 4;
	int iYLine4 = DrawTextLine( pDC, xtmp, iYLine3, xtmp + deltaX, str, DrawNow );

	// 4th line
	str = TASApp.LoadLocalizedString( IDS_VALVE );
	str += CString( _T(":") );
	xtmp = x;
	DrawTextLine( pDC, xtmp, iYLine4, xtmp + deltaX, str, DrawNow );

	str = TASApp.LoadLocalizedString( IDS_DTBETWPTS );
	str += CString( _T(":") );
	xtmp = x + 7 * deltaX / 4;
	int iYLine5 = DrawTextLine( pDC, xtmp, iYLine4, xtmp + deltaX, str, DrawNow );
	
	// 5th line
	//str = TASApp.LoadLocalizedString( IDS_VALVENAME );
	//str += CString( _T(":") );
	//xtmp = x;
	//DrawTextLine( pDC, xtmp, iYLine5, xtmp + deltaX, str, DrawNow );	
	str = TASApp.LoadLocalizedString( IDS_LOWSAMPLEDVAL );
	str += CString( _T(":") );
	xtmp = x + 7 * deltaX / 4;
	int iYLine6 = DrawTextLine( pDC, xtmp, iYLine5, xtmp + deltaX, str, DrawNow );
	
	// 6th line
	str = TASApp.LoadLocalizedString( IDS_LOGTYPE );
	str += CString( _T(":") );
	xtmp = x;
	DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );

	str = TASApp.LoadLocalizedString( IDS_HIGHSAMPLEDVAL );
	str += CString( _T(":") );
	xtmp = x + 7 * deltaX / 4;
	int iYLine7 = DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );
	
	// Draw Logged data informations

	// Line 1 Plant and Starting Time
	pDC->SelectObject( &clText11 );
	str = pLD->GetSite(); 
	xtmp = x + 3 * deltaX / 4;
	DrawTextLine( pDC, xtmp, iYLine1, xtmp + deltaX, str, DrawNow );	

	CTimeUnic dtu;
	pLD->GetDateTime( 0, dtu ); 
	str = dtu.Format( IDS_LOGDATETIME_FORMAT );
	xtmp = x + 3 * deltaX;
	DrawTextLine( pDC, xtmp, iYLine1, xtmp + deltaX, str, DrawNow );	

	// Line 2 Module and Ending Time
	str = pLD->GetRef(); 
	xtmp = x + 3 * deltaX / 4;
	DrawTextLine( pDC, xtmp, iYLine2, xtmp + deltaX, str, DrawNow );

	pLD->GetDateTime( pLD->GetLength() - 1, dtu ); 
	str = dtu.Format( IDS_LOGDATETIME_FORMAT );
	xtmp = x + 3 * deltaX;
	DrawTextLine( pDC, xtmp, iYLine2, xtmp + deltaX, str, DrawNow );	

	// Line 3 Valve Index and Number of points
	str.Format( _T("%d"), pLD->GetValveIndex() ); 
	xtmp = x + 3 * deltaX / 4;
	DrawTextLine( pDC, xtmp, iYLine3, xtmp + deltaX, str, DrawNow );	

	str.Format( _T("%d"), pLD->GetLength() );
	xtmp = x + 3 * deltaX;
	DrawTextLine( pDC, xtmp, iYLine3, xtmp + deltaX, str, DrawNow );	
	
	// Line 4 Valve and Dt between points
	str = pLD->GetValveName();
	xtmp = x + 3 * deltaX / 4;
	DrawTextLine( pDC, xtmp, iYLine4, xtmp + deltaX, str, DrawNow );

	str.Format( _T("%d"), pLD->GetInterval() );
	CString stmp( TASApp.LoadLocalizedString( IDS_SEC ) );
	str += _T(" ") + stmp;
	xtmp = x + 3 * deltaX;
	DrawTextLine( pDC, xtmp, iYLine4, xtmp + deltaX, str, DrawNow );	
	
	// Line 5 Valve Name and Lowest sampled value
	//str = _T("-");
	//xtmp = x + 3 * deltaX / 4;
	//DrawTextLine( pDC, xtmp, iYLine5, xtmp + deltaX, str, DrawNow );	
	
	double dVal;

	switch( pLD->GetMode() )
	{
		case CLog::LOGTYPE_DP:

			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}
			
			str = WriteCUDouble( _U_DIFFPRESS, dVal, true );

			break;

		case CLog::LOGTYPE_TEMP:

			if( true == pLD->IsT1SensorUsed() )
			{
				if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
				{
					dVal = 0; // T1
				}
			}
			else 
			{
				if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
				{
					dVal = 0; // T2
				}
			}

			str = WriteCUDouble( _U_TEMPERATURE, dVal, true );
			
			break;

		case CLog::LOGTYPE_FLOW:

			if( false == pLD->GetLowestValue( CLog::ect_Flow, dVal ) )
			{
				dVal = 0; // Flow
			}

			str = WriteCUDouble( _U_FLOW, dVal, true );
			str += CString( _T("/") );
			
			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}

			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			
			break;

		case CLog::LOGTYPE_DPTEMP:

			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}

			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			str += CString( _T("/") );
			
			if( true == pLD->IsT1SensorUsed() )
			{
				if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
				{
					dVal = 0; // T1
				}
			}
			else
			{
				if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
				{
					dVal = 0; // T2
				}
			}
			
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			
			break;

		case CLog::LOGTYPE_FLOWTEMP:
			
			if( false == pLD->GetLowestValue( CLog::ect_Flow, dVal ) )
			{
				dVal = 0; // Flow
			}

			str = WriteCUDouble( _U_FLOW, dVal, true );
			str += CString( _T("/") );

			if( true == pLD->IsT1SensorUsed() )
			{
				if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
				{
					dVal = 0; // T1
				}
			}
			else
			{
				if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
				{
					dVal = 0; // T2
				}
			}
			
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += CString( _T("/") );
			
			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}

			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			
			break;

		case CLog::LOGTYPE_DT:

			if( false == pLD->GetLowestValue( CLog::ect_DeltaT, dVal ) )
			{
				dVal = 0; // DT
			}

			str = WriteCUDouble( _U_DIFFTEMP, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
			{
				dVal = 0; // T1
			}

			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += CString( _T("/") );
			
			if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
			{
				dVal = 0; // T2
			}

			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			
			break;

		case CLog::LOGTYPE_POWER:

			if( false == pLD->GetLowestValue( CLog::ect_Power, dVal ) )
			{
				dVal = 0; // Power
			}

			str = WriteCUDouble( _U_TH_POWER, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetLowestValue( CLog::ect_Flow, dVal ) )
			{
				dVal = 0; // Flow
			}

			str += WriteCUDouble( _U_FLOW, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}

			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetLowestValue( CLog::ect_DeltaT, dVal ) )
			{
				dVal = 0; // DT
			}

			str += WriteCUDouble( _U_DIFFTEMP, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
			{
				dVal = 0; // T1
			}
			
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
			{
				dVal = 0; // T2
			}

			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			
			break;
	};

	xtmp = x + 3 * deltaX;
	DrawTextLine( pDC, xtmp, iYLine5, xtmp + deltaX, str, DrawNow );	

	// Line 6 Logging type and highest sampled value
	xtmp = x + 3 * deltaX / 4;
	switch( pLD->GetMode() )
	{
		case CLog::LOGTYPE_DP:
			
			str = TASApp.LoadLocalizedString( IDS_DP );
			DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );

			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}

			str = WriteCUDouble( _U_DIFFPRESS, dVal, true );
			
			break;

		case CLog::LOGTYPE_TEMP:
			
			str = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
			DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );
			
			if( true == pLD->IsT1SensorUsed() )
			{
				if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
				{
					dVal = 0; // T1
				}
			}
			else
			{
				if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
				{
					dVal = 0; // T2
				}
			}
			
			str = WriteCUDouble( _U_TEMPERATURE, dVal, true );
			
			break;

		case CLog::LOGTYPE_FLOW:
			
			str = TASApp.LoadLocalizedString( IDS_FLOW );
			DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );
			
			if( false == pLD->GetHighestValue( CLog::ect_Flow, dVal ) )
			{
				dVal = 0; // Flow
			}

			str = WriteCUDouble( _U_FLOW, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}
			
			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			break;


		case CLog::LOGTYPE_DPTEMP:

			str = TASApp.LoadLocalizedString( IDS_DP );
			str += CString(_T(" / "));
			stmp = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
			str += stmp;
			DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );
			
			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}

			str = WriteCUDouble( _U_DIFFPRESS, dVal, true );
			str += CString( _T("/") );
			
			if( true == pLD->IsT1SensorUsed() )
			{
				if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
				{
					dVal = 0; // T1
				}
			}
			else
			{
				if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
				{
					dVal = 0; // T2
				}
			}
			
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			
			break;

		case CLog::LOGTYPE_FLOWTEMP:
			
			str = TASApp.LoadLocalizedString( IDS_FLOW );
			str += CString( _T(" / ") );
			stmp = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
			str += stmp;
			DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );
			
			if( false == pLD->GetHighestValue( CLog::ect_Flow, dVal ) )
			{
				dVal = 0; // Flow
			}

			str = WriteCUDouble( _U_FLOW, dVal, true );
			str += CString( _T("/") );
			
			if( true == pLD->IsT1SensorUsed() )
			{
				if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
				{
					dVal = 0; // T1
				}
			}
			else
			{
				if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
				{
					dVal = 0; // T2
				}
			}
			
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += CString( _T("/") );
			
			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}

			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			
			break;

		case CLog::LOGTYPE_DT:

			str = TASApp.LoadLocalizedString( IDS_DIFFTEMPERATURE );
			DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );

			if( false == pLD->GetHighestValue( CLog::ect_DeltaT, dVal ) )
			{
				dVal = 0; // DT
			}

			str = WriteCUDouble( _U_DIFFTEMP, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
			{
				dVal = 0; // T1
			}
			
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += CString( _T("/") );
			
			if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
			{
				dVal = 0; // T2
			}

			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			
			break;

		case CLog::LOGTYPE_POWER:
			
			str = TASApp.LoadLocalizedString( IDS_POWER );
			DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );

			if( false == pLD->GetHighestValue( CLog::ect_Power, dVal ) )
			{
				dVal = 0; // Power
			}

			str = WriteCUDouble( _U_TH_POWER, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetHighestValue( CLog::ect_Flow, dVal ) )
			{
				dVal = 0; // Flow
			}

			str += WriteCUDouble( _U_FLOW, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
			{
				dVal = 0; // Dp
			}

			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetHighestValue( CLog::ect_DeltaT, dVal ) )
			{
				dVal = 0; // DT
			}

			str += WriteCUDouble( _U_DIFFTEMP, dVal, true );
			str += CString( _T("/") );

			if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
			{
				dVal = 0; // T1
			}

			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += CString( _T("/") );
			
			if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
			{
				dVal = 0; // T2
			}

			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			
			break;
	};

	xtmp = x + 3 * deltaX;
	y = DrawTextLine( pDC, xtmp, iYLine6, xtmp + deltaX, str, DrawNow );	

	y += m_iLineSpace * m_Xfactor / 100;

	// Draw bottom Line
	if( true == DrawNow )			
	{
		pDC->MoveTo( CPoint( x, y ) );
		pDC->LineTo( CPoint( x + width, y ) );
	}

	pDC->SelectObject( pOldFont );
	pDC->SelectObject( pOldPen );
	pen.DeleteObject();
	clText11.DeleteObject();
	clText11Bold.DeleteObject();

	return y;
}

int CDrawSet::DrawHMInfo( CDC *pDC, CDS_HydroMod *pHM, int x, int y, int iWidth, bool DrawNow, bool bMeasInfo, bool bPrint )
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	
	CPen pen;
	pen.CreatePen( PS_SOLID, 1, RGB( 0, 0, 0 ) );
	CPen *pOldPen;
	pOldPen = pDC->SelectObject( &pen );

	CFont clText, clTextBold;

	if( true == bPrint )
	{	
		_CreateFont( pDC, clText, 11, FW_NORMAL, CString( _T("Arial") ) );
		_CreateFont( pDC, clTextBold, 11, FW_BOLD, CString( _T("Arial") ) );
	}
	else
	{
		_CreateFont( pDC, clText, 8, FW_NORMAL, CString( _T("Arial") ) );
		_CreateFont( pDC, clTextBold, 8, FW_BOLD, CString( _T("Arial") ) );
	}
	CFont *pOldFont = pDC->SelectObject( &clTextBold );

	pDC->SetBkMode( TRANSPARENT );

	// Draw upper line.
	if( true == DrawNow)
	{
		pDC->MoveTo( CPoint( x, y ) );
		pDC->LineTo( CPoint( x + iWidth, y ) );
	}

	y += m_iLineSpace * m_Xfactor / 100;

	// Draw all titles.
	CString str; 
	int dX = iWidth / 24;
	const int iCol1 = x;
	const int iCol2 = x + 6 * iWidth / 24;
	const int iCol3 = x + 9 * iWidth / 24;
	const int iCol4 = x + 16 * iWidth / 24;
	const int iCol5 = x + 19 * iWidth / 24;
	const int iCol6 = x + 21 * iWidth / 24;

	double dBalIndex = 0.0;
	bool bBalIndex = ( NULL != pHM ) ? pHM->GetBalancingIndex( &dBalIndex ) : false;
	TCHAR tcName[_MAXCHARS];
		
	// Keep a trace of vertical position, we will use it when we draw text associated to titles
	int iYTemp = y;
	int iXTemp = iCol1;
	
	// HYS-1090: This function is reviewed to manage multiline text
	// First line, left text.
	str = TASApp.LoadLocalizedString( IDS_MODULEDESCRIPTION );
	str += CString( _T(":") );
	CString strDescriptionValue;
	//Value description
	strDescriptionValue = _T( "" );

	if( NULL != pHM )
	{
		strDescriptionValue = pHM->GetDescription();
	}

	if( true == strDescriptionValue.IsEmpty() )
	{
		strDescriptionValue = _T("-");
	}

	strDescriptionValue.Replace( _T("\r\n" ), _T( " ") );
	strDescriptionValue.Left( 100 );
	// This function draw a part of line composed of name: value until iWidth coordinate
	iYTemp = DrawHMLineInfo( pDC, iXTemp, iYTemp, iCol2, iWidth, str, strDescriptionValue, DrawNow );

	
	str = TASApp.LoadLocalizedString( IDS_REMARK );
	str += CString( _T(":") );
	iXTemp = iCol1;
	// value remark
	CString strRemarkValue;
	strRemarkValue = _T( "" );

	if( NULL != pHM )
	{
		strRemarkValue = pHM->GetRemL1();
	}

	if( true == strRemarkValue.IsEmpty() )
	{
		strRemarkValue = _T("-");
	}
	iYTemp = DrawHMLineInfo( pDC, iXTemp, iYTemp, iCol2, iWidth, str, strRemarkValue, DrawNow );
	iYTemp += m_iLineSpace * m_Xfactor / 100;
	
	if( true == DrawNow )
	{
		pDC->MoveTo( CPoint( iCol1, iYTemp ) );
		pDC->LineTo( CPoint( iCol1 + iWidth, iYTemp ) );
	}

	iYTemp += m_iLineSpace * m_Xfactor / 100;

	// To save position of each line.
	int iYMemLine1 = -1;
	int iYMemLine2 = -1;
	int iYMemLine3 = -1;
	int iYMemLine4 = -1;
	int iYMemLine5 = -1;
	CString strBIndexFlow;
	CString strPlantValue;
	CString strHminValue;
	CString strParentModuleValue;
	CString strHAvailableValue;
	CString strPosParentModuleValue;

	CString str1, str2;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Measurement & diagnostic (RViewHMCalc)
	//
	// Col 1										Col 2
	// Balancing index:								27,5 %
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Measurement & diagnostic (Print)
	//
	// Col 1										Col 2										Col 3										Col 4
	// Balancing index:								27,5 %										Plant:										0136
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Distribution & circuit (RViewHMCalc)
	//
	// Col 1										Col 2
	// Total flow:									3883 l/h									Supply temperature:							65 °C
	// Hmin:										9,01 kPa									Return temperature:							55 °C
	// Havailale:									9,01 kPa									Power:										12340 W
	// VSP Dp sensor (Position/Mion. Dp setting):	*A/ 26,7 kPa (*A.1)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Distribution & circuit (Print)
	//	
	// Col 1										Col 2										Col 3										Col 4
	// Total flow:									3883 l/h									Plant:										0136
	// Hmin:										9,01 kPa									Parent module:								-
	// Havailale:									9,01 kPa									Position in parent module:					1
	// Supply temperature:							65 °C										Return temperature:							55 °C
	// Power:										12340 W
	// VSP Dp sensor (Position/Mion. Dp setting):																							*A/ 26,7 kPa (*A.1)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 'Balancing index' or 'Total flow' title.
	if( true == bMeasInfo )
	{
		str = TASApp.LoadLocalizedString( IDS_BALANCINGINDEX );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_TOTALFLOW );
	}
	
	// Balancing index value or total flow value.
	if( true == bMeasInfo )
	{
		if( true == bBalIndex )
		{
			strBIndexFlow = WriteDouble( 100.0 * dBalIndex, 2, 1 ) + (CString)_T(" %");
		}
		else
		{
			strBIndexFlow = _T("-");
		}
	}
	else
	{
		strBIndexFlow = _T("-");

		if( NULL != pHM )
		{
			strBIndexFlow = WriteCUDouble( _U_FLOW, pHM->GetQ() );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
			strBIndexFlow += CString( _T(" ") ) + tcName;
		}
	}

	str += CString( _T(":") );
	iXTemp = iCol1;
	iYMemLine1 = iYTemp;

	if( true == bPrint )
	{
		str1 = str;
		iYTemp = DrawHMLineInfo( pDC, iXTemp, iYMemLine1, iCol2, iCol3, str1, strBIndexFlow, DrawNow );
	}
	else
	{
		iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine1, iCol2 - dX, str, DrawNow );
		
		pDC->SelectObject( &clText );
		int xValue = iCol2;
		int iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine1, iCol5 - dX, strBIndexFlow, DrawNow );
		pDC->SelectObject( &clTextBold );

		iYTemp = max( iYTemp, iYTemp2 );
	}

	// Plant.
	if( true == bPrint )
	{
		str = TASApp.LoadLocalizedString( IDS_PLANT );
		str += CString( _T(":") );
		str1 = str;
		iXTemp = iCol3;
		
		// Plant value.
		strPlantValue = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );
		iYTemp = DrawHMLineInfo( pDC, iXTemp, iYMemLine1, iCol4, iWidth, str1, strPlantValue, DrawNow );
	}
	else if( false == bMeasInfo )
	{
		// Supply temperature.
		str1 = TASApp.LoadLocalizedString( IDS_SUPPLYTEMPERATURE );
		str1 += CString( _T(":") );
		CString strSupplyTemperature = _T("-");

		if( NULL != pHM )
		{
			double dSupplyTemperature = pHM->GetTemperature( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply );

			if( -273.15 != dSupplyTemperature )
			{
				strSupplyTemperature = WriteCUDouble( _U_TEMPERATURE, dSupplyTemperature, true );
			}
		}

		iXTemp = iCol3;
		iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine1, iCol4 - dX, str1, DrawNow );
		
		pDC->SelectObject( &clText );
		int xValue = iCol4;
		int iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine1, iCol5 - dX, strSupplyTemperature, DrawNow );
		pDC->SelectObject( &clTextBold );

		iYTemp = max( iYTemp, iYTemp2 );
	}

	iYMemLine2 = iYTemp;

	// HMin.
	if( false == bMeasInfo )
	{
		str = TASApp.LoadLocalizedString( IDS_HMIN );
		str += CString( _T(":") );
		
		// Hmin value.
		strHminValue = _T("-");

		if( NULL != pHM )
		{
			strHminValue = WriteCUDouble( _U_DIFFPRESS, pHM->GetHminForUserDisplay(), true );
		}
		
		iXTemp = iCol1;
		
		if( true == bPrint )
		{
			str1 = str;
			iYTemp = DrawHMLineInfo( pDC, iXTemp, iYMemLine2, iCol2, iCol3, str1, strHminValue, DrawNow );
		}
		else
		{
			iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine2, iCol2 - dX, str, DrawNow );
			
			pDC->SelectObject( &clText );
			int xValue = iCol2;
			int iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine2, iCol5 - dX, strHminValue, DrawNow );
			pDC->SelectObject( &clTextBold );

			iYTemp = max( iYTemp, iYTemp2 );
		}
	}

	// Parent module.
	if( true == bPrint )
	{
		str = TASApp.LoadLocalizedString( IDS_PARENTMODULE );
		str += CString( _T(":") );
		iXTemp = iCol3;
		str1 = str;
		
		// Parent module value.
		if( NULL != pHM && pHM->GetLevel() > 0 )
		{
			strParentModuleValue = ( (CDS_HydroMod*)pHM->GetIDPtr().PP )->GetHMName();
		}
		else
		{
			strParentModuleValue = _T("-");
		}

		iYTemp = DrawHMLineInfo( pDC, iXTemp, iYMemLine2, iCol4, iWidth, str1, strParentModuleValue, DrawNow );
	}
	else if( false == bMeasInfo )
	{
		// Return temperature.
		str1 = TASApp.LoadLocalizedString( IDS_RETURNTEMPERATURE );
		str1 += CString( _T(":") );
		CString strReturnTemperature = _T("-");

		if( NULL != pHM )
		{
			double dReturnTemperature = pHM->GetTemperature( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );

			if( -273.15 != dReturnTemperature )
			{
				strReturnTemperature = WriteCUDouble( _U_TEMPERATURE, dReturnTemperature, true );
			}
		}
			
		iXTemp = iCol3;
		iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine2, iCol4 - dX, str1, DrawNow );
		
		pDC->SelectObject( &clText );
		int xValue = iCol4;
		int iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine2, iCol5 - dX, strReturnTemperature, DrawNow );
		pDC->SelectObject( &clTextBold );

		iYTemp = max( iYTemp, iYTemp2 );
	}

	iYMemLine3 = iYTemp;

	// Havailable.
	if( false == bMeasInfo )
	{
		str = TASApp.LoadLocalizedString( IDS_HAVAILABLE );
		str += CString( _T(":") );
		
		// Havail value.
		double dHAvail = 0.0;

		if( NULL != pHM )
		{
			dHAvail = pHM->GetHAvail();

			if( NULL != pHM->GetpPump() && pHM->GetpSchcat() != NULL && true == pHM->GetpSchcat()->IsPump() )
			{
				dHAvail = pHM->GetpPump()->GetHAvail();
			}
		}

		if( NULL != pHM )
		{
			strHAvailableValue = WriteCUDouble( _U_DIFFPRESS, dHAvail );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
			strHAvailableValue += CString( _T(" ") ) + tcName;
		}
		else
		{
			strHAvailableValue = _T("-");
		}

		iXTemp = iCol1;
		iYMemLine3 = iYTemp;

		if( true == bPrint )
		{
			str1 = str;
			iYTemp = DrawHMLineInfo( pDC, iXTemp, iYMemLine3, iCol2, iCol3, str1, strHAvailableValue, DrawNow );
		}
		else
		{
			iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine3, iCol2 - dX, str, DrawNow );
			
			pDC->SelectObject( &clText );
			int xValue = iCol2;
			int iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine3, iCol5 - dX, strHAvailableValue, DrawNow );
			pDC->SelectObject( &clTextBold );

			iYTemp = max( iYTemp, iYTemp2 );
		}
	}

	// Position in parent module.
	if( true == bPrint )
	{
		str = TASApp.LoadLocalizedString( IDS_POSINPARENTMOD );
		str += CString( _T(":") );
		str1 = str;
		iXTemp = iCol3;
	
		// Position value.
		if( NULL != pHM )
		{
			strPosParentModuleValue.Format( _T("%d"), pHM->GetPos() );
		}
		else
		{
			strPosParentModuleValue = _T("-");
		}

		iYTemp = DrawHMLineInfo( pDC, iXTemp, iYMemLine3, iCol4, iWidth, str1, strPosParentModuleValue, DrawNow );
	}
	else if( false == bMeasInfo )
	{
		// Power.
		str1 = TASApp.LoadLocalizedString( IDS_POWER );
		str1 += CString( _T(":") );
		CString strPower = _T("-");

		if( NULL != pHM )
		{
			double dPower = pHM->GetPower( CAnchorPt::CircuitSide_Primary );

			if( dPower > 0.0 )
			{
				strPower = WriteCUDouble( _U_TH_POWER, dPower, true );
			}
		}

		iXTemp = iCol3;
		iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine3, iCol4 - dX, str1, DrawNow );
		
		pDC->SelectObject( &clText );
		int xValue = iCol4;
		int iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine3, iCol5 - dX, strPower, DrawNow );
		pDC->SelectObject( &clTextBold );

		iYTemp = max( iYTemp, iYTemp2 );
	}

	iYMemLine4 = iYTemp;

	// Supply & return temperatures in print mode.
	if( false == bMeasInfo && true == bPrint )
	{
		// Supply temperature.
		str = TASApp.LoadLocalizedString( IDS_SUPPLYTEMPERATURE );
		str += CString( _T(":") );
		
		CString strSupplyTemeprature = _T("-");

		if( NULL != pHM )
		{
			double dSupplyTemperature = pHM->GetTemperature( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply );

			if( -273.15 != dSupplyTemperature )
			{
				strSupplyTemeprature = WriteCUDouble( _U_TEMPERATURE, dSupplyTemperature, true );
			}
		}
		
		iXTemp = iCol1;
		iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine4, iCol2 - dX, str, DrawNow );
		
		pDC->SelectObject( &clText );
		int xValue = iCol2;
		int iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine4, iCol5 - dX, strSupplyTemeprature, DrawNow );
		pDC->SelectObject( &clTextBold );

		iYTemp = max( iYTemp, iYTemp2 );

		// Return temperature.
		str = TASApp.LoadLocalizedString( IDS_RETURNTEMPERATURE );
		str += CString( _T(":") );
		str1 = str;
	
		CString strReturnTemeprature = _T("-");

		if( NULL != pHM )
		{
			double dReturnTemperature = pHM->GetTemperature( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );

			if( -273.15 != dReturnTemperature )
			{
				strReturnTemeprature = WriteCUDouble( _U_TEMPERATURE, dReturnTemperature, true );
			}
		}

		iXTemp = iCol3;
		iYTemp2 = DrawTextLine( pDC, iXTemp, iYMemLine4, iCol4 - dX, str, DrawNow );
		iYTemp = max( iYTemp, iYTemp2 );
		
		pDC->SelectObject( &clText );
		xValue = iCol4;
		iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine4, iCol5 - dX, strReturnTemeprature, DrawNow );
		pDC->SelectObject( &clTextBold );

		iYTemp = max( iYTemp, iYTemp2 );
	}

	iYMemLine5 = iYTemp;

	if( false == bMeasInfo && true == bPrint )
	{
		// Power.
		str1 = TASApp.LoadLocalizedString( IDS_POWER );
		str1 += CString( _T(":") );
		CString strPower = _T("-");

		if( NULL != pHM )
		{
			double dPower = pHM->GetPower( CAnchorPt::CircuitSide_Primary );

			if( dPower > 0.0 )
			{
				strPower = WriteCUDouble( _U_TH_POWER, dPower, true );
			}
		}

		iXTemp = iCol1;
		iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine5, iCol2 - dX, str1, DrawNow );
		
		pDC->SelectObject( &clText );
		int xValue = iCol2;
		int iYTemp2 = DrawTextLine( pDC, xValue, iYMemLine5, iCol3 - dX, strPower, DrawNow );
		pDC->SelectObject( &clTextBold );

		iYTemp = max( iYTemp, iYTemp2 );
	}

	int iYMemLine6 = iYTemp;

	// VSP.
	bool bVSPDpSensor = false;
	CDS_HydroMod *pIndexCircuit = NULL;
	CDS_HydroMod *pHMMostRequesting = NULL;
	double dRequestedDp = 0;
	
	// HYS-1090: saved coordinate with multiline fonction.
	int iYEndMultiLine = -1;

	if( NULL != pHM && NULL != pHM->GetpSchcat() )
	{
		CDS_HydroMod *pPressInterface = ( true == pHM->GetpSchcat()->IsSecondarySideExist() ) ? pHM : pHM->GetpPressIntHM();
		pIndexCircuit = pPressInterface->FindIndexCircuit();
		bVSPDpSensor = pPressInterface->AnalyzeVSPDpSensorPosition( &pHMMostRequesting, dRequestedDp );
	}

	if( false == bMeasInfo && true == bVSPDpSensor )
	{
		str = TASApp.LoadLocalizedString( IDS_HMCALC_VSPDPSENSOR );
		iXTemp = iCol1;

		if( true == bPrint )
		{
			iYEndMultiLine = DrawTextLine( pDC, iXTemp, iYMemLine6, iCol4, str, DrawNow );
		}
		else
		{
			iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine6, iCol2 - dX, str, DrawNow );
		}
	}
	
	if( iYTemp < iYEndMultiLine )
	{
		iYTemp = iYEndMultiLine;
	}
	
	int iYTemp2 = iYTemp;
	
	// Draw module info.
	pDC->SelectObject( &clText );
	iXTemp = iCol2;

	// VSP.
	if( false == bMeasInfo && true == bVSPDpSensor )
	{
		CString VSPDpSensorPosition, strd, strHMNameMR, strHMIndex;
		strd = WriteCUDouble( _U_DIFFPRESS, dRequestedDp, true );
		strHMNameMR = pHMMostRequesting->GetHMName();
		
		if( NULL != pIndexCircuit )
		{
			CDS_HydroMod *pPressInterf = pIndexCircuit->GetpPressIntHM();
			strHMIndex = pPressInterf->GetHMName();
			VSPDpSensorPosition.Format( _T("%s / %s ( %s )"), strHMIndex, strd, strHMNameMR);
			str = VSPDpSensorPosition;

			if( true == bPrint )
			{
				iXTemp = iCol4;
				iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine6, iWidth, str, DrawNow );
			}
			else
			{
				iXTemp = iCol2;
				iYTemp = DrawTextLine( pDC, iXTemp, iYMemLine6, iCol5 - dX, str, DrawNow );
			}
		}
	}

	iYTemp += m_iLineSpace * m_Xfactor / 100;

	// Draw bottom line.
	if( true == DrawNow )			
	{
		pDC->MoveTo( CPoint( iCol1, iYTemp ) );
		pDC->LineTo( CPoint( iCol1 + iWidth, iYTemp ) );
	}

	pDC->SelectObject( pOldFont );
	pDC->SelectObject( pOldPen );
	pen.DeleteObject();
	clText.DeleteObject();
	clTextBold.DeleteObject();

	return iYTemp;
}

int CDrawSet::DrawHMLineInfo( CDC * pDC, int x, int y, int width1, int width2, CString str1, CString str2, bool DrawNow )
{
	int yline1 = y;
	int yline2 = y;
	CFont clText;
	CFont *pCurrentBoldFont = pDC->GetCurrentFont();
	LOGFONT LogFont;
	pCurrentBoldFont->GetLogFont( &LogFont );
	clText.CreateFont( LogFont.lfHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, CString( _T("Arial") ) );
	yline1 = DrawTextLine( pDC, x, y, width1, str1, DrawNow );
	pDC->SelectObject( &clText );
	yline2 = DrawTextLine( pDC, width1, y, width2, str2, DrawNow );
	pDC->SelectObject( pCurrentBoldFont );
	clText.DeleteObject();
	return max( yline1, yline2 );
}


HANDLE CDrawSet::DDBToDIB( CBitmap& bitmap, DWORD dwCompression, CPalette* pPal )
{
	BITMAP bm;
	BITMAPINFOHEADER bi;
	LPBITMAPINFOHEADER lpbi;
	DWORD dwLen;
	HANDLE hDIB;
	HANDLE handle;
	HDC hDC;
	HPALETTE hPal;

	ASSERT( bitmap.GetSafeHandle() );

	// The function has no arg for bitfields
	if ( dwCompression == BI_BITFIELDS )
		return NULL;

	// If a palette has not been supplied use defaul palette
	hPal = (HPALETTE) pPal->GetSafeHandle();
	if (hPal==NULL)
		hPal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

	// Get bitmap information
	bitmap.GetObject(sizeof(bm),(LPSTR)&bm);

	// Initialize the bitmapinfoheader
	bi.biSize               = sizeof(BITMAPINFOHEADER);
	bi.biWidth              = bm.bmWidth;
	bi.biHeight             = bm.bmHeight;
	bi.biPlanes             = 1;
	bi.biBitCount           = bm.bmPlanes * bm.bmBitsPixel;
	bi.biCompression        = dwCompression;
	bi.biSizeImage          = 0;
	bi.biXPelsPerMeter      = 0;
	bi.biYPelsPerMeter      = 0;
	bi.biClrUsed            = 0;
	bi.biClrImportant       = 0;

	// Compute the size of the  infoheader and the color table
	int nColors = (1 << bi.biBitCount);
	if ( nColors > 256 ) 
		nColors = 0;
	dwLen = bi.biSize + nColors * sizeof(RGBQUAD);

	// We need a device context to get the DIB from
	hDC = ::GetDC(NULL);
	hPal = SelectPalette(hDC,hPal,FALSE);
	RealizePalette(hDC);

	// Allocate enough memory to hold bitmapinfoheader and color table
	hDIB = GlobalAlloc(GMEM_FIXED,dwLen);

	if (!hDIB)
	{
		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	lpbi = (LPBITMAPINFOHEADER)hDIB;

	*lpbi = bi;

	// Call GetDIBits with a NULL lpBits param, so the device driver 
	// will calculate the biSizeImage field 
	GetDIBits(hDC, (HBITMAP)bitmap.GetSafeHandle(), 0L, (DWORD)bi.biHeight,
		(LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

	bi = *lpbi;

	// If the driver did not fill in the biSizeImage field, then compute it
	// Each scan line of the image is aligned on a DWORD (32bit) boundary
	if (bi.biSizeImage == 0)
	{
		bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) 
			* bi.biHeight;

		// If a compression scheme is used the result may infact be larger
		// Increase the size to account for this.
		if (dwCompression != BI_RGB)
			bi.biSizeImage = (bi.biSizeImage * 3) / 2;
	}

	// Realloc the buffer so that it can hold all the bits
	dwLen += bi.biSizeImage;
	if (handle = GlobalReAlloc(hDIB, dwLen, GMEM_MOVEABLE))
		hDIB = handle;
	else
	{
		GlobalFree(hDIB);

		// Reselect the original palette
		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	// Get the bitmap bits
	lpbi = (LPBITMAPINFOHEADER)hDIB;

	// FINALLY get the DIB
	BOOL bGotBits = GetDIBits( hDC, (HBITMAP)bitmap.GetSafeHandle(),
							0L,                             // Start scan line
							(DWORD)bi.biHeight,             // # of scan lines
							(LPBYTE)lpbi                    // address for bitmap bits
							+ (bi.biSize + nColors * sizeof(RGBQUAD)),
							(LPBITMAPINFO)lpbi,             // address of bitmapinfo
							(DWORD)DIB_RGB_COLORS);         // Use RGB for color table

	if( !bGotBits )
	{
		GlobalFree(hDIB);

		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	SelectPalette(hDC,hPal,FALSE);
	::ReleaseDC(NULL,hDC);
	return hDIB;
}

CPoint CDrawSet::PixelsToTwipsPt(CDC* pDC, CPoint &Pt)
{
	CRect rect(Pt.x,Pt.y,0,0);
	PixelsToTwips(pDC,rect);
	Pt.x = rect.left;
	Pt.y = rect.top;
	return Pt;
}

void CDrawSet::PixelsToTwips(CDC* pDC,CRect &rect)
{
	double ppi;
	ppi = pDC->GetDeviceCaps(LOGPIXELSY);
	rect.top=(int)((rect.top*1440)/ppi);
	rect.bottom=(int)((rect.bottom*1440)/ppi);
	ppi = pDC->GetDeviceCaps(LOGPIXELSX);
	rect.right=(int)((rect.right*1440)/ppi);
	rect.left=(int)((rect.left*1440)/ppi);
}

void CDrawSet::PixelsToInch( CDC* pDC, CRect &rect, float &leftInch, float &topInch, float &rightInch, float &bottomInch )
{
	float dHorzPixelByInch = (float)pDC->GetDeviceCaps( LOGPIXELSX );
	float dVertPixelByInch = (float)pDC->GetDeviceCaps( LOGPIXELSY );
	leftInch = (float)rect.left / dHorzPixelByInch;
	rightInch = (float)rect.right / dHorzPixelByInch;
	topInch = (float)rect.top / dVertPixelByInch;
	bottomInch = (float)rect.bottom / dVertPixelByInch;
}

CPoint CDrawSet::TwipsToPixelsPt(CDC* pDC, CPoint &Pt)
{
	CRect rect(Pt.x,Pt.y,0,0);
	PixelsToTwips(pDC,rect);
	Pt.x = rect.left;
	Pt.y = rect.top;
	return Pt;
}

void CDrawSet::TwipsToPixels(CDC* pDC,CRect &rect)
{
	double ppi;
	ppi = pDC->GetDeviceCaps(LOGPIXELSY);
	rect.top=(int)((rect.top*ppi)/1440.0);
	rect.bottom=(int)((rect.bottom*ppi)/1440);
	ppi = pDC->GetDeviceCaps(LOGPIXELSX);
	rect.right=(int)(rect.right*ppi/1440);
	rect.left=(int)(rect.left*ppi/1440);
}

void CDrawSet::GetMargins( CDC* pDC, CRect &rectMargin )
{
	double dHorzPixelByInch = pDC->GetDeviceCaps( HORZRES ) / pDC->GetDeviceCaps( HORZSIZE ) * 25.4;
	double dVertPixelByInch = pDC->GetDeviceCaps( VERTRES ) / pDC->GetDeviceCaps( VERTSIZE ) * 25.4;

	// 'GetMargin' returns in meter and we converted it in pixel.
	// Remark: VERTRES/VERTSIZE gives number of pixels by mm * 1000 for pixel/m
	double dHeaderMargin = m_pTADSPageSetup->GetMargin( epsHEADER ) * ( (double)pDC->GetDeviceCaps( VERTRES ) / (double)pDC->GetDeviceCaps( VERTSIZE ) ) * 1000.0;

	// Top in pixels ( corrected by 80% of m_XFactor)
	rectMargin.top = (LONG)( ( dHeaderMargin * (double)m_Xfactor ) / 100.0 / dVertPixelByInch );

	double dFooterMargin = m_pTADSPageSetup->GetMargin( epsFOOTER ) * ( (double)pDC->GetDeviceCaps( VERTRES ) / (double)pDC->GetDeviceCaps( VERTSIZE ) ) * 1000.0;
	// Bottom in pixels ( corrected by 80% of m_XFactor)
	rectMargin.bottom = (LONG)( ( dFooterMargin * (double)m_Xfactor ) / 100.0 / dVertPixelByInch );

	// Left in pixels
	rectMargin.left = (LONG)( ( (double)m_iLeftMargin * (double)m_Xfactor) / 100.0 / dHorzPixelByInch );

	// Right in pixel
	// Remark: HORZRES gives resolution in pixel
	rectMargin.right = (LONG)( (double)( pDC->GetDeviceCaps( HORZRES ) - m_iLeftMargin ) * (double)m_Xfactor / 100.0 / dHorzPixelByInch );
}

void CDrawSet::CopyImage( HBITMAP hBitmap, HDC hDCSource, HDC hDCDest, int xDest, int yDest )
{
	int wBitCount = 32;

	BITMAP Bitmap;  
	BITMAPINFOHEADER bi;
 
	GetObject( hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap ); 
 
	bi.biSize = sizeof(BITMAPINFOHEADER);    
	bi.biWidth = Bitmap.bmWidth;    
	bi.biHeight = Bitmap.bmHeight;  
	bi.biPlanes = 1;    
	bi.biBitCount = wBitCount;    
	bi.biCompression = BI_RGB;    
	bi.biSizeImage = 0;  
	bi.biXPelsPerMeter = 0;    
	bi.biYPelsPerMeter = 0;    
	bi.biClrUsed = 0;    
	bi.biClrImportant = 0; 
 
	DWORD dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight; 
 
	HANDLE hDib = GlobalAlloc( GHND, dwBmBitsSize + sizeof( BITMAPINFOHEADER ) ); 
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);    
	*lpbi = bi;
 
	int iError = GetDIBits( hDCSource, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER), (BITMAPINFO *)lpbi, DIB_RGB_COLORS );
 
	HBITMAP hbmDest = ::CreateCompatibleBitmap( hDCDest, Bitmap.bmWidth, Bitmap.bmHeight );
	iError = SetDIBits( hDCDest, hbmDest, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER), (BITMAPINFO *)lpbi, DIB_RGB_COLORS );
	
	HDC hMemDC = CreateCompatibleDC( hDCDest );
	hbmDest = (HBITMAP)SelectObject( hMemDC, hbmDest );

	BOOL fError = BitBlt( hDCDest, xDest, yDest, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, SRCCOPY );
	
	DeleteObject( SelectObject( hMemDC, hBitmap ) );
	DeleteDC( hMemDC );

	GlobalUnlock( hDib );
	GlobalFree( hDib );
}


// PROTECTED MEMBERS

void CDrawSet::_CreateFont( CDC* pDC, CFont &clFont, int iSize, int nWeight, CString clFontName )
{
	// Compute new font height in regards to new logical pixel in context device (ppi).
	// PS: 72 is ppi for screen resolution
	int lfHeight = -MulDiv( iSize, pDC->GetDeviceCaps( LOGPIXELSY ), 72 );
	clFont.CreateFont( lfHeight, 0, 0, 0, nWeight, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, clFontName );
}
