//
// SSheet.cpp: implementation of the CSSheet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <float.h>
#include <math.h>

#include "picture.h"
#include "TASelect.h"
#include "Select.h"
#include "DrawSet.h"

#include "SSheet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSSheet::CTextPatternStructure::CTextPatternStructure()
{ 
	m_fFontBold = FALSE;
	m_fFontItalic = FALSE;
	m_fFontUnderline = FALSE;
	m_iFontSize = 0;
	m_TextColor = 0;
	m_BackColor = 0;
	m_staticTextPattern = _SSTextPattern::Arial8;
	m_pclFont = NULL;
	hFont = (HFONT)0;
	m_strFontName = _T("");
	m_lTextStyle = 0;
	m_fChanged = false;
}

CSSheet::CTextPatternStructure::CTextPatternStructure( _SSTextPattern eTextPattern, bool fLight )
{
	m_fFontBold = FALSE;
	m_fFontItalic = FALSE;
	m_fFontUnderline = FALSE;
	m_iFontSize = 0;
	m_TextColor = 0;
	m_BackColor = 0;
	m_staticTextPattern = _SSTextPattern::Arial8;
	m_pclFont = NULL;
	hFont = (HFONT)0;
	m_strFontName = _T("");
	m_lTextStyle = 0;
	m_fChanged = false;
	_FillTextPattern( eTextPattern, fLight );
}

CSSheet::CTextPatternStructure::CTextPatternStructure( CSSheet::CTextPatternStructure *pclTextPatternStructure )
{
	m_fFontBold = pclTextPatternStructure->m_fFontBold;
	m_fFontItalic = pclTextPatternStructure->m_fFontItalic;
	m_fFontUnderline = pclTextPatternStructure->m_fFontUnderline;
	m_iFontSize = pclTextPatternStructure->m_iFontSize;
	m_TextColor = pclTextPatternStructure->m_TextColor;
	m_BackColor = pclTextPatternStructure->m_BackColor;
	m_staticTextPattern = pclTextPatternStructure->m_staticTextPattern;
	m_pclFont = NULL;
	hFont = (HFONT)0;
	m_strFontName = pclTextPatternStructure->m_strFontName;
	m_lTextStyle = pclTextPatternStructure->m_lTextStyle;
	m_fChanged = false;
}

CSSheet::CTextPatternStructure::~CTextPatternStructure()
{ 
	if( NULL != m_pclFont )
		delete m_pclFont;
}

void CSSheet::CTextPatternStructure::SetProperty( _SSTextPatternProperty eProperty, LPARAM lpValue )
{
	switch( eProperty )
	{
		case TPP_FontName:
			SetFontName( ( (CString*)lpValue )->GetString() );
			break;

		case TPP_FontBold:
			SetFontBold( ( 0 == (long)lpValue ) ? FALSE : TRUE );
			break;

		case TPP_FontItalic:
			SetFontItalic( ( 0 == (long)lpValue ) ? FALSE : TRUE );
			break;

		case TPP_FontUnderline:
			SetFontUnderline( ( 0 == (long)lpValue ) ? FALSE : TRUE );
			break;

		case TPP_FontSize:
			SetFontSize( (int)lpValue );
			break;

		case TPP_FontForeColor:
			SetForeColor( (COLORREF)lpValue );
			break;

		case TPP_FontBackColor:
			SetBackColor( (COLORREF)lpValue );
			break;

		case TPP_StyleHorizontalAlign:
			SetStyleHorizontalAlign( (long)lpValue );
			break;

		case TPP_StyleVerticalAlign:
			SetStyleVerticalAlign( (long)lpValue );
			break;

		case TPP_StyleWordWrap:
			SetStyleWordWrap( ( 0 == (long)lpValue ) ? FALSE : TRUE );
			break;

		case TPP_StyleEllipses:
			SetStyleEllipses( ( 0 == (long)lpValue ) ? FALSE : TRUE );
			break;
	}
}

LPARAM CSSheet::CTextPatternStructure::GetProperty( _SSTextPatternProperty eProperty )
{
	LPARAM lpValue = (LPARAM)-1;
	switch( eProperty )
	{
		case TPP_FontName:
			lpValue = (LPARAM)GetFontName();

		case TPP_FontBold:
			lpValue = ( FALSE == GetFontBold() ) ? (LPARAM)0 : (LPARAM)1;
			break;

		case TPP_FontItalic:
			lpValue = ( FALSE == GetFontItalic() ) ? (LPARAM)0 : (LPARAM)1;
			break;

		case TPP_FontUnderline:
			lpValue = ( FALSE == GetFontUnderline() ) ? (LPARAM)0 : (LPARAM)1;
			break;

		case TPP_FontSize:
			lpValue = (LPARAM)GetFontSize();
			break;

		case TPP_FontForeColor:
			lpValue = (LPARAM)GetForeColor();
			break;

		case TPP_FontBackColor:
			lpValue = (LPARAM)GetBackColor();
			break;

		case TPP_StyleHorizontalAlign:
			lpValue = (LPARAM)GetStyleHorizontalAlign();
			break;

		case TPP_StyleVerticalAlign:
			lpValue = (LPARAM)GetStyleVerticalAlign();
			break;

		case TPP_StyleWordWrap:
			lpValue = ( FALSE == GetStyleWordWrap() ) ? (LPARAM)0 : (LPARAM)1;
			break;

		case TPP_StyleEllipses:
			lpValue = ( FALSE == GetStyleEllipses() ) ? (LPARAM)0 : (LPARAM)1;
			break;
	}
	return lpValue;
}

