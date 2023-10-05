#pragma once

#include "SSheet.h"

class Excel_Workbook
{
	struct CellXfs
	{
		int fillId;
		int borderTypeId;
		int fontId;
		long halign;
		long valign;
	};

	struct DrawingParam
	{
		long fromCol;
		long fromRow;
		long toCol;
		long toRow;
		long cellWidth;
		long cellHeight;
		long pictWidth;
		long pictHeight;
	};

	struct FontParam
	{
		double fontSize;
		DWORD fontColor;
		bool bold;
	};

	struct BorderParam
	{
		WORD wStyleLeft = 0;
		WORD wStyleTop = 0;
		WORD wStyleRight = 0;
		WORD wStyleBottom = 0;
		COLORREF clrLeft = 0;
		COLORREF clrTop = 0;
		COLORREF clrRight = 0;
		COLORREF clrBottom = 0;
	};

public:
	// Constructors
	Excel_Workbook();

	// Public Methods
	int AddSheet( CSSheet *sheet );
	// HYS-436: To group some columns
	bool Write( const TCHAR *filePath, vector<int> *vectShownCols = NULL, bool bCustomSheet = false );

    // HYS-1786: Remove illegal XML characters from a string.
	string SanitizeXmlString( string xml );
	// Whether a given character is allowed by XML 1.0.
	bool IsLegalXmlChar( int character );

private:
	// Methods
	void CreateSubDirs();
	void CreateContentTypes();
	void CreateRels();
	void CreateCore();
	void CreateApp();
	void CreateXlStyles();

	void WriteVAlign( int i, FILE *fpStyles );

	void WriteHAlign( int i, FILE *fpStyles );

	void CreateXlWorkbook();
	void CreateXlRels();
	void CreateXlDrawings();
	void CreateSheets();
	// HYS-436: To group some columns
	void CreateCustomizedSheets( vector<int> vectShownCols );

	void SaveSheetName( CSSheet *currentSpread, int page );

	void CreateSheetRows( int maxRows, CSSheet *currentSpread, FILE *fpSheet, int maxCols, std::vector<std::string> &mergeCell );

	int AddStyle( COLORREF cellBg, COLORREF cellFg, double fontSize, BorderParam &rBorderParam, long halign, long valign, bool fontBold );
	int AddFill( DWORD color );

	void AddColsWidth( FILE *fpSheet, int maxCols, CSSheet *currentSpread );

	void ComputeMergeCell( CSSheet *currentSpread, int *cols, int *rows, std::vector<std::string> &mergeCell );
	void AddMergeCell( std::vector<std::string> &mergeCell, FILE *fpSheet );

	bool ZipOutput( const TCHAR *filePath );
	//uLong filetime( const std::wstring f, tm_zip *tmzip, uLong *dt );

	void WriteBorders( BorderParam &rBorderParam, FILE *fpStyles );
	int AddBorder( BorderParam &rBorderParam );
	void WriteBorder( int iType, int iStyle, COLORREF rColor, FILE *fpStyles );
	void CreateXlWorksheetsRels( int pageNum );

	int PicturesUsed();
	short PicturesStartAt( unsigned short pageZC );
	void CreateXlDrawingsRels();
	int AddFont( double fontSize, COLORREF cellFg, bool bold );

// Private variables.
private:
	TCHAR m_tempPath[MAX_PATH + 1];
	TCHAR m_xlPath[MAX_PATH + 1];
	TCHAR m_xlrelsPath[MAX_PATH + 1];
	TCHAR m_xlworksheetsPath[MAX_PATH + 1];
	TCHAR m_xlworksheetsRelsPath[MAX_PATH + 1];
	TCHAR m_xlmediaPath[MAX_PATH + 1];
	TCHAR m_xldrawingsPath[MAX_PATH + 1];
	TCHAR m_xldrawingsRelsPath[MAX_PATH + 1];
	TCHAR m_docPropsPath[MAX_PATH + 1];
	TCHAR m_relsPath[MAX_PATH + 1];
	std::vector<CSSheet *> m_vSheet;
	std::vector<std::wstring> m_vSheetName;
	std::vector<DWORD> m_vColor;
	std::vector<BorderParam> m_vBorderType;
	std::vector<CellXfs> m_vCellXfs;
	std::vector<DrawingParam> m_vDrawingParam;
	std::vector<FontParam> m_vFontParam;
	short m_sheetCount;
	std::vector<short> m_vPictureCount;

};