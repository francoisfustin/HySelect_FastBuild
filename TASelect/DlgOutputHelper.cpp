#include "stdafx.h"
#include "TASelect.h"
#include "DlgOutputHelper.h"

using namespace DlgOutputHelper;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFontDef class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFontDef::CFontDef()
{
	m_strFontName = _T("Arial Unicode MS");
	m_iSize = 9;
	m_iHorzAlign = SSS_ALIGN_LEFT;
	m_iVertAlign = SSS_ALIGN_VCENTER;
	m_fBold = false;
	m_fItalic = false;
	m_fUnderline = false;
	m_TextForegroundColor = _BLACK;
	m_TextBackgroundColor = _WHITE;
}

CFontDef::CFontDef( const CFontDef& clFontDef )
{
	*this = clFontDef;
}

CFontDef::CFontDef( CString strFontName, int iSize, HorzAlign eHorzAlign, VertAlign eVertAlign, bool fBold, bool fItalic, bool fUnderline, COLORREF backColor, COLORREF foreColor )
{
	SetAllParams( strFontName, iSize, eHorzAlign, eVertAlign, fBold, fItalic, fUnderline, backColor, foreColor );
}

CFontDef &CFontDef::operator=( const CFontDef &clFontDef )
{
	m_strFontName = clFontDef.GetFontName();
	m_iSize = clFontDef.GetSize();
	SetHorzAlign( clFontDef.GetHorzAlign() );
	SetVertAlign( clFontDef.GetVertAlign() );
	m_fBold = clFontDef.GetBold();
	m_fItalic = clFontDef.GetItalic();
	m_fUnderline = clFontDef.GetUnderline();
	m_TextBackgroundColor = clFontDef.GetTextBackgroundColor();
	m_TextForegroundColor = clFontDef.GetTextForegroundColor();
	return *this;
}

void CFontDef::SetAllParams( CString strFontName, int iSize, HorzAlign eHorzAlign, VertAlign eVertAlign, bool fBold, bool fItalic, bool fUnderline, COLORREF backColor, COLORREF foreColor )
{
	m_strFontName = strFontName;
	m_iSize = iSize;
	SetHorzAlign( eHorzAlign );
	SetVertAlign( eVertAlign );
	m_fBold = fBold;
	m_fItalic = fItalic;
	m_fUnderline = fUnderline;
	m_TextBackgroundColor = backColor;
	m_TextForegroundColor = foreColor;
}

void CFontDef::SetHorzAlign( HorzAlign eHorzAlign )
{
	switch( eHorzAlign )
	{
		case HorzAlign::HA_Left:
			m_iHorzAlign = SSS_ALIGN_LEFT;
			break;

		case HorzAlign::HA_Center:
			m_iHorzAlign = SSS_ALIGN_CENTER;
			break;

		case HorzAlign::HA_Right:
			m_iHorzAlign = SSS_ALIGN_RIGHT;
			break;
	}
}

void CFontDef::SetVertAlign( VertAlign eVertAlign )
{
	switch( eVertAlign )
	{
		case VertAlign::VA_Top:
			m_iVertAlign = SSS_ALIGN_TOP;
			break;

		case VertAlign::VA_Center:
			m_iVertAlign = SSS_ALIGN_VCENTER;
			break;

		case VertAlign::VA_Bottom:
			m_iVertAlign = SSS_ALIGN_BOTTOM;
			break;
	}
}

CFontDef::HorzAlign CFontDef::GetHorzAlign( void ) const
{
	HorzAlign eReturnValue;
	switch( m_iHorzAlign )
	{
		case SSS_ALIGN_LEFT:
			eReturnValue = HA_Left;
			break;

		case SSS_ALIGN_CENTER:
			eReturnValue = HA_Center;
			break;

		case SSS_ALIGN_RIGHT:
			eReturnValue = HA_Right;
			break;
	}
	return eReturnValue;
}