void CSSheet::CTextPatternStructure::SetFontName( CString strFontName )
{
	if( 0 != strFontName.Compare( m_strFontName )  )
	{ 
		m_strFontName = strFontName; 
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetFontBold( BOOL fBold )
{ 
	if( fBold != m_fFontBold ) 
	{ 
		m_fFontBold = fBold; 
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetFontItalic( BOOL fItalic )
{ 
	if( fItalic != m_fFontItalic )
	{ 
		m_fFontItalic = fItalic; 
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetFontUnderline( BOOL fUnderline )
{
	if( fUnderline != m_fFontUnderline )
	{
		m_fFontUnderline = fUnderline; 
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetFontSize( int iSize )
{ 
	if( iSize != m_iFontSize )
	{ 
		m_iFontSize = iSize; 
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetForeColor( COLORREF foreColor )
{ 
	if( foreColor != m_TextColor )
	{ 
		m_TextColor = foreColor;
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetBackColor( COLORREF backColor )
{
	if( backColor != m_BackColor )
	{
		m_BackColor = backColor;
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetStyleHorizontalAlign( long lHorizontalAlign )
{
	if( lHorizontalAlign < SSS_ALIGN_LEFT || lHorizontalAlign > SSS_ALIGN_RIGHT )
	{
		return;
	}

	// If not the same as the old one...
	long lMask = SSS_ALIGN_LEFT | SSS_ALIGN_CENTER | SSS_ALIGN_RIGHT;

	if( ( m_lTextStyle & lMask ) != lHorizontalAlign )
	{
		// Must clear previous horizontal alignment
		m_lTextStyle &= ~lMask;
	
		// Set new horizontal alignment
		m_lTextStyle |= lHorizontalAlign;
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetStyleVerticalAlign( long lVerticalAlign )
{
	if( lVerticalAlign < SSS_ALIGN_TOP || lVerticalAlign > SSS_ALIGN_BOTTOM )
		return;

	// If not the same as the old one...
	long lMask = SSS_ALIGN_VCENTER | SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM;
	if( ( m_lTextStyle & lMask ) != lVerticalAlign )
	{
		// Must clear previous vertical alignment
		m_lTextStyle &= ~lMask;
		// Set new vertical alignment
		m_lTextStyle |= lVerticalAlign;
		m_fChanged = true;
	}
}

void CSSheet::CTextPatternStructure::SetStyleWordWrap( bool fSet )
{ 
	if( true == fSet )
	{
		// If not the same as the old one...
		if( 0 == ( m_lTextStyle & SS_TEXT_WORDWRAP ) )
		{
			m_lTextStyle |= SS_TEXT_WORDWRAP;
			m_fChanged = true;
		}
	}
	else
	{
		// If not the same as the old one...
		if( SS_TEXT_WORDWRAP == ( m_lTextStyle & SS_TEXT_WORDWRAP ) )
		{
			m_lTextStyle &= ( ~SS_TEXT_WORDWRAP );
			m_fChanged = true;
		}
	}
}

void CSSheet::CTextPatternStructure::SetStyleEllipses( bool fSet )
{ 
	if( true == fSet )
	{
		// If not the same as the old one...
		if( 0 == ( m_lTextStyle & SSS_ELLIPSES ) )
		{
			m_lTextStyle |= SSS_ELLIPSES;
			m_fChanged = true;
		}
	}
	else
	{
		// If not the same as the old one...
		if( SSS_ELLIPSES == ( m_lTextStyle & SSS_ELLIPSES ) )
		{
			m_lTextStyle &= ( ~SSS_ELLIPSES );
			m_fChanged = true;
		}
	}
}

CString* CSSheet::CTextPatternStructure::GetFontName( void ) { return &m_strFontName; }
BOOL CSSheet::CTextPatternStructure::GetFontBold( void ) { return m_fFontBold; }
BOOL CSSheet::CTextPatternStructure::GetFontItalic( void )  { return m_fFontItalic; }
BOOL CSSheet::CTextPatternStructure::GetFontUnderline( void )  { return m_fFontUnderline; }
int CSSheet::CTextPatternStructure::GetFontSize( void )  { return m_iFontSize; }
COLORREF CSSheet::CTextPatternStructure::GetForeColor( void )  { return m_TextColor; }
COLORREF CSSheet::CTextPatternStructure::GetBackColor( void )  { return m_BackColor; }
long CSSheet::CTextPatternStructure::GetStyleHorizontalAlign( void ) { return ( m_lTextStyle & ( SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER ) ); }
long CSSheet::CTextPatternStructure::GetStyleVerticalAlign( void ) { return ( m_lTextStyle & ( SSS_ALIGN_VCENTER | SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM ) ); }
bool CSSheet::CTextPatternStructure::GetStyleWordWrap( void ) {	return ( ( 0 == ( m_lTextStyle & SS_TEXT_WORDWRAP ) ) ? false : true ); }
bool CSSheet::CTextPatternStructure::GetStyleEllipses( void ) { return ( ( 0 == ( m_lTextStyle & SSS_ELLIPSES ) ) ? false : true ); }

// PRIVATE METHODS
bool CSSheet::CTextPatternStructure::_FillTextPattern( _SSTextPattern eTextPattern, bool fLight )
{
	bool fCreated = true;

	// CSSheet has fully access to all members and variables of CTextPattern!

	m_fFontBold = FALSE;
	m_fFontItalic = FALSE;
	m_fFontUnderline = FALSE;
	m_staticTextPattern = eTextPattern;
	
	switch( eTextPattern )
	{
		case TitlePage:
			m_strFontName =_T("Verdana");
			m_iFontSize = 16;
			m_BackColor = _TAH_TITLE_MAIN;
			m_TextColor = _TAH_WHITE;
			m_lTextStyle = SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER;
			break;

		case SubTitle:
			m_strFontName = _T("Verdana");
			m_iFontSize = 12;
			m_BackColor = _TAH_TITLE_SECOND;
			m_TextColor = _WHITE;
			m_lTextStyle = SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER;
			break;

		case TitleGroup:
			m_strFontName = _T("Verdana");
			m_iFontSize = 9;
			m_BackColor = _IMI_TITLE_GROUP1;
			m_TextColor = _WHITE;
			m_fFontBold = TRUE;
			m_lTextStyle = SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER;
			break;

		case UserStaticColumn:
			m_strFontName = _T("Arial Unicode MS");
			m_iFontSize = 8;
			m_BackColor = _WHITE;
			m_TextColor = ( false == fLight ) ? _TAH_BLACK : _TAH_BLACK_LIGHT;
			m_fFontBold = TRUE;
			m_lTextStyle = SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER;
			break;

		case TitleBox:
			m_strFontName = _T("Arial Unicode MS");
			m_iFontSize = 8;
			m_BackColor = _TAH_TITLE_MAIN;
			m_TextColor = _WHITE;
			m_fFontBold = TRUE;
			m_lTextStyle = SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER;
			break;

		case Arial8:
			m_strFontName = _T("Arial Unicode MS");
			m_iFontSize = 8;
			m_BackColor = _WHITE;
			m_TextColor = _BLACK;
			m_lTextStyle = SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER;
			break;

		case ArialItalic8:
			m_strFontName = _T("Arial Unicode MS");
			m_iFontSize = 8;
			m_BackColor = _WHITE;
			m_TextColor = _BLACK;
			m_lTextStyle = SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER;
			m_fFontItalic = TRUE;
			break;

		case ColumnHeader:
		case ColumnHeaderWordWrap:
			m_strFontName = _T("Arial Unicode MS");
			m_iFontSize = 8;
			m_BackColor = _WHITE;
			m_TextColor=_BLACK;
			m_fFontBold = TRUE;
			m_lTextStyle = SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER;
			if( ColumnHeaderWordWrap == eTextPattern )
				m_lTextStyle |= SS_TEXT_WORDWRAP;
			break;

		default:
			fCreated = false;
			break;
	}
	return fCreated;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CSSheet, TSpread )
	ON_WM_MOUSEMOVE()
	ON_MESSAGE( WM_MOUSELEAVE, OnMouseLeave )
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_CAPTURECHANGED()
	ON_WM_SIZE()
	ON_MESSAGE( WM_USER_SSHEETCOLWIDTHCHANGE, OnSSheetColWidthChange )
	ON_WM_TIMER()
	ON_WM_PAINT()
END_MESSAGE_MAP()

CSSheet::CSSheet()
	: CMessageManager( CMessageManager::ClassID::CID_CSSheet )
{
	m_mapSelectedRow.clear();
	m_mapSelectedRowsBySheet.clear();
	ZeroMemory( &m_ssPrintFormat, sizeof( m_ssPrintFormat ) );
	m_bInitialized = false;
	m_fEditionEnabled = false;
	m_vecTextPatternList.clear();
	m_pclCurrentTextPatternStructure = NULL;
	m_pclSavedTextPatternStructure = NULL;
	m_mapBitmapList.clear();
	m_mapImageToImageSelected.clear();
	m_mapImageSelectedToImage.clear();
	m_hHandOpen = (HICON)0;
	m_hHandClosed = (HICON)0;
	m_rBlockSelectionParams.Reset();
	m_rRowSelectionParams.Reset();
	m_rSaveSelectionsContext.SetSSheetPointer( this );
	m_hOnTimerEvent = NULL;
	m_pNotificationHandler = NULL;
}

CSSheet::~CSSheet()
{
	_ClearAllSelectedRows( NULL );
	m_rSaveSelectionsContext.CleanSelections();

	// Clear bitmap container
	_ClearBitmapContainer();

	// Don't delete 'm_pclSheetDescription' because here it's only a pointer on a Sheet Description from
	// a child class.

	if( m_vecTextPatternList.size() > 0 )
	{
		for( vecTextPatternListIter iter = m_vecTextPatternList.begin(); iter != m_vecTextPatternList.end(); iter++ )
		{
			if( NULL != *iter )
			{
				if( NULL != m_pclCurrentTextPatternStructure && m_pclCurrentTextPatternStructure == *iter )
				{
					m_pclCurrentTextPatternStructure = NULL;
				}

				delete *iter;
			}
		}
		m_vecTextPatternList.clear();
	}
	
	if( NULL != m_pclCurrentTextPatternStructure )
	{
		delete m_pclCurrentTextPatternStructure;
	}

	if( NULL != GetSafeHwnd() )
	{
		DestroyWindow();
	}
}

//////////////////////////////////////////////////////////////////////
// CSSheet member functions

BOOL CSSheet::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	m_bInitialized = false;
	BSTR LicenseKey = SysAllocString( (OLECHAR FAR *)L"Copyright (c) 1995 FarPoint" );

	if( FALSE == TSpread::Create( dwStyle, rect, pParentWnd, nID ) ) 
	{
		return FALSE;
	}
	
	Init();
	SysFreeString( LicenseKey );
	return TRUE;
}

void CSSheet::PostPoneDelete( void )
{
	m_bInitialized = false;
	::PostMessage( GetSafeHwnd(), WM_USER_DELETETSPREADINSTANCE, (WPARAM)this, 0 );
}

void CSSheet::Init( )
{
	m_bInitialized = false;
	m_fEditionEnabled = true;
	
	// Clean only sheet concerned.
	if( 0 != m_mapSelectedRowsBySheet.count( GetSheet() ) )
		_ClearAllSelectedRows( &m_mapSelectedRowsBySheet[GetSheet()] );
	
	m_rSaveSelectionsContext.CleanSelections();

	m_rBlockSelectionParams.Reset();
	m_rRowSelectionParams.Reset();

	// Clear bitmap container for current sheet.
	_ClearBitmapContainer( GetSheet() );

	// Set up main parameters
	SetMaxCols( 1 );
	SetMaxRows( 1 );

	// Force all cells to be static as default (any cell will becoming editable).
	SetTextPattern( ColumnHeader );
	FormatStaticText( -1, -1, -1, -1, _T("") );

	SetBool( SSB_ALLOWCOLMOVE, FALSE );
	SetBool( SSB_ALLOWROWMOVE, FALSE );
	SetUnitType( SS_UNITTYPE_NORMAL );
	SetBool( SSB_SHOWROWHEADERS, FALSE );
	SetBool( SSB_SHOWCOLHEADERS, FALSE );
	SetBool( SSB_ALLOWUSERFORMULAS, FALSE );
	SetBool( SSB_HORZSCROLLBAR, TRUE );
	SetBool( SSB_VERTSCROLLBAR, TRUE );
	SetBool( SSB_SCROLLBAREXTMODE, TRUE );						// To display scroll bars only when needed/
	SetBool( SSB_SCROLLBARSHOWMAX, TRUE );						// Changes scroll box position to reflect the maximum rows and columns.
	SetBool( SSB_HSCROLLBARTRACK, FALSE );						// Don't synchronize the horizontal scroll box with rows.
	SetBool( SSB_VSCROLLBARTRACK, FALSE );						// Don't synchronize the vertical scroll box with rows.
	SetBool( SSB_ALLOWDRAGDROP, TRUE );							// Allow drag and drop.
	SetBool( SSB_MOVEACTIVEONFOCUS, TRUE );

	SetCursor( SS_CURSORTYPE_DEFAULT, SS_CURSOR_ARROW );

	// Set the pointer to use the icon specified in the CursorIcon property
	m_hHandOpen = AfxGetApp()->LoadIcon( IDI_HANDOPEN );
	SetCursor( SS_CURSORTYPE_DRAGDROPAREA, m_hHandOpen );

	m_hHandClosed = AfxGetApp()->LoadIcon( IDI_HANDCLOSED );
	SetCursor( SS_CURSORTYPE_DRAGDROP, m_hHandClosed );

	SetAppearanceStyle( SS_APPEARANCESTYLE_VISUALSTYLES ); // Set by default the visual style appearance

	// Normal operation mode show only data in a spread sheet and do nothing more (no selection available as in 
	// single selection operation mode for example). Thus we can manage ourself navigation, selection and so on.
	// To manage navigation, we have to catch PreTranslateMessage (for example in RViewSSelSS) and do all needed tasks.
	// To avoid interference with Spread, we have also to catch 'LeaveCell' event for spread and return always 'TRUE' to force
	// spread to ignore any internal navigation.
	SetOperationMode( SS_OPMODE_NORMAL );
	SetUserResize( 0 );
	SetBool( SSB_GRIDLINES, FALSE );
	SetGrayAreaColor( _WHITE, SPREAD_COLOR_NONE );
	SetBackColorStyle( SS_BACKCOLORSTYLE_OVERGRID );

	SetBool( SSB_NOBORDER, TRUE );
//	SetAppearance( SS_APPEARANCE_FLAT );
	SetAppearance( SS_APPEARANCE_DEFAULT );
	SetSelBlockOptions( 0 );
	SetButtonDrawMode( SS_BDM_CURRENTCELL | SS_BDM_ALWAYSBUTTON );
	SetActiveCell( 0, 0 );
	SetBool( SSB_EDITMODEPERMANENT, FALSE );
	SetBool( SSB_EDITMODEREPLACE, TRUE );
	SetActiveCellHighlightStyle( SS_ACTIVECELLHIGHLIGHTSTYLE_OFF );

	// For Printing
	LONG_PTR lAppData = 0;
	GetPrintOptions( &m_ssPrintFormat, NULL, &lAppData );

	m_ssPrintFormat.fDrawBorder = FALSE;
	m_ssPrintFormat.fDrawShadows = FALSE;
	m_ssPrintFormat.fDrawColors = TRUE;
#ifdef DEBUG
 	SetBool( SSB_GRIDLINES, TRUE );
 	m_ssPrintFormat.fShowGrid = TRUE;
#endif
	m_ssPrintFormat.fShowColHeaders = FALSE;
	m_ssPrintFormat.fShowRowHeaders = FALSE;
	m_ssPrintFormat.fUseDataMax = TRUE;
	m_ssPrintFormat.dPrintType = 0;				// Print all the page
	// m_ssPrintFormat.x1CellRange
	// m_ssPrintFormat.y1CellRange
	// m_ssPrintFormat.x2CellRange
	// m_ssPrintFormat.y2CellRange
	// m_ssPrintFormat.nPageStart
	// m_ssPrintFormat.nPageEnd
	// m_ssPrintFormat.fMarginLeft
	// m_ssPrintFormat.fMarginTop
	// m_ssPrintFormat.fMarginRight
	// m_ssPrintFormat.fMarginBottom
	// m_ssPrintFormat.hDCPrinter
	// m_ssPrintFormat.wOrientation
	// m_ssPrintFormat.fSmartPrint
	m_ssPrintFormat.wScalingMethod = SS_PRINTSCALINGMETHOD_BESTFIT;
	// m_ssPrintFormat.ZoomFactor
	m_ssPrintFormat.nBestFitPagesWide = 1;
	m_ssPrintFormat.nBestFitPagesTall = 999;
	m_ssPrintFormat.fCenterOnPageH = FALSE;
	m_ssPrintFormat.fCenterOnPageV = FALSE;

	SetPrintOptions( &m_ssPrintFormat, NULL, lAppData );

	// Because bug in spread and windows, we must in particular case unlock the document after this one
	// has been created.
	SetLock( -1, -1, FALSE );

	if( NULL != m_hOnTimerEvent )
	{
		CloseHandle( m_hOnTimerEvent );
		m_hOnTimerEvent = NULL;
	}
	m_hOnTimerEvent = ::CreateEvent( NULL, TRUE, TRUE, NULL );

	m_bInitialized = true;
}

void CSSheet::Reset( void )
{
	// Kill notification handler.
	CMessageManager::MM_UnregisterAllNotificationHandlers( this );
	
	// Pay attention: there is 'Reset' and 'ResetSheet' in TSpread. First one reset all the control. That means that all other sheets in
	//                workbook will be impacted by this call. Second one clear only the current sheet.
	TSpread::Reset();

	Init();
}

void CSSheet::SetREDRAW(BOOL b)
{
	if( false == m_bInitialized )
		return;
	CWnd::SetRedraw(b);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Printing methods.
void CSSheet::OwnerPrintPreviewDraw( CDC *pDC, CRect &rectPage, short nPage, bool fAllSheetInOnePage )
{
	if( false == m_bInitialized )
		return;

	CDrawSet DrawSet;
	DrawSet.Init( pDC );
	
	LONG_PTR lAppData;
	GetPrintOptions( &m_ssPrintFormat, NULL, &lAppData );

	// In spread DLL there is no more the 'OwnerPrintDraw' function. And by the way 'TSpread::Print' does not the job with
	// a preview screen! In OCX version, internal API (SS_Print) is called with 'wAction' parameter set to 'SS_PRINT_ACTION_PREVIEW'.
	// In DLL there is no way to set ourself this variable.
	
	// To workaround, we have created a new exported function called 'PrintPreview'. Internally, spread functions work with
	// printer resolution (retrieved by calling 'GetDeviceCaps( hDC, HORZRES )' ). In the case of screen resolution we have for
	// example 1920x1080.

	// Bitmap created in 'CRviewProj::PrintPreview' has the size of the real portion of the screen where preview will be painted.
	// (for example 800x500). If we call 'TSpread::PrintPreview' without changes, spread will draw sheet while trying to occupy the 
	// total space available. Thus we will have a bitmap with only a portion of the sheet drawn.
	
	// And we DON'T WANT a bitmap corresponding to screen resolution. Because when spread has finished the printing job, 'CRviewProj::PrintPreview'
	// transfers bitmap from screen compatible device to print preview device. Thus print preview device paints in the screen portion available
	// (for example 800x500). If we use a 1920x1080 bitmap by calling 'TSpread::PrintPreview', distortions will occur when print preview
	// device will paint because it would need to reduce bitmap resolution from 1920x1080 to 800x500.

	// It's the reason why we set margin variables to force spread to draw only on area available for the bitmap.

	float dPrinterHorzRes = (float)GetDeviceCaps( pDC->GetSafeHdc(), HORZRES );
	float dPrinterVertRes = (float)GetDeviceCaps( pDC->GetSafeHdc(), VERTRES );
	float dPrinterHorzPixelsByInch = (float)GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSX );
	float dPrinterVertPixelsByInch = (float)GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSY );

	// Pay attention that these variables must be set in inch!
	m_ssPrintFormat.fMarginLeft = 0.0;
	m_ssPrintFormat.fMarginTop = 0.0;
	m_ssPrintFormat.fMarginRight = ( dPrinterHorzRes - (float)rectPage.right ) / dPrinterHorzPixelsByInch;
	m_ssPrintFormat.fMarginBottom = ( dPrinterVertRes - (float)rectPage.bottom ) / dPrinterVertPixelsByInch;

	// In SS_PRINT.c at line 934, spread add by default 1/16" pixel to the left margin if this one is set to 0.
	float dMinOffsetFactor = 16.0;
	m_ssPrintFormat.fMarginRight = max( (float)0.0, m_ssPrintFormat.fMarginRight - ( 1 / dMinOffsetFactor ) );
	m_ssPrintFormat.fMarginBottom = max( (float)0.0, m_ssPrintFormat.fMarginBottom - ( 1 / dMinOffsetFactor ) );

	m_ssPrintFormat.nPageStart = nPage;
	m_ssPrintFormat.nPageEnd = nPage;

	// fAllSheetInOnePage
	// When computing zoom factor, we try to set the correct setting to allow to print the total width of a sheet in the
	// available width of the printer. It's possible that even with a zoom factor below 1, total height can't be print in
	// only one page. In this particular case, we split our sheet height as many pages as necessary.
	// But there is some special where we don't want to split our sheet but print this one in only one printable page.
	// It's the case for example when printing 'Circuit detailed' (called by 'CRViewProj::OP_Circuit' method).
	// Here we must thus compute zoom factor to try to print width BUT ALSO the total height of a sheet in the printable area.

	// Compute zoom factor.
	CRect rcSpreadPixel = GetSheetSizeInPixels( false );
	CRect rcTempCalc( rectPage );
	if( false == fAllSheetInOnePage || ( rcSpreadPixel.Width() - rectPage.Width() > rcSpreadPixel.Height() - rectPage.Height() ) )
	{
		m_ssPrintFormat.ZoomFactor = rcTempCalc.Width();
		if( 0.0 == m_ssPrintFormat.fMarginLeft )
			m_ssPrintFormat.ZoomFactor -= ( dPrinterHorzPixelsByInch / dMinOffsetFactor );
		if( 0.0 == m_ssPrintFormat.fMarginRight )
			m_ssPrintFormat.ZoomFactor -= ( dPrinterHorzPixelsByInch / dMinOffsetFactor );
		m_ssPrintFormat.ZoomFactor /= (double)rcSpreadPixel.right;
	}
	else
	{
		m_ssPrintFormat.ZoomFactor = rcTempCalc.Height();
		if( 0.0 == m_ssPrintFormat.fMarginTop )
			m_ssPrintFormat.ZoomFactor -= ( dPrinterVertPixelsByInch / dMinOffsetFactor );
		if( 0.0 == m_ssPrintFormat.fMarginBottom )
			m_ssPrintFormat.ZoomFactor -= ( dPrinterVertPixelsByInch / dMinOffsetFactor );
		m_ssPrintFormat.ZoomFactor /= (double)rcSpreadPixel.bottom;
	}

	// Do not scale when zoom factor is bigger than 1.0
	if( m_ssPrintFormat.ZoomFactor < 1.0 )
		m_ssPrintFormat.wScalingMethod = SS_PRINTSCALINGMETHOD_ZOOM;
	else
	{
		m_ssPrintFormat.ZoomFactor = 1.0;
		m_ssPrintFormat.wScalingMethod = SS_PRINTSCALINGMETHOD_NONE;
	}
		
	m_ssPrintFormat.hDCPrinter = pDC->GetSafeHdc();
	SetPrintOptions( &m_ssPrintFormat, NULL, lAppData );
	
	TSpread::PrintPreview( _T(""), &m_ssPrintFormat, NULL, 0, NULL );
}

void CSSheet::OwnerPrintDraw( CDC *pDC, CRect &rectPage, short nPage, bool fAllSheetInOnePage )
{
	if( false == m_bInitialized )
		return;

	CDrawSet DrawSet;
	DrawSet.Init( pDC );
	
	LONG_PTR lAppData;
	GetPrintOptions( &m_ssPrintFormat, NULL, &lAppData );

	CDC* pScreenDC = GetDC();
	float dScreenHorzPixelsByInch = (float)pScreenDC->GetDeviceCaps( LOGPIXELSX );
	float dScreenVertPixelsByInch = (float)pScreenDC->GetDeviceCaps( LOGPIXELSY );
	ReleaseDC( pScreenDC );

	float dPrinterHorzRes = (float)GetDeviceCaps( pDC->GetSafeHdc(), HORZRES );
	float dPrinterVertRes = (float)GetDeviceCaps( pDC->GetSafeHdc(), VERTRES );
	float dPrinterHorzPixelsByInch = (float)GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSX );
	float dPrinterVertPixelsByInch = (float)GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSY );

	m_ssPrintFormat.fMarginLeft = (float)rectPage.left / dPrinterHorzPixelsByInch;
	m_ssPrintFormat.fMarginTop = (float)rectPage.top / dPrinterVertPixelsByInch;
	m_ssPrintFormat.fMarginRight = ( dPrinterHorzRes - (float)rectPage.right ) / dPrinterHorzPixelsByInch;
	m_ssPrintFormat.fMarginBottom = ( dPrinterVertRes - (float)rectPage.bottom ) / dPrinterVertPixelsByInch;

	// In SS_PRINT.c at line 934, spread add by default 1/16" pixel to the left margin if this one is set to 0.
	float dMinOffsetFactor = 16.0;
	m_ssPrintFormat.fMarginRight = max( (float)0.0, m_ssPrintFormat.fMarginRight - ( 1 / dMinOffsetFactor ) );
	m_ssPrintFormat.fMarginBottom = max( (float)0.0, m_ssPrintFormat.fMarginBottom - ( 1 / dMinOffsetFactor ) );

	m_ssPrintFormat.nPageStart = nPage;
	m_ssPrintFormat.nPageEnd = nPage;

	// fAllSheetInOnePage
	// When computing zoom factor, we try to set the correct setting to allow to print the total width of a sheet in the
	// available width of the printer. It's possible that even with a zoom factor below 1, total height can't be print in
	// only one page. In this particular case, we split our sheet height as many pages as necessary.
	// But there is some special where we don't want to split our sheet but print this one in only one printable page.
	// It's the case for example when printing 'Circuit detailed' (called by 'CRViewProj::OP_Circuit' method).
	// Here we must thus compute zoom factor to try to print width BUT ALSO the total height of a sheet in the printable area.

	// Compute zoom factor.
	CRect rcSpreadPixel = GetSheetSizeInPixels( false );
	CRect rcTempCalc( rectPage );
	if( false == fAllSheetInOnePage || ( rcSpreadPixel.Width() > rcSpreadPixel.Height() ) )
	{
		m_ssPrintFormat.ZoomFactor = rcTempCalc.Width(); 
		if( 0.0 == m_ssPrintFormat.fMarginLeft )
			m_ssPrintFormat.ZoomFactor -= ( dPrinterHorzPixelsByInch / dMinOffsetFactor );
		if( 0.0 == m_ssPrintFormat.fMarginRight )
			m_ssPrintFormat.ZoomFactor -= ( dPrinterHorzPixelsByInch / dMinOffsetFactor );
		m_ssPrintFormat.ZoomFactor /= ( (double)rcSpreadPixel.Width() / dScreenHorzPixelsByInch * dPrinterHorzPixelsByInch );
	}
	else
	{
		m_ssPrintFormat.ZoomFactor = rcTempCalc.Height();
		if( 0.0 == m_ssPrintFormat.fMarginTop )
			m_ssPrintFormat.ZoomFactor -= ( dPrinterVertPixelsByInch / dMinOffsetFactor );
		if( 0.0 == m_ssPrintFormat.fMarginBottom )
			m_ssPrintFormat.ZoomFactor -= ( dPrinterVertPixelsByInch / dMinOffsetFactor );
		m_ssPrintFormat.ZoomFactor /= ( (double)rcSpreadPixel.Height() / dScreenVertPixelsByInch * dPrinterVertPixelsByInch );
	}

	// Do not scale when zoom factor is bigger than 1.0
	if( m_ssPrintFormat.ZoomFactor < 1.0 )
		m_ssPrintFormat.wScalingMethod = SS_PRINTSCALINGMETHOD_ZOOM;
	else
	{
		m_ssPrintFormat.ZoomFactor = 1.0;
		m_ssPrintFormat.wScalingMethod = SS_PRINTSCALINGMETHOD_NONE;
	}
	
	m_ssPrintFormat.hDCPrinter = pDC->GetSafeHdc();
	SetPrintOptions( &m_ssPrintFormat, NULL, lAppData );
	
	TSpread::PrintPreview( _T(""), &m_ssPrintFormat, NULL, 0, NULL );
}

void CSSheet::OwnerPrintPageCount( CDC *pDC, CRect &rectPage, long &lPageCount )
{
	if( false == m_bInitialized )
		return;

	CDrawSet DrawSet;
	DrawSet.Init( pDC );

	LONG_PTR lAppData;
	GetPrintOptions( &m_ssPrintFormat, NULL, &lAppData );

	// Convert rectPage (contains page size in pixel) to rectMargins (contains page borders in pixel)
	CRect rectMargins( rectPage );

	CDC* pScreenDC = GetDC();
	float dScreenHorzPixelsByInch = (float)pScreenDC->GetDeviceCaps( LOGPIXELSX );
	ReleaseDC( pScreenDC );

	CPreviewDC* pPreviewDC = (CPreviewDC*)pDC;
	float dPrinterHorzRes = (float)GetDeviceCaps( pPreviewDC->m_hAttribDC, HORZRES );
	float dPrinterVertRes = (float)GetDeviceCaps( pPreviewDC->m_hAttribDC, VERTRES );
	float dPrinterHorzPixelsByInch = (float)GetDeviceCaps( pPreviewDC->m_hAttribDC, LOGPIXELSX );
	float dPrinterVertPixelsByInch = (float)GetDeviceCaps( pPreviewDC->m_hAttribDC, LOGPIXELSY );

	m_ssPrintFormat.fMarginLeft = (float)rectPage.left / dPrinterHorzPixelsByInch;
	m_ssPrintFormat.fMarginTop = (float)rectPage.top / dPrinterVertPixelsByInch;
	m_ssPrintFormat.fMarginRight = ( dPrinterHorzRes - (float)rectPage.right ) / dPrinterHorzPixelsByInch;
	m_ssPrintFormat.fMarginBottom = ( dPrinterVertRes - (float)rectPage.bottom ) / dPrinterVertPixelsByInch;

	// In SS_PRINT.c at line 934, spread add by default 1/16" pixel to the left margin if this one is set to 0.
	float dMinOffsetFactor = 16.0;
	m_ssPrintFormat.fMarginRight = max( (float)0.0, m_ssPrintFormat.fMarginRight - ( 1 / dMinOffsetFactor ) );
	m_ssPrintFormat.fMarginBottom = max( (float)0.0, m_ssPrintFormat.fMarginBottom - ( 1 / dMinOffsetFactor ) );
		
	// Compute zoom factor.
	CRect rcSpreadPixel = GetSheetSizeInPixels( false );
	CRect rcTempCalc( rectPage );
	m_ssPrintFormat.ZoomFactor = rcTempCalc.Width(); 
	if( 0.0 == m_ssPrintFormat.fMarginLeft )
		m_ssPrintFormat.ZoomFactor -= ( dPrinterHorzPixelsByInch / dMinOffsetFactor );
	if( 0.0 == m_ssPrintFormat.fMarginRight )
		m_ssPrintFormat.ZoomFactor -= ( dPrinterHorzPixelsByInch / dMinOffsetFactor );
	m_ssPrintFormat.ZoomFactor /= ( (double)rcSpreadPixel.Width() / dScreenHorzPixelsByInch * dPrinterHorzPixelsByInch );
	
	// Do not scale when zoom factor is bigger than 1.0
	if( m_ssPrintFormat.ZoomFactor < 1.0 )
		m_ssPrintFormat.wScalingMethod = SS_PRINTSCALINGMETHOD_ZOOM;
	else
	{
		m_ssPrintFormat.ZoomFactor = 1.0;
		m_ssPrintFormat.wScalingMethod = SS_PRINTSCALINGMETHOD_NONE;
	}

	m_ssPrintFormat.hDCPrinter = pDC->GetSafeHdc();

	SetPrintOptions( &m_ssPrintFormat, NULL, lAppData );

	lPageCount = TSpread::GetPrintPageCount();
}
// End of printing methods.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods relative to coordinate.
void CSSheet::TwipsToPixels( CRect &rect )
{
	if( false == m_bInitialized )
		return;

	double ppi;
	CDC* pDC = GetDC();
	ppi = pDC->GetDeviceCaps( LOGPIXELSY );
	rect.top = (int)( ( rect.top * ppi ) / 1440.0 );
	rect.bottom = (int)( ( rect.bottom * ppi ) / 1440 );
	ppi = pDC->GetDeviceCaps( LOGPIXELSX );
	rect.right = (int)( rect.right * ppi / 1440 );
	rect.left = (int)( rect.left * ppi / 1440 );
	ReleaseDC( pDC );
}

void CSSheet::PixelsToTwips( CRect &rect )
{
	if( false == m_bInitialized )
		return;

	double ppi;
	CDC* pDC = GetDC();
	ppi = pDC->GetDeviceCaps( LOGPIXELSY );
	rect.top = (int)( rect.top * 1440.0 / ppi );
	rect.bottom = (int)( rect.bottom * 1440.0 / ppi );
	ppi = pDC->GetDeviceCaps( LOGPIXELSX );
	rect.right = (int)( rect.right * 1440.0 / ppi );
	rect.left = (int)( rect.left * 1440.0 / ppi );
	ReleaseDC( pDC );
}

CRect CSSheet::GetSelectionInPixels( long lFromColumn, long lFromRow, long lToColumn, long lToRow )
{
	if( false == m_bInitialized )
		return CRect( 0, 0, 0, 0 );

	CRect rect( 0, 0, 0, 0 );
	
	if( lFromColumn > 0 && lToColumn > 0 )
	{
		for( long lLoopCol = lFromColumn; lLoopCol <= lToColumn; lLoopCol++ )
		{
			if( FALSE == IsColHidden( lLoopCol ) )
			{
				int iColWidth = 0;
				GetColWidthInPixels( lLoopCol, &iColWidth );
				rect.right += iColWidth;
			}
		}
	}

	if( lFromRow > 0 && lToRow > 0 )
	{
		for( long lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
		{
			if( FALSE == IsRowHidden( lLoopRow ) )
			{
				int iRowHeight = 0;
				GetRowHeightInPixels( lLoopRow, &iRowHeight );
				rect.bottom += iRowHeight;
			}
		}
	}
	
	return rect;
}

CRect CSSheet::GetSelectionInPixels( CRect rectRange )
{
	if( false == m_bInitialized )
		return CRect( 0, 0, 0, 0 );

	return GetSelectionInPixels( rectRange.left, rectRange.top, rectRange.right, rectRange.bottom );
}

CRect CSSheet::GetSelectionCoordInPixels( CRect rectRange )
{
	if( false == m_bInitialized )
		return CRect( 0, 0, 0, 0 );

	CRect rectSelection = GetSelectionInPixels( rectRange.left, rectRange.top, rectRange.right, rectRange.bottom );
	CRect rectTopLeftSelection = GetCellCoordInPixel( rectRange.left, rectRange.top );
	rectSelection.MoveToXY( rectTopLeftSelection.left, rectTopLeftSelection.top );
	return rectSelection;
}

CRect CSSheet::GetCellCoordInPixel( long lColumn, long lRow )
{
	if( false == m_bInitialized )
		return CRect( 0, 0, 0, 0 );

	CRect rect( 0, 0, 0, 0 );
	for( long lLoopCol = 0; lLoopCol <= lColumn; lLoopCol++ )
	{
		if( FALSE == IsColHidden( lLoopCol ) )
		{
			int iColWidth = 0;
			GetColWidthInPixels( lLoopCol, &iColWidth );
			rect.left = rect.right;
			rect.right += iColWidth;
		}
	}
	for( long lLoopRow = 0; lLoopRow <= lRow; lLoopRow++ )
	{
		if( FALSE == IsRowHidden( lLoopRow ) )
		{
			int iRowHeight = 0;
			GetRowHeightInPixels( lLoopRow, &iRowHeight );
			rect.top = rect.bottom;
			rect.bottom += iRowHeight;
		}
	}
	return rect;
}

double CSSheet::GetSheetWidth( void )
{
	if( false == m_bInitialized )
		return 0.0;

	CRect rect = GetSheetSizeInPixels();
	double dWidth = 0.0;
	LogUnitsToColWidth( rect.Width(), &dWidth );
	return dWidth;
}

double CSSheet::GetSheetHeight( void )
{
	if( false == m_bInitialized )
		return 0.0;

	CRect rect = GetSheetSizeInPixels();
	double dHeight = 0.0;
	LogUnitsToColWidth( rect.Height(), &dHeight );
	return dHeight;
}

CRect CSSheet::GetSheetSizeInPixels( bool fWithScrollBar )
{
	if( false == m_bInitialized )
		return CRect( 0, 0, 0, 0 );

	CRect rect = GetSelectionInPixels( 1, 1, GetMaxCols(), GetMaxRows() );
	if( true == fWithScrollBar )
	{
		// Check if scrollbar exist.
		if( TRUE == IsHorizontalScrollBarVisible() )
			rect.bottom += GetSystemMetrics( SM_CXHSCROLL );
		
		if( TRUE == IsVerticalScrollBarVisible() )
			rect.right += GetSystemMetrics( SM_CXVSCROLL );
	}
	return rect;
}

DOUBLE CSSheet::GetColWidthW( long lColumn )
{
	if( false == m_bInitialized )
		return -1.0;

	DOUBLE dColWidth = -1.0;
	GetColWidth( lColumn, &dColWidth );
	return dColWidth;
}

long CSSheet::GetColWidthInPixelsW( long lColumn )
{
	if( false == m_bInitialized )
		return -1;

	int iColWidth = -1;
	GetColWidthInPixels( lColumn, &iColWidth );
	return (long)iColWidth;
}

long CSSheet::GetRowHeightInPixelsW( long lRow )
{
	if( false == m_bInitialized )
		return -1;

	int iRowHeight = -1;
	GetRowHeightInPixels( lRow, &iRowHeight );
	return (long)iRowHeight;
}

DOUBLE CSSheet::GetMaxTextColWidthW( long lColumn )
{
	if( false == m_bInitialized )
		return -1.0;

	DOUBLE dColWidth;
	GetMaxTextColWidth( lColumn, &dColWidth );
	return dColWidth;
}

// End of methods relative to coordinate.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage Expand/Collapse columns feature.
void CSSheet::CreateExpandCollapseColumnButton( long lButtonCol, long lButtonRow, long lFromColumn, long lToColumn, long lLastRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	ASSERT( lButtonRow <= GetMaxRows() );
	ASSERT( lButtonCol <= GetMaxCols() );
	ASSERT( lFromColumn <= lToColumn && lToColumn <= GetMaxCols() );

	// Remark: if col has the same width, 'SetColWidth' returns 'FALSE'.
	SetColWidth( lButtonCol, .8 );
	SetRowHeight( lButtonRow, 8 );

	SetPictureCellWithID( __GET_ECCB_COLLAPSE_ICON, lButtonCol, lButtonRow, CSSheet::PictureCellType::Icon );

	// Remark: 'GetColUserData' returns 'FALSE' if user data is not yet been set.
	LONG_PTR lUserData = 0;
	GetColUserData( lButtonCol, &lUserData );
	
	// Set column and row range concerned by this collapse button.
	_SETCOLSTARTCOLLAPSE( lUserData, lFromColumn );
	_SETCOLENDCOLLAPSE( lUserData, lToColumn );
	_SETEXPANDCOLLAPSEVALUE( lUserData, _ECCBSTATE_COLLAPSE );

	// Save user data.
	SetColUserData( lButtonCol, lUserData );

	SetFlagCanBeCollapsed( lFromColumn, lToColumn, true );
	SetFlagAlwaysHidden( lFromColumn, lToColumn, false );

	// Identify cell as a expand/collapse column button.
	SetCellProperty( lButtonCol, lButtonRow, _SSCellProperty::CellExpandCollapseColumnButton, true );
}

void CSSheet::DeleteExpandCollapseColumnButton( long lButtonCol, long lButtonRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	// If cell at lButtonCol and lButtonRow is well a collapse button ...
	if( true == IsExpandCollapseColumnButton( lButtonCol, lButtonRow ) )
	{
		// It is no more a expand/collapse column button.
		SetCellProperty( lButtonCol, lButtonRow, _SSCellProperty::CellExpandCollapseColumnButton, false );

		// Change cell type to text.
		SS_CELLTYPE rCellType;

		if( NULL == SetTypeStaticText( &rCellType, SS_TEXT_LEFT ) )
		{
			return;
		}

		SetCellType( lButtonCol, lButtonRow, &rCellType );

		// Because the magic number (see the 'SetPictureCellWithHandle' method), we must reset the text in the cell.
		SetCellText( lButtonCol, lButtonRow, _T("") );
	}
}

bool CSSheet::ChangeExpandCollapseColumnButtonState( long lButtonCol, long lButtonRow, long lFromRow, long lToRow, ExpandCollapseColumnAction eAction, bool fRedraw )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	// Remark: 'lColumn' and 'lRow' are the button position.

	// Check if cell at col and row is not a collapse button ...
	if( false == IsExpandCollapseColumnButton( lButtonCol, lButtonRow ) )
	{
		return false;
	}

	// Retrieve column status.
	// Remark: 'GetColUserData' returns 'FALSE' if user data is not yet been set.
	LONG_PTR lUserData = 0;
	GetColUserData( lButtonCol, &lUserData );
	
	// Check if show or hide.
	bool fMustCollapseColumn = false;

	if( ECCA_Toggle == eAction )
	{
		// Must toggle state.
		fMustCollapseColumn = ( _ECCBSTATE_EXPAND == _GETEXPANDCOLLAPSEVALUE( lUserData ) ) ? false : true;
	}
	else 
	{
		fMustCollapseColumn = ( ECCA_ToCollapse == eAction ) ? true: false;
	}

	SetPictureCellWithID( ( true == fMustCollapseColumn ) ? __GET_ECCB_EXPAND_ICON : __GET_ECCB_COLLAPSE_ICON,  lButtonCol, lButtonRow, CSSheet::PictureCellType::Icon );

	// Save user data.
	_SETEXPANDCOLLAPSEVALUE( lUserData, ( true == fMustCollapseColumn ) ? _ECCBSTATE_EXPAND : _ECCBSTATE_COLLAPSE );
	SetColUserData( lButtonCol, lUserData );

	// Retrieve set of columns controlled by button								
	int iColStart = _GETCOLSTARTCOLLAPSE( lUserData );
	int iColEnd = _GETCOLENDCOLLAPSE( lUserData );
	ASSERT( iColStart <= iColEnd && iColEnd <= GetMaxCols() );

	// Run all columns
	for( int iLoopCol = iColStart; iLoopCol <= iColEnd; iLoopCol++ )
	{
		// Remark: 'GetColUserData' returns 'FALSE' if user data is not yet been set
		lUserData = 0;
		GetColUserData( iLoopCol, &lUserData );

		if( true == _IS_FLAG_ALWAYSHIDDEN( lUserData ) )
		{
			ShowCol( iLoopCol, FALSE );
		}
		else if( true == _IS_FLAG_CANBECOLLAPSED( lUserData ) )
		{
			// ... else if column can be collapsed

			bool fFlag = !fMustCollapseColumn;
			
			// If we must show column...
			if( false == fMustCollapseColumn )
			{
				// If flag _SHOWISEMPTY is not set...
				if( false == _IS_FLAG_SHOWEVENEMPTY( lUserData ) )
				{
					// Because 'lRow' is the row where button is, we can test if column is empty on this row!
					bool fIsColEmpty = IsColumnEmpty( iLoopCol, lFromRow, lToRow );

					// If column is empty then we must not show it
					if( true == fIsColEmpty )
						fFlag = false;
				}
			}

			ShowCol( iLoopCol, ( true == fFlag ) ? TRUE : FALSE );
		}
	}

	return true;
}

int CSSheet::GetExpandCollapseColumnState( long lButtonCol, long lButtonRow )
{
	if( false == m_bInitialized )
	{
		return ECCS_Invalid;
	}

	if( false == IsExpandCollapseColumnButton( lButtonCol, lButtonRow ) )
	{
		return ECCS_Invalid;
	}

	LONG_PTR lUserData = 0;
	GetColUserData( lButtonCol, &lUserData );

	return ( _ECCBSTATE_EXPAND == _GETEXPANDCOLLAPSEVALUE( lUserData ) ) ? ECCS_Collapsed : ECCS_Expanded;
}

bool CSSheet::IsExpandCollapseColumnButton( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	SS_CELLTYPE rCellType;
	
	if( FALSE == GetCellType( lColumn, lRow, &rCellType ) )
	{
		return false;
	}

	if( SS_TYPE_PICTURE != rCellType.Type )
	{
		return false;
	}

	// If flag is not set...
	return IsCellProperty( lColumn, lRow, _SSCellProperty::CellExpandCollapseColumnButton );
}

void CSSheet::GetListOffAllExpandCollapseButton( long lButtonRow, std::vector<long>* pvecButtonList )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( lButtonRow < 1 || lButtonRow > GetMaxRows() || NULL == pvecButtonList )
	{
		return;
	}

	for( long lLoopColumn = 1; lLoopColumn <= GetMaxCols(); lLoopColumn++ )
	{
		if( true == IsExpandCollapseColumnButton( lLoopColumn, lButtonRow ) )
		{
			pvecButtonList->push_back( lLoopColumn );
		}
	}
}

bool CSSheet::GetColsManagedByButtonCollapse( long lButtonCol, long lButtonRow, long &lFromColumn, long &lToColumn )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	if( false == IsExpandCollapseColumnButton( lButtonCol, lButtonRow ) )
	{
		return false;
	}

	// Remark: 'GetColUserData' returns 'FALSE' if user data is not yet been set.
	LONG_PTR lUserData = 0;
	GetColUserData( lButtonCol, &lUserData );

	lFromColumn = _GETCOLSTARTCOLLAPSE( lUserData );
	lToColumn = _GETCOLENDCOLLAPSE( lUserData );
	return true;
}

bool CSSheet::IsColManagedByButtonCollapse( long lColumnToTest, long lButtonRow )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	bool fReturn = false;
	std::vector<long> vecButtonList;
	GetListOffAllExpandCollapseButton( lButtonRow, &vecButtonList );
	
	if( vecButtonList.size() > 0 )
	{
		std::vector<long>::iterator iter = vecButtonList.begin();
		while( iter != vecButtonList.end() && false == fReturn )
		{
			long lFromColumn, lToColumn;
			long lCol = *iter;

			if( true == GetColsManagedByButtonCollapse( lCol, lButtonRow, lFromColumn, lToColumn ) )
			{
			
				if( lColumnToTest >= lFromColumn && lColumnToTest <= lToColumn )
				{
					fReturn = true;
				}

			}

			++iter;
		}
	}
	return fReturn;
}

void CSSheet::VerifyExpandCollapseColumnButtons( CRect &clAreaToVerify, long lButtonRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	// Scan column range to check if there is some Expand/Collapse column buttons.
	for( long lLoopButton = clAreaToVerify.left; lLoopButton <= clAreaToVerify.right; lLoopButton++ )
	{
		if( false == IsCellProperty( lLoopButton, lButtonRow, _SSCellProperty::CellExpandCollapseColumnButton ) )
		{
			continue;
		}

		// Button found. 
		long lColStart;
		long lColEnd;

		if( false == GetColsManagedByButtonCollapse( lLoopButton, lButtonRow, lColStart, lColEnd ) )
		{
			continue;
		}

		// Run column range covered by this button.
		bool fButtonNeeded = false;
		for( int iLoopCol = lColStart; iLoopCol <= lColEnd; iLoopCol++ )
		{
			// No Expand/Collapse if this column can't be collapsed.
			if( false == IsFlagCanBeCollapsed( iLoopCol ) )
			{
				continue;
			}

			// No Expand/Collapse if this column must be always hidden.
			if( true == IsFlagAlwaysHidden( iLoopCol ) )
			{
				continue;
			}
			
			// If column must be shown even if empty.
			// OR if column can be hidden but there is some data...
			bool fIsShowEvenEmpty = IsFlagShowEvenEmpty( iLoopCol );
			
			if( ( true == fIsShowEvenEmpty) || ( ( false == fIsShowEvenEmpty ) && ( false == IsColumnEmpty( iLoopCol, clAreaToVerify.top, clAreaToVerify.bottom ) ) ) )
			{
				// In only in this condition we are sure that there is at least one column that can be collapsed. And then we need the button.
				fButtonNeeded = true;
				break;
			}
		}

		// If no more need of collapse button.
		if( false == fButtonNeeded )
		{
			DeleteExpandCollapseColumnButton( lLoopButton, lButtonRow );
		}
	}
}

void CSSheet::CreateMainExpandCollapseButton( long lButtonCol, long lButtonRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	// Identify cell as a CollapseButton.
	SetCellProperty( lButtonCol, lButtonRow, CellMainExpandCollapseButton, true );
	SetPictureCellWithID( __GET_BUTTON_MAINEXPAND_MODE_ICON, lButtonCol, lButtonRow, CSSheet::PictureCellType::Icon );
	
	long lUserData = 0;
	_SETEXPANDCOLLAPSEVALUE( lUserData, _ECCBSTATE_EXPAND );

	// Save user data.
	SetColUserData( lButtonCol, lUserData );
}

void CSSheet::DeleteMainExpandCollapseButton( long lButtonCol, long lButtonRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( true == IsMainExpandCollapseButton( lButtonCol, lButtonRow ) )
	{
		// Delete general collapse/button
		SS_CELLTYPE rCellType;
		SetTypeStaticText( &rCellType, SSS_ALIGN_LEFT );
		SetCellType( lButtonCol, lButtonRow, &rCellType );

		// Because the magic number (see the 'SetPictureCellWithHandle' method), we must reset the text in the cell.
		SetCellText( lButtonCol, lButtonRow, _T("") );
	}
}

void CSSheet::ChangeMainExpandCollapseButtonState( long lButtonCol, long lButtonRow, ExpandCollapseColumnAction eAction )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( true == IsMainExpandCollapseButton( lButtonCol, lButtonRow ) )
	{
		LONG_PTR lUserData = 0;
		GetColUserData( lButtonCol, &lUserData );

		int iPictureID = 0;
		
		switch( eAction )
		{
			case ECCA_ToCollapse:
				iPictureID = __GET_BUTTON_MAINCOLLAPSE_MODE_ICON;
				break;

			case ECCA_ToExpand:
				iPictureID = __GET_BUTTON_MAINEXPAND_MODE_ICON;
				break;

			case ECCA_Toggle:
				if( _ECCBSTATE_COLLAPSE == _GETEXPANDCOLLAPSEVALUE( lUserData ) )
				{
					iPictureID = __GET_BUTTON_MAINEXPAND_MODE_ICON;
				}
				else if( _ECCBSTATE_EXPAND == _GETEXPANDCOLLAPSEVALUE( lUserData ) )
				{
					iPictureID = __GET_BUTTON_MAINCOLLAPSE_MODE_ICON;
				}
				break;
		}
		
		if( 0 != iPictureID )
		{
			SetPictureCellWithID( iPictureID, lButtonCol, lButtonRow, CSSheet::PictureCellType::Icon );
			
			_SETEXPANDCOLLAPSEVALUE( lUserData, ( __GET_BUTTON_MAINCOLLAPSE_MODE_ICON == iPictureID ) ? _ECCBSTATE_COLLAPSE : _ECCBSTATE_EXPAND );
			SetColUserData( lButtonCol, lUserData );
		}
	}
}

int CSSheet::GetMainExpandCollapseColumnState( long lButtonCol, long lButtonRow )
{
	if( false == m_bInitialized )
	{
		return ECCS_Invalid;
	}

	if( false == IsMainExpandCollapseButton( lButtonCol, lButtonRow ) )
	{
		return ECCS_Invalid;
	}

	LONG_PTR lUserData = 0;
	GetColUserData( lButtonCol, &lUserData );

	return ( ( _ECCBSTATE_EXPAND == _GETEXPANDCOLLAPSEVALUE( lUserData ) ) ? ECCS_Collapsed : ECCS_Expanded );
}

bool CSSheet::IsMainExpandCollapseButton( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	SS_CELLTYPE rCellType;

	if( FALSE == GetCellType( lColumn, lRow, &rCellType ) )
	{
		return false;
	}

	if( SS_TYPE_PICTURE != rCellType.Type )
	{
		return false;
	}

	// If flag is not set...
	return IsCellProperty( lColumn, lRow, _SSCellProperty::CellMainExpandCollapseButton );
}

void CSSheet::VerifyMainExpandCollapseButton( long lMainButtonCol, long lMainButtonRow, long lChildButtonRow, long lFromCol, long lToCol )
{
	if( false == m_bInitialized )
	{
		return;
	}

	// Run all columns and check state of all Expand/Collapse column buttons.
	bool fAtLeastOneGroupCollapsed = false;
	bool fAtLeastOneGroupExpanded = false;
	CheckExpandCollapseColumnButtonState( lFromCol, lToCol, lChildButtonRow, &fAtLeastOneGroupCollapsed, &fAtLeastOneGroupExpanded );
	
	// If we need main Expand/Collapse column button...
	if( true == fAtLeastOneGroupCollapsed || true == fAtLeastOneGroupExpanded )
	{
		// Check which button we must set.
		bool fGeneralStateCollapse;
		bool fDoChange = true;

		if( true == fAtLeastOneGroupCollapsed && true == fAtLeastOneGroupExpanded )
		{
			// If there is a mix of columns collapsed and expanded, we must not change
			// general button state.
			fDoChange = false;
		}
		else if( true == fAtLeastOneGroupCollapsed )
		{
			// It means that all groups are collapsed and then general button must be set in expand mode.
			fGeneralStateCollapse = false;
		}
		else if( true == fAtLeastOneGroupExpanded )
		{
			// It means that all groups are expanded and then general button must be set in collapse mode.
			fGeneralStateCollapse = true;
		}
		else
		{
			// This case not happens here!
		}

		// If general button state must be changed...
		if( true == fDoChange )
		{
			ChangeMainExpandCollapseButtonState( lMainButtonCol, lMainButtonRow, ( true == fGeneralStateCollapse ) ? ECCA_ToCollapse : ECCA_ToExpand );
		}
	}
	else
	{
		// Delete general collapse/button
		DeleteMainExpandCollapseButton( lMainButtonCol, lMainButtonRow );
	}
}

void CSSheet::CheckExpandCollapseColumnButtonState( long lFromColumn, long lToColumn, long lButtonRow, bool *pfAtLeastOneGroupCollapsed, bool *pfAtLeastOneGroupExpanded )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( NULL == pfAtLeastOneGroupCollapsed || NULL == pfAtLeastOneGroupExpanded )
	{
		return;
	}

	for( long lLoopCol = lFromColumn; lLoopCol <= lToColumn; lLoopCol++ )
	{
		// If it's not a collapse/expand button...
		if( false == IsExpandCollapseColumnButton( lLoopCol, lButtonRow ) )
		{
			continue;
		}

		LONG_PTR lUserData = 0;
		GetColUserData( lLoopCol, &lUserData );

		// If button is in collapse mode that means that column is expanded !!!
		if( _ECCBSTATE_COLLAPSE == _GETEXPANDCOLLAPSEVALUE( lUserData ) )
		{
			// That means that current group is expanded.
			*pfAtLeastOneGroupExpanded = true;
		}
		else
		{
			// That means that current group is collapsed.
			*pfAtLeastOneGroupCollapsed = true;
		}
	}
}
// End of methods to manage Expand/Collapse columns button feature.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage cell flags.
void CSSheet::SetFlagCanBeCollapsed( long lFromColumn, long lToColumn, bool fSet )
{
	if( false == m_bInitialized )
		return;
	_SetColumnFlag( lFromColumn, lToColumn, _FLAG_COLUMN_CANBECOLLAPSED, fSet );
}

void CSSheet::SetFlagAlwaysHidden( long lFromColumn, long lToColumn, bool fSet )
{
	if( false == m_bInitialized )
		return;
	_SetColumnFlag( lFromColumn, lToColumn, _FLAG_COLUMN_ALWAYSHIDDEN, fSet );
}

void CSSheet::SetFlagShowEvenEmpty( long lFromColumn, long lToColumn, bool fSet )
{
	if( false == m_bInitialized )
		return;
	_SetColumnFlag( lFromColumn, lToColumn, _FLAG_COLUMN_SHOWEVENEMPTY, fSet );
}

void CSSheet::SetFlagRowNoBreakable( long lFromRow, long lToRow, bool fSet )
{
	if( false == m_bInitialized )
	{
		return;
	}

	// Sanity check.
	if( lFromRow <= 0 || lToRow <= 0 )
	{
		return;
	}

	if( lFromRow > lToRow )
	{
		lFromRow = lToRow;
	}

	// First line of a group non breakable is always set as breakable.
	// Because we flag each row, if two successive groups have both all their lines set to no breakable,
	// there is no row between the two group to apply a page break. This is why we need here to set the
	// first line to breakable.
	if( true == fSet )
	{
		_SetRowFlag( lFromRow, lFromRow, _FLAG_ROW_NOBREAKABLE, false );
		lFromRow++;
	}
	
	if( lFromRow <= lToRow )
	{
		_SetRowFlag( lFromRow, lToRow, _FLAG_ROW_NOBREAKABLE, fSet );
	}
}

bool CSSheet::IsFlagCanBeCollapsed( long lColumn )
{
	if( false == m_bInitialized )
		return false;

	// Remark: 'GetColUserData' returns 'FALSE' if user data is not yet been set.
	LONG_PTR lUserData = 0;
	GetColUserData( lColumn, &lUserData );
	return _IS_FLAG_CANBECOLLAPSED( lUserData );
}

bool CSSheet::IsFlagAlwaysHidden( long lColumn )
{
	if( false == m_bInitialized )
		return false;

	// Remark: 'GetColUserData' returns 'FALSE' if user data is not yet been set.
	LONG_PTR lUserData = 0;
	GetColUserData( lColumn, &lUserData );
	return _IS_FLAG_ALWAYSHIDDEN( lUserData );
}

bool CSSheet::IsFlagShowEvenEmpty( long lColumn )
{
	if( false == m_bInitialized )
		return false;

	// Remark: 'GetColUserData' returns 'FALSE' if user data is not yet been set.
	LONG_PTR lUserData = 0;
	GetColUserData( lColumn, &lUserData );
	return _IS_FLAG_SHOWEVENEMPTY( lUserData );
}

bool CSSheet::IsFlagRowNoBreakable( long lRow )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	// Remark: 'GetRowUserData' returns 'FALSE' if user data is not yet been set.
	LONG_PTR lUserData = 0;
	GetRowUserData( lRow, &lUserData );
	return _IS_FLAG_ROWNOBREAKABLE( lUserData );
}

void CSSheet::CheckLastColumnFlagCantBePainted( long lRowButton )
{
	if( false == m_bInitialized )
	{
		return;
	}

	long lLoopCol = GetMaxCols();

	while( lLoopCol )
	{
		if( FALSE == IsColHidden( lLoopCol ) )
		{
			if( true == IsExpandCollapseColumnButton( lLoopCol, lRowButton ) )
			{
				SetCellProperty( lLoopCol, lRowButton, lLoopCol, GetMaxRows(), CellCantSelect, true );
			}

			break;
		}
		
		lLoopCol--;
	}
}

void CSSheet::SetColumnAlwaysHidden( long lFromColumn, long lToColumn, bool fHidden )
{
	if( false == m_bInitialized )
	{
		return;
	}

	ShowColRange( lFromColumn, lToColumn, ( true == fHidden ) ? FALSE : TRUE );
	SetFlagAlwaysHidden( lFromColumn, lToColumn, fHidden );
}
// End of methods that manage cell flags.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage cell property.
void CSSheet::SetCellProperty( long lColumn, long lRow, long lCellProperties, bool fSet, bool fResetBefore )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( lColumn < 1 || lRow < 1 || lColumn > GetMaxCols() || lRow > GetMaxRows() )	
	{
		return;
	}

	TCHAR tcCellTag[7] = _T("\1\1\1\1\1\1");
	GetCellTag( lColumn, lRow, tcCellTag );

	long lPrevCellProperties = 0;

	if( false == fResetBefore )
	{
		// Extract previous property.
		lPrevCellProperties = ( (unsigned short)tcCellTag[0] - 1 ) << 30;
		lPrevCellProperties += ( (unsigned short)tcCellTag[1] - 1 ) << 15;
		lPrevCellProperties += ( (unsigned short)tcCellTag[2] - 1 );
	}

	// Change property.
	if( true == fSet )
	{
		lPrevCellProperties |= lCellProperties;
	}
	else
	{
		lPrevCellProperties &= ~lCellProperties;
	}

	// Copy new property in cell tag.
	tcCellTag[0] = (TCHAR)( ( ( lPrevCellProperties & 0xC0000000) >> 30 ) + 1 );
	tcCellTag[1] = (TCHAR)( ( ( lPrevCellProperties & 0x3FFF8000) >> 15 ) + 1 );
	tcCellTag[2] = (TCHAR)( ( lPrevCellProperties & 0x00007FFF) + 1 );

	SetCellTag( lColumn, lRow, tcCellTag );
}

void CSSheet::SetCellProperty( long lFromColumn, long lFromRow, long lToColumn, long lToRow, long lCellProperties, bool fSet, bool fResetBefore )
{
	if( false == m_bInitialized )
	{
		return;
	}

	for( long lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
	{
		for( long lLoopCol = lFromColumn; lLoopCol <= lToColumn; lLoopCol++ )
		{
			SetCellProperty( lLoopCol, lLoopRow, lCellProperties, fSet, fResetBefore );
		}
	}
}

bool CSSheet::IsCellProperty( long lColumn, long lRow, long lCellProperties )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	if( lColumn < 1 || lRow < 1 || lColumn > GetMaxCols() || lRow > GetMaxRows() )	
	{
		return false;
	}
	
	// Remark: Cell tag contains 8 bytes for property and 8 bytes for parameter.

	TCHAR tcCellTag[7];
	short nLen = GetCellTag( lColumn, lRow, tcCellTag );
	
	if( nLen != 6 )
	{
		return false;
	}

	// Extract previous property.
	long lCurrentCellProperties = ( (unsigned short)tcCellTag[0] - 1 ) << 30;
	lCurrentCellProperties += ( (unsigned short)tcCellTag[1] - 1 ) << 15;
	lCurrentCellProperties += ( (unsigned short)tcCellTag[2] - 1 );

	return ( lCellProperties == ( lCurrentCellProperties & lCellProperties ) );
}
// End of methods that manage cell property.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage cell parameter.
void CSSheet::SetCellParam( long lColumn, long lRow, long lParam )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( lColumn < 1 || lRow < 1 || lColumn > GetMaxCols() || lRow > GetMaxRows() )	
	{
		return;
	}

	// Retrieve cell tag.
	TCHAR tcCellTag[7];
	short nlen = GetCellTag( lColumn, lRow, tcCellTag );
	
	// String not initialized, force a value to avoid fake property.
	if( nlen != 6 )
	{
		tcCellTag[0] = '\1';
		tcCellTag[1] = '\1';
		tcCellTag[2] = '\1';
	}
	
	// Copy param in the cell tag.
	tcCellTag[3] = (TCHAR)( ( ( lParam & 0xC0000000) >> 30 ) + 1 );
	tcCellTag[4] = (TCHAR)( ( ( lParam & 0x3FFF8000) >> 15 ) + 1 );
	tcCellTag[5] = (TCHAR)( ( lParam & 0x00007FFF) + 1 );
	tcCellTag[6] = _T('\0');

	SetCellTag( lColumn, lRow, tcCellTag );
}

void CSSheet::SetCellParam( long lFromColumn, long lFromRow, long lToColumn, long lToRow, long lParam )
{
	if( false == m_bInitialized )
	{
		return;
	}

	for( long lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
	{
		for( long lLoopCol = lFromColumn; lLoopCol <= lToColumn; lLoopCol++ )
		{
			SetCellParam( lLoopCol, lLoopRow, lParam );
		}
	}
}

long CSSheet::GetCellParam( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return 0;
	}

	if( lColumn < 1 || lRow < 1 || lColumn > GetMaxCols() || lRow > GetMaxRows() )	
	{
		return 0;
	}
	
	// Remark: Cell tag contains 8 bytes for property and 8 bytes for parameter.

	// Retrieve cell tag
	TCHAR tcCellTag[7];
	short nLen = GetCellTag( lColumn, lRow, tcCellTag );
	
	// If nothing...
	if( nLen != 6 )
	{
		return 0;
	}
	
	// Extract param.
	long lCurrentParam = ( (unsigned short)tcCellTag[3] - 1 ) << 30;
	lCurrentParam += ( (unsigned short)tcCellTag[4] - 1 ) << 15;
	lCurrentParam += ( (unsigned short)tcCellTag[5] - 1 );

	return lCurrentParam;
}
// End of methods that manage cell parameter.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage cell style.
bool CSSheet::SetTextPattern( _SSTextPattern eTextPattern, bool fLight )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	// Check if pattern is already in map
	bool bExist = false;

	if( m_vecTextPatternList.size() > 0 )
	{
		for( vecTextPatternListIter iter = m_vecTextPatternList.begin(); iter != m_vecTextPatternList.end() && false == bExist ; ++iter )
		{
			// If same text pattern and this one has not been changed by calling SetTextPatternProperty...
			if( eTextPattern == (*iter)->m_staticTextPattern && ( false == (*iter)->m_fChanged ) )
			{
				// If current structure exist but has not yet been saved in m_vecTextPatternList (m_pclFont == NULL )
				if( NULL != m_pclCurrentTextPatternStructure && NULL == m_pclCurrentTextPatternStructure->m_pclFont )
				{
					delete m_pclCurrentTextPatternStructure;
				}

				m_pclCurrentTextPatternStructure = *iter;
				bExist = true;
				break;
			}
		}
	}

	// If text pattern doesn't exist yet...
	if( false == bExist )
	{
		CTextPatternStructure *pclTextPattern = new CTextPatternStructure( eTextPattern, fLight );

		if( NULL == pclTextPattern )
		{
			return false;
		}

		// If current structure exist but has not yet been saved in m_vecTextPatternList (m_pclFont == NULL )
		if( NULL != m_pclCurrentTextPatternStructure && NULL == m_pclCurrentTextPatternStructure->m_pclFont )
		{
			delete m_pclCurrentTextPatternStructure;
		}

		m_pclCurrentTextPatternStructure = pclTextPattern;
	}

	return true;
}

bool CSSheet::SetTextPatternProperty( _SSTextPatternProperty ePropertyToChange, LPARAM lpValue )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	if( NULL == m_pclCurrentTextPatternStructure )
	{
		return false;
	}
	
	// If font is already created we must create a new text pattern structure
	if( NULL != m_pclCurrentTextPatternStructure->m_pclFont )
	{
		// At this stage, if font has been created, text pattern structure is saved in 'm_vecTextPatternList'.
		// We can thus create a new one.
		m_pclCurrentTextPatternStructure = new CTextPatternStructure( m_pclCurrentTextPatternStructure );

		if( NULL == m_pclCurrentTextPatternStructure )
		{
			return false;
		}
	}
	
	m_pclCurrentTextPatternStructure->SetProperty( ePropertyToChange, lpValue );
	return true;
}

long CSSheet::GetTextPatternProperty( _SSTextPatternProperty eProperty )
{
	if( false == m_bInitialized )
	{
		return -1;
	}

	long lProperty = -1;

	if( NULL != m_pclCurrentTextPatternStructure )
	{
		lProperty = m_pclCurrentTextPatternStructure->GetProperty( eProperty );
	}

	return lProperty;
}

bool CSSheet::ApplyTextPattern( long lFromColumn, long lFromRow, long lToColumn, long lToRow, bool fChangeAlsoStyle )
{
	if( false == m_bInitialized )
	{
		return false;
	}
	
	if( NULL == m_pclCurrentTextPatternStructure )
	{
		return false;
	}

	// Check first if 'm_pclCurrentTextPatternStructure' is well created.
	if( NULL == m_pclCurrentTextPatternStructure->m_pclFont )
	{
		bool bExist = false;

		for( vecTextPatternListIter iter = m_vecTextPatternList.begin(); iter != m_vecTextPatternList.end() && false == bExist ; ++iter )
		{
			// If pattern found...
			if( m_pclCurrentTextPatternStructure->m_staticTextPattern == (*iter)->m_staticTextPattern )
			{
				// Must check if properties are all the same.
				if( m_pclCurrentTextPatternStructure->m_fFontBold != (*iter)->m_fFontBold )
				{
					continue;
				}

				if( m_pclCurrentTextPatternStructure->m_fFontItalic != (*iter)->m_fFontItalic )
				{
					continue;
				}

				if( m_pclCurrentTextPatternStructure->m_fFontUnderline != (*iter)->m_fFontUnderline )
				{
					continue;
				}

				if( m_pclCurrentTextPatternStructure->m_iFontSize != (*iter)->m_iFontSize )
				{
					continue;
				}
				
				if( m_pclCurrentTextPatternStructure->m_TextColor != (*iter)->m_TextColor )
				{
					continue;
				}

				if( m_pclCurrentTextPatternStructure->m_BackColor != (*iter)->m_BackColor )
				{
					continue;
				}

				if( m_pclCurrentTextPatternStructure->m_lTextStyle != (*iter)->m_lTextStyle )
				{
					continue;
				}

				delete m_pclCurrentTextPatternStructure;
				m_pclCurrentTextPatternStructure = *iter;
				bExist = true;
				break;
			}
		}

		// If not exist, we must create one.
		if( false == bExist )
		{
			m_pclCurrentTextPatternStructure->m_pclFont = new CFont();

			if( NULL == m_pclCurrentTextPatternStructure->m_pclFont )
			{
				return false;
			}

			CDC *pDC = CWnd::GetDC();
			int lfHeight = -MulDiv( m_pclCurrentTextPatternStructure->m_iFontSize, pDC->GetDeviceCaps( LOGPIXELSY ), 72 );

			// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
			CWnd::ReleaseDC( pDC );

			m_pclCurrentTextPatternStructure->m_pclFont->CreateFont(lfHeight,																				// nHeight
																	0,																						// nWidth
																	0,																						// nEscapement
																	0,																						// nOrientation
																	( FALSE == m_pclCurrentTextPatternStructure->m_fFontBold ) ? FW_NORMAL : FW_BOLD,		// nWeight
																	m_pclCurrentTextPatternStructure->m_fFontItalic,										// bItalic
																	m_pclCurrentTextPatternStructure->m_fFontUnderline,										// bUnderline
																	FALSE,																					// cStrikeOut
																	DEFAULT_CHARSET,																		// tbCharSet
																	OUT_DEFAULT_PRECIS,																		// tbOutputPrecision
																	CLIP_DEFAULT_PRECIS,																	// tbClipPrecision
																	DEFAULT_QUALITY,																		// tbQuality
																	DEFAULT_PITCH | FF_DONTCARE,															// tbPitchAndFamily
																	(LPCTSTR)m_pclCurrentTextPatternStructure->m_strFontName );								// lpszFacename 

			// Insert new text pattern in vector list.
			m_vecTextPatternList.push_back( m_pclCurrentTextPatternStructure );
		}
	}

	// Apply font.
	if( FALSE == SetFontRange( lFromColumn, lFromRow, lToColumn, lToRow, (HFONT)m_pclCurrentTextPatternStructure->m_pclFont->GetSafeHandle(), FALSE ) )
	{
		return false;
	}

	// Apply cell type (style).
	if( true == fChangeAlsoStyle )
	{
		SS_CELLTYPE rCellType;

		if( FALSE == GetCellType( lFromColumn, lFromRow, &rCellType ) )
		{
			return false;
		}
		
		// Transform SSS_ALIGN_XXX to SSS_TEXT_XXX style (because spread doesn't support SSS_ALIGN_XXX style with static text cell type).
		long lStyle = m_pclCurrentTextPatternStructure->m_lTextStyle;

		if( SS_TYPE_STATICTEXT == rCellType.Type )
		{
			_ConvertSSStoSSStyle( lStyle );
		}

		rCellType.Style = lStyle;
		
		if( FALSE == SetCellTypeRange( lFromColumn, lFromRow, lToColumn, lToRow, &rCellType ) )
		{
			return false;
		}
	}

	// Apply color.
	if( FALSE == SetColorRange( lFromColumn, lFromRow, lToColumn, lToRow, m_pclCurrentTextPatternStructure->m_BackColor, m_pclCurrentTextPatternStructure->m_TextColor ) )
	{
		return false;
	}

	return true;
}

bool CSSheet::SaveTextPattern( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	HFONT hFont = GetFont( lColumn, lRow );

	if( INVALID_HANDLE_VALUE == hFont )
	{
		return false;
	}

	// Search in text pattern list if this font exist
	bool bExist = false;
	m_pclSavedTextPatternStructure = NULL;

	for( vecTextPatternListIter iter = m_vecTextPatternList.begin(); iter != m_vecTextPatternList.end(); ++iter )
	{
		if( hFont == (*iter)->m_pclFont->GetSafeHandle() )
		{
			m_pclSavedTextPatternStructure = *iter;
			break;
		}
	}

	return true;
}

bool CSSheet::RestoreTextPattern( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	if( NULL == m_pclSavedTextPatternStructure )
	{
		return false;
	}
	
	// Apply font.
	if( FALSE == SetFont( lColumn, lRow, (HFONT)m_pclSavedTextPatternStructure->m_pclFont->GetSafeHandle() , FALSE ) )
	{
		return false;
	}

	// Transform SSS_ALIGN_XXX to SSS_TEXT_XXX style (because spread doesn't support SSS_ALIGN_XXX style with static text cell type).
	long lStyle = m_pclCurrentTextPatternStructure->m_lTextStyle;
	_ConvertSSStoSSStyle( lStyle );

	// Apply style.
	SS_CELLTYPE rCellType;

	if( FALSE == GetCellType( lColumn, lRow, &rCellType ) )
	{
		return false;
	}
	
	rCellType.Style = lStyle;

	if( FALSE == SetCellType( lColumn, lRow, &rCellType ) )
	{
		return false;
	}

	// Apply color.
	if( FALSE == SetColor( lColumn, lRow, m_pclSavedTextPatternStructure->m_BackColor, m_pclSavedTextPatternStructure->m_TextColor ) )
	{
		return false;
	}

	return true;
}

void CSSheet::SetBackColor( long lFromColumn, long lFromRow, long lToColumn, long lToRow, COLORREF color, bool fForceColorWhenSelection )
{
	if( false == m_bInitialized )
	{
		return;
	}

	SetColorRange( lFromColumn, lFromRow, lToColumn, lToRow, color, GetForeColor( lFromColumn, lFromRow ) );
	SetCellProperty( lFromColumn, lFromRow, lToColumn, lToRow, _SSCellProperty::CellCantSelect, fForceColorWhenSelection );
}

COLORREF CSSheet::GetBackColor( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return COLORREF(-1);
	}

	COLORREF colorBack, colorFore;
	GetColor( lColumn, lRow, &colorBack, &colorFore );
	return colorBack;
}

void CSSheet::SetForeColor( long lFromColumn, long lFromRow, long lToColumn, long lToRow, COLORREF color, bool fForceColorWhenSelection )
{
	if( false == m_bInitialized )
	{
		return;
	}
	
	SetColorRange( lFromColumn, lFromRow, lToColumn, lToRow, GetBackColor( lFromColumn, lFromRow ), color );

	if( true == fForceColorWhenSelection )
	{
		SetCellProperty( lFromColumn, lFromRow, lToColumn, lToRow, _SSCellProperty::CellNoSelection, true );
	}
}

COLORREF CSSheet::GetForeColor( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return COLORREF(-1);
	}

	COLORREF colorBack, colorFore;
	GetColor( lColumn, lRow, &colorBack, &colorFore );
	return colorFore;
}

