#include "stdafx.h"

#include "TASelect.h"
#include "MFCRibbonEditEx.h"

CMFCRibbonEditEx::CMFCRibbonEditEx() : CMFCRibbonEdit()
{
	m_TypeRichEditCtrl = eTypeRichEditCtrl::eLastEditCtrl;
	m_pWndSpinEx = NULL;
	m_peditNum = NULL;
	m_peditCbiRest = NULL;
}

CMFCRibbonEditEx::CMFCRibbonEditEx( UINT nID, int nWidth, LPCTSTR lpszlabel, int nImage, eTypeRichEditCtrl eTypeEditCtrl )
	:CMFCRibbonEdit( nID, nWidth, lpszlabel, nImage )
{
	m_TypeRichEditCtrl = eTypeEditCtrl;
	m_pWndSpinEx = NULL;
	m_peditNum = NULL;
	m_peditCbiRest = NULL;
}


CMFCRibbonEditEx::~CMFCRibbonEditEx()
{
	DestroyCtrl();
}


void CMFCRibbonEditEx::DestroyCtrl()
{
	ASSERT_VALID(this);

	if (m_pWndSpinEx != NULL)
	{
		m_pWndSpinEx->DestroyWindow();
		delete m_pWndSpinEx;
		m_pWndSpinEx = NULL;
		m_pWndSpin = NULL;
	}

	if (m_peditNum != NULL)
	{
		m_peditNum->DestroyWindow();
		delete m_peditNum;
		m_peditNum = NULL;
		m_pWndEdit = NULL;
	}

	if (m_peditCbiRest != NULL)
	{
		m_peditCbiRest->DestroyWindow();
		delete m_peditCbiRest;
		m_peditCbiRest = NULL;
		m_pWndEdit = NULL;
	}
}

CMFCRibbonRichEditCtrl* CMFCRibbonEditEx::CreateEdit(CWnd *pWndParent, DWORD dwEditStyle)
{
	ASSERT_VALID( this );
	ASSERT_VALID( pWndParent );

	// If already created...
	if( NULL != m_peditNum && NULL != m_peditNum->GetSafeHwnd() && eTypeRichEditCtrl::eNumericalEditCtrl == m_TypeRichEditCtrl )
	{
		return m_peditNum;
	}
	else if( NULL != m_peditCbiRest && NULL != m_peditCbiRest->GetSafeHwnd() && m_TypeRichEditCtrl == eTypeRichEditCtrl::eCBIRestrEditCtrl )
	{
		return m_peditCbiRest;
	}

	if( eTypeRichEditCtrl::eNumericalEditCtrl == m_TypeRichEditCtrl )
	{
		m_peditNum = new CMFCRibbonRichEditCtrlNum( *this );

		if( NULL == m_peditNum )
		{
			return NULL;
		}
		
		if( FALSE == m_peditNum->Create( dwEditStyle, CRect( 0, 0, 0, 0 ), pWndParent, m_nID ) )
		{
			delete m_peditNum;
			return NULL;
		}

		if( TRUE == m_bHasSpinButtons )
		{
			CreateSpinButtonEx( m_peditNum,  pWndParent );
		}

		return m_peditNum;
	}
	else if( eTypeRichEditCtrl::eCBIRestrEditCtrl == m_TypeRichEditCtrl )
	{
		m_peditCbiRest = new CMFCRibbonRichEditCtrlCbiRestr( *this );

		if( NULL == m_peditCbiRest )
		{
			return NULL;
		}

		if( FALSE == m_peditCbiRest->Create( dwEditStyle, CRect( 0, 0, 0, 0 ), pWndParent, m_nID ) )
		{
			delete m_peditCbiRest;
			return NULL;
		}

		if( TRUE == m_bHasSpinButtons )
		{
			CreateSpinButtonEx( m_peditCbiRest, pWndParent );
		}

		return m_peditCbiRest;
	}
	
	return NULL;
}

void CMFCRibbonEditEx::SetPhysicalTypeEdit( ePHYSTYPE eType )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetPhysicalType( eType );
}

void CMFCRibbonEditEx::SetPhysicalTypeEdit( CString strType )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetPhysicalTypeString( strType );
}