CFontDef::VertAlign CFontDef::GetVertAlign( void ) const
{
	VertAlign eReturnValue;
	switch( m_iVertAlign )
	{
		case SSS_ALIGN_TOP:
			eReturnValue = VertAlign::VA_Top;
			break;

		case SSS_ALIGN_VCENTER:
			eReturnValue = VertAlign::VA_Center;
			break;

		case SSS_ALIGN_BOTTOM:
			eReturnValue = VertAlign::VA_Bottom;
			break;
	}
	return eReturnValue;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CColDef class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CColDef::CColDef()
{
	m_eContentType = Undefined;
	m_dWidth = -1.0;
	m_lWidthInPixels = -1;
	m_iMainHeaderID = -1;
	m_iMainHeaderSubID = -1;
	m_fAutoResized = false;
	m_fIsVisible = true;
	m_fMouseEventAllowed = true;
	m_fSelectionAllowed = true;
	m_fBlockSelectionAllowed = false;
	m_fRowSelectionAllowed = false;
	m_fColSelectionAllowed = false;
	m_fCanPasteData = false;
	m_fCanCopyData = false;
	m_fEditAllowed = false;
	m_iPhysicalType = ePHYSTYPE::_U_NODIM;
	m_iMaxDigit = -1;
	m_iMinDecimal = -1;
	m_lColAbsolutePos = 0;
	m_fEnableRowSeparator = false;
	m_clRowSeparatorColor = -1;
	m_fEnableColSeparator = false;
	m_clColSeparatorColor = -1;
}

CColDef::CColDef( const CColDef& clColDef )
{
	*this = clColDef;
}

CColDef::CColDef( ContentType eContentType, double dWidth, int iMainHeaderID, int iMainHeaderSubID, bool fIsResizable, bool fIsVisible, bool fMouseEventFlag, bool fSelectionFlag, 
		bool fBlockSelectionFlag, bool fRowSelectionFlag, bool fCanPasteData, bool fCanCopyData, bool fEditable )
{
	SetAllParams( eContentType, dWidth, iMainHeaderID, iMainHeaderSubID, fIsResizable, fIsVisible, fMouseEventFlag, fSelectionFlag, fBlockSelectionFlag, fRowSelectionFlag, fCanPasteData, fCanCopyData, fEditable );
}

CColDef &CColDef::operator=( const CColDef &clColDef )
{
	m_eContentType = clColDef.GetContentType();
	m_dWidth = clColDef.GetWidth();
	m_lWidthInPixels = clColDef.GetWidthInPixels();
	clColDef.GetMainHeaderLinks( m_iMainHeaderID, m_iMainHeaderSubID );
	m_fAutoResized = clColDef.GetAutoResized();
	m_fIsVisible = clColDef.GetVisible();
	m_fMouseEventAllowed = clColDef.IsMouseEventAllowed();
	m_fSelectionAllowed = clColDef.IsSelectionAllowed();
	m_fBlockSelectionAllowed = clColDef.IsBlockSelectionAllowed();
	m_fRowSelectionAllowed = clColDef.IsRowSelectionAllowed();
	m_fCanPasteData = clColDef.IsPasteDataAllowed();
	m_fCanCopyData = clColDef.IsCopyDataAllowed();
	m_fEditAllowed = clColDef.IsEditAllowed();
	m_clFontDef = clColDef.GetFontDef();
	m_iPhysicalType = clColDef.GetPhysicalType();
	m_iMaxDigit = clColDef.GetMaxDigit();
	m_iMinDecimal = clColDef.GetMinDecimal();
	m_lColAbsolutePos = clColDef.GetInternalColPos();
	m_fEnableRowSeparator = clColDef.GetRowSeparatorFlag();
	m_clRowSeparatorColor = clColDef.GetRowSeparatorColor();
	m_fEnableColSeparator = clColDef.GetColSeparatorFlag();
	m_clColSeparatorColor = clColDef.GetColSeparatorColor();
	return *this;
}

void CColDef::SetAllParams( ContentType eContentType, double dWidth, int iMainHeaderID, int iMainHeaderSubID, bool fIsAutoResized, bool fIsVisible, bool fMouseEventFlag, 
		bool fSelectionFlag, bool fBlockSelectionFlag, bool fRowSelectionFlag, bool fCanPasteData, bool fCanCopyData, bool fEditable )
{
	m_eContentType = eContentType;
	m_dWidth = dWidth;
	m_lWidthInPixels = -1;
	m_iMainHeaderID = iMainHeaderID;
	m_iMainHeaderSubID = iMainHeaderSubID;
	m_fAutoResized = fIsAutoResized;
	m_fIsVisible = fIsVisible;
	m_fMouseEventAllowed = fMouseEventFlag;
	m_fSelectionAllowed = fSelectionFlag;
	m_fBlockSelectionAllowed = fBlockSelectionFlag;
	m_fRowSelectionAllowed = fRowSelectionFlag;
	m_fCanPasteData = fCanPasteData;
	m_fCanCopyData = fCanCopyData;
	m_fEditAllowed = fEditable;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCellBase class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCellBase::CCellBase( CCellBase &clCellBase )
{
	*this = clCellBase;
}

CCellBase::CCellBase( int iColumnID, CColDef::ContentType eContentType ) 
{ 
	m_iColumnID = iColumnID; 
	m_eContentType = eContentType;
	m_strInternalHelper = _T("");
	m_fTextForeColorDefined = false;
	m_TextForegroundColor = 0;
	m_fTextBackColorDefined = false;
	m_TextBackgroundColor = 0;
}

CCellBase& CCellBase::operator=( const CCellBase &clCellBase )
{
	m_iColumnID = clCellBase.GetColumnID();
	m_eContentType = clCellBase.GetContentType();
	m_strInternalHelper = clCellBase.GetInternalHelper();
	m_fTextBackColorDefined = clCellBase.GetTextBackgroundColor( m_TextBackgroundColor );
	m_fTextForeColorDefined = clCellBase.GetTextForegroundColor( m_TextForegroundColor );
	return *this;
}

bool CCellBase::CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator )
{
	if( NULL == pclCellBase || m_eContentType != pclCellBase->GetContentType() || m_iColumnID != pclCellBase->GetColumnID() )
		return false;
	return true;
}

void CCellBase::Copy( CCellBase* pclCellBase )
{
	m_iColumnID = pclCellBase->GetColumnID();
	m_eContentType = pclCellBase->GetContentType();
	m_strInternalHelper = pclCellBase->GetInternalHelper();
	m_fTextBackColorDefined = pclCellBase->GetTextBackgroundColor( m_TextBackgroundColor );
	m_fTextForeColorDefined = pclCellBase->GetTextForegroundColor( m_TextForegroundColor );
}

bool CCellBase::GetTextBackgroundColor( COLORREF& textBackgroundColor ) const
{
	bool fReturn = false;
	if( true == m_fTextBackColorDefined )
	{
		textBackgroundColor = m_TextBackgroundColor;
		fReturn = true;
	}
	return fReturn;
}

bool CCellBase::GetTextForegroundColor( COLORREF& textForegroundColor  ) const
{
	bool fReturn = false;
	if( true == m_fTextForeColorDefined )
	{
		textForegroundColor = m_TextForegroundColor;
		fReturn = true;
	}
	return fReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCellText class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCellText::CCellText()
	: CCellBase( -1, CColDef::Text ) 
{ 
	m_strText = _T(""); 
	m_iStrID = -1;
}

CCellText::CCellText( int iColumnID )
	: CCellBase( iColumnID, CColDef::Text ) 
{ 
	m_strText = _T(""); 
	m_iStrID = -1;
}

CCellText& CCellText::operator=( const CCellText &clCellText )
{
	CCellBase::operator=( clCellText );
	m_strText = clCellText.GetText();
	m_iStrID = clCellText.GetTextID();
	return *this;
}

CString CCellText::GetText( void ) const 
{
	CString strReturn( _T("") );
	if( false == m_strText.IsEmpty() )
		strReturn = m_strText;
	else if( m_iStrID != -1 )
		strReturn = TASApp.LoadLocalizedString( m_iStrID );
	return strReturn;
}

bool CCellText::CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator )
{
	if( false == CCellBase::CanFilter( pclCellBase, eTestOperator ) )
		return false;
	
	CCellText* pclCellText = dynamic_cast<CCellText*>( pclCellBase );
	if( NULL == pclCellText )
		return false;

	bool fReturn = false;
	int iResult = m_strText.Compare( pclCellText->GetText() );
	switch( eTestOperator )
	{
		case TestOperator::lower:
			fReturn = ( iResult < 0 );
			break;

		case TestOperator::equal:
			fReturn = ( iResult == 0 );
			break;

		case TestOperator::higher:
			fReturn = ( iResult > 0 );
			break;

		case TestOperator::different:
			fReturn = ( iResult != 0 );
			break;
	}
	return fReturn;
}

void CCellText::Copy( CCellBase* pclCellBase )
{
	CCellBase::Copy( pclCellBase );
	m_strText = ( (CCellText*)pclCellBase )->GetText();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCellNumber class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCellNumber::CCellNumber()
	: CCellBase( -1, CColDef::Number ) 
{ 
	m_dNumber = -1.0;
	m_strNumberAsText = _T("");
	m_eError = ReadDoubleReturn_enum::RD_OK;
	m_iPhysicalType = ePHYSTYPE::_U_NODIM;
	m_iMaxDigit = -1;
	m_iMinDecimal = -1;
}

CCellNumber::CCellNumber( int iColumnID, int iPhysicalType, int iMaxDigit, int iMinDecimal )
	: CCellBase( iColumnID, CColDef::Number ) 
{ 
	m_dNumber = -1.0;
	m_strNumberAsText = _T("");
	m_iPhysicalType = iPhysicalType;
	m_iMaxDigit = iMaxDigit;
	m_iMinDecimal = iMinDecimal;
}

CCellNumber::CCellNumber( int iColumnID, double dNumber, int iPhysicalType, int iMaxDigit, int iMinDecimal ) 
	: CCellBase( iColumnID, CColDef::Number )
{
	m_iPhysicalType = iPhysicalType;
	m_iMaxDigit = iMaxDigit;
	m_iMinDecimal = iMinDecimal;
	SetNumber( dNumber );
}

CCellNumber::CCellNumber( int iColumnID, CString strText, int iPhysicalType, int iMaxDigit, int iMinDecimal ) 
	: CCellBase( iColumnID, CColDef::Number )
{
	m_iPhysicalType = iPhysicalType;
	m_iMaxDigit = iMaxDigit;
	m_iMinDecimal = iMinDecimal;
	SetNumberAsText( strText );
}

CCellNumber& CCellNumber::operator=( const CCellNumber &clCellNumber )
{
	CCellBase::operator=( clCellNumber );
	m_eError = clCellNumber.GetNumber( m_dNumber );
	m_strNumberAsText = clCellNumber.GetNumberAsText();
	m_iPhysicalType = clCellNumber.GetPhysicalType();
	m_iMaxDigit = clCellNumber.GetMaxDigit();
	m_iMinDecimal = clCellNumber.GetMinDecimal();
	return *this;
}

void CCellNumber::SetNumberAsText( CString strText )
{
	m_strNumberAsText = strText;
	m_eError = ReadCUDouble( m_iPhysicalType, strText, &m_dNumber );
}

void CCellNumber::SetNumber( double dNumber )
{
	m_dNumber = dNumber;
	m_eError = ReadDoubleReturn_enum::RD_OK;
	m_strNumberAsText = WriteCUDouble( m_iPhysicalType, m_dNumber, false, m_iMaxDigit, m_iMinDecimal );
}

bool CCellNumber::CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator )
{
	if( false == CCellBase::CanFilter( pclCellBase, eTestOperator ) )
		return false;
	
	CCellNumber* pclCellNumber = dynamic_cast<CCellNumber*>( pclCellBase );
	if( NULL == pclCellNumber )
		return false;

	double dValue;
	ReadDoubleReturn_enum eError = pclCellNumber->GetNumber( dValue );
	if( ReadDoubleReturn_enum::RD_OK != eError )
		return false;

	bool fReturn = false;
	switch( eTestOperator )
	{
		case TestOperator::lower:
			fReturn = ( m_dNumber < dValue );
			break;

		case TestOperator::equal:
			fReturn = ( m_dNumber == dValue );
			break;

		case TestOperator::higher:
			fReturn = ( m_dNumber > dValue );
			break;

		case TestOperator::different:
			fReturn = ( m_dNumber != dValue );
			break;
	}
	return fReturn;
}

void CCellNumber::Copy( CCellBase* pclCellBase )
{
	CCellBase::Copy( pclCellBase );
	m_eError = ( (CCellNumber*)pclCellBase )->GetNumber( m_dNumber );
	m_strNumberAsText = ( (CCellNumber*)pclCellBase )->GetNumberAsText();
	m_iPhysicalType = ( (CCellNumber*)pclCellBase )->GetPhysicalType();
	m_iMaxDigit = ( (CCellNumber*)pclCellBase )->GetMaxDigit();
	m_iMinDecimal = ( (CCellNumber*)pclCellBase )->GetMinDecimal();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCellBitmap class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCellBitmap::CCellBitmap()
	: CCellBase( -1, CColDef::Bitmap ) 
{ 
	m_iBitmapID = -1;
}

CCellBitmap::CCellBitmap( int iColumnID )
	: CCellBase( iColumnID, CColDef::Bitmap ) 
{ 
	m_iBitmapID = -1;
}

CCellBitmap& CCellBitmap::operator=( const CCellBitmap &clCellBitmap )
{
	CCellBase::operator=( clCellBitmap );
	m_iBitmapID = clCellBitmap.GetBitmapID();
	return *this;
}

bool CCellBitmap::CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator )
{
	if( false == CCellBase::CanFilter( pclCellBase, eTestOperator ) )
		return false;
	
	CCellBitmap* pclCellBitmap = dynamic_cast<CCellBitmap*>( pclCellBase );
	if( NULL == pclCellBitmap )
		return false;

	bool fReturn = false;
	switch( eTestOperator )
	{
		case TestOperator::equal:
			fReturn = ( m_iBitmapID == pclCellBitmap->GetBitmapID() );
			break;

		case TestOperator::different:
			fReturn = ( m_iBitmapID != pclCellBitmap->GetBitmapID() );
			break;
	}
	return fReturn;
}

void CCellBitmap::Copy( CCellBase* pclCellBase )
{
	CCellBase::Copy( pclCellBase );
	m_iBitmapID = ( (CCellBitmap*)pclCellBase )->GetBitmapID();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCellParam class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCellParam::CCellParam()
	: CCellBase( -1, CColDef::Param ) 
{ 
	m_lpParam = NULL;
}

CCellParam::CCellParam( int iColumnID )
	: CCellBase( iColumnID, CColDef::Param ) 
{ 
	m_lpParam = NULL;
}

CCellParam& CCellParam::operator=( const CCellParam &clCellParam )
{
	CCellBase::operator=( clCellParam );
	m_lpParam = clCellParam.GetParam();
	return *this;
}

bool CCellParam::CanFilter( CCellBase *pclCellBase, TestOperator eTestOperator )
{
	if( false == CCellBase::CanFilter( pclCellBase, eTestOperator ) )
		return false;
	
	CCellParam* pclCellParam = dynamic_cast<CCellParam*>( pclCellBase );
	if( NULL == pclCellParam )
		return false;

	bool fReturn = false;
	switch( eTestOperator )
	{
		case TestOperator::equal:
			fReturn = ( m_lpParam == pclCellParam->GetParam() );
			break;

		case TestOperator::different:
			fReturn = ( m_lpParam != pclCellParam->GetParam() );
			break;
	}
	return fReturn;
}

void CCellParam::Copy( CCellBase* pclCellBase )
{
	CCellBase::Copy( pclCellBase );
	m_lpParam = ( (CCellParam*)pclCellBase )->GetParam();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCellMaster class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCellMaster::CCellMaster( int iColumnID, CColDef::ContentType eContentType )
{
	m_eContentType = eContentType;
	m_clCellText.SetColumnID( iColumnID );
	m_clCellBitmap.SetColumnID( iColumnID );
	m_clCellParam.SetColumnID( iColumnID );
}

CCellBase* CCellMaster::GetCellBase( void )
{
	CCellBase* pclCellBase = NULL;
	switch( m_eContentType )
	{
		case CColDef::ContentType::Text:
			pclCellBase = &m_clCellText;
			break;
		
		case CColDef::ContentType::Bitmap:
			pclCellBase = &m_clCellBitmap;
			break;
		
		case CColDef::ContentType::Param:
			pclCellBase = &m_clCellParam;
			break;
	}
	return pclCellBase;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPreJobBase class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreJobBase::Copy( CPreJobBase* pclPreJobBase )
{
	if( pclPreJobBase != NULL )
	{
		m_ePreJobType = pclPreJobBase->GetPreJobType();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPreJobBase class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreJobSpan::Copy( CPreJobBase* pclPreJobBase )
{
	if( pclPreJobBase != NULL )
	{
		CPreJobBase::Copy( pclPreJobBase );
		CPreJobSpan* pclPreJobSpan = dynamic_cast<CPreJobSpan*>( pclPreJobBase );
		if( pclPreJobSpan != NULL )
		{
			pclPreJobSpan->GetRange( m_lFromColumn, m_lToColumn );
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMessageBase class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMessageBase::CMessageBase()
{
	m_vecCellList.clear();
	m_fDoPreJob = false;
	m_vecPreJobList.clear();
	m_iIterJob = 0;
}

CMessageBase::~CMessageBase()
{
	_Reset();
}

CCellBase* CMessageBase::GetCellByID( int iColumnID )
{
	CCellBase* pclCell = NULL;
	if( m_vecCellList.size() > 0 )
	{
		for( vecCCellBasePtrIter iter = m_vecCellList.begin(); iter != m_vecCellList.end(); iter++ )
		{
			if( iColumnID == (*iter)->GetColumnID() )
			{
				pclCell = *iter;
				break;
			}
		}
	}
	return pclCell;
}

CCellBase* CMessageBase::SetCellText( int iColumnID, CString strText, bool fCreateIfNotExist )
{
	CCellBase* pclCellBase = GetCellByID( iColumnID );
	if( NULL == pclCellBase && false == fCreateIfNotExist )
		return NULL;

	CCellText* pclCellText = NULL;
	if( NULL == pclCellBase )
	{
		pclCellText = new CCellText( iColumnID, strText );
		if( NULL == pclCellText )
			return NULL;
		m_vecCellList.push_back( pclCellText );
	}
	else
	{
		pclCellText = dynamic_cast<CCellText*>( pclCellBase );
		if( NULL == pclCellText )
			return NULL;
		pclCellText->SetText( strText );
	}
	return pclCellText;
}

CCellBase* CMessageBase::SetCellNumber( int iColumnID, double dNumber, int iPhysicalType, int iMaxDigit, int iMinDecimal, bool fCreateIfNotExist )
{
	CCellBase* pclCellBase = GetCellByID( iColumnID );
	if( NULL == pclCellBase && false == fCreateIfNotExist )
		return NULL;

	CCellNumber* pclCellNumber = NULL;
	if( NULL == pclCellBase )
	{
		pclCellNumber = new CCellNumber( iColumnID, dNumber, iPhysicalType, iMaxDigit, iMinDecimal );
		if( NULL == pclCellNumber )
			return NULL;
		m_vecCellList.push_back( pclCellNumber );
	}
	else
	{
		pclCellNumber = dynamic_cast<CCellNumber*>( pclCellBase );
		if( NULL == pclCellNumber )
			return NULL;
		pclCellNumber->SetNumber( dNumber );
	}
	return pclCellNumber;
}

CCellBase* CMessageBase::SetCellNumberAsText( int iColumnID, CString strNumberAsText, int iPhysicalType, int iMaxDigit, int iMinDecimal, bool fCreateIfNotExist )
{
	CCellBase* pclCellBase = GetCellByID( iColumnID );
	if( NULL == pclCellBase && false == fCreateIfNotExist )
		return NULL;

	CCellNumber* pclCellNumber = NULL;
	if( NULL == pclCellBase )
	{
		pclCellNumber = new CCellNumber( iColumnID, strNumberAsText, iPhysicalType, iMaxDigit, iMinDecimal );
		if( NULL == pclCellNumber )
			return NULL;
		m_vecCellList.push_back( pclCellNumber );
	}
	else
	{
		pclCellNumber = dynamic_cast<CCellNumber*>( pclCellBase );
		if( NULL == pclCellNumber )
			return NULL;
		pclCellNumber->SetNumberAsText( strNumberAsText );
	}
	return pclCellNumber;
}

CCellBase* CMessageBase::SetCellBitmap( int iColumnID, int iBitmapID, bool fCreateIfNotExist )
{
	CCellBase* pclCellBase = GetCellByID( iColumnID );
	if( NULL == pclCellBase && false == fCreateIfNotExist )
		return NULL;

	CCellBitmap* pclCellBitmap = NULL;
	if( NULL == pclCellBase )
	{
		pclCellBitmap = new CCellBitmap( iColumnID, iBitmapID );
		if( NULL == pclCellBitmap )
			return NULL;
		m_vecCellList.push_back( pclCellBitmap );
	}
	else
	{
		pclCellBitmap = dynamic_cast<CCellBitmap*>( pclCellBase );
		if( NULL == pclCellBitmap )
			return NULL;
		pclCellBitmap->SetBitmapID( iBitmapID );
	}
	return pclCellBitmap;
}

void CMessageBase::AddPreJob( CPreJobBase* pclPreJob )
{ 
	if( pclPreJob != NULL )
	{
		m_vecPreJobList.push_back( pclPreJob );
		m_fDoPreJob = true;
	}
}

CPreJobBase* CMessageBase::GetFirstPreJob( void )
{
	m_iIterJob = 0;
	if( 0 == m_vecPreJobList.size() )
		return NULL;
	return m_vecPreJobList[m_iIterJob];
}

CPreJobBase* CMessageBase::GetNextPreJob( void )
{
	m_iIterJob++;
	if( 0 == m_vecPreJobList.size() || m_iIterJob >= (int)m_vecPreJobList.size() )
		return NULL;
	return m_vecPreJobList[m_iIterJob];
}

void CMessageBase::Copy( CMessageBase* pclMessageBase )
{
	_Reset();
	vecCCellBasePtr* pclCellList = pclMessageBase->GetCellList();
	if( pclCellList->size() > 0 )
	{
		for( vecCCellBasePtrIter iter = pclCellList->begin(); iter != pclCellList->end(); iter++ )
		{
			CCellBase* pclCellBase = *iter;
			if( NULL == pclCellBase )
				continue;
			
			switch( pclCellBase->GetContentType() )
			{
				case CColDef::ContentType::Text:
				{
					CCellText* pclCellText = new CCellText();
					if( pclCellText != NULL )
					{
						pclCellText->Copy( pclCellBase );
						m_vecCellList.push_back( pclCellText );
					}
					break;
				}

				case CColDef::ContentType::Bitmap:
				{
					CCellBitmap* pclCellBitmap = new CCellBitmap();
					if( pclCellBitmap != NULL )
					{
						pclCellBitmap->Copy( pclCellBase );
						m_vecCellList.push_back( pclCellBitmap );
					}
					break;
				}

				case CColDef::ContentType::Param:
				{
					CCellParam* pclCellParam = new CCellParam();
					if( pclCellParam != NULL )
					{
						pclCellParam->Copy( pclCellBase );
						m_vecCellList.push_back( pclCellParam );
					}
					break;
				}
			}
		}
	}

	// Copy pre job.
	CPreJobBase* pclPreJob = pclMessageBase->GetFirstPreJob();
	while( pclPreJob != NULL )
	{
		switch( pclPreJob->GetPreJobType() )
		{
			case PreJobType::Spaning:
			{
				CPreJobSpan* pclPreJobSpan = new CPreJobSpan();
				if( pclPreJobSpan != NULL )
				{
					pclPreJobSpan->Copy( pclPreJob );
					m_vecPreJobList.push_back( pclPreJobSpan );
				}
				break;
			}
		}
		pclPreJob = pclMessageBase->GetNextPreJob();
	}
}

void CMessageBase::_Reset( void )
{
	if( m_vecCellList.size() > 0 )
	{
		for( vecCCellBasePtrIter iter = m_vecCellList.begin(); iter != m_vecCellList.end(); iter++ )
		{
			if( *iter != NULL )
				delete (*iter);
		}
		m_vecCellList.clear();
	}
	if( m_vecPreJobList.size() > 0 )
	{
		for( int iLoop = 0; iLoop < (int)m_vecPreJobList.size(); iLoop++ )
		{
			if( m_vecPreJobList[iLoop] != NULL )
				delete m_vecPreJobList[iLoop];
		}
	}
	m_fDoPreJob = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMessageList class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMessageList::~CMessageList()
{
	if( m_mapMessageList.size() > 0 )
	{
		for( std::map<int, CMessageBase*>::iterator iter = m_mapMessageList.begin(); iter != m_mapMessageList.end(); iter++ )
		{
			if( iter->second != NULL )
				delete iter->second;
		}
		m_mapMessageList.clear();
	}
}

void CMessageList::AddMessage( int iMessageID, CMessageBase* pclMessage )
{
	if( NULL == pclMessage )
		return;
	if( m_mapMessageList.count( iMessageID ) > 0 )
	{
		if( m_mapMessageList[iMessageID] != NULL )
			delete m_mapMessageList[iMessageID];
		m_mapMessageList.erase( iMessageID );
	}
	m_mapMessageList[iMessageID] = pclMessage;
}

CMessageBase* CMessageList::GetMessage( int iMessageID )
{
	if( 0 == m_mapMessageList.count( iMessageID ) || NULL == m_mapMessageList[iMessageID] )
		return NULL;
	return m_mapMessageList[iMessageID];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCellFilter class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCellFilter::CCellFilter( CCellBase* pclCellBase, TestOperator eTestOperator )
{ 
	m_pclCellBase = NULL;
	m_eTestOperator = eTestOperator;
	m_pclCellBase = pclCellBase;
}

CCellFilter::~CCellFilter( )
{
	if( m_pclCellBase != NULL )
		delete m_pclCellBase;
}

bool CCellFilter::CanFilter( CCellBase *pclCellBase )
{
	if( NULL == m_pclCellBase )
		return false;
	return m_pclCellBase->CanFilter( pclCellBase, m_eTestOperator );
}

int CCellFilter::GetColumnID( void )
{
	if( NULL == m_pclCellBase )
		return -1;
	return m_pclCellBase->GetColumnID();
}

CColDef::ContentType CCellFilter::GetContentType( void )
{
	if( NULL == m_pclCellBase )
		return CColDef::ContentType::Undefined;
	return m_pclCellBase->GetContentType();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCellFilterList class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCellFilterList::~CCellFilterList()
{
	if( m_vecCellFilterList.size() > 0 )
	{
		for( int iLoop = 0; iLoop < (int)m_vecCellFilterList.size(); iLoop++ )
		{
			if( m_vecCellFilterList[iLoop] != NULL )
				delete m_vecCellFilterList[iLoop];
		}
		m_vecCellFilterList.clear();
	}
}

void CCellFilterList::AddCellFilter( CCellBase* pclCellBase, TestOperator eTestOperator )
{
	m_vecCellFilterList.push_back( new CCellFilter( pclCellBase, eTestOperator ) );
}

CCellFilter* CCellFilterList::GetFirstFilter( void )
{
	m_iIter = 0;
	if( 0 == m_vecCellFilterList.size() )
		return NULL;
	return m_vecCellFilterList[m_iIter];
}

CCellFilter* CCellFilterList::GetNextFilter( void )
{
	m_iIter++;
	if( 0 == m_vecCellFilterList.size() || m_iIter >= (int)m_vecCellFilterList.size() )
		return NULL;
	return m_vecCellFilterList[m_iIter];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFilterList class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CFilterList::CFilterList()
{
}

CFilterList::~CFilterList()
{
	if( m_mapFilterList.size() > 0 )
	{
		for( m_iIter = m_mapFilterList.begin(); m_iIter != m_mapFilterList.end(); m_iIter++ )
		{
			if( m_iIter->second != NULL )
				delete m_iIter->second;
		}
		m_mapFilterList.clear();
	}
}

void CFilterList::AddFilter( short nID, CCellFilterList* pclCellFilterList )
{
	if( pclCellFilterList != NULL && 0 == m_mapFilterList.count( nID ) )
	{
		m_mapFilterList[nID] = pclCellFilterList;
	}
}

CCellFilterList* CFilterList::GetCellFilter( short nID )
{
	if( 0 == m_mapFilterList.count( nID ) )
		return NULL;
	return m_mapFilterList[nID];
}

CCellFilterList* CFilterList::GetFirstCellFilter( void )
{
	m_iIter = m_mapFilterList.begin();
	if( m_iIter == m_mapFilterList.end() )
		return NULL;
	return m_iIter->second;
}

CCellFilterList* CFilterList::GetNextCellFilter( void )
{
	m_iIter++;
	if( 0 == m_mapFilterList.size() || m_iIter == m_mapFilterList.end() )
		return NULL;
	return m_iIter->second;
}

void DlgOutputHelper::DOH_DeleteMapCell( mapIntCellBase* pmapIntCellBase )
{
	if( NULL == pmapIntCellBase )
	{
		return;
	}

	for( mapIntCellBaseIter iter = pmapIntCellBase->begin(); iter != pmapIntCellBase->end(); iter++ )
	{
		if( NULL != iter->second )
		{
			delete iter->second;
		}
	}

	pmapIntCellBase->clear();
}