void CSSheet::SetFontBold( long lColumn, long lRow, BOOL bBold )
{
	if( false == m_bInitialized )
	{
		return;
	}

	SetFontBold( lColumn, lRow, lColumn, lRow, bBold );
}

void CSSheet::SetFontBold( long lFromColumn, long lFromRow, long lToColumn, long lToRow, BOOL bBold )
{
	if( false == m_bInitialized )
	{
		return;
	}

	HFONT hCurrentFont = GetFont( lFromColumn, lFromRow );
	
	if( INVALID_HANDLE_VALUE == hCurrentFont )
	{
		return;
	}

	CFont *pclCurrentFont = CFont::FromHandle( hCurrentFont );

	if( NULL == pclCurrentFont )
	{
		return;
	}

	LOGFONT rFontParameters;

	if( 0 == pclCurrentFont->GetLogFont( &rFontParameters ) )
	{
		return;
	}

	if( ( TRUE == bBold && FW_BOLD == rFontParameters.lfWeight )
			|| ( FALSE == bBold && FW_NORMAL == rFontParameters.lfWeight ) )
	{
		return;
	}

	CFont *pclTemporaryFont = new CFont();

	if( NULL == pclTemporaryFont )
	{
		return;
	}

	rFontParameters.lfWeight = ( TRUE == bBold ) ? FW_BOLD : FW_NORMAL;
	pclTemporaryFont->CreateFontIndirect( &rFontParameters );
	SetFontRange( lFromColumn, lFromRow, lToColumn, lToRow, (HFONT)pclTemporaryFont->GetSafeHandle(), TRUE );
	delete pclTemporaryFont;
}

void CSSheet::SetFontSize( long lColumn, long lRow, int iSize )
{
	if( false == m_bInitialized )
	{
		return;
	}

	SetFontSize( lColumn, lRow, lColumn, lRow, iSize );
}

void CSSheet::SetFontSize( long lFromColumn, long lFromRow, long lToColumn, long lToRow, int iSize )
{
	if( false == m_bInitialized )
	{
		return;
	}

	HFONT hCurrentFont = GetFont( lFromColumn, lFromRow );
	
	if( INVALID_HANDLE_VALUE == hCurrentFont )
	{
		return;
	}

	CFont *pclCurrentFont = CFont::FromHandle( hCurrentFont );

	if( NULL == pclCurrentFont )
	{
		return;
	}

	LOGFONT rFontParameters;

	if( 0 == pclCurrentFont->GetLogFont( &rFontParameters ) )
	{
		return;
	}

	CDC *pDC = CWnd::GetDC();
	int iCurrentSize = -MulDiv( rFontParameters.lfHeight, 72, pDC->GetDeviceCaps( LOGPIXELSY ) );

	if( iSize == iCurrentSize )
	{
		return;
	}

	CFont *pclTemporaryFont = new CFont();

	if( NULL == pclTemporaryFont )
	{
		return;
	}

	rFontParameters.lfHeight = -MulDiv( iSize, pDC->GetDeviceCaps( LOGPIXELSY ), 72 );
	pclTemporaryFont->CreateFontIndirect( &rFontParameters );
	SetFontRange( lFromColumn, lFromRow, lToColumn, lToRow, (HFONT)pclTemporaryFont->GetSafeHandle(), TRUE );
	delete pclTemporaryFont;
}

bool CSSheet::GetFontBold( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	HFONT hFont = GetFont( lColumn, lRow );

	if( INVALID_HANDLE_VALUE == hFont )
	{
		return false;
	}

	CFont *pclFont = CFont::FromHandle( hFont );

	if( NULL == pclFont )
	{
		return false;
	}

	LOGFONT rLogFont;
	pclFont->GetLogFont( &rLogFont );

	return ( ( FW_NORMAL == rLogFont.lfWeight ) ? false : true );
}

double CSSheet::GetFontSize( long lColumn, long lRow )
{
	if( false == m_bInitialized )
		return -1.0;

	HFONT hFont = GetFont( lColumn, lRow );
	if( INVALID_HANDLE_VALUE == hFont )
		return -1.0;

	CFont *pclFont = CFont::FromHandle( hFont );
	if( NULL == pclFont )
		return -1.0;

	LOGFONT rLogFont;
	pclFont->GetLogFont( &rLogFont );

	CDC* pDC = CWnd::GetDC();
	double dFontSize = abs( ( (double)rLogFont.lfHeight * 72.0 / (double)( pDC->GetDeviceCaps( LOGPIXELSY ) ) ) );

	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	CWnd::ReleaseDC( pDC );

	return dFontSize;
}

void CSSheet::Underline( long lFromColumn, long lFromRow, long lToColumn, long lToRow, BOOL fUnderline )
{
	if( false == m_bInitialized )
		return;

	HFONT hCurrentFont = GetFont( lFromColumn, lFromRow );
	if( INVALID_HANDLE_VALUE != hCurrentFont )
	{
		CFont *pclCurrentFont = CFont::FromHandle( hCurrentFont );
		if( NULL != pclCurrentFont )
		{
			LOGFONT rFontParameters;
			if( 0 != pclCurrentFont->GetLogFont( &rFontParameters ) )
			{
				if( fUnderline != rFontParameters.lfUnderline )
				{
					CFont *pclTemporaryFont = new CFont();
					if( NULL != pclTemporaryFont )
					{
						rFontParameters.lfUnderline = fUnderline;
						pclTemporaryFont->CreateFontIndirect( &rFontParameters );
						SetFontRange( lFromColumn, lFromRow, lToColumn, lToRow, (HFONT)pclTemporaryFont->GetSafeHandle(), TRUE );
						delete pclTemporaryFont;
					}
				}
			}
		}
	}
}

// Show or hide Cells border
void CSSheet::SetCellBorder( long lFromColumn, long lFromRow, long lToColumn, long lToRow, bool fShow, WORD wBorderType, WORD wBorderStyle, COLORREF color )
{
	if( false == m_bInitialized )
		return;

	if( true == fShow )
		SetBorderRange( lFromColumn, lFromRow, lToColumn, lToRow, wBorderType, wBorderStyle, color );
	else
		SetBorderRange( lFromColumn, lFromRow, lToColumn, lToRow, wBorderType, SS_BORDERSTYLE_NONE, SPREAD_COLOR_NONE );
}