ePHYSTYPE CMFCRibbonEditEx::GetPhysicalTypeEdit()
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return _U_NODIM;
	}

	if( NULL == m_peditNum )
	{
		return _U_NODIM;
	}

	return m_peditNum->GetPhysicalType();
}

void CMFCRibbonEditEx::SetEditSign( CNumString::eEditSign eSign )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetEditSign( eSign );
}

void CMFCRibbonEditEx::SetMaxDblValue( double dMaxVal )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetMaxDblValue( dMaxVal );
}

void CMFCRibbonEditEx::SetMinDblValue( double dMinVal )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetMinDblValue( dMinVal );
}

void CMFCRibbonEditEx::SetMinIntValue( int iMinVal )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetMinIntValue( iMinVal );
}

void CMFCRibbonEditEx::SetMaxIntValue( int iMaxVal )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetMaxIntValue( iMaxVal );
}

void CMFCRibbonEditEx::SetEditType( CNumString::eEditType EdiType, CNumString::eEditSign EditSign )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetEditType( EdiType, EditSign );
}

void CMFCRibbonEditEx::SetCurrentValSI( double dSIValue )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetCurrentValSI( dSIValue );

	// Do the default function.
	SetEditText( m_peditNum->GetCurrentString() );
}

void CMFCRibbonEditEx::SetEditTextEx( CString strText, bool bSIValue )
{
	// Call the default function.
	SetEditText( strText );

	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	// Initialize the SI iValue member.
	double dValue = 0.0;
	ReadDouble( strText, &dValue );

	if( false == bSIValue )
	{
		m_peditNum->SetCurrentValSI( CDimValue::CUtoSI( m_peditNum->GetPhysicalType(), dValue ) );
	}
	else
	{
		m_peditNum->SetCurrentValSI( dValue );
	}

	m_peditNum->SetWindowText( GetCurrentString() );
}

LPCTSTR CMFCRibbonEditEx::GetCurrentString()
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return _T("");
	}

	if( NULL == m_peditNum )
	{
		return _T("");
	}

	return m_peditNum->GetCurrentString();
}

double CMFCRibbonEditEx::GetCurrentValSI()
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return 0.0;
	}

	if( NULL == m_peditNum )
	{
		return 0.0;
	}

	return m_peditNum->GetCurrentValSI();
}

void CMFCRibbonEditEx::SetUnitsUsed( bool bUnitUsed )
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetUnitsUsed( bUnitUsed );
}

void CMFCRibbonEditEx::SetEditForTemperature()
{
	if( eTypeRichEditCtrl::eNumericalEditCtrl != m_TypeRichEditCtrl )
	{
		return;
	}

	if( NULL == m_peditNum )
	{
		return;
	}

	m_peditNum->SetEditForTemperature();
}

BOOL CMFCRibbonEditEx::CreateSpinButtonEx( CMFCRibbonRichEditCtrl *pWndEdit, CWnd *pWndParent )
{
	ASSERT_VALID( this );
	ASSERT( NULL == m_pWndSpin );

	if( false == m_bHasSpinButtons )
	{
		ASSERT( FALSE );
		return FALSE;
	}

	m_pWndSpinEx = new CMFCRibbonSpinButtonCtrlEx( this );

	if( NULL == m_pWndSpinEx )
	{
		return FALSE;
	}
	
	if( FALSE == m_pWndSpinEx->Create( WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT, CRect(0, 0, 0, 0), pWndParent, m_nID ) )
	{
		delete m_pWndSpinEx;
		return FALSE;
	}

	m_pWndSpinEx->SetBuddy( pWndEdit );
	m_pWndSpinEx->SetRange32( m_nMin, m_nMax );

	m_pWndSpin = (CMFCRibbonSpinButtonCtrl *)m_pWndSpinEx;

	return TRUE;
}

