#include "stdafx.h"

#include "TASelect.h"
#include "Global.h"
#include "Units.h"
#include "Utilities.h"

#include "DlgUnitConv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgUnitConv::CDlgUnitConv( CWnd *pParent )
	: CDialogEx( CDlgUnitConv::IDD, pParent )
{
	m_pUnitDB = NULL;
	m_pTADB = NULL;
	m_iPhysType = 0;
	m_iUnit1 = 0;
	m_iUnit2 = 0;
	m_bModified = false;
}

int CDlgUnitConv::Display()
{
	m_pUnitDB = CDimValue::AccessUDB();
	m_pTADB = TASApp.GetpTADB();

	return DoModal();
}

BEGIN_MESSAGE_MAP( CDlgUnitConv, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOPHYSTYPE, OnCbnSelChangePhysicalType )
	ON_CBN_SELCHANGE( IDC_COMBOUNIT1, OnCbnSelChangeUnit1 )
	ON_CBN_SELCHANGE( IDC_COMBOUNIT2, OnCbnSelChangeUnit2 )
	ON_EN_CHANGE( IDC_EDITVALUE1, OnChangeEditValue1 )
	ON_EN_CHANGE( IDC_EDITVALUE2, OnChangeEditValue2 )
	ON_EN_KILLFOCUS( IDC_EDITVALUE1, OnKillFocusEditValue1 )
	ON_EN_KILLFOCUS( IDC_EDITVALUE2, OnKillFocusEditValue2 )
END_MESSAGE_MAP()

void CDlgUnitConv::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control( pDX, IDC_STATICLRARROW, m_StaticLrArrow );
	DDX_Control( pDX, IDC_GROUP1, m_Group1 );
	DDX_Control( pDX, IDC_GROUP2, m_Group2 );
	DDX_Control( pDX, IDC_EDITVALUE1, m_EditValue1 );
	DDX_Control( pDX, IDC_EDITVALUE2, m_EditValue2 );
	DDX_Control( pDX, IDC_STATICUNIT1, m_StaticUnit1 );
	DDX_Control( pDX, IDC_STATICUNIT2, m_StaticUnit2 );
	DDX_Control( pDX, IDC_COMBOUNIT1, m_ComboUnit1 );
	DDX_Control( pDX, IDC_COMBOUNIT2, m_ComboUnit2 );
	DDX_Control( pDX, IDC_COMBOPHYSTYPE, m_ComboPhysType );
}

BOOL CDlgUnitConv::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGUNITCONV_CAPTION );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGUNITCONV_STATICPHYSTYPE );
	GetDlgItem( IDC_STATICPHYSTYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGUNITCONV_STATICUNITTIT1 );
	GetDlgItem( IDC_STATICUNITTIT1 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGUNITCONV_STATICUNITTIT2 );
	GetDlgItem( IDC_STATICUNITTIT2 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGUNITCONV_STATICVAL1 );
	GetDlgItem( IDC_STATICVAL1 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGUNITCONV_STATICVAL2 );
	GetDlgItem( IDC_STATICVAL2 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGUNITCONV_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	str.Empty();

	// Add icons to the 2 groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_Group1.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Ruler1 );
		m_Group2.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Ruler2 );
	}
	
	// Load bitmap for Left-Right Arrow Stap functions and associate it to the static control.
	m_BitmapLrArrow.LoadMappedBitmap( IDB_LRARROW );
	HBITMAP hBitmap = (HBITMAP)m_BitmapLrArrow.GetSafeHandle();
	m_StaticLrArrow.SetBitmap( hBitmap );

	// Recover last selected physical type and units from registry.
	m_iPhysType = (int)::AfxGetApp()->GetProfileInt( _T("DialogUnitConv"), _T("Physical Type"), 1 );
	m_iUnit1 = (int)::AfxGetApp()->GetProfileInt( _T("DialogUnitConv"), _T("Unit1"), 0 );
	m_iUnit2 = (int)::AfxGetApp()->GetProfileInt( _T("DialogUnitConv"), _T("Unit2"), 0 );

	// Fill the combo boxes with Physical type 1 selected by default.
	_FillComboPhysicalType();
	_FillComboUnit12();

	// Set the text in m_StaticUnit1 and 2.
	_SetStaticUnit1();
	_SetStaticUnit2();

	return TRUE;
}

