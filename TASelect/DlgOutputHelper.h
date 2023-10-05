#pragma once

namespace DlgOutputHelper
{
	typedef enum PasteDataError
	{
		PDE_Ok,
		PDE_BadRowStart,
		PDE_BadColStart,
		PDE_TooMuchCols,
		PDE_NoDataToPaste,
		PDE_CanPasteData,			// All columns where to paste data have the 'CanPasteDate' property set to false.
		PDE_ClipInError,
		PDE_NoDialog,
		PDE_CantInit
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Classes to help initialize and use of output.
	class CFontDef
	{
	public:
		typedef enum HorzAlign
		{
			HA_Left,
			HA_Center,
			HA_Right
		};
		typedef enum VertAlign
		{
			VA_Top,
			VA_Center,
			VA_Bottom
		};
		enum
		{
			BoldEnabled = true,
			BoldDisabled = false,
			ItalicEnabled = true,
			ItalicDisabled = false,
			UnderlineEnabled = true,
			UnderlineDisabled = false
		};
		CFontDef();
		CFontDef( const CFontDef& clFontDef );
		CFontDef( CString strFontName, int iSize, HorzAlign eHorzAlign, VertAlign eVertAlign, bool fBold = false, bool fItalic = false, bool fUnderline = false, 
				COLORREF backColor = _WHITE, COLORREF foreColor = _BLACK );
		CFontDef &operator=( const CFontDef &clFontDef );
		
		// Setter.
		void SetAllParams( CString strFontName, int iSize, HorzAlign eHorzAlign, VertAlign eVertAlign, bool fBold = false, bool fItalic = false, bool fUnderline = false, 
				COLORREF backColor = _WHITE, COLORREF foreColor = _BLACK );
		void SetFontName( CString strFontName ) { m_strFontName = strFontName; }
		void SetSize( int iSize ) { m_iSize = iSize; }
		void SetHorzAlign( HorzAlign eHorzAlign );
		void SetVertAlign( VertAlign eVertAlign );
		void SetBold( bool fBold ) { m_fBold = fBold; }
		void SetItalic( bool fItalic ) { m_fItalic = fItalic; }
		void SetUnderline( bool fUnderline ) { m_fUnderline = fUnderline; }
		void SetTextBackgroundColor( COLORREF textBackgroundColor ) { m_TextBackgroundColor = textBackgroundColor; }
		void SetTextForegroundColor( COLORREF textForegroundColor ) { m_TextForegroundColor = textForegroundColor; }

		// Getter.
		CString GetFontName( void ) const { return m_strFontName; }
		int GetSize( void ) const { return m_iSize; }
		HorzAlign GetHorzAlign( void ) const;
		VertAlign GetVertAlign( void ) const;
		bool GetBold( void ) const { return m_fBold; }
		bool GetItalic( void ) const { return m_fItalic; }
		bool GetUnderline( void ) const { return m_fUnderline; }
		COLORREF GetTextBackgroundColor( void ) const { return m_TextBackgroundColor; }
		COLORREF GetTextForegroundColor( void ) const { return m_TextForegroundColor; }

		// For internal use.
		int GetInternalHorzAlign( void ) { return m_iHorzAlign; }
		int GetInternalVertAlign( void ) { return m_iVertAlign; }

	private:
		CString		m_strFontName;
		int			m_iSize;
		int			m_iHorzAlign;
		int			m_iVertAlign;
		bool		m_fBold;
		bool		m_fItalic;
		bool		m_fUnderline;
		COLORREF	m_TextForegroundColor;
		COLORREF	m_TextBackgroundColor;
	};

	class CColDef
	{
	public:
		typedef enum
		{
			Undefined = -1,
			Text = 0,
			Number,
			Bitmap,
			Param
		}ContentType;
		enum
		{
			AutoResizeEnabled = true,
			AutoResizeDisabled = false,
			Visible = true,
			NotVisible = false,
			MouseEventEnabled = true,
			MouseEventDisabled = false,
			SelectionEnabled = true,
			SelectionDisabled = false,
			BlockSelectionEnabled = true,
			BlockSelectionDisabled = false,
			RowSelectionEnabled = true,
			RowSelectionDisabled = false,
			PasteDataEnabled = true,
			PasteDataDisabled = false,
			CopyDataEnabled = true,
			CopyDataDisabled = false,
			EditEnabled = true,
			EditDisabled = false
		};
		CColDef();
		CColDef( const CColDef& clColDef );
		CColDef( ContentType eContentType, double dWidth, int iMainHeaderID = -1, int iMainHeaderSubID = -1, bool fIsAutoResized = false, bool fIsVisible = true, bool fMouseEventFlag = false, 
			bool fSelectionFlag = false, bool fBlockSelectionFlag = false, bool fRowSelectionFlag = false, bool fCanPasteData = false, bool fCanCopyData = false, bool fEditable = false );
		CColDef &operator=( const CColDef &clColDef );
		
		// Setter.
		void SetAllParams( ContentType eContentType, double dWidth, int iMainHeaderID = -1, int iMainHeaderSubID = -1, bool fIsAutoResized = false, bool fIsVisible = true, bool fMouseEventFlag = false, 
			bool fSelectionFlag = false, bool fBlockSelectionFlag = false, bool fRowSelectionFlag = false, bool fCanPasteData = false, bool fCanCopyData = false, bool fEditable = false );
		void SetContentType( ContentType eContentType ) { m_eContentType = eContentType; }
		void SetWidth( double dWidth ) { m_dWidth = dWidth; }
		void SetWidthInPixels( long lWidthInPixels ) { m_lWidthInPixels = lWidthInPixels; }
		void SetMainHeaderLinks( int iMainHeaderID, int iMainHeaderSubID ) { m_iMainHeaderID = iMainHeaderID; m_iMainHeaderSubID = iMainHeaderSubID; }
		void SetAutoResized( bool fIsResizable ) { m_fAutoResized = fIsResizable; }
		void SetVisible( bool fIsVisible) { m_fIsVisible = fIsVisible; }
		void SetMouseEventFlag( bool fMouseEventFlag ) { m_fMouseEventAllowed = fMouseEventFlag; }
		void SetSelectionFlag( bool fSelectionFlag ) { m_fSelectionAllowed = fSelectionFlag; }
		void SetBlockSelectionFlag( bool fBlockSelectableFlag ) { m_fBlockSelectionAllowed = fBlockSelectableFlag; }
		void SetRowSelectionFlag( bool fRowSelectionFlag ) { m_fRowSelectionAllowed = fRowSelectionFlag; }
		void SetPasteDataFlag( bool fCanPasteData ) { m_fCanPasteData = fCanPasteData; }
		void SetCopyDataFlag( bool fCanCopyData ) { m_fCanCopyData = fCanCopyData; }
		void SetEditableFlag( bool fEditableFlag ) { m_fEditAllowed = fEditableFlag; }
		void SetFontDef( CFontDef clFontDef ) { m_clFontDef = clFontDef; }
		// Special methods when 'ContentType' is a number.
		void SetPhysicalType( int iPhysicalType ) { m_iPhysicalType = iPhysicalType; }
		void SetMaxDigit( int iMaxDigit ) { m_iMaxDigit = iMaxDigit; }
		void SetMinDecimal( int iMinDecimal ) { m_iMinDecimal = iMinDecimal; }
		// Special methods for border.
		void SetRowSeparatorFlag( bool fEnable, COLORREF clColor ) { m_fEnableRowSeparator = fEnable; m_clRowSeparatorColor = clColor; }
		void SetColSeparatorFlag( bool fEnable, COLORREF clColor ) { m_fEnableColSeparator = fEnable; m_clColSeparatorColor = clColor; }

		// Getter.
		ContentType GetContentType( void) const { return m_eContentType; }
		double GetWidth( void ) const { return m_dWidth; }
		long GetWidthInPixels( void ) const { return m_lWidthInPixels; }
		void GetMainHeaderLinks( int& iMainHeaderID, int& iMainHeaderSubID ) const { iMainHeaderID = m_iMainHeaderID; iMainHeaderSubID = m_iMainHeaderSubID; }
		bool GetAutoResized( void ) const { return m_fAutoResized; }
		bool GetVisible( void ) const { return m_fIsVisible; }
		bool IsMouseEventAllowed( void ) const { return m_fMouseEventAllowed; }
		bool IsSelectionAllowed( void ) const { return m_fSelectionAllowed; }
		bool IsBlockSelectionAllowed( void ) const { return m_fBlockSelectionAllowed; }
		bool IsRowSelectionAllowed( void ) const { return m_fRowSelectionAllowed; }
		bool IsPasteDataAllowed( void ) const { return m_fCanPasteData; }
		bool IsCopyDataAllowed( void ) const { return m_fCanCopyData; }
		bool IsEditAllowed( void ) const { return m_fEditAllowed; }
		CFontDef GetFontDef( void ) const { return m_clFontDef; }
		// Special methods when 'ContentType' is a number.
		int GetPhysicalType( void ) const { return m_iPhysicalType; }
		int GetMaxDigit( void ) const { return m_iMaxDigit; }
		int GetMinDecimal( void ) const { return m_iMinDecimal; }
		// Special methods for border.
		bool GetRowSeparatorFlag( void ) const { return m_fEnableRowSeparator; }
		COLORREF GetRowSeparatorColor( void ) const { return m_clRowSeparatorColor; }
		bool GetColSeparatorFlag( void ) const { return m_fEnableColSeparator; }
		COLORREF GetColSeparatorColor( void ) const { return m_clColSeparatorColor; }

		// For internal use.
		void SetInternalColPos( long lColPos ) { m_lColAbsolutePos = lColPos; }
		long GetInternalColPos( void ) const { return m_lColAbsolutePos; }

	private:
		ContentType m_eContentType;
		double m_dWidth;
		long m_lWidthInPixels;
		int m_iMainHeaderID;			// Allow to link column header with a main header.
		int m_iMainHeaderSubID;			// Allow to link column header with a sub main header in the main header identified by 'iMainHeaderID'.
		bool m_fAutoResized;			// 'true' when column can be automatically resized when client area is resized.
		bool m_fIsVisible;
		bool m_fMouseEventAllowed;
		bool m_fSelectionAllowed;
		bool m_fBlockSelectionAllowed;
		bool m_fRowSelectionAllowed;
		bool m_fColSelectionAllowed;
		bool m_fCanPasteData;
		bool m_fCanCopyData;
		bool m_fEditAllowed;
		CFontDef m_clFontDef;
		// For number type.
		int m_iPhysicalType;
		int m_iMaxDigit;
		int m_iMinDecimal;
		// For borders.
		bool m_fEnableRowSeparator;
		COLORREF m_clRowSeparatorColor;
		bool m_fEnableColSeparator;
		COLORREF m_clColSeparatorColor;

		// For internal use.
		long m_lColAbsolutePos;
	};

	enum TestOperator
	{
		lower, 
		equal, 
		higher, 
		different
	};

	class CCellBase
	{
	public:
		CCellBase( CCellBase &clCellBase );
		CCellBase( int iColumnID, CColDef::ContentType eContentType );
		virtual ~CCellBase() {}
		virtual CCellBase &operator=( const CCellBase &clCellBase );
		bool IsUndefined( void ) { return ( CColDef::ContentType::Undefined == m_eContentType ) ? true : false; }
		void SetColumnID( int iColumnID ) { m_iColumnID = iColumnID; }
		int GetColumnID( void ) const { return m_iColumnID; }
		CColDef::ContentType GetContentType( void ) const { return m_eContentType; }
		void SetInternalHelper( CString strData ) { m_strInternalHelper = strData; }
		CString GetInternalHelper( void ) const { return m_strInternalHelper; }
		virtual CString GetText( void ) const { return _T(""); }
		virtual int GetTextID( void ) const { return -1; }
		virtual ReadDoubleReturn_enum GetNumber( double& dValue ) const { return ReadDoubleReturn_enum::RD_EMPTY; }
		virtual int GetBitmapID( void ) const { return -1; }
		virtual LPARAM GetParam( void ) const { return (LPARAM)-1; }
		virtual bool CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator );
		virtual void Copy( CCellBase* pclCellBase );

		void SetTextBackgroundColor( COLORREF textBackgroundColor ) { m_TextBackgroundColor = textBackgroundColor; m_fTextBackColorDefined = true; }
		void SetTextForegroundColor( COLORREF textForegroundColor ) { m_TextForegroundColor = textForegroundColor; m_fTextForeColorDefined = true; }
		bool GetTextBackgroundColor( COLORREF& textBackgroundColor ) const;
		bool GetTextForegroundColor( COLORREF& textForegroundColor ) const;
	private:
		int m_iColumnID;
		CColDef::ContentType m_eContentType;
		CString		m_strInternalHelper;
		bool		m_fTextForeColorDefined;
		COLORREF	m_TextForegroundColor;
		bool		m_fTextBackColorDefined;
		COLORREF	m_TextBackgroundColor;
	};