void CMFCRibbonEditEx::OnSetFocus( BOOL B )
{
	CMFCRibbonEdit::OnSetFocus( B );
	m_bIsFocused = B;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonRichEditCtrlNum
CMFCRibbonRichEditCtrlNum::CMFCRibbonRichEditCtrlNum( CMFCRibbonEditEx &edit )
	:CMFCRibbonRichEditCtrl( edit )
{
	m_bEditForTemp = false;
}

BEGIN_MESSAGE_MAP( CMFCRibbonRichEditCtrlNum, CMFCRibbonRichEditCtrl )
	ON_WM_SETFOCUS()
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
END_MESSAGE_MAP()

BOOL CMFCRibbonRichEditCtrlNum::PreTranslateMessage( MSG *pMsg )
{
	// Interpret the enter key to avoid that function :
	// NotifyCommand() twice
	// One time with the __super::PreTranslateMessage(VK_RETURN)
	// One time with the __super::OnKillFocus(pNewWnd)
	// This problem could be solved in a further version of MFC9.0
	if( WM_KEYDOWN == pMsg->message && VK_RETURN == pMsg->wParam )
	{
		if( FALSE == m_edit.IsDroppedDown() && NULL != GetTopLevelFrame() )
		{
			GetTopLevelFrame()->SetFocus();
			return TRUE;
		}
	}
	
	return CMFCRibbonRichEditCtrl::PreTranslateMessage( pMsg );
}

void CMFCRibbonRichEditCtrlNum::OnSetFocus( CWnd *pOldWnd )
{
	PostMessage( EM_SETSEL, 0, -1 );
	CMFCRibbonRichEditCtrl::OnSetFocus( pOldWnd );
}

void CMFCRibbonRichEditCtrlNum::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	// Variables.
	CString str, NewStr, FormattedStr;

	// Get the string before adding new character.
	GetWindowText( str );

	// Call the default function.
	CMFCRibbonRichEditCtrl::OnChar( nChar, nRepCnt, nFlags );

	// Get the string after adding new character.
	GetWindowText( NewStr );

	// Define the cursor position.
	long lStartChar, lEndChar;
	GetSel( lStartChar, lEndChar );
	int iPosCursor = (int)lStartChar;

	// Format numerical string.
	FormattedStr = CNumString::FormatChar( nChar, str, NewStr, &iPosCursor );

	// Special treatment for temperature edit boxes.
	if( true == m_bEditForTemp )
	{
		// Get the double iValue of the FormattedStr.
		double val = 0.0;
		double valSI = 0.0;
		ReadDouble( FormattedStr, &val );
		valSI = CDimValue::CUtoSI( _U_TEMPERATURE, val );

		// Verify it is not below the absolute zero.
		if( valSI < -273.15 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_BELOW_ABSZERO );
			SetFocus();
			FormattedStr = CNumString::FormatChar( nChar, str, _T(""), &iPosCursor );
			iPosCursor = 0;
		}
	}

	// Written formatted string into the edit.
	SetWindowText( FormattedStr );
	SetSel( (long)iPosCursor, (long)iPosCursor );
}

void CMFCRibbonRichEditCtrlNum::OnKillFocus( CWnd *pNewWnd )
{
	// Do the default function.
	CMFCRibbonRichEditCtrl::OnKillFocus( pNewWnd );
	
	// Init variables.
	CString str;
	double val = 0.0;
	GetWindowText( str );
	ReadDouble( str, &val );

	// Get the iValue expressed in SI units.
	if( _T("") != str )
	{
		m_dCurrentValSI = CDimValue::CUtoSI( m_ePhysType, val);
	}

	// Correct the string to be sure the units are shown correctly, following the m_bUnitsUsed iValue.
	// It could append if the user push the "delete" button on the units and kill the focus.
	SetWindowText( GetCurrentString() );

	m_edit.OnSetFocus( FALSE );
}

void CMFCRibbonRichEditCtrlNum::OnContextMenu( CWnd *pWnd, CPoint point )
{
	// Do not use the ContextMenu for our CMFCRibbonRichEditCtrlNum.
	return;
}