bool CSSheet::GetCellBorder( long lColumn, long lRow, WORD wBorderType, WORD *pwBorderStyle, COLORREF *pColor )
{
	if( false == m_bInitialized )
		return false;

	WORD wStyleLeft, wStyleTop, wStyleRight, wStyleBottom;
	COLORREF clrLeft, clrTop, clrRight, clrBottom;

	wStyleLeft = 0;
	wStyleTop = 0;
	wStyleRight = 0;
	wStyleBottom = 0;
	clrLeft = 0;
	clrTop = 0;
	clrRight = 0;
	clrBottom = 0;
	if( FALSE == GetBorderEx( lColumn, lRow, &wStyleLeft, &clrLeft, &wStyleTop, &clrTop, &wStyleRight, &clrRight, &wStyleBottom, &clrBottom ) )
	{
		*pwBorderStyle = SS_BORDERTYPE_NONE;
		*pColor = SPREAD_COLOR_NONE;
		return false;
	}

	if( SS_BORDERTYPE_LEFT == ( wBorderType & SS_BORDERTYPE_LEFT ) )
	{
		*pwBorderStyle = wStyleLeft;
		*pColor = clrLeft;
	}
	else if( SS_BORDERTYPE_TOP == ( wBorderType & SS_BORDERTYPE_TOP ) )
	{
		*pwBorderStyle = wStyleTop;
		*pColor = clrTop;
	}
	else if( SS_BORDERTYPE_RIGHT == ( wBorderType & SS_BORDERTYPE_RIGHT ) )
	{
		*pwBorderStyle = wStyleRight;
		*pColor = clrRight;
	}
	else if( SS_BORDERTYPE_BOTTOM == ( wBorderType & SS_BORDERTYPE_BOTTOM ) )
	{
		*pwBorderStyle = wStyleBottom;
		*pColor = clrBottom;
	}
	else 
		return false;

	return true;
}

bool CSSheet::GetCellBorder( long lColumn, long lRow, WORD *pwBorderType, WORD *pwBorderStyle, COLORREF *pColor )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	WORD wStyleLeft, wStyleTop, wStyleRight, wStyleBottom;
	COLORREF clrLeft, clrTop, clrRight, clrBottom;

	wStyleLeft = 0;
	wStyleTop = 0;
	wStyleRight = 0;
	wStyleBottom = 0;
	clrLeft = 0;
	clrTop = 0;
	clrRight = 0;
	clrBottom = 0;

	if( FALSE == GetBorderEx( lColumn, lRow, &wStyleLeft, &clrLeft, &wStyleTop, &clrTop, &wStyleRight, &clrRight, &wStyleBottom, &clrBottom ) )
	{
		*pwBorderType = SS_BORDERTYPE_NONE;
		*pwBorderStyle = SS_BORDERTYPE_NONE;
		*pColor = SPREAD_COLOR_NONE;
		return false;
	}

	*pwBorderType = ( 0 != wStyleLeft ) ? SS_BORDERTYPE_LEFT : 0;
	*pwBorderType |= ( 0 != wStyleTop ) ? SS_BORDERTYPE_TOP : 0;
	*pwBorderType |= ( 0 != wStyleRight ) ? SS_BORDERTYPE_RIGHT : 0;
	*pwBorderType |= ( 0 != wStyleBottom ) ? SS_BORDERTYPE_BOTTOM : 0;
	
	if( 0 != wStyleLeft )
	{
		*pwBorderStyle = wStyleLeft;
		*pColor = clrLeft;
	}
	else if( 0 != wStyleTop )
	{
		*pwBorderStyle = wStyleTop;
		*pColor = clrTop;
	}
	else if( 0 != wStyleRight )
	{
		*pwBorderStyle = wStyleRight;
		*pColor = clrRight;
	}
	else if( 0 != wStyleBottom )
	{
		*pwBorderStyle = wStyleBottom;
		*pColor = clrBottom;
	}
	else
	{
		*pwBorderStyle = SS_BORDERTYPE_NONE;
		return false;
	}

	return true;
}

COLORREF CSSheet::GetColHeaderBackColor( bool fEven, bool fModification )
{
	if( false == m_bInitialized )
		return (COLORREF)-1;

	COLORREF colHeaderBackColor;
	if( true == fModification )
	{
		colHeaderBackColor = ( true == fEven ) ? _TAH_TITLE_SECOND_REEDIT : _TAH_TITLE_MAIN_REEDIT;
	}
	else
	{
		colHeaderBackColor = ( true == fEven ) ? _TAH_TITLE_SECOND : _TAH_TITLE_MAIN;
	}
	return colHeaderBackColor;
}

COLORREF CSSheet::GetColHeaderBackColorForHub( bool fHub, bool fEven, bool fModification )
{
	if( false == m_bInitialized )
		return (COLORREF)-1;

	COLORREF colHeaderBackColorForHub;
	if( true == fHub )		// Orange based
	{
		if( true == fModification)
		{
			colHeaderBackColorForHub = ( true == fEven ) ? _TAH_RED_LIGHT : _TAH_RED;
		}
		else
		{
			colHeaderBackColorForHub = ( true == fEven ) ? _TAH_ORANGE_MED : _TAH_ORANGE;
		}
	
	}
	else					// Beryllium based 
	{	
		if( true == fModification )
		{
			colHeaderBackColorForHub = ( true == fEven ) ? _TAH_RED_XLIGHT : _TAH_RED_MED;
		}
		else
		{
			colHeaderBackColorForHub = ( true == fEven ) ? _IMI_GRAY_XLIGHT : _IMI_GRAY_MED;
		}
	}

	return colHeaderBackColorForHub;
}

bool CSSheet::IsCellIsStaticText( long lColumn, long lRow, SS_CELLTYPE& rCellType )
{
	if( FALSE == GetCellType( lColumn, lRow, &rCellType ) || SS_TYPE_STATICTEXT != rCellType.Type )
	{
		return false;
	}

	return true;
}

// End of methods that manage cell style.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage cell formating value.
bool CSSheet::FormatStaticText( long lFromColumn, long lFromRow, long lToColumn, long lToRow, CString strText, long lStyle, bool fKeepValue, bool fKeepTextPattern, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	// Transform SSS_ALIGN_XXX to SS_TEXT_XXX style (because spread doesn't support SSS_ALIGN_XXX style with static text cell type).
	_ConvertSSStoSSStyle( lStyle );

	// Set cell type.
	SS_CELLTYPE rCellType;
	if( false == fKeepTextPattern )
	{
		if( NULL == SetTypeStaticText( &rCellType, lStyle ) )
			return false;
	}
	else
	{
		GetCellType( lFromColumn, lFromRow, &rCellType );
		if( SS_TYPE_EDIT == rCellType.Type )
		{
			_ConvertSSStoSSStyle( rCellType.Style );
			rCellType.Style &= ~SS_TEXT_SHADOWIN;
		}
		if( NULL == SetTypeStaticText( &rCellType, rCellType.Style ) )
			return false;
	}

	if( FALSE == SetCellTypeRange( lFromColumn, lFromRow, lToColumn, lToRow, &rCellType ) )
		return false;
	
	if( false == fKeepTextPattern )
	{
		// User can change text pattern properties and directly call 'FormatStaticText' with these new properties.
		// Remark: 'false' to not change cell type (and style).
		if( false == ApplyTextPattern( lFromColumn, lFromRow, lToColumn, lToRow, false ) )
			return false;
	}

	if( false == fKeepValue )
	{
		if( FALSE == SetDataRange( lFromColumn, lFromRow, lToColumn, lToRow, (LPCTSTR)strText ) )
			return false;
	}

	SetCellProperty( lFromColumn, lFromRow, lToColumn, lToRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );

	return true;
}

bool CSSheet::FormatEditText( long lColumn, long lRow, CString strText, long lStyle, short nMaxEditLen, bool fKeepValue, bool fKeepTextPattern, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;
	
	// Set cell type.
	SS_CELLTYPE rCellType;
	if( false == fKeepTextPattern )
	{
		if( NULL == SetTypeEdit( &rCellType, lStyle, nMaxEditLen, SS_CHRSET_CHR, SS_CASE_NOCASE ) )
			return false;
	}
	else
	{
		GetCellType( lColumn, lRow, &rCellType );
		if( SS_TYPE_STATICTEXT == rCellType.Type )
			_ConvertSStoSSSStyle( rCellType.Style );
		if( NULL == SetTypeEdit( &rCellType, rCellType.Style, nMaxEditLen, SS_CHRSET_CHR, SS_CASE_NOCASE ) )
			return false;
	}
	SetCellType( lColumn, lRow, &rCellType );
	
	if( false == fKeepTextPattern )
	{
		// User can change text pattern properties and directly call 'FormatEditText' with these new properties.
		// Remark: 'false' to not change cell type (and style).
		ApplyTextPattern( lColumn, lRow, lColumn, lRow, false );
	}

	// Set cell properties.
	SetCellProperty( lColumn, lRow, _SSCellProperty::CellText, true );
	SetCellProperty( lColumn, lRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );

	if( false == fKeepValue )
		SetData( lColumn, lRow, (LPCTSTR)strText );

	return true;
}

bool CSSheet::FormatEditTextRowRange( long lColumn, long lFromRow, long lToRow, bool fForceRowAllSame, CString strText, long lStyle, short nMaxEditLen, bool fKeepValue, bool fKeepTextPattern, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	if( lFromRow > lToRow )
	{
		long lTemp = lFromRow;
		lFromRow = lToRow;
		lToRow = lTemp;
	}

	// Set cell type.
	SS_CELLTYPE rCellType;
	if( true == fForceRowAllSame || false == fKeepTextPattern )
	{
		if( NULL == SetTypeEdit( &rCellType, lStyle, nMaxEditLen, SS_CHRSET_CHR, SS_CASE_NOCASE ) )
			return false;
		if( FALSE == SetCellTypeRange( lColumn, lFromRow, lColumn, lToRow, &rCellType ) )
			return false;

		// User can change text pattern properties and directly call 'FormatEditText' with these new properties.
		// Remark: 'false' to not change cell type (and style).
		ApplyTextPattern( lColumn, lFromRow, lColumn, lToRow, false );
	}
	else
	{
		for( long lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
		{
			if( FALSE == GetCellType( lColumn, lLoopRow, &rCellType ) )
				return false;
			if( SS_TYPE_STATICTEXT == rCellType.Type )
				_ConvertSStoSSSStyle( rCellType.Style );
			if( NULL == SetTypeEdit( &rCellType, rCellType.Style, nMaxEditLen, SS_CHRSET_CHR, SS_CASE_NOCASE ) )
				return false;
			if( FALSE == SetCellType( lColumn, lLoopRow, &rCellType ) )
				return false;
		}
	}

	// Set cell properties.
	SetCellProperty( lColumn, lFromRow, lColumn, lToRow, _SSCellProperty::CellText, true );
	SetCellProperty( lColumn, lFromRow, lColumn, lToRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );

	if( false == fKeepValue )
	{
		SetDataRange( lColumn, lFromRow, lColumn, lToRow, strText );
	}

	return true;
}

bool CSSheet::FormatEditDouble( long lColumn, long lRow, CString strText, long lStyle, bool fKeepValue, bool fKeepTextPattern, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	// Set cell type
	SS_CELLTYPE rCellType;
	if( false == fKeepTextPattern )
	{
		if( NULL == SetTypeEdit( &rCellType, lStyle, 32000, SS_CHRSET_CHR, SS_CASE_NOCASE ) )
			return false;
	}
	else
	{
		GetCellType( lColumn, lRow, &rCellType );
		if( SS_TYPE_STATICTEXT == rCellType.Type )
			_ConvertSStoSSSStyle( rCellType.Style );
		if( NULL == SetTypeEdit( &rCellType, rCellType.Style, 32000, SS_CHRSET_CHR, SS_CASE_NOCASE ) )
			return false;
	}
	SetCellType( lColumn, lRow, &rCellType );
	
	if( false == fKeepTextPattern )
	{
		// User can change text pattern properties and directly call 'FormatEditDouble' with these new properties.
		// Remark: 'false' to not change cell type (and style)
		ApplyTextPattern( lColumn, lRow, lColumn, lRow, false );
	}

	// Set cell property.
	SetCellProperty( lColumn, lRow, _SSCellProperty::CellDouble, true );
	SetCellProperty( lColumn, lRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );

	if( false == fKeepValue )
		SetData( lColumn, lRow, (LPCTSTR)strText );

	return true;
}

bool CSSheet::FormatEditDoubleRowRange( long lColumn, long lFromRow, long lToRow, bool fForceRowAllSame, CString strText, long lStyle, bool fKeepValue, bool fKeepTextPattern, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	if( lFromRow > lToRow )
	{
		long lTemp = lFromRow;
		lFromRow = lToRow;
		lToRow = lTemp;
	}

	// Set cell type
	SS_CELLTYPE rCellType;
	if( true == fForceRowAllSame || false == fKeepTextPattern )
	{
		if( NULL == SetTypeEdit( &rCellType, lStyle, 32000, SS_CHRSET_CHR, SS_CASE_NOCASE ) )
			return false;
		if( FALSE == SetCellTypeRange( lColumn, lFromRow, lColumn, lToRow, &rCellType ) )
			return false;
		
		// User can change text pattern properties and directly call 'FormatEditDouble' with these new properties.
		// Remark: 'false' to not change cell type (and style)
		ApplyTextPattern( lColumn, lFromRow, lColumn, lToRow, false );
	}
	else
	{
		for( long lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
		{
			if( FALSE == GetCellType( lColumn, lLoopRow, &rCellType ) )
				return false;
			if( SS_TYPE_STATICTEXT == rCellType.Type )
				_ConvertSStoSSSStyle( rCellType.Style );
			if( NULL == SetTypeEdit( &rCellType, rCellType.Style, 32000, SS_CHRSET_CHR, SS_CASE_NOCASE ) )
				return false;
			if( FALSE == SetCellType( lColumn, lLoopRow, &rCellType ) )
				return false;
		}
	}

	// Set cell property.
	SetCellProperty( lColumn, lFromRow, lColumn, lToRow, _SSCellProperty::CellDouble, true );
	SetCellProperty( lColumn, lFromRow, lColumn, lToRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );

	if( false == fKeepValue )
	{
		SetDataRange( lColumn, lFromRow, lColumn, lToRow, strText );
	}

	return true;
}

bool CSSheet::FormatEditDouble( long lColumn, long lRow, int iPhysicalType, double dSI, long lStyle, bool fEmptyIfNull, bool fKeepValue, bool fKeepTextPattern, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	SetCellProperty( lColumn, lRow, _SSCellProperty::CellDouble, true );
	SetCellProperty( lColumn, lRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );
	return FormatCUDouble( lColumn, lRow, iPhysicalType, dSI, lStyle, false, fEmptyIfNull, fKeepValue, fKeepTextPattern );
}

bool CSSheet::FormatEditCombo( long lColumn, long lRow, CString strText, long lStyle, LPARAM lParam, bool fKeepValue, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	FormatComboList( lColumn, lRow, lStyle );

	if( false == fKeepValue )
	{
		ComboBoxSendMessage( lColumn, lRow, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( lColumn, lRow, SS_CBM_ADDSTRING, 0, (LPARAM)( (LPCTSTR)strText ) );
	}
	ComboBoxSendMessage( lColumn, lRow, SS_CBM_SETCURSEL, 0, 0 );

	SetCellParam( lColumn, lRow, lParam );
	SetCellProperty( lColumn, lRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );
	return true;
}

bool CSSheet::FormatEditCombo( long lColumn, long lRow, int iPhysicalType, double dSI, long lStyle, LPARAM lParam, bool fKeepValue, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	CString str = WriteCUDouble( iPhysicalType, dSI );
	FormatEditCombo( lColumn, lRow, str, lStyle, lParam, fKeepValue );
	SetCellProperty( lColumn, lRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );
	return true;
}

bool CSSheet::FormatComboList( long lColumn, long lRow, long lStyle, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	// Set cell type
	SS_CELLTYPE rCellType;
	if( NULL == SetTypeComboBoxEx( &rCellType, lStyle, _T(""), 0, 0, 0, NULL, 0 ) )
		return false;
	SetCellType( lColumn, lRow, &rCellType );

	// User can change text pattern properties and directly call 'FormatStaticText' with these new properties.
	// Remark: 'false' to not change cell type (and style)
	ApplyTextPattern( lColumn, lRow, lColumn, lRow, false );

	SetCellProperty( lColumn, lRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );
	return true;
}

bool CSSheet::FormatCUDouble( long lColumn, long lRow, int iPhysicalType, double dSI, long lStyle, bool fStatic, bool fEmptyIfNull, bool fKeepValue, bool fKeepTextPattern, bool fNoMouseEvent )
{
	if( false == m_bInitialized )
		return false;

	if( NULL == m_pclCurrentTextPatternStructure )
		return false;

	SS_CELLTYPE rCellType;
	if( true == fStatic )
	{
		// Transform SSS_ALIGN_XXX to SS_TEXT_XXX style (because spread doesn't support SSS_ALIGN_XXX style with static text cell type).
		_ConvertSSStoSSStyle( lStyle );

		if( false == fKeepTextPattern )
		{
			if( NULL == SetTypeStaticText( &rCellType, lStyle ) )
				return false;
		}
		else
		{
			GetCellType( lColumn, lRow, &rCellType );
			if( SS_TYPE_EDIT == rCellType.Type )
			{
				_ConvertSSStoSSStyle( rCellType.Style );
				rCellType.Style &= ~SS_TEXT_SHADOWIN;
			}
			if( NULL == SetTypeStaticText( &rCellType, rCellType.Style ) )
				return false;
		}
	}
	else
	{
		if( false == fKeepTextPattern )
		{
			if( NULL == SetTypeEdit( &rCellType, lStyle, 31, SS_CHRSET_CHR, SS_CASE_NOCASE  ) )
				return false;
		}
		else
		{
			GetCellType( lColumn, lRow, &rCellType );
			if( SS_TYPE_STATICTEXT == rCellType.Type )
				_ConvertSStoSSSStyle( rCellType.Style );
			if( NULL == SetTypeEdit( &rCellType, rCellType.Style, 31, SS_CHRSET_CHR, SS_CASE_NOCASE  ) )
				return false;
		}
	}
	if( FALSE == SetCellType( lColumn, lRow, &rCellType ) )
		return false;

	if( false == fKeepTextPattern )
	{
		// User can change text pattern properties and directly call 'FormatStaticText' with these new properties.
		// Remark: 'false' to not change cell type (and style)
		ApplyTextPattern( lColumn, lRow, lColumn, lRow, false );
	}

	bool fReturn = true;
	if( false == fKeepValue )
		fReturn = SetCUDouble( lColumn, lRow, iPhysicalType, dSI, fEmptyIfNull );

	SetCellProperty( lColumn, lRow, _SSCellProperty::CellNoMouseEvent, fNoMouseEvent );

	return fReturn;
}
// End of methods that manage cell formating value.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage cell value.
bool CSSheet::SetStaticText( long lFromColumn, long lToColumn, long lRow, int iTextID )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	CString strText = TASApp.LoadLocalizedString( iTextID );
	return SetStaticText( lFromColumn, lToColumn, lRow, strText );
}

bool CSSheet::SetStaticText( long lFromColumn, long lToColumn, long lRow, CString strText )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	ApplyTextPattern( lFromColumn, lRow, lToColumn, lRow );
	return ( TRUE == SetDataRange( lFromColumn, lRow, lToColumn, lRow, (LPCTSTR)strText ) ) ? true : false;
}

bool CSSheet::SetStaticText( long lColumn, long lRow, int iTextID )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	CString str = TASApp.LoadLocalizedString( iTextID );
	return SetStaticText( lColumn, lColumn, lRow, str );
}

bool CSSheet::SetStaticText( long lColumn, long lRow, CString strText )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	return SetStaticText( lColumn, lColumn, lRow, strText );
}

bool CSSheet::SetStaticTextOnCol( long lColumn, long lFromRow, long lToRow, CString strText )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	ApplyTextPattern( lColumn, lFromRow, lColumn, lToRow );
	return ( TRUE == SetDataRange( lColumn, lFromRow, lColumn, lToRow, (LPCTSTR)strText ) ) ? true : false;
}

void CSSheet::SetStaticTextEx( long lColumn, long lRow, CString strText, bool fChangeWidth )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( true == fChangeWidth )
	{
		long lSave = GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
		SetStaticText( lColumn, lColumn, lRow, strText );

		double dWidth, dMaxWidth, dMaxHeight;
		GetColWidth( lColumn, &dWidth );
		GetMaxTextCellSize( lColumn, lRow, &dMaxWidth, &dMaxHeight );

		// Adapt column width if needed...
		if( dWidth < dMaxWidth )
		{
			SetColWidth( lColumn, dMaxWidth );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lSave );
	}
	else
	{
		double dCurrentHeight;
		GetRowHeight( lRow, &dCurrentHeight );
		
		SetStaticText( lColumn, lColumn, lRow, strText );
		
		double dWidth, dHeight;
		GetMaxTextCellSize( lColumn, lRow, &dWidth, &dHeight );

		if( dHeight > dCurrentHeight )
		{
			SetRowHeight( lRow, dHeight );
		}
	}
}

bool CSSheet::IsTextTruncated( long lColumn, long lRow )
{
	if( false == m_bInitialized )
		return false;

	bool fTruncated = false;
	
	SS_CELLTYPE rCellType;
	GetCellType( lColumn, lRow, &rCellType );
	if( SSS_ELLIPSES == ( rCellType.Style & SSS_ELLIPSES ) )
	{
		double dWidth, dHeight;
		GetMaxTextCellSize( lColumn, lRow, &dWidth, &dHeight );
		short nMinWidth	= ColWidthToLogUnits( dWidth );
		long lCurrentColWidth = GetColWidthInPixelsW( lColumn );
		if( lCurrentColWidth <= nMinWidth )
			fTruncated = true;
	}
	return fTruncated;
}

bool CSSheet::SetCUDouble( long lColumn, long lRow, int iPhysicalType, double dSI, bool fEmptyIfNull )
{
	if( false == m_bInitialized )
		return false;

	if( 0.0 == dSI && true == fEmptyIfNull )
	{
		if( FALSE == SetData( lColumn, lRow, _T("") ) )
			return false;
	}
	else
	{
		CString str = WriteCUDouble( iPhysicalType, dSI, false );
		if( FALSE == SetData( lColumn, lRow, str ) )
			return false;
	}
	return true;
}

void CSSheet::SetCellText( long lColumn, long lRow, CString strText )
{
	if( false == m_bInitialized )
		return;

	SetData( lColumn, lRow, (LPCTSTR)strText );
}

CString CSSheet::GetCellText( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return _T("");
	}

	CString strText( _T("") );
	int iLen = GetData( lColumn, lRow, NULL );

	if( iLen > 0 )
	{
		GetData( lColumn, lRow, strText.GetBufferSetLength( iLen + 1 ) );
	}

	strText.ReleaseBuffer();

	return strText;
}

bool CSSheet::SetPictureCellWithID( int iPicID, long lFromColumn, long lFromRow, long lToColumn, long lToRow, PictureCellType ePictureType )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	if( lFromColumn != lToColumn || lFromRow != lToRow )
	{
		AddCellSpanW( lFromColumn, lFromRow, lToColumn - lFromColumn, lToRow - lFromRow );
	}
	
	HBITMAP hBitmap = (HBITMAP)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( iPicID ), ( Icon == ePictureType ) ? IMAGE_ICON : IMAGE_BITMAP, 0, 0, LR_SHARED );

	if( NULL == hBitmap )
	{
		return false;
	}

	if( false == SetPictureCellWithHandle( hBitmap, lFromColumn, lFromRow, false, VPS_CENTER | ( ( Icon == ePictureType ) ? VPS_ICON : VPS_BMP ) ) )
	{
		return false;
	}

	return true;
}

bool CSSheet::SetPictureCellWithHandle( HANDLE hImage, long lColumn, long lRow, bool fSaveInPictureContainer, long lStyle, HANDLE hImageSelected )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	if( INVALID_HANDLE_VALUE == hImage )
	{
		return false;
	}

	// Save a copy of the bitmap in the container.
	// When 'SetPictureCellWithHandle' is called from 'SetPictureCellWithID' we don't need to keep a copy in container because picture
	// is in the TASelect resources and thus is always available.
	// When 'SetPictureCellWithHandle' is directly called with a handle coming from a dynamic bitmap (created on runtime like in SSheetPanelCirc1 and 2),
	// we need to keep a copy in container. Calling function will release handle after this method and thus handle becomes invalid.
	if( true == fSaveInPictureContainer && ( lStyle & VPS_BMP ) )
	{
		// Verify first if a bitmap is not existing yet.
		for( auto &iter : m_mapBitmapList[GetSheet()] )
		{
			if( iter.m_lCol == lColumn && iter.m_lRow == lRow && NULL != iter.m_pclBitmap )
			{
				delete iter.m_pclBitmap;
				m_mapBitmapList[GetSheet()].erase( std::find( m_mapBitmapList[GetSheet()].begin(), m_mapBitmapList[GetSheet()].end(), iter ) );
				break;
			}
		}

		CEnBitmap *pCEnBitmap = new CEnBitmap;
		pCEnBitmap->CopyImage( (HBITMAP)hImage );
		BitmapInfo clBitmapInfo;
		clBitmapInfo.m_lCol = lColumn;
		clBitmapInfo.m_lRow = lRow;
		clBitmapInfo.m_pclBitmap = pCEnBitmap;
		m_mapBitmapList[GetSheet()].push_back( clBitmapInfo );
		hImage = pCEnBitmap->GetSafeHandle();
	}

	// 'SetTypePictureHandle' in TSpread has the last argument 'fDeleteHandle'. If it is set to 'true' when calling function, it means that
	// TSpread keeps a copy of handle and destroys it when it doesn't need it anymore. For example when cell that contains picture becomes 
	// invalid. If this argument is set to 'false' TSpread doesn't manage handle and it is our responsibility to release handle when it needs to
	// be done.

	SS_CELLTYPE rCellType;

	if( FALSE == SetTypePictureHandle( &rCellType, lStyle, hImage, 0, FALSE ) )
	{
		return false;
	}

	if( FALSE == SetCellType( lColumn, lRow, &rCellType ) )
	{
		return false;
	}

	// Magic data to force Spread to not consider a row with only pictures as an empty line. When printing, if there is no data, 
	// the row is printed!!
	SetData( lColumn, lRow, _T("1" ) );

	if( INVALID_HANDLE_VALUE != hImageSelected )
	{
		m_mapImageToImageSelected[hImage] = hImageSelected;
		m_mapImageSelectedToImage[hImageSelected] = hImage;
	}

	return true;
}

bool CSSheet::ClearPictureCell( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	// Verify first if a bitmap is not existing in the container.
	for( auto &iter : m_mapBitmapList[GetSheet()] )
	{
		if( iter.m_lCol == lColumn && iter.m_lRow == lRow && NULL != iter.m_pclBitmap )
		{
			delete iter.m_pclBitmap;
			m_mapBitmapList[GetSheet()].erase( std::find( m_mapBitmapList[GetSheet()].begin(), m_mapBitmapList[GetSheet()].end(), iter ) );
			break;
		}
	}

	SS_CELLTYPE rCellType;
	SetTypeStaticText( &rCellType, 0 );
	SetCellType( lColumn, lRow, &rCellType );
	return SetStaticText( lColumn, lRow, _T(" ") );
}

bool CSSheet::IsCellIsAPicture( long lColumn, long lRow, SS_CELLTYPE& rCellType )
{
	if( FALSE == GetCellType( lColumn, lRow, &rCellType ) || SS_TYPE_PICTURE != rCellType.Type )
		return false;

	return true;
}

bool CSSheet::SetCheckBox( long lColumn, long lRow, CString strText, bool fCheck, bool fEnabled )
{
	if( false == m_bInitialized )
		return false;

	SS_CELLTYPE rCellType;

	if( NULL == SetTypeCheckBox( &rCellType, SSS_ALIGN_VCENTER, 
								(LPCTSTR)strText, 
								( false == fEnabled ) ? ( ( true == fCheck ) ? _T("IDB_CHECKBOXGRAYEDCHECK") : _T("IDB_CHECKBOXGRAYEDUNCHECK") ) : NULL, 
								( false == fEnabled ) ? BT_BITMAP : BT_NONE,
								NULL, BT_NONE, NULL, BT_NONE, NULL, BT_NONE, NULL, BT_NONE, NULL, BT_NONE ) )
		return false;

	if( FALSE == SetCellType( lColumn, lRow, &rCellType ) )
		return false;
	if( FALSE == SetData( lColumn, lRow, ( true == fCheck ) ? _T("1") : _T("0") ) )
		return false;

	SetLock( lColumn, lRow, !fEnabled );

	return true;
}