void CDlgUnitConv::OnOK() 
{
	PREVENT_ENTER_KEY

	// Save the selected physical type and units in the registry.
	::AfxGetApp()->WriteProfileInt( _T("DialogUnitConv"), _T("Physical Type"), m_iPhysType );
	::AfxGetApp()->WriteProfileInt( _T("DialogUnitConv"), _T("Unit1"), m_iUnit1 );
	::AfxGetApp()->WriteProfileInt( _T("DialogUnitConv"), _T("Unit2"), m_iUnit2 );
	
	CDialogEx::OnOK();
}

void CDlgUnitConv::OnCbnSelChangePhysicalType() 
{
	m_iPhysType = m_ComboPhysType.GetCurSel() + 1;
	m_bModified = true;

	_FillComboUnit12();

	// Modify the text in m_StaticUnit1 and 2.
	_SetStaticUnit1();
	_SetStaticUnit2();

	// Clear the value edit boxes.
	m_EditValue1.SetWindowText( _T("") );
	m_EditValue2.SetWindowText( _T("") );
}

void CDlgUnitConv::OnCbnSelChangeUnit1() 
{
	m_iUnit1 = m_ComboUnit1.GetCurSel();
	m_bModified = true;

	// Modify the text in m_StaticUnit1.
	_SetStaticUnit1();

	// If Edit box Value2 is not empty, find correspondent value 1.
	_Convert2to1();
}

void CDlgUnitConv::OnCbnSelChangeUnit2() 
{
	m_iUnit2 = m_ComboUnit2.GetCurSel();
	m_bModified = true;
	
	// Modify the text in m_StaticUnit2.
	_SetStaticUnit2();

	// If Edit box Value1 is not empty, find correspondent value2.
	_Convert1to2();
}

void CDlgUnitConv::OnChangeEditValue1() 
{
	if( GetFocus() != &m_EditValue1 )
	{
		return; 
	}

	_Convert1to2();
}

void CDlgUnitConv::OnChangeEditValue2() 
{
	if( GetFocus() != &m_EditValue2 )
	{
		return;
	}

	_Convert2to1();
}

void CDlgUnitConv::OnKillFocusEditValue1() 
{
	double dValue;

	if( RD_OK == ReadDouble( m_EditValue1, &dValue ) )
	{
		m_EditValue1.SetWindowText( WriteDouble( dValue, 4,0 , true ) );
	}
}

void CDlgUnitConv::OnKillFocusEditValue2() 
{
	double dValue;

	if( RD_OK == ReadDouble( m_EditValue2, &dValue ) )
	{
		m_EditValue2.SetWindowText( WriteDouble( dValue, 4, 0, true ) );
	}
}

void CDlgUnitConv::_FillComboPhysicalType()
{
	ASSERT( ( m_iPhysType > 0 ) && ( m_iPhysType < _UNITS_NUMBER_OF ) );

	CTable *pTab = (CTable*)( m_pTADB->Get( _T("PHYSTYPE_TAB") ).MP );
	ASSERT( NULL != pTab );
	
	CString str;

	for( int i = 1; i < _UNITS_NUMBER_OF; i++ )
	{
		str = ( (CDB_String*)m_pTADB->Get( _UNIT_STRING_ID[i] ).MP )->GetString();
		m_ComboPhysType.AddString( str );
	}

	m_ComboPhysType.SetCurSel( m_iPhysType - 1 );
}

void CDlgUnitConv::_FillComboUnit12()
{
	ASSERT( ( m_iPhysType > 0 ) && ( m_iPhysType < _UNITS_NUMBER_OF ) );
		
	TCHAR tcName[_MAXCHARS];

	if( m_ComboUnit1.GetCount() > 0 )
	{
		m_ComboUnit1.ResetContent();
	}

	if( m_ComboUnit2.GetCount() > 0 )
	{
		m_ComboUnit2.ResetContent();
	}
	
	for( int i = 0 ; i < m_pUnitDB->GetLength( m_iPhysType ) ; i++ )
	{
		GetNameOf( m_pUnitDB->GetUnit( m_iPhysType, i), tcName );
		m_ComboUnit1.AddString( tcName );
		m_ComboUnit2.AddString( tcName );
	}
	
	if( true == m_bModified )
	{
		m_iUnit1 = m_pUnitDB->GetDefaultUnitIndex( m_iPhysType );
		m_iUnit2 = m_iUnit1;
	}

	m_ComboUnit1.SetCurSel( m_iUnit1 );
	m_ComboUnit2.SetCurSel( m_iUnit2 );
}