LRESULT CMFCRibbonRichEditCtrlNum::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	// Verify one iValue already exist, if it doesn't, do not refresh the edit control.
	CString str, strUnknown;
	GetWindowText( str );
	strUnknown = TASApp.LoadLocalizedString( IDS_UNKNOWN );

	if( _T("") != str && str != strUnknown )
	{
		SetWindowText( GetCurrentString() );
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonRichEditCtrlCbiRestr

CMFCRibbonRichEditCtrlCbiRestr::CMFCRibbonRichEditCtrlCbiRestr( CMFCRibbonEditEx &edit )
	:CMFCRibbonRichEditCtrl( edit )
{
}

BEGIN_MESSAGE_MAP( CMFCRibbonRichEditCtrlCbiRestr, CMFCRibbonRichEditCtrl )
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CMFCRibbonRichEditCtrlCbiRestr::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	// Init variable.
	CString str; 
	GetWindowText( str );
	WCHAR wchar = TASApp.GetSiteNameMaxChar();

	// Verify if the number of characters is not exceeded.
	if( str.GetLength() + 1 > wchar )
	{
		return;
	}

	// Call the default function.
	CMFCRibbonRichEditCtrl::OnChar( nChar, nRepCnt, nFlags );

	// Get the text after the char is inserted.
	GetWindowText( str );

	// Verify if the project is CBI compatible or not.
	if( CBIMAXSITENAMECHAR != wchar )
	{
		return;
	}

	// Check the CBI Character inserted.
	if( false == CCbiRestrString::CheckCBICharSet( &str ) )
	{
		SetWindowText( str );
		TASApp.AfxLocalizeMessageBox( AFXMSG_CBICHARRESTRICTION );
		PostMessage( WM_KEYDOWN, VK_END, 0 );
		SetFocus();  
	}
}

void CMFCRibbonRichEditCtrlCbiRestr::OnContextMenu(CWnd* pWnd, CPoint point )
{
	// Do not use the ContextMenu for our CMFCRibbonRichEditCtrlCbiRestr.
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonSpinButtonCtrlEx

CMFCRibbonSpinButtonCtrlEx::CMFCRibbonSpinButtonCtrlEx( CMFCRibbonEditEx *pEdit )
{
	m_bQuickAccessMode = FALSE;
	m_pEdit = pEdit;
}

BEGIN_MESSAGE_MAP( CMFCRibbonSpinButtonCtrlEx, CMFCSpinButtonCtrl )
	ON_NOTIFY_REFLECT( UDN_DELTAPOS, &CMFCRibbonSpinButtonCtrlEx::OnDeltapos )
END_MESSAGE_MAP()

void CMFCRibbonSpinButtonCtrlEx::OnDraw( CDC *pDC )
{
	BOOL bIsDrawOnGlass = CMFCToolBarImages::m_bIsDrawOnGlass;

	if( TRUE == m_bQuickAccessMode )
	{
		CMFCRibbonBar *pRibbonBar = DYNAMIC_DOWNCAST( CMFCRibbonBar, GetParent() );

		if( NULL != pRibbonBar )
		{
			ASSERT_VALID( pRibbonBar );

			if( TRUE == pRibbonBar->IsQuickAccessToolbarOnTop() && TRUE == pRibbonBar->IsTransparentCaption() )
			{
				CMFCToolBarImages::m_bIsDrawOnGlass = TRUE;
			}
		}
	}

	CMFCSpinButtonCtrl::OnDraw( pDC );

	CMFCToolBarImages::m_bIsDrawOnGlass = bIsDrawOnGlass;
}

void CMFCRibbonSpinButtonCtrlEx::OnDeltapos( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( NULL != m_pEdit && NULL != GetBuddy()->GetSafeHwnd() )
	{
		NM_UPDOWN *pNMUpDown = (NM_UPDOWN*)pNMHDR;

		// Verify the current range.
		// If the new iValue is outside the boundaries, do not insert them.
		int iLower = 0;
		int iUpper = 0;
		int iValue;
		iValue = pNMUpDown->iPos + pNMUpDown->iDelta;
		GetRange32( iLower, iUpper );

		if( iValue >= iLower && iValue <= iUpper )
		{
			CString str;
			str.Format( L"%d", pNMUpDown->iPos + pNMUpDown->iDelta );
			GetBuddy()->SetFocus();			
			m_pEdit->SetEditText( str );

			if( NULL != GetTopLevelFrame() )
			{
				GetTopLevelFrame()->SetFocus();
			}
		}
	}

	*pResult = 0;
}