bool CSSheet::IsColumnEmpty( long lColumn, long lFromRow, long lToRow )
{
	if( false == m_bInitialized )
		return false;

	// Run all rows
	bool fIsColEmpty = true;
	for( long lLoopRow = lFromRow; lLoopRow <= lToRow && true == fIsColEmpty; lLoopRow++ )
		fIsColEmpty = ( 0 == GetData( lColumn, lLoopRow, NULL ) ) ? true : false;

	return fIsColEmpty;
}
// End of methods that manage cell value.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage row selection.
void CSSheet::SelectMutipleRows( CArray<long> *parlSelRows, COLORREF backgroundColor, COLORREF textColor )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( NULL != parlSelRows )
	{
		for( long lLoopRow = 0; lLoopRow < parlSelRows->GetCount(); lLoopRow++ )
		{
			SelectOneRow( parlSelRows->GetAt( lLoopRow ), 1, GetMaxCols(), backgroundColor, textColor );
		}
	}
}

void CSSheet::SelectMutipleRows( long lFromRow, long lToRow, long lFromColumn, long lToColumn, COLORREF backgroundColor, COLORREF textColor )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( lFromRow > lToRow )
	{
		long lTemp = lToRow;
		lToRow = lFromRow;
		lFromRow = lTemp;
	}

	if( 0 != lFromColumn && -1 != lToColumn && lFromColumn > lToColumn )
	{
		long lTemp = lToColumn;
		lToColumn = lFromColumn;
		lFromColumn = lTemp;
	}

	for( long lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
	{
		SelectOneRow( lLoopRow, lFromColumn, lToColumn, backgroundColor, textColor );
	}
}

void CSSheet::SelectOneRow( long lRow, long lFromColumn, long lToColumn, COLORREF backgroundColor, COLORREF textColor )
{
	if( false == m_bInitialized )
	{
		return;
	}

	long lFirstCol = 1;
	long lLastCol = GetMaxCols();
	long lLastRow = GetMaxRows();

	if( lToColumn < lFromColumn || lToColumn > lLastCol )
	{
		lToColumn = lLastCol;
	}

	if( lRow > lLastRow )
	{
		return;
	}

	if( lFromColumn < lFirstCol )
	{
		lFromColumn = lFirstCol;
	}

	arCellProperties *parCellProperty = new arCellProperties;

	if( NULL == parCellProperty )
	{
		return;
	}

	_sCellProperties CellProp;

	long lLoopCol = lFromColumn;
	long lStartRange = lFromColumn;

	// For optimization.
	long lEndRange = -1;

	while( lLoopCol <= lToColumn )
	{
		bool fReset = false;

		if( false == IsCellProperty( lLoopCol, lRow, _SSCellProperty::CellCantSelect ) ) 
		{
			// While cell can be selected, we save cell property in the 'parCellProperty'.
			// For optimization, we do not apply now but only when we have a cell that can't be selected.

			CellProp.lColumn = lLoopCol;
			CellProp.fFontBold = GetFontBold( lLoopCol, lRow );
			GetColor( lLoopCol, lRow, &CellProp.BackColor, &CellProp.TextColor );
			parCellProperty->Add( CellProp );

			lEndRange = lLoopCol;
		}
		else
		{
			// The cell can't be selected.

			if( lEndRange != -1 )
			{
				// If we already saved some cells with 'CellCanSelect' set to false (above) then 'lEndRange' tells us
				// until which column we can set the color.
				SetColorRange( lStartRange, lRow, lEndRange, lRow, backgroundColor, textColor );
			}

			// Now we can reset the start end end range for the next call to the 'SetColorRange'.
			fReset = true;
		}

		// Check image.
		SS_CELLTYPE rCellType;

		if( true == IsCellIsAPicture( lLoopCol, lRow, rCellType ) && 0 != m_mapImageToImageSelected.count( (HANDLE)rCellType.Spec.ViewPict.hPictName ) )
		{
			rCellType.Spec.ViewPict.hPictName = (TBGLOBALHANDLE)m_mapImageToImageSelected[(HANDLE)rCellType.Spec.ViewPict.hPictName];
			SetCellType( lLoopCol, lRow, &rCellType );
		}

		lLoopCol++;

		if( true == fReset )
		{
			lStartRange = lLoopCol;
			lEndRange = -1;
		}
	}

	if( lEndRange != -1 )
	{
		// If we yet a range to set.
		SetColorRange( lStartRange, lRow, lEndRange, lRow, backgroundColor, textColor );
	}

	if( parCellProperty->GetCount() > 0 )
	{
		m_mapSelectedRowsBySheet[GetSheet()][lRow].push_back( parCellProperty );
	}
	else
	{
		delete parCellProperty;
	}
}

void CSSheet::UnSelectMultipleRows( long lSelectedRow, bool fCleanSheet, bool fOnlyLastSelection )
{
	if( false == m_bInitialized )
	{
		return;
	}

	short nSheet = GetSheet();

	if( 0 == m_mapSelectedRowsBySheet.count( nSheet ) )
	{
		return;
	}

	// Remove the elements.
	long lRow = 0;
	arCellProperties* parCellProperty = NULL;

	if( true == m_rRowSelectionParams.m_fIsActivated && false == m_rRowSelectionParams.m_fIsRunning )
	{
		m_rRowSelectionParams.m_lStartRowSelection = -1;
		m_rRowSelectionParams.m_lEndRowSelection = -1;
	}

	for( mapLongVecIter iter = m_mapSelectedRowsBySheet[nSheet].begin(); iter != m_mapSelectedRowsBySheet[nSheet].end(); )
	{
		// Get next element in map
		lRow = iter->first;

		// If only one row must be unselected and this is not the good one...
		if( lSelectedRow != -1 && lSelectedRow != lRow )
		{
			++iter;
			continue;
		}

		long lFirstRow = 0;
		long lLastRow = GetMaxRows();

		while( iter->second.size() > 0 )
		{
			arCellProperties *parCellProperty = iter->second.back();

			if( NULL != parCellProperty && parCellProperty->GetSize() > 0 )
			{

				if( true == fCleanSheet && lRow > lFirstRow && lRow <= lLastRow )		// Reset Original row properties
				{

					for( int iLoopArray = 0; iLoopArray < parCellProperty->GetSize(); iLoopArray++ )
					{

						if( false == IsCellProperty( (*parCellProperty)[iLoopArray].lColumn, lRow, _SSCellProperty::CellCantSelect ) )
						{
							SetFontBold( (*parCellProperty)[iLoopArray].lColumn, lRow, (*parCellProperty)[iLoopArray].fFontBold );
							SetColor( (*parCellProperty)[iLoopArray].lColumn, lRow, (*parCellProperty)[iLoopArray].BackColor, (*parCellProperty)[iLoopArray].TextColor );
						}

						// Check image.
						SS_CELLTYPE rCellType;

						if( true == IsCellIsAPicture( (*parCellProperty)[iLoopArray].lColumn, lRow, rCellType ) && 0 != m_mapImageSelectedToImage.count( (HANDLE)rCellType.Spec.ViewPict.hPictName ) )
						{
							rCellType.Spec.ViewPict.hPictName = (TBGLOBALHANDLE)m_mapImageSelectedToImage[(HANDLE)rCellType.Spec.ViewPict.hPictName];
							SetCellType( (*parCellProperty)[iLoopArray].lColumn, lRow, &rCellType );
						}
					}

				}

				parCellProperty->RemoveAll();
				delete parCellProperty;
			}

			iter->second.pop_back();

			if( true == fOnlyLastSelection )
			{
				break;
			}

		}

		if( 0 == iter->second.size() )
		{
			mapLongVecIter nextiter = iter;
			++nextiter;
			m_mapSelectedRowsBySheet[nSheet].erase( iter );
			iter = nextiter;
		}
		else
		{
			++iter;
		}

		if( -1 != lSelectedRow )
		{
			break;
		}
	}

	if( -1 == lSelectedRow )
		m_mapSelectedRowsBySheet[nSheet].clear();
}

int	CSSheet::GetSelectedRows( CArray<long> *parlSelRows )
{
	if( false == m_bInitialized )
		return -1;

	short nSheet = GetSheet();
	if( 0 == m_mapSelectedRowsBySheet.count( nSheet ) )
		return -1;
	
	// If pointer is NULL, function sends back only number of selected rows
	if( NULL == parlSelRows )
		return m_mapSelectedRowsBySheet[nSheet].size();

	// Because there is no MFC collection with sort feature, use CRank to sort
	// column.
	CRank rkl;
	for( mapLongVecIter iter = m_mapSelectedRowsBySheet[nSheet].begin(); iter != m_mapSelectedRowsBySheet[nSheet].end(); ++iter )
		rkl.Add( _T(""), iter->first, (LPARAM)iter->first );

	// Run all items in CRank
	CString str;
	LPARAM lRow;
	for( BOOL fContinue = rkl.GetFirst( str, lRow ); (TRUE == fContinue); fContinue = rkl.GetNext( str, lRow ) )
		parlSelRows->Add( lRow );

	return m_mapSelectedRowsBySheet[nSheet].size();
}

void CSSheet::GetSelectionPos( long& lStartCol, long& lStartRow, long& lEndCol, long& lEndRow )
{
	if( false == m_bInitialized )
		return;

	short nSheet = GetSheet();
	if( 0 == m_mapSelectedRowsBySheet.count( nSheet ) )
		return;
	
	if( 0 == m_mapSelectedRowsBySheet[nSheet].size() )
	{
		lStartCol = 0;
		lStartRow = 0;
		lEndCol = 0;
		lEndRow = 0;
		return;
	}
	lStartCol = LONG_MAX;
	lEndCol = 0;
	lStartRow = LONG_MAX;
	lEndRow = 0;
	for( mapLongVecIter iter = m_mapSelectedRowsBySheet[nSheet].begin(); iter != m_mapSelectedRowsBySheet[nSheet].end(); ++iter )
	{
		arCellProperties* pArray = iter->second.back();
		for( int iLoop = 0; iLoop < pArray->GetCount(); iLoop++ )
		{
			if( pArray->GetAt( iLoop ).lColumn < lStartCol )
				lStartCol = pArray->GetAt( iLoop ).lColumn;
			if( pArray->GetAt( iLoop ).lColumn > lEndCol )
				lEndCol = pArray->GetAt( iLoop ).lColumn;
		}
		if( iter->first < lStartRow )
			lStartRow = iter->first;
		if( iter->first > lEndRow )
			lEndRow = iter->first;
	}
}

// End of methods that manage row selection.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage block selection.
void CSSheet::BS_Activate( bool fManageBlockSelection, WORD wBorderStyle, COLORREF backgroundColor )
{
	if( false == m_bInitialized )
		return;

	m_rBlockSelectionParams.m_fIsActivated = fManageBlockSelection;
	m_rBlockSelectionParams.m_wBorderStyle = wBorderStyle;
	m_rBlockSelectionParams.m_BorderColor = backgroundColor;
}

bool CSSheet::BS_IsActivated( void )
{
	return m_rBlockSelectionParams.m_fIsActivated;
}

bool CSSheet::BS_IsSelectionExist( void )
{
	if( false == m_bInitialized )
		return false;

	return m_rBlockSelectionParams.IsSelectionExist();
}

bool CSSheet::BS_GetSelection( long& lStartCol, long& lStartRow, long& lEndCol, long& lEndRow )
{
	if( false == m_bInitialized )
		return false;

	return m_rBlockSelectionParams.GetSelection( lStartCol, lStartRow, lEndCol, lEndRow );
}

bool CSSheet::BS_SetSelection( long lStartCol, long lStartRow, long lEndCol, long lEndRow, bool fVerifyOnlyCorners )
{
	if( lStartCol < 1 || lStartRow < 1 || lEndCol < 1 || lEndRow < 1 )
		return false;
	if( lStartCol > GetMaxCols() || lStartRow > GetMaxRows() || lEndCol > GetMaxCols() || lStartRow > GetMaxRows() )
		return false;
	if( lStartCol > lEndCol )
	{
		long lTemp = lStartCol;
		lStartCol = lEndCol;
		lEndCol = lTemp;
	}
	if( lStartRow > lEndRow )
	{
		long lTemp = lStartRow;
		lStartRow = lEndRow;
		lEndRow = lTemp;
	}
	
	bool fOK = true;
	if( false == fVerifyOnlyCorners )
	{
		// Check horizontal perimeters.
		for( long lLoopCol = lStartCol; lLoopCol <= lEndCol && true == fOK; lLoopCol++ )
		{
			if( true == IsCellProperty( lLoopCol, lStartRow, _SSCellProperty::CellCantBlockSelect ) )
				fOK = false;
			if( true == fOK && lStartRow != lEndRow )
			{
				if( true == IsCellProperty( lLoopCol, lEndRow, _SSCellProperty::CellCantBlockSelect ) )
					fOK = false;
			}
		}
		if( false == fOK )
			return false;

		// Check vertical perimeters.
		if( lEndRow - lStartRow > 1 )
		{
			for( long lLoopRow = lStartRow + 1; lLoopRow < lEndRow && true == fOK; lLoopRow++ )
			{
				if( true == IsCellProperty( lStartCol, lLoopRow, _SSCellProperty::CellCantBlockSelect ) )
					fOK = false;
				if( true == fOK && lStartCol != lEndCol )
				{
					if( true == IsCellProperty( lEndCol, lLoopRow, _SSCellProperty::CellCantBlockSelect ) )
						fOK = false;
				}
			}
		}
		if( false == fOK )
			return false;
	}
	else
	{
		// Only corners.
		if( true == IsCellProperty( lStartCol, lStartRow, _SSCellProperty::CellCantBlockSelect ) )
			return false;
		if( true == IsCellProperty( lStartCol, lEndRow, _SSCellProperty::CellCantBlockSelect ) )
			return false;
		if( true == IsCellProperty( lEndCol, lStartRow, _SSCellProperty::CellCantBlockSelect ) )
			return false;
		if( true == IsCellProperty( lEndCol, lEndRow, _SSCellProperty::CellCantBlockSelect ) )
			return false;
	}

	m_rBlockSelectionParams.m_lColumnStartSelection = lStartCol;
	m_rBlockSelectionParams.m_lColumnEndSelection = lEndCol;
	m_rBlockSelectionParams.m_lRowStartSelection = lStartRow;
	m_rBlockSelectionParams.m_lRowEndSelection = lEndRow;
	m_rBlockSelectionParams.m_lOriginColumn = lStartCol;
	m_rBlockSelectionParams.m_lOriginRow = lStartRow;
	_BS_DoSelection();

	return true;
}

bool CSSheet::BS_IsCellInSelection( long lColumn, long lRow )
{
	if( false == m_bInitialized )
		return false;

	if( false == m_rBlockSelectionParams.m_fIsActivated || false == BS_IsSelectionExist() )
		return false;

	long lColStart = ( m_rBlockSelectionParams.m_lColumnStartSelection <= m_rBlockSelectionParams.m_lColumnEndSelection ) ? m_rBlockSelectionParams.m_lColumnStartSelection : m_rBlockSelectionParams.m_lColumnEndSelection;
	long lColEnd = ( m_rBlockSelectionParams.m_lColumnEndSelection >= m_rBlockSelectionParams.m_lColumnStartSelection ) ? m_rBlockSelectionParams.m_lColumnEndSelection : m_rBlockSelectionParams.m_lColumnStartSelection;
	long lRowStart = ( m_rBlockSelectionParams.m_lRowStartSelection <= m_rBlockSelectionParams.m_lRowEndSelection ) ? m_rBlockSelectionParams.m_lRowStartSelection : m_rBlockSelectionParams.m_lRowEndSelection;
	long lRowEnd = ( m_rBlockSelectionParams.m_lRowEndSelection >= m_rBlockSelectionParams.m_lRowStartSelection ) ? m_rBlockSelectionParams.m_lRowEndSelection : m_rBlockSelectionParams.m_lRowStartSelection;

	return ( lColumn >= lColStart && lColumn <= lColEnd && lRow >= lRowStart && lRow <= lRowEnd ) ? true : false;
}

void CSSheet::BS_CancelSelection( bool fResetOrigin )
{
	if( false == m_bInitialized )
		return;

	if( false == m_rBlockSelectionParams.m_fIsActivated )
		return;
	
	// Clear current block selection.
	_BS_UndoSelection();
	
	// Stop scrolling if being running.
	if( true == m_rBlockSelectionParams.m_fScrollStart )
	{
		if( 0 != m_rBlockSelectionParams.m_nTimer )
		{
			KillTimer( m_rBlockSelectionParams.m_nTimer );
			m_rBlockSelectionParams.m_nTimer = (UINT_PTR)0;
		}
	}

	// If column and row origins are defined, we reset block selection on this cell.
	if( true == fResetOrigin && -1 != m_rBlockSelectionParams.m_lOriginColumn && -1 !=  m_rBlockSelectionParams.m_lOriginRow )
	{
		m_rBlockSelectionParams.m_lColumnStartSelection = m_rBlockSelectionParams.m_lOriginColumn;
		m_rBlockSelectionParams.m_lColumnEndSelection = m_rBlockSelectionParams.m_lOriginColumn;
		m_rBlockSelectionParams.m_lRowStartSelection = m_rBlockSelectionParams.m_lOriginRow;
		m_rBlockSelectionParams.m_lRowEndSelection = m_rBlockSelectionParams.m_lOriginRow;
		_BS_DoSelection();
	}
	else
	{	
		m_rBlockSelectionParams.m_lColumnStartSelection = -1;
		m_rBlockSelectionParams.m_lColumnEndSelection = -1;
		m_rBlockSelectionParams.m_lRowStartSelection = -1;
		m_rBlockSelectionParams.m_lRowEndSelection = -1;
	}
}

bool CSSheet::BS_IsMousePointerInSelection( CPoint ptMouse )
{
	if( false == m_bInitialized )
		return false;

	if( false == m_rBlockSelectionParams.m_fIsActivated || false == BS_IsSelectionExist() )
		return false;

	long lStartCol, lEndCol, lStartRow, lEndRow;
	if( false == BS_GetSelection( lStartCol, lStartRow, lEndCol, lEndRow ) )
		return false;
	
	CRect rectSizeLog( lStartCol, lStartRow, lEndCol, lEndRow );
	CRect rectSizePixel = GetSelectionCoordInPixels( rectSizeLog );
	ScreenToClient( &ptMouse );
	return ( TRUE == rectSizePixel.PtInRect( ptMouse ) ) ? true : false;
}

bool CSSheet::BS_DeleteDataInSelection( void )
{
	if( false == m_bInitialized )
		return false;

	if( false == m_rBlockSelectionParams.m_fIsActivated || false == BS_IsSelectionExist() )
		return false;

	long lStartCol = m_rBlockSelectionParams.m_lColumnStartSelection;
	long lEndCol = m_rBlockSelectionParams.m_lColumnEndSelection;
	if( lStartCol > lEndCol )
	{
		long lTemp = lStartCol;
		lStartCol = lEndCol;
		lEndCol = lTemp;
	}
	
	long lStartRow = m_rBlockSelectionParams.m_lRowStartSelection;
	long lEndRow = m_rBlockSelectionParams.m_lRowEndSelection;
	if( lStartRow > lEndRow )
	{
		long lTemp = lStartRow;
		lStartRow = lEndRow;
		lEndRow= lTemp;
	}
	return ( TRUE == ClearDataRange( lStartCol, lStartRow, lEndCol, lEndRow ) ) ? true : false;
}

// End of methods that manage block selection.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage row selection.
void CSSheet::RS_Activate( bool fManageLineSelection, COLORREF backgroundColor ) 
{
	if( false == m_bInitialized )
		return;

	m_rRowSelectionParams.m_fIsActivated = fManageLineSelection;
	m_rRowSelectionParams.m_BackgroundColor = backgroundColor;
}

void CSSheet::RS_SetBackground( COLORREF backgroundColor )
{
	if( false == m_bInitialized )
		return;

	m_rRowSelectionParams.m_BackgroundColor = backgroundColor;
}

void CSSheet::RS_SetCursor( UINT uiIconID )
{
	if( false == m_bInitialized )
		return;

	if( (HCURSOR)NULL != m_rRowSelectionParams.m_hSelectionCursor )
		DeleteObject( m_rRowSelectionParams.m_hSelectionCursor );
	m_rRowSelectionParams.m_hSelectionCursor = AfxGetApp()->LoadIcon( uiIconID );
}

void CSSheet::RS_SetPixelTolerance( short nPixels )
{
	if( false == m_bInitialized )
		return;

	m_rRowSelectionParams.m_nPixelTolerance = nPixels;
}

void CSSheet::RS_SetColumnReference( long lColumn )
{
	if( false == m_bInitialized )
		return;

	m_rRowSelectionParams.m_lColumnReference = lColumn;
}

bool CSSheet::RS_IsSelectionExist()
{
	if( false == m_bInitialized )
		return false;
	return m_rRowSelectionParams.IsSelectionExist();
}

bool CSSheet::RS_GetSelection( long& lStartRow, long& lEndRow )
{
	if( false == m_bInitialized )
		return false;
	return m_rRowSelectionParams.GetSelection( lStartRow, lEndRow );
}

// End of methods that manage row selection.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that wraps some TSpread methods.
BOOL CSSheet::AddCellSpanW( long lColumn, long lRow, long lNumCol, long lNumRow )
{
	if( false == m_bInitialized )
		return FALSE;

	return ( TSpread::AddCellSpan( lColumn, lRow, lNumCol, lNumRow ) );
}

void CSSheet::RemoveCellSpanW( long lFromColumn, long lFromRow, long lToColumn, long lToRow )
{
	if( false == m_bInitialized )
		return;

	for( long lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
	{
		for( long lLoopcol = lFromColumn; lLoopcol <= lToColumn; lLoopcol++)
			TSpread::RemoveCellSpan( lLoopcol, lLoopRow );
	}
}

BYTE CSSheet::GetCellTypeW( long lColumn, long lRow )
{
	if( false == m_bInitialized )
		return -1;

	BYTE bCellType = -1;
	SS_CELLTYPE rCellType;
	if( TRUE == GetCellType( lColumn, lRow, &rCellType ) )
		bCellType = rCellType.Type;
	return bCellType;
}

void CSSheet::ShowColRange( long lFromColumn, long lToColumn, BOOL fShow )
{
	if( false == m_bInitialized )
		return;

	for( long lLoopCol = lFromColumn; lLoopCol <= lToColumn; lLoopCol++ )
		ShowCol( lLoopCol, fShow );
}

long CSSheet::GetActiveCol( void )
{
	if( false == m_bInitialized )
		return -1;

	long lCol, lRow;
	GetActiveCell( &lCol, &lRow );
	return lCol;
}

long CSSheet::GetActiveRow( void )
{
	if( false == m_bInitialized )
		return -1;

	long lCol, lRow;
	GetActiveCell( &lCol, &lRow );
	return lRow;
}

long CSSheet::GetTopCol( void )
{
	if( false == m_bInitialized )
		return -1;

	long lColumn, lRow;
	GetTopLeftCell( &lColumn, &lRow );
	return lColumn;
}

long CSSheet::GetTopRow( void )
{
	if( false == m_bInitialized )
		return -1;

	long lColumn, lRow;
	GetTopLeftCell( &lColumn, &lRow );
	return lRow;
}

long CSSheet::GetColsFreeze( void )
{
	if( false == m_bInitialized )
		return -1;

	long lColsFreezed = -1;
	long lRowsFreezed = -1;
	GetFreeze( &lColsFreezed, &lRowsFreezed );
	return lColsFreezed;
}

long CSSheet::GetRowsFreeze( void )
{
	if( false == m_bInitialized )
		return -1;

	long lColsFreezed = -1;
	long lRowsFreezed = -1;
	GetFreeze( &lColsFreezed, &lRowsFreezed );
	return lRowsFreezed;
}

double CSSheet::LogUnitsToColWidthW( short nUnits )
{
	if( false == m_bInitialized )
		return -1.0;

	double dWidth;
	LogUnitsToColWidth( nUnits, &dWidth );
	return dWidth;
}

// End of methods that wraps some TSpread methods.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


short CSSheet::GetTipTextWidth( CString str )
{
	WORD wStatus;
	LONG lDelay;
	LOGFONT rLogFont;
	COLORREF clrBack, clrFore;
	GetTextTip( &wStatus, &lDelay, &rLogFont, &clrBack, &clrFore );

	CFont rTipFont;
	rTipFont.CreateFontIndirect( &rLogFont );

	CDC *pDC = GetDC();
	CFont *pOldFont = pDC->SelectObject( &rTipFont );

	CRect rect( 0, 0, 0, 0);
	pDC->DrawText( str, &rect, DT_CALCRECT | DT_NOPREFIX );

	pDC->SelectObject( pOldFont );
	ReleaseDC( pDC );
	rTipFont.DeleteObject();

	return (short)( rect.right + 5 );
}

void CSSheet::MergeRowsWithSameValues( long lFromRow, long lToRow, vector<long>& vecColumnList )
{
	if( false == m_bInitialized )
		return;

	CString strFirstValue( _T("") );
	long lFirstRowToSpan;

	long lLoopRow;
	for( lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
	{
		// Prepare key
		CString strKey( _T("") );
		for( unsigned int uiLoopColumn = 0; uiLoopColumn < vecColumnList.size(); uiLoopColumn++ )
			strKey += GetCellText( vecColumnList[uiLoopColumn], lLoopRow );
	
		if( false == strFirstValue.IsEmpty() )
		{
			// If current one is the same as the first one
			if( strKey != strFirstValue )
			{
				if( lLoopRow - lFirstRowToSpan > 1 )
				{
					for( unsigned int uiLoopColumn = 0; uiLoopColumn < vecColumnList.size(); uiLoopColumn++ )
						AddCellSpanW( vecColumnList[uiLoopColumn], lFirstRowToSpan, 1, lLoopRow - lFirstRowToSpan );
				}
				lFirstRowToSpan = lLoopRow;
				strFirstValue = strKey;
			}
		}
		else
		{
			// First row
			lFirstRowToSpan = lLoopRow;
			strFirstValue = strKey;
		}
	}

	if( lLoopRow - lFirstRowToSpan > 1 )
	{
		for( unsigned int uiLoopColumn = 0; uiLoopColumn < vecColumnList.size(); uiLoopColumn++ )
			AddCellSpanW( vecColumnList[uiLoopColumn], lFirstRowToSpan, 1, lLoopRow - lFirstRowToSpan );
	}
}

void CSSheet::ResetTabStripPosition( void )
{
	if( false == m_bInitialized )
		return;

	SetTabStripLeftSheet( 1 ); 
}

void CSSheet::EmptyMessageQueue( void )
{
	// clear message queue
	MSG msg;
	while( PeekMessage( &msg, GetSafeHwnd(),  0, 0, PM_REMOVE ) )
	{ 
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}

bool CSSheet::SaveSelections( SaveSelectionChoice eWhichSelection )
{
	if( false == m_bInitialized || SaveSelectionChoice::SSC_None == eWhichSelection )
		return false;

	// Clear all previous data saved.
	m_rSaveSelectionsContext.CleanSelections();
	m_rSaveSelectionsContext.m_eSelectionSaved = eWhichSelection;

	// Simple selection.
	short nSheet = GetSheet();
	if( 0 != m_mapSelectedRowsBySheet.count( nSheet) && SaveSelectionChoice::SSC_Selection == ( eWhichSelection & SaveSelectionChoice::SSC_Selection ) )
	{
		if( m_mapSelectedRowsBySheet[nSheet].size() > 0 )
		{
			for( mapLongVecIter mapIter = m_mapSelectedRowsBySheet[nSheet].begin(); mapIter != m_mapSelectedRowsBySheet[nSheet].end(); ++mapIter )
			{
				for( vecCellPropertiesIter vecIter = mapIter->second.begin(); vecIter != mapIter->second.end(); ++vecIter )
				{
					arCellProperties* parCells = new arCellProperties;
					if( NULL == parCells )
						continue;

					// Why '**': One to get the value in the iterator (that is a pointer on 'arCellProperties) and second one to pass array as
					//           a reference.
					parCells->Copy( **vecIter );
					m_rSaveSelectionsContext.m_mapSelection[mapIter->first].push_back( parCells );
				}

				// We need also to save the last selection that has be done over the last saved 'arCellProperties'.
				arCellProperties* parCells = new arCellProperties;
				if( NULL == parCells )
					continue;

				for( int iLoopArray = 0; iLoopArray < mapIter->second.back()->GetSize(); iLoopArray++ )
				{
					_sCellProperties CellProp;
					CellProp.lColumn = mapIter->second.back()->GetAt( iLoopArray ).lColumn;
					CellProp.fFontBold = GetFontBold( mapIter->second.back()->GetAt( iLoopArray ).lColumn, mapIter->first );
					GetColor( mapIter->second.back()->GetAt( iLoopArray ).lColumn, mapIter->first, &CellProp.BackColor, &CellProp.TextColor );
					parCells->Add( CellProp );
				}
				m_rSaveSelectionsContext.m_mapSelection[mapIter->first].push_back( parCells );
			}
		}
	}

	// Block selection.
	if( SaveSelectionChoice::SSC_BlockSelection == ( eWhichSelection & SaveSelectionChoice::SSC_BlockSelection ) )
	{
		m_rBlockSelectionParams.GetSelection( m_rSaveSelectionsContext.m_lColumnStartBlockSelection, m_rSaveSelectionsContext.m_lRowStartBlockSelection, 
			m_rSaveSelectionsContext.m_lColumnEndBlockSelection, m_rSaveSelectionsContext.m_lRowEndBlockSelection );
	}

	return true;
}

bool CSSheet::ClearCurrentSelections( SaveSelectionChoice eWhichSelection )
{
	if( false == m_bInitialized || SaveSelectionChoice::SSC_None == eWhichSelection )
		return false;

	// Simple selection.
	if( SaveSelectionChoice::SSC_Selection == ( eWhichSelection & SaveSelectionChoice::SSC_Selection ) )
		UnSelectMultipleRows();

	// Block selection.
	if( SaveSelectionChoice::SSC_BlockSelection == ( eWhichSelection & SaveSelectionChoice::SSC_BlockSelection ) )
	{
		_BS_UndoSelection();
		m_rBlockSelectionParams.ClearSelection();
	}

	return true;
}

bool CSSheet::RestoreSelections()
{
	if( false == m_bInitialized || SaveSelectionChoice::SSC_None == m_rSaveSelectionsContext.m_eSelectionSaved )
		return false;

	// Simple selection.
	short nSheet = GetSheet();
	if( 0 != m_mapSelectedRowsBySheet.count( nSheet ) && SaveSelectionChoice::SSC_Selection == ( m_rSaveSelectionsContext.m_eSelectionSaved & SaveSelectionChoice::SSC_Selection ) )
	{
		_ClearAllSelectedRows( &m_mapSelectedRowsBySheet[nSheet] );
		if( m_rSaveSelectionsContext.m_mapSelection.size() > 0 )
		{
			for( mapLongVecIter mapIter = m_rSaveSelectionsContext.m_mapSelection.begin(); mapIter != m_rSaveSelectionsContext.m_mapSelection.end(); ++mapIter )
			{
				arCellProperties* parCells = NULL;
				for( int iVecLoop = 0; iVecLoop < (int)mapIter->second.size(); iVecLoop++ )
				{
					if( iVecLoop < (int)mapIter->second.size() - 1 )
					{
						parCells = new arCellProperties;
						if( NULL == parCells )
							continue;
					}
					
					for( int iLoopArray = 0; iLoopArray < mapIter->second[iVecLoop]->GetSize(); iLoopArray++ )
					{
						if( false == IsCellProperty( (*mapIter->second[iVecLoop])[iLoopArray].lColumn, mapIter->first, _SSCellProperty::CellCantSelect ) )
						{
							_sCellProperties CellProp;
							CellProp.lColumn = (*mapIter->second[iVecLoop])[iLoopArray].lColumn;
							CellProp.fFontBold = (*mapIter->second[iVecLoop])[iLoopArray].fFontBold;
							CellProp.BackColor = (*mapIter->second[iVecLoop])[iLoopArray].BackColor;
							CellProp.TextColor = (*mapIter->second[iVecLoop])[iLoopArray].TextColor;

							if( NULL != parCells )
								parCells->Add( CellProp );

							SetFontBold( CellProp.lColumn, mapIter->first, CellProp.fFontBold );
							SetColor( CellProp.lColumn, mapIter->first, CellProp.BackColor, CellProp.TextColor );
						}
					}
					
					if( NULL != parCells )
					{
						m_mapSelectedRowsBySheet[nSheet][mapIter->first].push_back( parCells );
						parCells = NULL;
					}
				}
			}
		}
	}

	// Block selection.
	if( SaveSelectionChoice::SSC_BlockSelection == ( m_rSaveSelectionsContext.m_eSelectionSaved & SaveSelectionChoice::SSC_BlockSelection ) &&
		true == m_rBlockSelectionParams.m_fIsActivated &&
		true == m_rBlockSelectionParams.IsSelectionExist( m_rSaveSelectionsContext.m_lColumnStartBlockSelection ) )
	{
		m_rBlockSelectionParams.m_lOriginColumn = m_rSaveSelectionsContext.m_lColumnStartBlockSelection;
		m_rBlockSelectionParams.m_lOriginRow = m_rSaveSelectionsContext.m_lRowStartBlockSelection;
		m_rBlockSelectionParams.m_lColumnStartSelection = m_rSaveSelectionsContext.m_lColumnStartBlockSelection;
		m_rBlockSelectionParams.m_lColumnEndSelection = m_rSaveSelectionsContext.m_lColumnEndBlockSelection;
		m_rBlockSelectionParams.m_lRowStartSelection = m_rSaveSelectionsContext.m_lRowStartBlockSelection;
		m_rBlockSelectionParams.m_lRowEndSelection = m_rSaveSelectionsContext.m_lRowEndBlockSelection;
		_BS_DoSelection();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// CSSheet protected members
//////////////////////////////////////////////////////////////////////

void CSSheet::OnMouseMove( UINT nFlags, CPoint point )
{
	if( false == m_bInitialized )
		return;

	TSpread::OnMouseMove( nFlags, point );

	_BS_CheckOnMouseMove( point );
	_RS_CheckOnMouseMove( point );

	if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_MouseMove ) )
	{
		long lColumn, lRow;
		GetCellFromPixel( &lColumn, &lRow, point.x, point.y );
		MM_PM_SSheet_MouseMove( GetSafeHwnd(), lColumn, lRow, point );
	}
}

LRESULT CSSheet::OnMouseLeave( WPARAM wParam, LPARAM lParam )
{
	// This message is generated because we ask a 'TrackMouseEvent' with TME_LEAVE parameter in the 'SS_Proc' function of the 'SS_Main.c' of the TSpread. 
	// In this function, TSpread call itself 'TrackMouseEvent' when event 'WM_MOUSEMOVE' is received.
	if( true == m_bInitialized )
	{
		CPoint point;
		GetCursorPos( &point );
		ScreenToClient( &point );
		_BS_CheckOnMouseMove( point );
		_RS_CheckOnMouseMove( point );
	}

	// We call 'DefWindowProc' to be sure that 'TSpread' will also receive this message.
	return DefWindowProc( WM_MOUSELEAVE, wParam, lParam );
}

void CSSheet::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bInitialized )
		return;

	_ManageOnLButtonDown( nFlags, point );
}

void CSSheet::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	if( false == m_bInitialized )
		return;

	_ManageOnLButtonDblClk( nFlags, point );
}

