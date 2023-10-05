#pragma once

#define A4WIDTH		0x1296
#define A4HEIGHT	0x1A90
#define RIGHTVIEWWIDTH		900
#define RIGHTVIEWHEIGHT		600

class CSheet;
class CDrawSet
{
public:
	CDrawSet(void);
	~CDrawSet(void);

	void Init(CDC* pDC);

	// Draw a bitmap at x,y coordinates.
	// If pRect exist adjust bitmap size to the pRect Size
	// If DllName exist, try to load with BmpID from this DLL
	// If DllName doesn't exist, try to load in program resources with BmpID in BMP format
	// If fIsGIF is set to true, try to load in program resources with BmpID and GIF format
	// return coordinates in pRect
	void DrawBmp( CDC* pDC, int x, int y, CString DllName, int BmpID, bool fIsGIF, CRect *pRect, bool DrawNow = true );

	// Draw Header, if logo exist adjust text on the right of the logo.
	// Page number is on the page right side.
	int DrawHeader(CDC* pDC, int x, int y, int page, bool DrawNow=true);
	
	// Draw Footer, if logo exist adjust text on the Left of the logo.
	// returned y is the coordinate y of the upper left corner
	int DrawFooter(CDC* pDC, int x, int y,int page, bool DrawNow=true);
	int Getmm2pt(CDC* pDC, double mm, bool vertical=true);
	
	// Draw Title 'title' on the pDC at position x,y
	//	Use : text font is m_TitleFont; text color is m_TitleColor
	//		  rectangle drawn throughout the page width 
	//        with background color m_TitleBGColor
	//	Return bottom y coordinate or -1 if there is not enough space
	int DrawTitle(CDC* pDC, int x, int y, CString title, COLORREF BGcolor, bool DrawNow=true );
	
	//Draw SubTitle 'title' on the pDC at position x,y
	//	Use : text font is m_SubTitleFont; text color is m_SubTitleColor
	//		  rectangle drawn on page width/2 
	//        with background color m_SubTitleBGColor
	//	Return bottom y coordinate
	int DrawSubTitle(CDC* pDC, int x, int y, CString title, bool DrawNow=true);
	
	int DrawTextLine(CDC* pDC,int &x, int y, int maxx, CString str, bool DrawNow=true);
	
	// Draw Project information return the position 
	int DrawPrjInfo(CDC* pDC,int x, int y, bool DrawNow=true);
	
	// Just date and project name
	int DrawShortPrjInfo(CDC* pDC,int x, int y, bool DrawNow=true);
	
	// Draw all logged data Informations used by CRightViewLogData and CRightViewChart 
	int DrawLogInfo( CDC* pDC, CLog *pLD, int x, int y, int width = RIGHTVIEWWIDTH, bool DrawNow = true ); 
	int DrawHMInfo( CDC* pDC, CDS_HydroMod *pHM, int x, int y, int width, bool DrawNow, bool bMeasInfo = false, bool bPrint = false );
	// HYS-1090: Draw one HM info with multiline management
	int DrawHMLineInfo( CDC * pDC, int x, int y, int width1, int width2, CString str1, CString str2, bool DrawNow );

	// Device Dependant Bitmap to Device Independent Bitmap conversion
	HANDLE DDBToDIB( CBitmap& bitmap, DWORD dwCompression, CPalette* pPal );

	CPoint PixelsToTwipsPt( CDC* pDC, CPoint &Pt );	// Convert one point from pixel to twips and retrieve the coordinations of this point
	void PixelsToTwips( CDC* pDC, CRect &rect );
	CPoint TwipsToPixelsPt( CDC* pDC, CPoint &Pt );	// Convert one point from pixel to twips and retrieve the coordinations of this point
	void TwipsToPixels( CDC* pDC, CRect &rect );
	void PixelsToInch( CDC* pDC, CRect &rect, float &leftInch, float &topInch, float &rightInch, float &bottomInch );

	// Allow to retrieve all margins in inches
	void GetMargins( CDC* pDC, CRect &rectMargin );

	// Allow to copy bitmap between two incompatible device context
	void CopyImage( HBITMAP hBitmap, HDC hDCSource, HDC hDCDest, int xDest, int yDest );
		
protected:
	void _CreateFont( CDC* pDC, CFont &clFont, int iSize, int nWeight, CString clFontName );

private:	
	CRect _DrawTagline( CDC* pDC, bool fDrawNow );
	CRect DrawTALogo( CDC* pDC, bool bDrawNow );
	// HYS-1090: Draw custo logo
	CRect DrawCustoLogo( CDC* pDC, bool bDrawNow );

public:
	UINT			m_CurrentPage;
	int				m_CurPrintPos,m_CurPrintIndex;
	bool			m_bContinuePrinting,m_bPrintThisPage;
	CFont			m_TitleFont,m_SubTitleFont,m_Text10,m_Text11,m_TextBold;
	CFont			m_PrintTitleFont,m_PrintSubTitleFont,m_PrintText10,m_PrintText11,m_PrintTextBold;
	CBrush			m_BrushWindow,m_BrushSelect;
	COLORREF		m_TitleBGColor,m_SubTitleBGColor;
	COLORREF		m_TitleColor,m_SubTitleColor,m_StringColor;
	int				m_iLeftMargin,m_iTopMargin;
	int				m_iLineSpace,m_iVerticalSpace;

	CTADatastruct	*m_pTADS;
	CDB_PageSetup	*m_pTADSPageSetup;
	bool			m_fRedraw;
	int				m_Xfactor;
	CRect			m_PrintRect;
};