void CDlgUnitConv::_Convert1to2()
{
	double dValue;
	double dValueSI;
	CString str;

	switch( ReadDouble( m_EditValue1, &dValue ) )
	{
		case RD_EMPTY:
			m_EditValue2.SetWindowText( _T("") );
			break;

		case RD_NOT_NUMBER:
			m_EditValue1.GetWindowText( str );

			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) 
					|| _T("e-") == str.Right( 2 ) )
			{
				return;
			}

			if( _U_TEMPERATURE == m_iPhysType )
			{
				if( _T("-") == str )
				{
					return;
				}
				else
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
				}
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}

			m_EditValue1.SetWindowText( _T("") );
			break;

		case RD_OK:
			UnitDesign_struct ud = m_pUnitDB->GetUnit( m_iPhysType, m_iUnit1 );
			dValueSI = GetConvOf( ud ) * ( dValue + GetOffsetOf( ud ) );

			if( _U_TEMPERATURE == m_iPhysType && dValueSI < -273.15 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_BELOW_ABSZERO );
				m_EditValue1.SetWindowText( _T("") );
				return;
			}
			else
			{
				ud = m_pUnitDB->GetUnit( m_iPhysType, m_iUnit2 );
				dValue = dValueSI / GetConvOf( ud ) - GetOffsetOf( ud );
				m_EditValue2.SetWindowText( WriteDouble( dValue, 4, 0, true ) );
			}

			break;
	};
}

void CDlgUnitConv::_Convert2to1()
{
	double dValue;
	double dValueSI;
	CString str;
	
	switch( ReadDouble( m_EditValue2, &dValue ) ) 
	{
		case RD_EMPTY:
			m_EditValue1.SetWindowText( _T("") );
			break;

		case RD_NOT_NUMBER:
			m_EditValue2.GetWindowText( str );

			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) 
					|| _T("e-") == str.Right( 2 ) )
			{
				return;
			}

			if( _U_TEMPERATURE == m_iPhysType )
			{
				if( _T("-") == str )
				{
					return;
				}
				else
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
				}
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}

			m_EditValue2.SetWindowText( _T("") );
			break;

		case RD_OK:
			UnitDesign_struct ud = m_pUnitDB->GetUnit( m_iPhysType, m_iUnit2 );
			dValueSI = GetConvOf( ud ) * ( dValue + GetOffsetOf( ud ) );

			if( _U_TEMPERATURE == m_iPhysType && dValueSI < -273.15 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_BELOW_ABSZERO );
				m_EditValue2.SetWindowText( _T("") );
			}
			else
			{
				ud = m_pUnitDB->GetUnit( m_iPhysType, m_iUnit1 );
				dValue = dValueSI / GetConvOf( ud ) - GetOffsetOf( ud );
				m_EditValue1.SetWindowText( WriteDouble( dValue, 4, 0, true ) );
			}

			break;
	};
}

void CDlgUnitConv::_SetStaticUnit1()
{
	// Modify the text in m_StaticUnit1.
	TCHAR tcName[_MAXCHARS];

	if( _C_KVCVCOEFF == m_iPhysType )
	{
		_tcsncpy_s( tcName, SIZEOFINTCHAR( tcName ), _T(""), SIZEOFINTCHAR( tcName ) - 1 );
	}
	else
	{
		GetNameOf( m_pUnitDB->GetUnit( m_iPhysType, m_iUnit1 ), tcName );
	}

	m_StaticUnit1.SetWindowText( tcName );
}

void CDlgUnitConv::_SetStaticUnit2()
{
	// Modify the text in m_StaticUnit2.
	TCHAR tcName[_MAXCHARS];
	
	if( _C_KVCVCOEFF == m_iPhysType )
	{
		_tcsncpy_s( tcName, SIZEOFINTCHAR( tcName ), _T(""), SIZEOFINTCHAR( tcName ) - 1 );
	}
	else
	{
		GetNameOf( m_pUnitDB->GetUnit( m_iPhysType, m_iUnit2 ), tcName );
	}

	m_StaticUnit2.SetWindowText( tcName );
}