void CSSheet::OnLButtonUp( UINT nFlags, CPoint point )
{
	if( false == m_bInitialized )
		return;

	TSpread::OnLButtonUp( nFlags, point );

	if( true == m_rBlockSelectionParams.m_fIsRunning )
	{
		::ReleaseCapture();
		m_rBlockSelectionParams.m_fIsRunning = false;
		if( true == m_rBlockSelectionParams.m_fScrollStart )
		{
			KillTimer( m_rBlockSelectionParams.m_nTimer );
			m_rBlockSelectionParams.m_nTimer = (UINT_PTR)0;
			m_rBlockSelectionParams.m_fScrollStart = false;
		}
	}

	if( true == m_rRowSelectionParams.m_fIsRunning )
	{
		::ReleaseCapture();
		m_rRowSelectionParams.m_fIsRunning = false;
		SetCursor( SS_CURSORTYPE_DEFAULT, m_rRowSelectionParams.m_hDefaultCursor );
		m_rRowSelectionParams.m_fMousePointerChanged = false;
	}

	if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonUp ) )
	{
		long lLButtonUpCol, lLButtonUpRow;
		GetCellFromPixel( &lLButtonUpCol, &lLButtonUpRow, point.x, point.y );
		MM_PM_SSheet_MouseLButtonUp( GetSafeHwnd(), lLButtonUpCol, lLButtonUpRow, point );
	}
}

void CSSheet::OnRButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bInitialized )
		return;

	TSpread::OnRButtonDown( nFlags, point );

	// For the focus if not yet on it.
	if( GetSafeHwnd() != GetFocus()->GetSafeHwnd() )
		SetFocus();

	long lRButtonDownCol, lRButtonDownRow;
	GetCellFromPixel( &lRButtonDownCol, &lRButtonDownRow, point.x, point.y );
	if( lRButtonDownCol > 0 && lRButtonDownRow > 0 && true == IsCellProperty( lRButtonDownCol, lRButtonDownRow, _SSCellProperty::CellCantRightClick ) )
		return;

	if( true == m_rBlockSelectionParams.m_fIsActivated )
	{
		if( false == BS_IsSelectionExist() || false == BS_IsCellInSelection( lRButtonDownCol, lRButtonDownRow ) )
			_BS_CheckAfterMouseEvent( point );
	}

	// Cancel row selection if enabled and running.
	if( true == m_rRowSelectionParams.m_fIsActivated && true == m_rRowSelectionParams.m_fMousePointerChanged )
	{
		if( true == m_rRowSelectionParams.m_fIsRunning )
		{
			::ReleaseCapture();
			m_rRowSelectionParams.m_fIsRunning = false;
		}
		SetCursor( SS_CURSORTYPE_DEFAULT, m_rRowSelectionParams.m_hDefaultCursor );
		m_rRowSelectionParams.m_fMousePointerChanged = false;
	}

	if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_MouseRButtonDown ) )
	{
		MM_PM_SSheet_MouseRButtonDown( GetSafeHwnd(), lRButtonDownCol, lRButtonDownRow, point );
	}
}

void CSSheet::OnCaptureChanged( CWnd* pWnd )
{
	if( false == m_bInitialized )
		return;

	if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_MouseCaptureChanged ) )
	{
		MM_PM_SSheet_MouseCaptureChanged( GetSafeHwnd(), pWnd->GetSafeHwnd() );
	}
	TSpread::OnCaptureChanged( pWnd );
}

void CSSheet::OnSize( UINT nType, int cx, int cy )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_SizeChanged ) )
	{
		MM_PM_SSheet_SizeChanged( GetSafeHwnd(), nType, cx, cy );
	}
	
	TSpread::OnSize( nType, cx, cy );
}

LRESULT CSSheet::OnSSheetColWidthChange( WPARAM wParam, LPARAM lParam )
{
	if( false == m_bInitialized )
	{
		return 0;
	}

	if( true == MM_IsNotificationHandlerRegistered( SSheetNHFlags::SSheetNHF_ColWidthChanged ) )
	{
		long lColumnID = (long)lParam;
		MM_PM_SSheet_ColWidthChanged( GetSafeHwnd(), lColumnID );
	}
	
	return 0;
}

// Uncomment this if you want to debug the block selection and the scrolling with the mouse (Active in 'OnTime' and '_BS_OnCheckMouseMove').
// #define DEBUG_BLOCKSELECTION_MOUSESCROLLING
void CSSheet::OnTimer( UINT_PTR nIDEvent )
{
	TSpread::OnTimer( nIDEvent );

	if( (UINT_PTR)0 != m_rBlockSelectionParams.m_nTimer && m_rBlockSelectionParams.m_nTimer == nIDEvent )
	{
		WaitForSingleObject( m_hOnTimerEvent, INFINITE );
		ResetEvent( m_hOnTimerEvent );

		POINT point;
		GetCursorPos( &point );
		ScreenToClient( &point );
		long lMouseOverCol, lMouseOverRow;
		GetCellFromPixel( &lMouseOverCol, &lMouseOverRow, point.x, point.y );

		CRect rectTSpread;
		GetClientRect( &rectTSpread );

		long lColFreeze = GetColsFreeze();
		long lRowFreeze = GetRowsFreeze();

		#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
		OutputDebugString( _T("Timer event starts...\n") );
		CString str;
		str.Format( _T("MOC: %i; MOR: %i\n"), lMouseOverCol, lMouseOverRow );
		OutputDebugString( str );
		#endif

		// Important note about 'ShowCell'. If a cell is partially visible and we use its coordinate with 'ShowCell' we are not sure of the result 
		// of the scrolling. Keep in mind that TSpread scrolls down/up or right/left and ensures that cells in the left and top part of the client edge are 
		// full visible. To correctly positioning, use only the SS_SHOW_TOPLEFT!

		// Check first the vertical scrolling if currently running.
		if( eBool3::eb3Undef != m_rBlockSelectionParams.m_b3ScrollDown )
		{
			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
			OutputDebugString( _T("\tVertical srcolling...\n") );
			#endif

			if( eBool3::eb3True == m_rBlockSelectionParams.m_b3ScrollDown )
			{
				// Currently scrolling down.

				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tScrolling down...\n") );
				#endif

				long lBottomRightCol, lBottomRightRow;
				long lTopLeftCol, lTopLeftRow;
				GetBottomRightCell( &lBottomRightCol, &lBottomRightRow );
				GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );

				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				str.Format( _T("\t\tTLC: %i; TLR: %i; BRC: %i; BRR: %i; MR: %i\n"), lTopLeftCol, lTopLeftRow, lBottomRightCol, lBottomRightRow, GetMaxRows() );
				OutputDebugString( str );
				#endif
			
				// Check if we have yet enough row to scroll down.
				if( lBottomRightRow < GetMaxRows() )
 				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t%i (BRR) < %i(MR) -> we have enougth space to sroll down.\n"), lBottomRightRow, GetMaxRows() );
					OutputDebugString( str );
					#endif

 					lTopLeftRow++;	
 					lBottomRightRow++;

					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\tNew TLR: %i; New BRR: %i\n"), lTopLeftRow, lBottomRightRow );
					OutputDebugString( str );
					#endif
 				}

				if( lBottomRightRow < GetMaxRows() )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t%i (BRR) < %i (MR) -> ShowCell(%i (TLC), %i (TLR), SS_SHOW_TOPLEFT\n"), lBottomRightRow, GetMaxRows(), lTopLeftCol, lTopLeftRow );
					OutputDebugString( str );
					#endif

					ShowCell( lTopLeftCol, lTopLeftRow, SS_SHOW_TOPLEFT );
				}
				else
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t%i (BRR) >= %i (MR) -> ShowCell(%i (TLC), %i (MR), SS_SHOW_NEAREST\n"), lBottomRightRow, GetMaxRows(), lTopLeftCol, GetMaxRows() );
					OutputDebugString( str );
					#endif

					ShowCell( lTopLeftCol, GetMaxRows(), SS_SHOW_NEAREST );
				}
				lMouseOverRow = lBottomRightRow;

				// If there is no more row to scroll down...
				if( lBottomRightRow >= GetMaxRows() )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t%i (BRR) >= %i (MR) -> No more scrolling down possible\n"), lBottomRightRow, GetMaxRows() );
					OutputDebugString( str );
					#endif

					m_rBlockSelectionParams.m_b3ScrollDown = eBool3::eb3Undef;

					// We can stop timer only iF there is no horizontal scroll running.
					if( eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollRight )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\t\tNo horizontal scrolling -> we stop the timer.\n") );
						#endif

						KillTimer( m_rBlockSelectionParams.m_nTimer );
						m_rBlockSelectionParams.m_nTimer = (UINT_PTR)0;
						m_rBlockSelectionParams.m_fScrollStart = false;
					}
				}
			}
			else
			{
				// Currently scrolling up.

				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tScrolling up...\n") );
				#endif

				long lTopLeftCol;
				long lTopLeftRow;
				GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );

				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				str.Format( _T("\t\tTLC: %i; TLR: %i\n"), lTopLeftCol, lTopLeftRow );
				OutputDebugString( str );
				#endif
			
				// Check if we have yet enough row to scroll up.
				if( ( 0 == lRowFreeze && lTopLeftRow > 1 ) || ( 0 != lRowFreeze && lTopLeftRow > lRowFreeze + 1 ) )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t( 0 == %i (RF) && %i (TLR) > 1 ) || ( 0 != %i (RF) && %i (TLR) > %i (RF) + 1 ) -> we have enougth space to sroll up.\n"), lRowFreeze, lTopLeftRow, lRowFreeze, lTopLeftRow, lRowFreeze );
					OutputDebugString( str );
					#endif

					lTopLeftRow--;

					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\tNew TLR: %i\n"), lTopLeftRow );
					OutputDebugString( str );
					#endif
				}

				ShowCell( lTopLeftCol, lTopLeftRow, SS_SHOW_TOPLEFT );
				lMouseOverRow = lTopLeftRow;

				// If there is no more row to scroll up...
				if( ( ( 0 == lRowFreeze && 1 == lTopLeftRow ) || ( 0 != lRowFreeze && lTopLeftRow == lRowFreeze + 1 ) ) ) 
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t( 0 == %i (RF) && 1 == %i (TLR) ) || ( 0 != %i (RF) && %i (TLR) == %i (RF) + 1 ) -> no more scrolling up possible.\n"), lRowFreeze, lTopLeftRow, lRowFreeze, lTopLeftRow, lRowFreeze );
					OutputDebugString( str );
					#endif

					m_rBlockSelectionParams.m_b3ScrollDown = eBool3::eb3Undef;

					// We can stop timer only iF there is no horizontal scroll running.
					if( eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollRight )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\t\tNo vertical scrolling -> we stop the timer.\n") );
						#endif

						KillTimer( m_rBlockSelectionParams.m_nTimer );
						m_rBlockSelectionParams.m_nTimer = (UINT_PTR)0;
						m_rBlockSelectionParams.m_fScrollStart = false;
					}
				}
			}
		}

		// Check now the horizontal scrolling if currently running.
		if( eBool3::eb3Undef != m_rBlockSelectionParams.m_b3ScrollRight )
		{
			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
			OutputDebugString( _T("\tHorizontal srcolling...\n") );
			#endif

			if( eBool3::eb3True == m_rBlockSelectionParams.m_b3ScrollRight )
			{
				// Currently scrolling right.

				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tScrolling right...\n") );
				#endif

				long lBottomRightCol, lBottomRightRow;
				long lTopLeftCol, lTopLeftRow;
				GetBottomRightCell( &lBottomRightCol, &lBottomRightRow );
				GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );

				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				str.Format( _T("\t\tTLC: %i; TLR: %i; BRC: %i; BRR: %i; MC: %i\n"), lTopLeftCol, lTopLeftRow, lBottomRightCol, lBottomRightRow, GetMaxCols() );
				OutputDebugString( str );
				#endif

				// Check if we have yet enough column to scroll right.
				if( lBottomRightCol < GetMaxCols() )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t%i (BRC) < %i(MC) -> we have enougth space to sroll right.\n"), lBottomRightCol, GetMaxCols() );
					OutputDebugString( str );
					#endif

					lBottomRightCol++;	
					lTopLeftCol++;

					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\tNew TLC: %i; New BRC: %i\n"), lTopLeftCol, lBottomRightCol );
					OutputDebugString( str );
					#endif
				}

				if( lBottomRightCol < GetMaxCols() )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t%i (BRC) < %i (MC) -> ShowCell(%i (TLC), %i (TLR), SS_SHOW_TOPLEFT\n"), lBottomRightCol, GetMaxCols(), lTopLeftCol, lTopLeftRow );
					OutputDebugString( str );
					#endif

					ShowCell( lTopLeftCol, lTopLeftRow, SS_SHOW_TOPLEFT );
				}
				else
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t%i (BRC) >= %i (MC) -> ShowCell(%i (GetMaxCols), %i (TLR), SS_SHOW_NEAREST\n"), lBottomRightCol, GetMaxCols(), GetMaxCols(), lTopLeftRow );
					OutputDebugString( str );
					#endif

					ShowCell( GetMaxCols(), lTopLeftRow, SS_SHOW_NEAREST );
				}
				lMouseOverCol = lBottomRightCol;

				// If there is no more column to scroll right...
				if( lBottomRightCol >= GetMaxCols() )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t%i (BRC) >= %i (MC) -> No more scrolling right possible\n"), lBottomRightCol, GetMaxCols() );
					OutputDebugString( str );
					#endif

					m_rBlockSelectionParams.m_b3ScrollRight = eBool3::eb3Undef;
				
					// We can stop timer only if there is no vertical scroll running.
					if( eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollDown )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\t\tNo vertical scrolling -> we stop the timer.\n") );
						#endif

						KillTimer( m_rBlockSelectionParams.m_nTimer );
						m_rBlockSelectionParams.m_nTimer = (UINT_PTR)0;
						m_rBlockSelectionParams.m_fScrollStart = false;
					}
				}
			}
			else
			{
				// Currently scrolling left.

				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tScrolling left...\n") );
				#endif

				long lTopLeftCol;
				long lTopLeftRow;
				GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );

				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				str.Format( _T("\t\tTLC: %i; TLR: %i\n"), lTopLeftCol, lTopLeftRow );
				OutputDebugString( str );
				#endif

				// Check if we have yet enough column to scroll right.
				if( ( 0 == lColFreeze && lTopLeftCol > 1 ) || ( 0 != lColFreeze && lTopLeftCol > lColFreeze + 1 ) )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t( 0 == %i (CF) && %i (TLC) > 1 ) || ( 0 != %i (CF) && %i (TLC) > %i (CF) + 1 ) -> we have enougth space to sroll left.\n"), lColFreeze, lTopLeftCol, lColFreeze, lTopLeftCol, lColFreeze );
					OutputDebugString( str );
					#endif

					lTopLeftCol--;

					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\tNew TLC: %i\n"), lTopLeftCol );
					OutputDebugString( str );
					#endif
				}

				ShowCell( lTopLeftCol, lTopLeftRow, SS_SHOW_TOPLEFT );
				lMouseOverCol = lTopLeftCol;

				// If there is no more column to scroll left...
				if( ( ( 0 == lColFreeze && 1 == lTopLeftCol ) || ( 0 != lColFreeze && lTopLeftCol == lColFreeze + 1 ) ) )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					str.Format( _T("\t\t\t( 0 == %i (CF) && 1 == %i (TLC) ) || ( 0 != %i (CF) && %i (TLC) == %i (CF) + 1 ) -> no more scrolling up possible.\n"), lColFreeze, lTopLeftCol, lColFreeze, lTopLeftCol, lColFreeze );
					OutputDebugString( str );
					#endif

					m_rBlockSelectionParams.m_b3ScrollRight = eBool3::eb3Undef;
				
					// We can stop timer only if there is no vertical scroll running.
					if( eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollDown )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\t\tNo vertical scrolling -> we stop the timer.\n") );
						#endif

						KillTimer( m_rBlockSelectionParams.m_nTimer );
						m_rBlockSelectionParams.m_nTimer = (UINT_PTR)0;
						m_rBlockSelectionParams.m_fScrollStart = false;
					}
				}
			}
		}

		#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
		OutputDebugString( _T("Timer event stops.\n\n") );
		#endif

		SetEvent( m_hOnTimerEvent );

		_BS_CheckOnMouseMove( point );
	}
}

void CSSheet::OnPaint()
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->SSheetOnBeforePaint();
	}

	TSpread::OnPaint();

	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->SSheetOnAfterPaint();
	}
}

BOOL CSSheet::PreTranslateMessage( MSG *pMsg )
{
	BOOL bReturn = FALSE;

	if( true == m_bInitialized )
	{
		switch( pMsg->message)
		{
			case WM_KEYDOWN:

				bReturn = _BS_CheckKeyboardDownEvent( (int)pMsg->wParam );

				if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardShortcut ) )
				{
					if( ( 0 != ( GetKeyState( VK_CONTROL ) & 0x8000 ) ) )
					{
						int iKeyCode = -1;
						bool bShortcut = true;

						switch( pMsg->wParam )
						{
							case 0x41:
								iKeyCode = KS_CONTROL_A;
								break;

							case 0x43:
								iKeyCode = KS_CONTROL_C;
								break;

							case 0x56:
								iKeyCode = KS_CONTROL_V;
								break;

							case 0x58:
								iKeyCode = KS_CONTROL_X;
								break;

							default:
								bShortcut = false;
								break;
						}

						if( true == bShortcut )
						{
							MM_PM_SSheet_KeyboardShortcut( GetSafeHwnd(), iKeyCode );
							// For specific shortcut, we don't dispatch message.
							bReturn = TRUE;
						}
					}
				}
				
				if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyDown ) )
				{
					switch( pMsg->wParam )
					{
						case VK_ESCAPE:
							if( true == m_rBlockSelectionParams.m_fIsActivated )
							{
								BS_CancelSelection();
							}
							break;
					}
					MM_PM_SSheet_KeyboardVirtualKeyDown( GetSafeHwnd(), pMsg->wParam );
				}

				// To not have trouble with our own navigation management, we don't let these key to be managed by TSpread.
				if( VK_NEXT == pMsg->wParam || VK_PRIOR == pMsg->wParam )
				{
					bReturn = TRUE;
				}

				break;

			case WM_KEYUP:

				if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyUp ) )
				{
					MM_PM_SSheet_KeyboardVirtualKeyUp( GetSafeHwnd(), pMsg->wParam );
				}

				break;

			case WM_MOUSEWHEEL:
				
				if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_MouseWheel ) )
				{
					MM_PM_SSheet_MouseWheel( GetSafeHwnd(), GET_WHEEL_DELTA_WPARAM( pMsg->wParam ) );
				}

				break;

			case WM_CHAR:

				if( false == m_fEditionEnabled )
				{
					// When we are not in edit mode, we don't dispatch message.
					bReturn = TRUE;
				}
				
				break;
		}
	}
	else
	{
		if( WM_USER_DELETETSPREADINSTANCE == pMsg->message )
		{
			if( this == (CSSheet*)pMsg->wParam )
			{
				DestroyWindow();
				delete this;
				return TRUE;
			}
		}
	}

	if( FALSE == bReturn )
	{
		bReturn = TSpread::PreTranslateMessage( pMsg );
	}
	
	return bReturn;
}

//////////////////////////////////////////////////////////////////////
// CSSheet private members
//////////////////////////////////////////////////////////////////////

void CSSheet::_ConvertSSStoSSStyle( long &lStyle )
{
	if( SSS_ALIGN_TOP == ( lStyle & SSS_ALIGN_TOP ) )
		lStyle |= SS_TEXT_TOP;
	if( SSS_ALIGN_VCENTER == ( lStyle & SSS_ALIGN_VCENTER ) )
		lStyle |= SS_TEXT_VCENTER;
	if( SSS_ALIGN_BOTTOM == ( lStyle & SSS_ALIGN_BOTTOM ) )
		lStyle |= SS_TEXT_BOTTOM;
	if( SSS_ALIGN_LEFT == ( lStyle & SSS_ALIGN_LEFT ) )
		lStyle |= SS_TEXT_LEFT;
	if( SSS_ALIGN_CENTER == ( lStyle & SSS_ALIGN_CENTER ) )
		lStyle |= SS_TEXT_CENTER;
	if( SSS_ALIGN_RIGHT == ( lStyle & SSS_ALIGN_RIGHT ) )
		lStyle |= SS_TEXT_RIGHT;
	lStyle &= ~( SSS_ALIGN_TOP | SSS_ALIGN_VCENTER | SSS_ALIGN_BOTTOM | SSS_ALIGN_LEFT | SSS_ALIGN_CENTER | SSS_ALIGN_RIGHT );
}

void CSSheet::_ConvertSStoSSSStyle( long &lStyle )
{
	if( SS_TEXT_TOP == ( lStyle & SS_TEXT_TOP ) )
	{
		lStyle |= SSS_ALIGN_TOP;
	}

	if( SS_TEXT_VCENTER == ( lStyle & SS_TEXT_VCENTER ) )
	{
		lStyle |= SSS_ALIGN_VCENTER;
	}

	if( SS_TEXT_BOTTOM == ( lStyle & SS_TEXT_BOTTOM ) )
	{
		lStyle |= SSS_ALIGN_BOTTOM;
	}

	if( SS_TEXT_LEFT == ( lStyle & SS_TEXT_LEFT ) )
	{
		lStyle |= SSS_ALIGN_LEFT;
	}

	if( SS_TEXT_CENTER == ( lStyle & SS_TEXT_CENTER ) )
	{
		lStyle |= SSS_ALIGN_CENTER;
	}

	if( SS_TEXT_RIGHT == ( lStyle & SS_TEXT_RIGHT ) )
	{
		lStyle |= SSS_ALIGN_RIGHT;
	}

	lStyle &= ~( SS_TEXT_TOP | SS_TEXT_VCENTER | SS_TEXT_BOTTOM | SS_TEXT_LEFT | SS_TEXT_CENTER | SS_TEXT_RIGHT );
}

void CSSheet::_ClearBitmapContainer( short nSheet )
{
	if( m_mapBitmapList.size() > 0 )
	{
		// Run all sheet available.
		for( mapBitmapListIter mapIter = m_mapBitmapList.begin(); mapIter != m_mapBitmapList.end(); ++mapIter )
		{
			if( mapIter->second.size() > 0 && ( -1 == nSheet || ( -1 != nSheet && mapIter->first == nSheet ) ) )
			{
				for( vecBitmapListIter vecIter = mapIter->second.begin(); vecIter != mapIter->second.end(); ++vecIter )
				{
					if( NULL != (*vecIter).m_pclBitmap )
					{
						delete (*vecIter).m_pclBitmap;
					}
				}
				
				mapIter->second.clear();
			}
		}
		
		if( -1 == nSheet )
		{
			m_mapBitmapList.clear();
		}
	}

	m_mapImageToImageSelected.clear();
	m_mapImageSelectedToImage.clear();
}

void CSSheet::_SetColumnFlag( long lFromColumn, long lToColumn, long lFlagValue, bool fSet )
{
	for( long lLoopCol = lFromColumn; lLoopCol <= lToColumn; lLoopCol++ )
	{
		// Remark: 'GetColUserData' returns 'FALSE' if user data is not yet been set
		LONG_PTR lUserData = 0;

		if( TRUE == GetColUserData( lLoopCol, &lUserData ) )
		{
			if( true == fSet )
			{
				lUserData |= lFlagValue;
			}
			else 
			{
				lUserData &= ~lFlagValue;
			}

			SetColUserData( lLoopCol, lUserData );
		}
		else if( true == fSet )
		{
			SetColUserData( lLoopCol, lFlagValue );
		}
	}
}