	class CCellText : public CCellBase
	{
	public:
		CCellText();
		CCellText( CCellText &clCellText ) : CCellBase( clCellText ) { *this = clCellText; }
		CCellText( int iColumnID );
		CCellText( int iColumnID, CString strText ) : CCellBase( iColumnID, CColDef::Text ) { SetText( strText ); }
		CCellText( int iColumnID, int iStrID ) : CCellBase( iColumnID, CColDef::Text ) { SetText( iStrID ); }
		virtual CCellText &operator=( const CCellText &clCellText );
		void SetText( CString strText ) { m_strText = strText; m_iStrID = -1; }
		void SetText( int iStrID ) { m_iStrID = iStrID; m_strText = CString( _T("") ); }
		virtual CString GetText( void ) const;
		virtual int GetTextID( void ) const { return m_iStrID; }
		virtual bool CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator );
		virtual void Copy( CCellBase* pclCellBase );
	private:
		CString m_strText;
		int m_iStrID;
	};

	class CCellNumber : public CCellBase
	{
	public:
		CCellNumber();
		CCellNumber( CCellNumber &clCellNumber ) : CCellBase( clCellNumber ) { *this = clCellNumber; }
		CCellNumber( int iColumnID, int iPhysicalType = _U_NODIM, int iMaxDigit = -1, int iMinDecimal = -1 );
		CCellNumber( int iColumnID, double dNumber, int iPhysicalType = _U_NODIM, int iMaxDigit = -1, int iMinDecimal = -1 );
		CCellNumber( int iColumnID, CString strNumber, int iPhysicalType = _U_NODIM, int iMaxDigit = -1, int iMinDecimal = -1 );
		virtual CCellNumber &operator=( const CCellNumber &clCellNumber );
		// In text format, it's the number without change.
		void SetNumberAsText( CString strText );
		// In number format, it's the SI format.
		void SetNumber( double dNumber );
		void SetPhysicalType( int iPhysicalType ) { m_iPhysicalType = iPhysicalType; }
		void SetMaxDigit( int iMaxDigit ) { m_iMaxDigit = iMaxDigit; }
		void SetMinDecimal( int iMinDecimal ) { m_iMinDecimal = iMinDecimal; }
		// Retrieve value as number to the SI format.
		virtual ReadDoubleReturn_enum GetNumber( double& dValue ) const { dValue = m_dNumber; return m_eError; }
		// Retrieve value as text to the current unit format.
		CString GetNumberAsText( void ) const { return m_strNumberAsText; }
		int GetPhysicalType( void ) const { return m_iPhysicalType; }
		int GetMaxDigit( void ) const { return m_iMaxDigit; }
		int GetMinDecimal( void ) const { return m_iMinDecimal; }
		ReadDoubleReturn_enum GetLastError( void ) { return m_eError; }
		virtual bool CanFilter( CCellBase* pclCellBase, TestOperator eTestOperator );
		virtual void Copy( CCellBase* pclCellBase );
	private:
		double m_dNumber;
		CString m_strNumberAsText;
		ReadDoubleReturn_enum m_eError;
		int m_iPhysicalType;
		int m_iMaxDigit;
		int m_iMinDecimal;
	};

	class CCellBitmap : public CCellBase
	{
	public:
		CCellBitmap();
		CCellBitmap( CCellBitmap &clCellBitmap ) : CCellBase( clCellBitmap ) { *this = clCellBitmap; }
		CCellBitmap( int iColumnID );
		CCellBitmap( int iColumnID, int iBitmapID ) : CCellBase( iColumnID, CColDef::Bitmap ) { SetBitmapID( iBitmapID ); }
		virtual CCellBitmap &operator=( const CCellBitmap &clCellBitmap );
		void SetBitmapID( int iBitmapID ) { m_iBitmapID = iBitmapID; }
		virtual int GetBitmapID( void ) const { return m_iBitmapID; }
		virtual bool CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator );
		virtual void Copy( CCellBase* pclCellBase );
	private:
		int m_iBitmapID;
	};

	class CCellParam: public CCellBase
	{
	public:
		CCellParam();
		CCellParam( int iColumnID );
		CCellParam( int iColumnID, LPARAM lpParam ) : CCellBase( iColumnID, CColDef::Param ) { SetParam( lpParam ); }
		virtual CCellParam &operator=( const CCellParam &clCellParam );
		void SetParam( LPARAM lpParam ) { m_lpParam = lpParam; }
		virtual LPARAM GetParam( void ) const { return m_lpParam; }
		virtual bool CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator );
		virtual void Copy( CCellBase* pclCellBase );
	private:
		LPARAM m_lpParam;
	};

	class CCellMaster
	{
	public:
		CCellMaster( int iColumnID, CColDef::ContentType eContentType );
		CColDef::ContentType GetCellContent( void ) { return m_eContentType; }
		CCellBase* GetCellBase( void );
	private:
		CColDef::ContentType m_eContentType;
		CCellText m_clCellText;
		CCellBitmap m_clCellBitmap;
		CCellParam m_clCellParam;
	};

	typedef std::vector<CCellBase* >	vecCCellBasePtr;
	typedef vecCCellBasePtr::iterator	vecCCellBasePtrIter;
	typedef std::vector<CCellParam>		vecCCellParam;
	typedef vecCCellParam::iterator		vecCCellParamIter;

	enum PreJobType
	{
		Spaning
	};
	class CPreJobBase
	{
	public:
		CPreJobBase( PreJobType ePreJobType ) { m_ePreJobType = ePreJobType; }
		virtual ~CPreJobBase() {}
		PreJobType GetPreJobType( void ) { return m_ePreJobType; }
		virtual void Copy( CPreJobBase* pclPreJobBase );
	private:
		PreJobType	m_ePreJobType;
	};

	class CPreJobSpan : public CPreJobBase
	{
	public:
		CPreJobSpan() : CPreJobBase( PreJobType::Spaning ) { m_lFromColumn = 0; m_lToColumn = 0; }
		CPreJobSpan( long lFromColumn, long lToColumn ) : CPreJobBase( PreJobType::Spaning ) { SetRange( lFromColumn, lToColumn ); }
		virtual ~CPreJobSpan() {}
		void SetRange( long lFromColumn, long lToColumn ) { m_lFromColumn = lFromColumn; m_lToColumn = lToColumn; }
		void GetRange( long& lFromColumn, long& lToColumn ) { lFromColumn = m_lFromColumn; lToColumn = m_lToColumn; }
		long GetFromColumn( void ) { return m_lFromColumn; }
		long GetToColumn( void ) { return m_lToColumn; }
		virtual void Copy( CPreJobBase* pclPreJobBase );
	private:
		long m_lFromColumn;
		long m_lToColumn;
	};

	class CMessageBase
	{
	public:
		CMessageBase();
		virtual ~CMessageBase();
		void Clear( void ){ _Reset(); }
		CCellBase* GetCellByID( int iColumnID );
		// Returns pointer on the cell base or NULL if error.
		CCellBase* SetCellText( int iColumnID, CString strText, bool fCreateIfNotExist = false );
		CCellBase* SetCellNumber( int iColumnID, double dNumber, int iPhysicalType, int iMaxDigit, int iMinDecimal, bool fCreateIfNotExist = false );
		CCellBase* SetCellNumberAsText( int iColumnID, CString strNumberAsText, int iPhysicalType, int iMaxDigit, int iMinDecimal, bool fCreateIfNotExist = false );
		CCellBase* SetCellBitmap( int iColumnID, int iBitmapID, bool fCreateIfNotExist = false );
		vecCCellBasePtr* GetCellList( void ) { return &m_vecCellList; }
		void AddPreJob( CPreJobBase* pclPreJob );
		bool HasPreJob( void ) { return m_fDoPreJob; }
		CPreJobBase* GetFirstPreJob( void );
		CPreJobBase* GetNextPreJob( void );
	protected:
		virtual void Copy( CMessageBase* pclMessageBase );
	private:
		void _Reset( void );
	protected:
		vecCCellBasePtr m_vecCellList;
		bool m_fDoPreJob;
		std::vector<CPreJobBase*>	m_vecPreJobList;
		int m_iIterJob;
	};

	class CMessageList
	{
	public:
		CMessageList() {}
		virtual ~CMessageList();
		void AddMessage( int iMessageID, CMessageBase* pclMessage );
		CMessageBase* GetMessage( int iMessageID );
	private:
		std::map<int, CMessageBase*> m_mapMessageList;
	};

	typedef std::map< int, CColDef > mapIntColDef;
	typedef mapIntColDef::iterator	 mapIntColDefIter;
	typedef mapIntColDef::reverse_iterator mapInColDefRITer;
	
	typedef std::map< int, CCellBase* > mapIntCellBase;
	typedef mapIntCellBase::iterator	mapIntCellBaseIter;
	typedef std::vector< CCellBase* >	vecCellBase;
	typedef vecCellBase::iterator		vecCellBaseIter;

	class CCellFilter
	{
	public:
		CCellFilter( CCellBase* pclCellBase, TestOperator eTestOperator );
		virtual ~CCellFilter();
		bool CanFilter( CCellBase *pclCellBase );
		int GetColumnID( void );
		CColDef::ContentType GetContentType( void );
	private:
		CCellBase* m_pclCellBase;
		TestOperator m_eTestOperator;
	};

	// This class contains list of all cells that must be used to filter one row.
	// For example: we can use one bitmap cell and one text cell.
	class CCellFilterList
	{
	public:
		CCellFilterList() { m_iIter = 0; m_fActive = false; }
		virtual ~CCellFilterList();
		void SetFlagActive( bool fActive ) { m_fActive = fActive; }
		bool GetFlagActive( void ) { return m_fActive; }
		void ToggleFlagActive( void ) { m_fActive = !m_fActive; }
		void AddCellFilter( CCellBase* pclCellBase, TestOperator eTestOperator );
		CCellFilter* GetFirstFilter( void );
		CCellFilter* GetNextFilter( void );
	private:
		std::vector<CCellFilter*>	m_vecCellFilterList;
		int m_iIter;
		bool m_fActive;
	};

	// This class contains list of all filters.
	class CFilterList
	{
	public:
		CFilterList();
		virtual ~CFilterList();
		void AddFilter( short nID, CCellFilterList* pclCellFilterList );
		CCellFilterList* GetCellFilter( short nID );
		CCellFilterList* GetFirstCellFilter( void );
		CCellFilterList* GetNextCellFilter( void );
	private:
		std::map<short, CCellFilterList*> m_mapFilterList;
		std::map<short, CCellFilterList*>::iterator m_iIter;
	};

	void DOH_DeleteMapCell( mapIntCellBase* pmapIntCellBase );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