void CSSheet::_SetRowFlag(long lFromRow, long lToRow, long lFlagValue, bool bSet )
{
	for( long lLoopRow = lFromRow; lLoopRow <= lToRow; lLoopRow++ )
	{
		// Remark: 'GetRowUserData' returns 'FALSE' if user data is not yet been set
		LONG_PTR lUserData = 0;

		if( TRUE == GetRowUserData( lLoopRow, &lUserData ) )
		{
			if( true == bSet )
			{
				lUserData |= lFlagValue;
			}
			else 
			{
				lUserData &= ~lFlagValue;
			}

			SetRowUserData( lLoopRow, lUserData );
		}
		else if( true == bSet )
		{
			SetRowUserData( lLoopRow, lFlagValue );
		}
	}
}

void CSSheet::_ClearAllSelectedRows( mapLongVec* pmapSelectedRows )
{
	// Just to clear all variables (this method does not clear the selection!!).

	if( NULL == pmapSelectedRows )
	{
		if( m_mapSelectedRowsBySheet.size() > 0 )
		{
			// Runs all sheet.
			for( mapShortMapLongVecIter iterSheet = m_mapSelectedRowsBySheet.begin(); iterSheet != m_mapSelectedRowsBySheet.end(); ++iterSheet )
			{
				mapLongVec* pmapSelectedRows = &iterSheet->second;
				if( NULL == pmapSelectedRows )
					continue;
				while( pmapSelectedRows->size() > 0 )
				{
					mapLongVecIter iter = pmapSelectedRows->begin();
					while( iter->second.size() > 0 )
					{
						arCellProperties *parCellProperty = iter->second.back();
						parCellProperty->RemoveAll();
						delete parCellProperty;
						iter->second.pop_back();
					}
					pmapSelectedRows->erase( iter );
				}
			}
		}
	}
	else
	{
		while( pmapSelectedRows->size() > 0 )
		{
			mapLongVecIter iter = pmapSelectedRows->begin();
			while( iter->second.size() > 0 )
			{
				arCellProperties *parCellProperty = iter->second.back();
				parCellProperty->RemoveAll();
				delete parCellProperty;
				iter->second.pop_back();
			}
			pmapSelectedRows->erase( iter );
		}
	}
}

long CSSheet::_GetLastColNotHidden( void )
{
	long lLastCol = 0;
	for( long lLoop = GetMaxCols(); lLoop > 0 && 0 == lLastCol ; lLoop-- )
	{
		if( FALSE == IsColHidden( lLoop ) )
			lLastCol = lLoop;
	}
	return lLastCol;
}

long CSSheet::_GetLastRowNotHidden( void )
{
	long lLastRow = 0;
	for( long lLoop = GetMaxRows(); lLoop > 0 && 0 == lLastRow ; lLoop-- )
	{
		if( FALSE == IsRowHidden( lLoop ) )
			lLastRow = lLoop;
	}
	return lLastRow;
}

void CSSheet::_ManageOnLButtonDown( UINT nFlags, CPoint point )
{
	TSpread::OnLButtonDown( nFlags, point );

	long lLButtonDownCol;
	long lLButtonDownRow;
	GetCellFromPixel( &lLButtonDownCol, &lLButtonDownRow, point.x, point.y );

	if( lLButtonDownCol > 0 && lLButtonDownRow > 0 && true == IsCellProperty( lLButtonDownCol, lLButtonDownRow, _SSCellProperty::CellCantLeftClick ) )
	{
		return;
	}

	if( true == _BS_CheckAfterMouseEvent( point ) )
	{
		m_rBlockSelectionParams.m_fIsRunning = true;
		// 'SetCapture' allows to force mouse messages to be sent only to TSpread even if pointer goes out of client area.
		::SetCapture( GetSafeHwnd() );
	}

	if( true == m_rRowSelectionParams.m_fIsActivated && true == m_rRowSelectionParams.m_fMousePointerChanged )
	{
		m_rRowSelectionParams.m_lStartRowSelection = lLButtonDownRow;
		m_rRowSelectionParams.m_lEndRowSelection = lLButtonDownRow;
		m_rRowSelectionParams.m_fIsRunning = true;
		SetBool( SSB_REDRAW, FALSE );
		UnSelectMultipleRows();
		SelectMutipleRows( m_rRowSelectionParams.m_lStartRowSelection, m_rRowSelectionParams.m_lStartRowSelection, 0, -1, m_rRowSelectionParams.m_BackgroundColor );
		SetBool( SSB_REDRAW, TRUE );
		// 'SetCapture' allows to force mouse messages to be sent only to TSpread even if pointer goes out of client area.
		::SetCapture( GetSafeHwnd() );
	}

	if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDown ) )
	{
		MM_PM_SSheet_MouseLButtonDown( GetSafeHwnd(), lLButtonDownCol, lLButtonDownRow, point );
	}
}

void CSSheet::_ManageOnLButtonDblClk( UINT nFlags, CPoint point )
{
	if( false == m_bInitialized )
		return;

	TSpread::OnLButtonDblClk( nFlags, point );

	if( false == m_fEditionEnabled )
		SetEditMode( FALSE );

	long lLButtonDblClkCol, lLButtonDblClkRow;
	GetCellFromPixel( &lLButtonDblClkCol, &lLButtonDblClkRow, point.x, point.y );
	if( lLButtonDblClkCol > 0 && lLButtonDblClkRow > 0 && true == IsCellProperty( lLButtonDblClkCol, lLButtonDblClkRow, _SSCellProperty::CellCantLeftDoubleClick ) )
		return;

	_BS_CheckAfterMouseEvent( point );

	if( true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDblClk ) )
	{
		MM_PM_SSheet_MouseLButtonDblClk( GetSafeHwnd(), lLButtonDblClkCol, lLButtonDblClkRow, point );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage block selection.
void CSSheet::_BS_DoSelection( void )
{
	BOOL fRedrawState = GetBool( SSB_REDRAW );
	if( TRUE == fRedrawState )
		SetBool( SSB_REDRAW, FALSE );

	_BS_UndoSelection();

	// Second, do the new selection.
	long lRowStart = ( m_rBlockSelectionParams.m_lRowStartSelection <= m_rBlockSelectionParams.m_lRowEndSelection ) ? m_rBlockSelectionParams.m_lRowStartSelection : m_rBlockSelectionParams.m_lRowEndSelection;
	long lRowEnd = ( m_rBlockSelectionParams.m_lRowEndSelection >= m_rBlockSelectionParams.m_lRowStartSelection ) ? m_rBlockSelectionParams.m_lRowEndSelection : m_rBlockSelectionParams.m_lRowStartSelection;
	long lColStart = ( m_rBlockSelectionParams.m_lColumnStartSelection <= m_rBlockSelectionParams.m_lColumnEndSelection ) ? m_rBlockSelectionParams.m_lColumnStartSelection : m_rBlockSelectionParams.m_lColumnEndSelection;
	long lColEnd = ( m_rBlockSelectionParams.m_lColumnEndSelection >= m_rBlockSelectionParams.m_lColumnStartSelection ) ? m_rBlockSelectionParams.m_lColumnEndSelection : m_rBlockSelectionParams.m_lColumnStartSelection;
	BSBorderSaveParams rBorderSaveParams;
	for( rBorderSaveParams.m_lRow = lRowStart; rBorderSaveParams.m_lRow <= lRowEnd; rBorderSaveParams.m_lRow++ )
	{
		WORD wBorderType = SS_BORDERTYPE_NONE;
		if( rBorderSaveParams.m_lRow == lRowStart )
			wBorderType |= SS_BORDERTYPE_TOP;
		if( rBorderSaveParams.m_lRow == lRowEnd )
			wBorderType |= SS_BORDERTYPE_BOTTOM;
		
		for( rBorderSaveParams.m_lColumn = lColStart; rBorderSaveParams.m_lColumn <= lColEnd; rBorderSaveParams.m_lColumn++ )
		{
			wBorderType &= ~( SS_BORDERTYPE_LEFT | SS_BORDERTYPE_RIGHT );
			if( rBorderSaveParams.m_lColumn == lColStart )
				wBorderType |= SS_BORDERTYPE_LEFT;
			if( rBorderSaveParams.m_lColumn == lColEnd )
				wBorderType |= SS_BORDERTYPE_RIGHT;

			if( 0 != wBorderType )
			{
				// Save current border.
				GetBorderEx( rBorderSaveParams.m_lColumn, rBorderSaveParams.m_lRow, &rBorderSaveParams.m_wStyleLeft, &rBorderSaveParams.m_ColorLeft, 
					&rBorderSaveParams.m_wStyleTop, &rBorderSaveParams.m_ColorTop, &rBorderSaveParams.m_wStyleRight, &rBorderSaveParams.m_ColorRight,
					&rBorderSaveParams.m_wStyleBottom, &rBorderSaveParams.m_ColorBottom );
				m_rBlockSelectionParams.m_vecBorderSave.push_back( rBorderSaveParams );

				// Set the new border.
				SetBorder( rBorderSaveParams.m_lColumn, rBorderSaveParams.m_lRow, wBorderType, m_rBlockSelectionParams.m_wBorderStyle, m_rBlockSelectionParams.m_BorderColor );
			}
		}
	}

	// To be able to directly edit a cell.
	// Remark: only if there is no more that one cell selected!
	if( -1 != m_rBlockSelectionParams.m_lOriginColumn && -1 != m_rBlockSelectionParams.m_lOriginRow &&
		m_rBlockSelectionParams.m_lColumnStartSelection == m_rBlockSelectionParams.m_lColumnEndSelection && 
		m_rBlockSelectionParams.m_lRowStartSelection == m_rBlockSelectionParams.m_lRowEndSelection && 
		m_rBlockSelectionParams.m_lOriginColumn == m_rBlockSelectionParams.m_lColumnStartSelection && 
		m_rBlockSelectionParams.m_lOriginRow == m_rBlockSelectionParams.m_lRowStartSelection )
	{
		SetActiveCell( m_rBlockSelectionParams.m_lOriginColumn, m_rBlockSelectionParams.m_lOriginRow );
	}

	if( TRUE == fRedrawState )
		SetBool( SSB_REDRAW, TRUE );
}

void CSSheet::_BS_UndoSelection()
{
	if( false == m_rBlockSelectionParams.m_fIsActivated || false == BS_IsSelectionExist() )
		return;

	// First restore previous border if exist.
	if( m_rBlockSelectionParams.m_vecBorderSave.size() > 0 )
	{
		BOOL fRedrawState = GetBool( SSB_REDRAW );
		if( TRUE == fRedrawState )
			SetBool( SSB_REDRAW, FALSE );
		for( std::vector<BSBorderSaveParams>::iterator iter = m_rBlockSelectionParams.m_vecBorderSave.begin(); iter != m_rBlockSelectionParams.m_vecBorderSave.end(); ++iter )
		{
			SetBorder( iter->m_lColumn, iter->m_lRow, SS_BORDERTYPE_LEFT, iter->m_wStyleLeft, iter->m_ColorLeft );
			SetBorder( iter->m_lColumn, iter->m_lRow, SS_BORDERTYPE_TOP, iter->m_wStyleTop, iter->m_ColorTop );
			SetBorder( iter->m_lColumn, iter->m_lRow, SS_BORDERTYPE_RIGHT, iter->m_wStyleRight, iter->m_ColorRight );
			SetBorder( iter->m_lColumn, iter->m_lRow, SS_BORDERTYPE_BOTTOM, iter->m_wStyleBottom, iter->m_ColorBottom );
		}
		m_rBlockSelectionParams.m_vecBorderSave.clear();
		if( TRUE == fRedrawState )
			SetBool( SSB_REDRAW, TRUE );
	}
}

bool CSSheet::_BS_CheckAfterMouseEvent( CPoint point )
{
	bool fReturn = false;
	if( true == m_rBlockSelectionParams.m_fIsActivated )
	{
		_BS_UndoSelection();
		m_rBlockSelectionParams.ClearSelection();

		// Determine col and row.
		long lClickedCol, lClickedRow;
		GetCellFromPixel( &lClickedCol, &lClickedRow, point.x, point.y );
		if( lClickedCol > 0 && lClickedRow > 0 )
		{
			// We are in the case where user has clicked inside TSpread.
			if( false == IsCellProperty( lClickedCol, lClickedRow, _SSCellProperty::CellCantBlockSelect ) )
			{
				m_rBlockSelectionParams.m_lColumnStartSelection = lClickedCol;
				m_rBlockSelectionParams.m_lRowStartSelection = lClickedRow; 
				m_rBlockSelectionParams.m_lColumnEndSelection = lClickedCol;
				m_rBlockSelectionParams.m_lRowEndSelection = lClickedRow;

				m_rBlockSelectionParams.m_lOriginColumn = lClickedCol;
				m_rBlockSelectionParams.m_lOriginRow = lClickedRow;

				_BS_DoSelection();
				fReturn = true;
			}
		}
		else
		{
			// We are in the case where user has clicked on the grayed area of TSpread.
			long lGoodCol = lClickedCol;
			long lGoodRow = lClickedRow;
			if( lClickedCol < 0 && lClickedRow > 0 )
			{
				// Find the good column where we can do the block selection.
				lGoodCol = GetMaxCols();
				bool fFound = false;
				while( false == fFound && lGoodCol > 0 )
				{
					if( false == IsCellProperty( lGoodCol, lClickedRow, _SSCellProperty::CellCantBlockSelect ) &&
						FALSE == IsColHidden( lGoodCol ) )
					{
						fFound = true;
					}
					else
					{
						lGoodCol--;
					}
				}
				if( true == fFound )
				{
					m_rBlockSelectionParams.m_lColumnStartSelection = lGoodCol;
					m_rBlockSelectionParams.m_lRowStartSelection = lClickedRow; 
					m_rBlockSelectionParams.m_lColumnEndSelection = lGoodCol;
					m_rBlockSelectionParams.m_lRowEndSelection = lClickedRow;

					m_rBlockSelectionParams.m_lOriginColumn = lGoodCol;
					m_rBlockSelectionParams.m_lOriginRow = lClickedRow;

					_BS_DoSelection();
					fReturn = true;
				}
			}
			else if( lClickedCol > 0 && lClickedRow < 0 )
			{
				// Find the good row where we can do the block selection.
				lGoodRow = GetMaxRows();
				bool fFound = false;
				while( false == fFound && lGoodRow > 0 )
				{
					if( false == IsCellProperty( lClickedCol, lGoodRow, _SSCellProperty::CellCantBlockSelect ) &&
						FALSE == IsRowHidden( lGoodRow ) )
					{
						fFound = true;
					}
					else
					{
						lGoodRow--;
					}
				}
				if( true == fFound )
				{
					m_rBlockSelectionParams.m_lColumnStartSelection = lClickedCol;
					m_rBlockSelectionParams.m_lRowStartSelection = lGoodRow; 
					m_rBlockSelectionParams.m_lColumnEndSelection = lClickedCol;
					m_rBlockSelectionParams.m_lRowEndSelection = lGoodRow;

					m_rBlockSelectionParams.m_lOriginColumn = lClickedCol;
					m_rBlockSelectionParams.m_lOriginRow = lGoodRow;

					_BS_DoSelection();
					fReturn = true;
				}
			}
			else
			{
				lGoodCol = GetMaxCols();
				lGoodRow = GetMaxRows();
				bool fFound = false;
				while( false == fFound && lGoodCol > 0 )
				{
					if( false == IsCellProperty( lGoodCol, lGoodRow, _SSCellProperty::CellCantBlockSelect ) &&
						FALSE == IsColHidden( lGoodCol ) && FALSE == IsRowHidden( lGoodRow ) )
					{
						fFound = true;
					}
					else
					{
						if( TRUE == IsColHidden( lGoodCol ) || TRUE == IsRowHidden( lGoodRow ) )
						{
							if( TRUE == IsColHidden( lGoodCol ) )
								lGoodCol--;
							if( TRUE == IsRowHidden( lGoodRow ) )
								lGoodRow--;
						}
						else
						{
							lGoodRow--;
							if( 0 == lGoodRow )
							{
								lGoodCol--;
								lGoodRow = GetMaxRows();
							}
						}
					}
				}
				if( true == fFound )
				{
					m_rBlockSelectionParams.m_lColumnStartSelection = lGoodCol;
					m_rBlockSelectionParams.m_lRowStartSelection = lGoodRow; 
					m_rBlockSelectionParams.m_lColumnEndSelection = lGoodCol;
					m_rBlockSelectionParams.m_lRowEndSelection = lGoodRow;

					m_rBlockSelectionParams.m_lOriginColumn = lGoodCol;
					m_rBlockSelectionParams.m_lOriginRow = lGoodRow;

					_BS_DoSelection();
					fReturn = true;
				}
			}
		}
	}
	return fReturn;
}

BOOL CSSheet::_BS_CheckKeyboardDownEvent( int iVirtualKey )
{
	if( false == m_rBlockSelectionParams.m_fIsActivated || 
		( VK_LEFT != iVirtualKey && VK_UP != iVirtualKey && VK_RIGHT != iVirtualKey && VK_DOWN != iVirtualKey && VK_HOME != iVirtualKey && VK_END != iVirtualKey ) )
		return FALSE;
	if( false == BS_IsSelectionExist() )
		return FALSE;

	// Remark concerning 'm_lColumnStartSelection' and 'm_lColumnEndSelection'.
	// If [shift] key is pressed while selecting, we do a large selection block. 'm_lColumnStartSelection' and 'm_lColumnEndSelection' are the absolute positions
	// of the respective start and end of the block selection. if 'm_lColumnStartSelection' is the same as 'm_lOriginColumn', that means the block selection is
	// currently increasing or decreasing by the right. In the opposite, if 'm_lColumnEndSelection' is the same as 'm_lOriginColumn', that means the block 
	// selection is currently increasing or decreasing by the left.

	// Important note about 'ShowCell'. If a cell is partially visible and we use its coordinate with 'ShowCell' we are not sure of the result 
	// of the scrolling. Keep in mind that TSpread scrolls down/up or right/left and ensures that cells in the left and top part of the client edge are 
	// full visible. To correctly position, use only the SS_SHOW_TOPLEFT!

	// For sending message.
	long lOldCol = m_rBlockSelectionParams.m_lOriginColumn;
	long lOldRow = m_rBlockSelectionParams.m_lOriginRow;
	long lNewCol = -1;
	long lNewRow = -1;

	bool fDoSelection = false;
	bool fShiftPressed = ( GetKeyState( VK_SHIFT ) < 0 ) ? true : false;
	bool fCtrlPressed = ( GetKeyState( VK_CONTROL ) < 0 ) ? true : false;
	if( false == fShiftPressed )
	{
		m_rBlockSelectionParams.m_lColumnStartSelection = m_rBlockSelectionParams.m_lOriginColumn;
		m_rBlockSelectionParams.m_lColumnEndSelection = m_rBlockSelectionParams.m_lOriginColumn;
		m_rBlockSelectionParams.m_lRowStartSelection = m_rBlockSelectionParams.m_lOriginRow;
		m_rBlockSelectionParams.m_lRowEndSelection = m_rBlockSelectionParams.m_lOriginRow;
		fDoSelection = true;
	}

	bool fFound = false;
	if( VK_RIGHT == iVirtualKey || VK_LEFT == iVirtualKey )
	{
		long lColumn;
		// See remark at the entry of this method concerning this condition.
		if( m_rBlockSelectionParams.m_lColumnStartSelection == m_rBlockSelectionParams.m_lOriginColumn )
			lColumn = ( VK_RIGHT == iVirtualKey ) ? m_rBlockSelectionParams.m_lColumnEndSelection + 1 : m_rBlockSelectionParams.m_lColumnEndSelection - 1;
		else
			lColumn = ( VK_RIGHT == iVirtualKey ) ? m_rBlockSelectionParams.m_lColumnStartSelection + 1 : m_rBlockSelectionParams.m_lColumnStartSelection - 1;

		while( false == fFound && ( ( VK_RIGHT == iVirtualKey && lColumn <= GetMaxCols() ) || ( VK_LEFT == iVirtualKey && lColumn > 0 ) ) )
		{
			if( false == IsCellProperty( lColumn, m_rBlockSelectionParams.m_lRowStartSelection, _SSCellProperty::CellCantBlockSelect ) &&
				FALSE == IsColHidden( lColumn ) )
			{
				if( false == fShiftPressed )
				{
					m_rBlockSelectionParams.m_lColumnStartSelection = lColumn;
					m_rBlockSelectionParams.m_lColumnEndSelection = lColumn;
					m_rBlockSelectionParams.m_lOriginColumn = lColumn;
				}
				else
				{
					if( lColumn < m_rBlockSelectionParams.m_lOriginColumn )
						m_rBlockSelectionParams.m_lColumnStartSelection = lColumn;
					else if( lColumn > m_rBlockSelectionParams.m_lOriginColumn )
						m_rBlockSelectionParams.m_lColumnEndSelection = lColumn;
					else
					{
						m_rBlockSelectionParams.m_lColumnStartSelection = lColumn;
						m_rBlockSelectionParams.m_lColumnEndSelection = lColumn;
					}
				}
				lNewCol = lColumn;
				fFound = true;
			}
			else
			{
				lColumn += ( VK_RIGHT == iVirtualKey ) ? 1 : -1;
			}
		}
	}
	else if( VK_DOWN == iVirtualKey || VK_UP == iVirtualKey )
	{
		long lRow;
		// See remark at the entry of this method concerning this condition.
		if( m_rBlockSelectionParams.m_lRowStartSelection == m_rBlockSelectionParams.m_lOriginRow )
			lRow = ( VK_DOWN == iVirtualKey ) ? m_rBlockSelectionParams.m_lRowEndSelection + 1 : m_rBlockSelectionParams.m_lRowEndSelection - 1;
		else
			lRow = ( VK_DOWN == iVirtualKey ) ? m_rBlockSelectionParams.m_lRowStartSelection + 1 : m_rBlockSelectionParams.m_lRowStartSelection - 1;
		
		while( false == fFound && ( ( VK_DOWN == iVirtualKey && lRow <= GetMaxRows() ) || ( VK_UP == iVirtualKey && lRow > 0 ) ) )
		{
			if( false == IsCellProperty( m_rBlockSelectionParams.m_lColumnStartSelection, lRow, _SSCellProperty::CellCantBlockSelect ) &&
				FALSE == IsRowHidden( lRow ) )
			{
				if( false == fShiftPressed )
				{
					m_rBlockSelectionParams.m_lRowStartSelection = lRow;
					m_rBlockSelectionParams.m_lRowEndSelection = lRow;
					m_rBlockSelectionParams.m_lOriginRow = lRow;
				}
				else
				{
					if( lRow < m_rBlockSelectionParams.m_lOriginRow )
						m_rBlockSelectionParams.m_lRowStartSelection = lRow;
					else if( lRow > m_rBlockSelectionParams.m_lOriginRow )
						m_rBlockSelectionParams.m_lRowEndSelection = lRow;
					else
					{
						m_rBlockSelectionParams.m_lRowStartSelection = lRow;
						m_rBlockSelectionParams.m_lRowEndSelection = lRow;
					}
				}
				lNewRow = lRow;
				fFound = true;
			}
			else
			{
				lRow += ( VK_DOWN == iVirtualKey ) ? 1 : -1;
			}
		}
	}
	else if( VK_HOME == iVirtualKey || VK_END == iVirtualKey )
	{
		fFound = false;
		long lColumn = ( VK_END == iVirtualKey ) ? GetMaxCols() : 1;
		bool fStop = false;
		while( false == fFound && false == fStop )
		{
			if( false == IsCellProperty( lColumn, m_rBlockSelectionParams.m_lRowStartSelection, _SSCellProperty::CellCantBlockSelect ) &&
				FALSE == IsColHidden( lColumn ) )
			{
				if( false == fShiftPressed )
				{
					m_rBlockSelectionParams.m_lColumnStartSelection = lColumn;
					m_rBlockSelectionParams.m_lColumnEndSelection = lColumn;
					m_rBlockSelectionParams.m_lOriginColumn = lColumn;
				}
				else
				{
					if( lColumn < m_rBlockSelectionParams.m_lOriginColumn )
						m_rBlockSelectionParams.m_lColumnStartSelection = lColumn;
					else if( lColumn > m_rBlockSelectionParams.m_lOriginColumn )
						m_rBlockSelectionParams.m_lColumnEndSelection = lColumn;
					else
					{
						m_rBlockSelectionParams.m_lColumnStartSelection = lColumn;
						m_rBlockSelectionParams.m_lColumnEndSelection = lColumn;
					}
				}
				lNewCol = lColumn;
				fFound = true;
			}
			else
			{
				lColumn += ( VK_HOME == iVirtualKey ) ? 1 : -1;
				fStop = ( VK_HOME == iVirtualKey ) ? ( lColumn < 1 ) : ( lColumn > GetMaxCols() );
			}
		}

		if( true == fFound && true == fCtrlPressed )
		{
			// Special case when [Ctrl] key is pressed. [Ctrl]+[Home] goes to the top left part of the spread. [Ctrl]+[End] goes to the
			// bottom right part of the spread.
			
			// Column is already determine, we must now check what is the row.
			fFound = false;
			long lRow = ( VK_END == iVirtualKey ) ? GetMaxRows() : 1;
			bool fStop = false;
			while( false == fFound && false == fStop )
			{
				if( false == IsCellProperty( m_rBlockSelectionParams.m_lColumnStartSelection, lRow, _SSCellProperty::CellCantBlockSelect ) &&
					FALSE == IsRowHidden( lRow ) )
				{
					if( false == fShiftPressed )
					{
						m_rBlockSelectionParams.m_lRowStartSelection = lRow;
						m_rBlockSelectionParams.m_lRowEndSelection = lRow;
						m_rBlockSelectionParams.m_lOriginRow = lRow;
					}
					else
					{
						if( lRow < m_rBlockSelectionParams.m_lOriginRow )
							m_rBlockSelectionParams.m_lRowStartSelection = lRow;
						else if( lRow > m_rBlockSelectionParams.m_lOriginRow )
							m_rBlockSelectionParams.m_lRowEndSelection = lRow;
						else
						{
							m_rBlockSelectionParams.m_lRowStartSelection = lRow;
							m_rBlockSelectionParams.m_lRowEndSelection = lRow;
						}
					}
					lNewRow = lRow;
					fFound = true;
				}
				else
				{
					lRow += ( VK_HOME == iVirtualKey ) ? 1 : -1;
					fStop = ( VK_HOME == iVirtualKey ) ? ( lRow > GetMaxRows() ) : ( lRow < 1 );
				}
			}
		}
	}

	if( true == fDoSelection || true == fFound )
	{
		_BS_DoSelection();

		if( true == fFound )
		{
			BOOL fVisibleAll;
			if( -1 == lNewCol )
				fVisibleAll = GetVisible( m_rBlockSelectionParams.m_lOriginColumn, lNewRow, SS_VISIBLE_ALL );
			else
				fVisibleAll = GetVisible( lNewCol, m_rBlockSelectionParams.m_lOriginRow, SS_VISIBLE_ALL );
			if( FALSE == fVisibleAll )
			{
				if( VK_DOWN == iVirtualKey && lNewRow >= m_rBlockSelectionParams.m_lOriginRow )
				{
					// Down and selection is being expanded.
					long lTopLeftCol, lTopLeftRow;
					GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );
					ShowCell( lTopLeftCol, lNewRow, SS_SHOW_BOTTOMLEFT );
				}
				else if( VK_UP == iVirtualKey && lNewRow <= m_rBlockSelectionParams.m_lOriginRow )
				{
					// Up and selection is being expanded.
					long lTopLeftCol, lTopLeftRow;
					GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );
					ShowCell( lTopLeftCol, lNewRow, SS_SHOW_TOPLEFT );
				}
				else if( VK_RIGHT == iVirtualKey && lNewCol >= m_rBlockSelectionParams.m_lOriginColumn )
				{
					// Right and selection is being expanded.
					long lTopLeftCol, lTopLeftRow;
					GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );
					ShowCell( lNewCol, lTopLeftRow, SS_SHOW_TOPRIGHT );
				}
				else if( VK_LEFT == iVirtualKey && lNewCol <= m_rBlockSelectionParams.m_lOriginColumn )
				{
					// Right and selection is being expanded.
					long lTopLeftCol, lTopLeftRow;
					GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );
					ShowCell( lNewCol, lTopLeftRow, SS_SHOW_TOPLEFT );
				}
			}

			if( false == fShiftPressed && true == MM_IsNotificationHandlerRegistered( CMessageManager::SSheetNHFlags::SSheetNHF_NavigateOccurs ) )
				MM_PM_SSheet_NavigationOccurs( GetSafeHwnd(), lOldCol, lOldRow, lNewCol, lNewRow );
		}
		else
		{
			// Here we are at the end of the spread in the current direction. We force then to go to the client edge.
			if( VK_RIGHT == iVirtualKey || VK_LEFT == iVirtualKey )
			{			
				long lColumn;
				// See remark at the entry of this method concerning this condition.
				if( m_rBlockSelectionParams.m_lColumnStartSelection == m_rBlockSelectionParams.m_lOriginColumn )
					lColumn = ( VK_RIGHT == iVirtualKey ) ? m_rBlockSelectionParams.m_lColumnEndSelection + 1 : m_rBlockSelectionParams.m_lColumnEndSelection - 1;
				else
					lColumn = ( VK_RIGHT == iVirtualKey ) ? m_rBlockSelectionParams.m_lColumnStartSelection + 1 : m_rBlockSelectionParams.m_lColumnStartSelection - 1;
				BOOL fVisibleAll = GetVisible( lColumn, m_rBlockSelectionParams.m_lOriginRow, SS_VISIBLE_ALL );
				if( FALSE == fVisibleAll )
				{
					if( VK_LEFT == iVirtualKey )
					{
						// Check if there is yet a horizontal scrolling possible on the left.
						long lTopCellCol, lTopCellRow;
						GetTopLeftCell( &lTopCellCol, &lTopCellRow );
						// Force to go totally to the left.
						ShowCell( 1, lTopCellRow, SS_SHOW_TOPLEFT );
					}
					else
					{
						// Check if there is yet a horizontal scrolling possible on the right.
						long lTopCellCol, lTopCellRow;
						GetTopLeftCell( &lTopCellCol, &lTopCellRow );
						// Force to go totally to the right.
						ShowCell( GetMaxCols(), lTopCellRow, SS_SHOW_TOPRIGHT );
					}
				}
			}
			else if( VK_DOWN == iVirtualKey || VK_UP == iVirtualKey )
			{
				long lRow;
				// See remark at the entry of this method concerning this condition.
				if( m_rBlockSelectionParams.m_lRowStartSelection == m_rBlockSelectionParams.m_lOriginRow )
					lRow = ( VK_DOWN == iVirtualKey ) ? m_rBlockSelectionParams.m_lRowEndSelection + 1 : m_rBlockSelectionParams.m_lRowEndSelection - 1;
				else
					lRow = ( VK_DOWN == iVirtualKey ) ? m_rBlockSelectionParams.m_lRowStartSelection + 1 : m_rBlockSelectionParams.m_lRowStartSelection - 1;
				BOOL fVisibleAll = GetVisible( m_rBlockSelectionParams.m_lOriginColumn, lRow, SS_VISIBLE_ALL );
				if( FALSE == fVisibleAll )
				{
					if( VK_UP == iVirtualKey )
					{
						// Check if there is yet a vertical scrolling possible on the top.
						long lTopCellCol, lTopCellRow;
						GetTopLeftCell( &lTopCellCol, &lTopCellRow );
						// Force to go totally to the top.
						ShowCell( lTopCellCol, 1, SS_SHOW_NEAREST );
					}
					else
					{
						// Check if there is yet a vertical scrolling possible on the bottom.
						long lTopCellCol, lTopCellRow;
						GetTopLeftCell( &lTopCellCol, &lTopCellRow );
						// Force to go totally to the bottom.
						ShowCell( lTopCellCol, GetMaxRows(), SS_SHOW_NEAREST );
					}
				}
			}
		}
	}

	BOOL fReturnValue;
	if( VK_HOME == iVirtualKey || VK_END == iVirtualKey )
	{
		// We return 'FALSE' in case of [Home] and [End] keys to allow TSpread to manage scrolling itself.
		fReturnValue = FALSE;
	}
	else
		fReturnValue = TRUE;
	return fReturnValue;
}

void CSSheet::_BS_CheckOnMouseMove( CPoint point )
{
	WaitForSingleObject( m_hOnTimerEvent, INFINITE );
	ResetEvent( m_hOnTimerEvent );

	if( true == m_rBlockSelectionParams.m_fIsActivated && true == m_rBlockSelectionParams.m_fIsRunning )
	{
		// Determine col and row.
		long lMouseOverCol, lMouseOverRow;
		GetCellFromPixel( &lMouseOverCol, &lMouseOverRow, point.x, point.y );

		CRect rectTSpread;
		GetClientRect( &rectTSpread );

		long lTopLeftCol;
		long lTopLeftRow;
		GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );

		long lBottomRightCol;
		long lBottomRightRow;
		GetBottomRightCell( &lBottomRightCol, &lBottomRightRow );

		long lRowsFreeze = GetRowsFreeze();
		long lColsFreeze = GetColsFreeze();

		// Remark: define 'DEBUG_BLOCKSELECTION_MOUSESCROLLING' is just above the 'OnTimer' method.
		#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
		CString str;
		OutputDebugString( _T("_BS_CheckOnMouseMove starting...\n") );
		str.Format( _T("MOC: %i; MOR: %i; TLC: %i; TLR: %i; BRC: %i; BRR: %i\n"), lMouseOverCol, lMouseOverRow, lTopLeftCol, lTopLeftRow, lBottomRightCol, lBottomRightRow );
		OutputDebugString( str );
		str.Format( _T("OC:%i; OR:%i; CSS: %i; CES: %i; RSS: %i; RES: %i\n"), m_rBlockSelectionParams.m_lOriginColumn, m_rBlockSelectionParams.m_lOriginRow, m_rBlockSelectionParams.m_lColumnStartSelection, m_rBlockSelectionParams.m_lColumnEndSelection, m_rBlockSelectionParams.m_lRowStartSelection, m_rBlockSelectionParams.m_lRowEndSelection );
		OutputDebugString( str );
		#endif
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Check first what is the selection to do.
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Check the column.
		long lColToSelect = m_rBlockSelectionParams.m_lColumnEndSelection;
		if( lMouseOverCol != m_rBlockSelectionParams.m_lColumnEndSelection )
		{
			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
 			OutputDebugString( _T("\tlMouseOverCol != m_rBlockSelectionParams.m_lColumnEndSelection\n") );
			#endif
			
			if( lMouseOverCol < 0 )
			{
				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
 				OutputDebugString( _T("\t\tlMouseOverCol < 0\n") );
				#endif
				
				if( point.x <= rectTSpread.left )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tpoint.x <= rectTSpread.right\n") );
					#endif;
					
					lColToSelect = lTopLeftCol;
				}
				else
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tpoint.x > rectTSpread.right\n") );
					#endif
					
					lColToSelect = lBottomRightCol;
				}
			}
			else
			{
				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tlMouseOverCol >= 0\n") );
				#endif
				
				// If mouse cursor is over a row frozen, we must take the top left row. Otherwise we can take the row on which is the mouse cursor.
				if( lMouseOverCol <= lColsFreeze )
					lColToSelect = lTopLeftCol;
				else
					lColToSelect = lMouseOverCol;
			}
			int iStep = ( lColToSelect < m_rBlockSelectionParams.m_lColumnEndSelection ) ? 1 : -1;
			
			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
			str.Format( _T("\tlColToSelect: %i; iStep: %i\n"), lColToSelect, iStep );
			OutputDebugString( str );
			#endif
			
			do
			{
				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				str.Format( _T("\t\tCell property(%i,%i)\n"), lColToSelect, m_rBlockSelectionParams.m_lRowEndSelection );
				OutputDebugString( str );
				#endif

				if( false == IsCellProperty( lColToSelect, m_rBlockSelectionParams.m_lRowEndSelection, _SSCellProperty::CellCantBlockSelect ) )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tCan block select -> break\n") );
					#endif

					break;
				}
				else
				{
					lColToSelect += iStep;

					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tCan't block select -> next column\n") );
					str.Format( _T("\t\t\tlColToSelect: %i\n"), lColToSelect );
					OutputDebugString( str );
					#endif
				}
			}while( lColToSelect != m_rBlockSelectionParams.m_lColumnEndSelection );

			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
			str.Format( _T("\t\tWhile finished ... lColToSelect: %i\n"), lColToSelect );
			OutputDebugString( str );
			#endif
		}

		#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
		OutputDebugString( _T("\n") );
		#endif

		// Check the row.
		long lRowToSelect = m_rBlockSelectionParams.m_lRowEndSelection;
		if( lMouseOverRow != m_rBlockSelectionParams.m_lRowEndSelection )
		{
			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
			OutputDebugString( _T("\tlMouseOverRow != m_rBlockSelectionParams.m_lRowEndSelection\n") );
			#endif

			if( lMouseOverRow < 0 )
			{
				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tlMouseOverRow < 0\n") );
				#endif

				if( point.y <= rectTSpread.top )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tpoint.y <= rectTSpread.top\n") );
					#endif

					lRowToSelect = lTopLeftRow;
				}
				else
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tpoint.y > rectTSpread.top\n") );
					#endif

					lRowToSelect = lBottomRightRow;
				}
			}
			else
			{
				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tlMouseOverRow >= 0\n") );
				#endif

				// If mouse cursor is over a row frozen, we must take the top left row. Otherwise we can take the row on which is the mouse cursor.
				if( lMouseOverRow <= lRowsFreeze )
					lRowToSelect = lTopLeftRow;
				else
					lRowToSelect = lMouseOverRow;
			}
			int iStep = ( lRowToSelect < m_rBlockSelectionParams.m_lRowEndSelection ) ? 1 : -1;
			
			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
			str.Format( _T("\tlRowToSelect: %i; iStep: %i\n"), lRowToSelect, iStep );
			OutputDebugString( str );
			#endif
			
			do
			{
				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				str.Format( _T("\t\tCell property(%i,%i)\n"), m_rBlockSelectionParams.m_lColumnEndSelection, lRowToSelect );
				OutputDebugString( str );
				#endif

				if( false == IsCellProperty( m_rBlockSelectionParams.m_lColumnEndSelection, lRowToSelect, _SSCellProperty::CellCantBlockSelect ) )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tCan block select -> break\n") );
					#endif

					break;
				}
				else
				{
					lRowToSelect += iStep;

					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tCan't block select -> next row\n") );
					str.Format( _T("\t\t\tlRowToSelect: %i\n"), lRowToSelect );
					OutputDebugString( str );
					#endif
				}
			}while( lRowToSelect != m_rBlockSelectionParams.m_lRowEndSelection );

			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
			str.Format( _T("\t\tWhile finished ... lRowToSelect: %i\n"), lRowToSelect );
			OutputDebugString( str );
			#endif
		}
		m_rBlockSelectionParams.m_lColumnEndSelection = lColToSelect;
		m_rBlockSelectionParams.m_lRowEndSelection = lRowToSelect;
		
		#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
		OutputDebugString( _T("\n") );
		str.Format( _T("Selection: %i, %i\n"), lColToSelect, lRowToSelect );
		OutputDebugString( str );
		#endif

		_BS_DoSelection();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Now check if we must do a vertical and/or horizontal scrolling.
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// If mouse pointer moves outside the TSpread client area...
		if( lMouseOverCol < 0 || lMouseOverRow < 0 )
		{
			#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
			OutputDebugString( _T("\tlMouseOverCol < 0 || lMouseOverRow < 0\n") );
			str.Format( _T("\tScroll start: %s"), ( true == m_rBlockSelectionParams.m_fScrollStart ) ? _T("true") : _T("false") );
			OutputDebugString( str );
			str.Format( _T("; Vertical scrolling: ") );
			if( eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollDown )
				str += _T("undefined");
			else if( eBool3::eb3True == m_rBlockSelectionParams.m_b3ScrollDown )
				str += _T("down");
			else if( eBool3::eb3False == m_rBlockSelectionParams.m_b3ScrollDown )
				str += _T("up");
			str += _T("; Horizontal scrolling: ");
			if( eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollRight )
				str += _T("undefined\n");
			else if( eBool3::eb3True == m_rBlockSelectionParams.m_b3ScrollRight )
				str += _T("right\n");
			else if( eBool3::eb3False == m_rBlockSelectionParams.m_b3ScrollRight )
				str += _T("left\n");
			OutputDebugString( str );
			#endif

			CRect rectTSpread;
			GetClientRect( &rectTSpread );

			// If rows are concerned and either there is no scroll started or started but with horizontal scrolling...
			if( lMouseOverRow < 0 &&
				( false == m_rBlockSelectionParams.m_fScrollStart || eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollDown ) )
			{
				if( point.y >= rectTSpread.bottom )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\tCASE 1 (point.y >= rectTSpread.bottom)\n") );
					#endif

					// Mouse pointer moves lower than the bottom of the TSpread client area.
					// We check if we can start a scrolling down with the timer.

					// By default, don't scroll down.
					bool fCanScroll = false;

					// If the last visible row is lower or equal to the max rows...
					if( lBottomRightRow <= GetMaxRows() )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tlBottomRightRow <= GetMaxRows()\n") );
						#endif

						if( lBottomRightRow < GetMaxRows() )
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tlBottomRightRow < GetMaxRows() -> scroll is true\n") );
							#endif
							
							// In this case, that means there is yet some rows after, we can thus begin to scroll down.
							fCanScroll = true;
						}
						else
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tlBottomRightRow = GetMaxRows()\n") );
							#endif

							// If the last visible row is the last row of TSpread, we check if it is completely visible.
							// If this cell is not fully visible, we can thus begin to scroll down.
							// PAY ATTENTION: last column can be hidden (user has called the 'TSpread::ShowCol' with 'FALSE' as argument).
							//                in this case, 'GetVisible' returns 'FALSE'. We must choose a visible column to test the row.
							long lLastColNotHidden = _GetLastColNotHidden();

							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							str.Format( _T("\t\t\t\tlLastColNotHidden: %i\n"), lLastColNotHidden );
							OutputDebugString( str );
							#endif

							if( lLastColNotHidden > 0 && FALSE == GetVisible( lLastColNotHidden, lBottomRightRow, SS_VISIBLE_ALL ) &&
								FALSE == IsRowHidden( lBottomRightRow ) )
							{
								fCanScroll = true;

								#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
								OutputDebugString( _T("\t\t\t\tlLastColNotHidden > 0 && FALSE == GetVisible( lLastColNotHidden, lBottomRightRow, SS_VISIBLE_ALL ) \n") );
								#endif
							}
							else
							{
								#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
								OutputDebugString( _T("\t\t\t\tlLastColNotHidden <=0 || TRUE == GetVisible... \n") );
								#endif
							}
						}
					}

					if( true == fCanScroll )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tCan scroll up\n") );
						#endif

						m_rBlockSelectionParams.m_b3ScrollDown = eBool3::eb3True;
						// Set timer only if it is not already active.
						if( false == m_rBlockSelectionParams.m_fScrollStart )
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tfalse == m_rBlockSelectionParams.m_fScrollStart -> timer is not running\n") );
							#endif

							m_rBlockSelectionParams.m_fScrollStart = true;
							m_rBlockSelectionParams.m_nTimer = SetTimer( _TIMERID_SHEETSELAUTOSCROLL, _SS_BLOCKSELECTIONSCROLLTIME, NULL );
						}
						else
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\ttrue == m_rBlockSelectionParams.m_fScrollStart -> timer is aldready running\n") );
							#endif
						}
					}
					else
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tCan't scroll up\n") );
						#endif
					}
				}
				else if( point.y <= rectTSpread.top )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\tCASE 2 (point.y <= rectTSpread.top)\n") );
					#endif

					// Mouse pointer moves higher than the top of the TSpread client area.
					// We check if we can start a scrolling up with the timer.

					if( lTopLeftRow > GetRowsFreeze() + 1 )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tlTopLeftRow > GetRowsFreeze() + 1\n") );
						#endif

						m_rBlockSelectionParams.m_b3ScrollDown = eBool3::eb3False;
						// Set timer only if it is not already active.
						if( false == m_rBlockSelectionParams.m_fScrollStart )
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tfalse == m_rBlockSelectionParams.m_fScrollStart -> timer is not running\n") );
							#endif

							m_rBlockSelectionParams.m_fScrollStart = true;
							m_rBlockSelectionParams.m_nTimer = SetTimer( _TIMERID_SHEETSELAUTOSCROLL, _SS_BLOCKSELECTIONSCROLLTIME, NULL );
						}
						else
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tfalse == m_rBlockSelectionParams.m_fScrollStart -> timer is aldready running\n") );
							#endif
						}
					}
					else
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tlTopLeftRow <= GetRowsFreeze() + 1\n") );
						#endif
					}
				}
			}
			else if( lMouseOverRow > 1 && true == m_rBlockSelectionParams.m_fScrollStart && eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollDown )
			{
				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tCASE 3 Stop the vertical scrolling\n") );
				#endif

				// Mouse pointer moves on a row greater than the first row and lower than the bottom of TSpread client area.
				// In this case, we stop the scrolling.
				// Remark: only if no horizontal scrolling is currently active.
				m_rBlockSelectionParams.m_b3ScrollDown = eBool3::eb3Undef;
				if( eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollRight )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tNo horizontal scrolling ... stop timer\n") );
					#endif

					KillTimer( m_rBlockSelectionParams.m_nTimer );
					m_rBlockSelectionParams.m_nTimer = (UINT_PTR)0;
					m_rBlockSelectionParams.m_fScrollStart = false;
				}
				else
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tHorizontal scrolling ... don't stop timer\n") );
					#endif
				}
			}
			
			// If columns are concerned and either there is no scroll started or started but with vertical scrolling...
			if( lMouseOverCol < 0 && ( point.x <= rectTSpread.left || point.x >= rectTSpread.right ) && 
				( false == m_rBlockSelectionParams.m_fScrollStart || eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollRight ) )
			{
				if( point.x >= rectTSpread.right )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\tCASE 4 (point.x >= rectTSpread.right)\n") );
					#endif

					// Mouse pointer moves away from the right edge of the TSpread client area.
					// We check if we can start a scrolling right with the timer.

					// By default, don't scroll right.
					bool fCanScroll = false;
					
					// If the last visible column is lower or equal to the max columns...
					if( lBottomRightCol <= GetMaxCols() )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tlBottomRightCol <= GetMaxCols()\n") );
						#endif

						if( lBottomRightCol < GetMaxCols() )
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tlBottomRightCol < GetMaxCols() -> scroll is true\n") );
							#endif

							// In this case, that means there is yet some columns after, we can thus begin to scroll right.
							fCanScroll = true;
						}
						else
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tlBottomRightCol = GetMaxCols()\n") );
							#endif

							// If the last visible column is the last column of TSpread, we check if it is completely visible.
							// If this cell is not fully visible, we can thus begin to scroll right.
							// PAY ATTENTION: last column can be hidden (user has called the 'TSpread::ShowCol' with 'FALSE' as argument).
							//                in this case, 'GetVisible' returns 'FALSE'. We must choose a visible column to test the row.
							long lLastRowNotHidden = _GetLastRowNotHidden();

							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							str.Format( _T("\t\t\t\tlLastRowNotHidden: %i\n"), lLastRowNotHidden );
							OutputDebugString( str );
							#endif

							if( lLastRowNotHidden > 0 && FALSE == GetVisible( lBottomRightCol, lLastRowNotHidden, SS_VISIBLE_ALL ) &&
								FALSE == IsColHidden( lBottomRightCol ) )
							{
								fCanScroll = true;

								#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
								OutputDebugString( _T("\t\t\t\tlLastRowNotHidden > 0 && FALSE == GetVisible( lBottomRightCol, lLastRowNotHidden, SS_VISIBLE_ALL ) \n") );
								#endif
							}
							else
							{
								#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
								OutputDebugString( _T("\t\t\t\tlLastRowNotHidden <=0 || TRUE == GetVisible... \n") );
								#endif
							}
						}
					}

					if( true == fCanScroll )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tCan scroll right\n") );
						#endif

						m_rBlockSelectionParams.m_b3ScrollRight = eBool3::eb3True;
						// Set timer only if it is not already active.
						if( false == m_rBlockSelectionParams.m_fScrollStart )
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tfalse == m_rBlockSelectionParams.m_fScrollStart -> timer is not running\n") );
							#endif

							m_rBlockSelectionParams.m_fScrollStart = true;
							m_rBlockSelectionParams.m_nTimer = SetTimer( _TIMERID_SHEETSELAUTOSCROLL, _SS_BLOCKSELECTIONSCROLLTIME, NULL );
						}
						else
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\ttrue == m_rBlockSelectionParams.m_fScrollStart -> timer is aldready running\n") );
							#endif
						}
					}
					else
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tCan't scroll right\n") );
						#endif
					}
				}
				else if( point.x <= rectTSpread.left )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\tCASE 5 (point.x <= rectTSpread.left)\n") );
					#endif

					// Mouse pointer moves far away from the left edge of the TSpread client area.
					// We check if we can start a scrolling left with the timer.
					long lTopLeftCol;
					long lTopLeftRow;
					GetTopLeftCell( &lTopLeftCol, &lTopLeftRow );
					if( lTopLeftCol > GetColsFreeze() + 1 )
					{
						#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
						OutputDebugString( _T("\t\t\tlTopLeftCol > GetColsFreeze() + 1\n") );
						#endif

						m_rBlockSelectionParams.m_b3ScrollRight = eBool3::eb3False;
						// Set timer only if it is not already active.
						if( false == m_rBlockSelectionParams.m_fScrollStart )
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tfalse == m_rBlockSelectionParams.m_fScrollStart -> timer is not running\n") );
							#endif

							m_rBlockSelectionParams.m_fScrollStart = true;
							m_rBlockSelectionParams.m_nTimer = SetTimer( _TIMERID_SHEETSELAUTOSCROLL, _SS_BLOCKSELECTIONSCROLLTIME, NULL );
						}
						else
						{
							#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
							OutputDebugString( _T("\t\t\t\tfalse == m_rBlockSelectionParams.m_fScrollStart -> timer is already runnig\n") );
							#endif
						}
					}
				}
			}
			else if( lMouseOverCol > 1 && true == m_rBlockSelectionParams.m_fScrollStart && eBool3::eb3Undef != m_rBlockSelectionParams.m_b3ScrollRight )
			{
				#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
				OutputDebugString( _T("\t\tCASE 6 Stop the horizontal scrolling\n") );
				#endif

				// Mouse pointer moves on a column greater than the first one and lower than the last one.
				// In this case, we stop the scrolling.
				// Remark: only if no vertical scrolling is currently active.
				m_rBlockSelectionParams.m_b3ScrollRight = eBool3::eb3Undef;
				if( eBool3::eb3Undef == m_rBlockSelectionParams.m_b3ScrollDown )
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tNo vertical scrolling ... stop timer\n") );
					#endif

					KillTimer( m_rBlockSelectionParams.m_nTimer );
					m_rBlockSelectionParams.m_nTimer = (UINT_PTR)0;
					m_rBlockSelectionParams.m_fScrollStart = false;
				}
				else
				{
					#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
					OutputDebugString( _T("\t\t\tVertical scrolling ... don't stop timer\n") );
					#endif
				}
			}
		}

		#ifdef DEBUG_BLOCKSELECTION_MOUSESCROLLING
		OutputDebugString( _T("_BS_CheckOnMouseMove ending.\n\n") );
		#endif
	}

	SetEvent( m_hOnTimerEvent );
}

// End of methods that manage block selection.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods that manage row selection.

void CSSheet::_RS_CheckOnMouseMove( CPoint point )
{
	if( true == m_rRowSelectionParams.m_fIsActivated && false == m_rBlockSelectionParams.m_fIsRunning )
	{
		// Determine col and row.
		bool fReverseCursor = true;
		long lMouseOverCol, lMouseOverRow;
		GetCellFromPixel( &lMouseOverCol, &lMouseOverRow, point.x, point.y );

		// If row selection is not yet engaged...
		if( false == m_rRowSelectionParams.m_fMousePointerChanged )
		{
			// ... then we check if we pass over the cell to change cursor into the right arrow.
			if( lMouseOverCol > 0 && lMouseOverRow > 0 )
			{
				CRect rectCellPixelPos = GetCellCoordInPixel( lMouseOverCol, lMouseOverRow );
				if( false == IsCellProperty( lMouseOverCol, lMouseOverRow, _SSCellProperty::CellCantRowSelect ) && lMouseOverCol == m_rRowSelectionParams.m_lColumnReference )
				{
					if( point.x - rectCellPixelPos.left < m_rRowSelectionParams.m_nPixelTolerance )
					{
						m_rRowSelectionParams.m_hDefaultCursor = GetCursor( SS_CURSORTYPE_DEFAULT );
						SetCursor( SS_CURSORTYPE_DEFAULT, m_rRowSelectionParams.m_hSelectionCursor );
						m_rRowSelectionParams.m_fMousePointerChanged = true;

						// Normally here, it is necessary to call 'TrackMouseEvent' to be alert when mouse pointer leaves the area client (with WM_MOUSELEAVE message).
						// In this way, it's possible to reset the default cursor. Here, it's not needed. Because in the 'SS_Proc' function of the 'SS_Main.c' of
						// TSpread, when event 'WM_MOUSEMOVE' is received, it call itself 'TrackMouseEvent'.
					}
				}
				fReverseCursor = false;
			}
		}
		else if( true == m_rRowSelectionParams.m_fIsRunning )
		{
			// In this case, we are already selecting rows.
			
			// If the current end row is different from the previous one...
			if( lMouseOverRow != m_rRowSelectionParams.m_lEndRowSelection )
			{
				if( lMouseOverRow > 0 )
				{
					if( false == IsCellProperty( m_rRowSelectionParams.m_lColumnReference, lMouseOverRow, _SSCellProperty::CellCantRowSelect ) )
					{
						SetBool( SSB_REDRAW, FALSE );
						UnSelectMultipleRows();
						m_rRowSelectionParams.m_lEndRowSelection = lMouseOverRow;
						SelectMutipleRows( m_rRowSelectionParams.m_lStartRowSelection, m_rRowSelectionParams.m_lEndRowSelection, 0, -1, m_rRowSelectionParams.m_BackgroundColor );
						SetBool( SSB_REDRAW, TRUE );
					}
				}
				else
				{
					// It means mouse pointer goes out of the TSpread client area. We need to find out the good row to select.
					long lStartRow, lEndRow, lStep;
					if( point.y <= 0 )
					{
						lStartRow = 1;
						lEndRow = GetMaxRows() + 1;
						lStep = 1;
					}
					else
					{
						lStartRow = GetMaxRows();
						lEndRow = 0;
						lStep = -1;
					}
					bool fStop = false;
					for( long lLoopRow = lStartRow; lLoopRow != lEndRow && false == fStop; lLoopRow += lStep )
					{
						if( false == IsCellProperty( m_rRowSelectionParams.m_lColumnReference, lLoopRow, _SSCellProperty::CellCantRowSelect ) )
						{
							if( lLoopRow != m_rRowSelectionParams.m_lEndRowSelection )
							{
								SetBool( SSB_REDRAW, FALSE );
								UnSelectMultipleRows();
								m_rRowSelectionParams.m_lEndRowSelection = lLoopRow;
								SelectMutipleRows( m_rRowSelectionParams.m_lStartRowSelection, m_rRowSelectionParams.m_lEndRowSelection, 0, -1, m_rRowSelectionParams.m_BackgroundColor );
								SetBool( SSB_REDRAW, TRUE );
							}
							fStop = true;
						}
					}
				}
			}

			// Don't reset cursor to the default.
			fReverseCursor = false;
		}

		if( true == fReverseCursor && true == m_rRowSelectionParams.m_fMousePointerChanged && false == m_rRowSelectionParams.m_fIsRunning )
		{
			bool fResetToDefault = false;
			if( lMouseOverCol > 0 && lMouseOverRow > 0 )
			{
				CRect rectCellPixelPos = GetCellCoordInPixel( lMouseOverCol, lMouseOverRow );
				if( lMouseOverCol != m_rRowSelectionParams.m_lColumnReference || point.x - rectCellPixelPos.left > m_rRowSelectionParams.m_nPixelTolerance || 
					true == IsCellProperty( lMouseOverCol, lMouseOverRow, _SSCellProperty::CellCantRowSelect ) )
				{
					fResetToDefault = true;
				}
			}
			else
			{
				// If any of both is negative, that means mouse pointer is out of the spread.
				fResetToDefault = true;
			}
			
			if( true == fResetToDefault )
			{
				SetCursor( SS_CURSORTYPE_DEFAULT, m_rRowSelectionParams.m_hDefaultCursor );
				m_rRowSelectionParams.m_fMousePointerChanged = false;
			}
		}
	}
}

// End of methods that manage row selection.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSSheet::_SaveBitmap( HDC hMemDC, HBITMAP hBitmap )
{
	int wBitCount = 32;

	BITMAP Bitmap;  
	BITMAPFILEHEADER bmfHdr;    
	BITMAPINFOHEADER bi;
 
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap); 
 
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
 
	HANDLE hDib = GlobalAlloc(GHND,dwBmBitsSize+sizeof(BITMAPINFOHEADER)); 
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);    
	*lpbi = bi;
 
	GetDIBits(hMemDC, hBitmap, 0, (UINT)Bitmap.bmHeight,  
	(LPSTR)lpbi + sizeof(BITMAPINFOHEADER), (BITMAPINFO *)lpbi, DIB_RGB_COLORS);    
 
	HANDLE fh = CreateFile( L"c:\\temp\\temp.bmp",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
	FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);   
	ASSERT(fh != INVALID_HANDLE_VALUE);
 
	DWORD dwDIBSize   =   sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmBitsSize;
 
	bmfHdr.bfType = 0x4D42;    
	bmfHdr.bfSize = dwDIBSize;  
	bmfHdr.bfReserved1 = 0;    
	bmfHdr.bfReserved2 = 0;    
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);    
 
	DWORD dwWritten;
 
	WriteFile(fh,   (LPSTR)&bmfHdr,   sizeof(BITMAPFILEHEADER),   &dwWritten,   NULL);    
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);    
	GlobalUnlock(hDib);    
	GlobalFree(hDib);    
	CloseHandle(fh);
}
