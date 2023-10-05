#include "stdafx.h"

#include <math.h>
#include "TASelect.h"
#include "MainFrm.h"
#include "WinSysUtil.h"
#include "Hydronic.h"
#include "DlgTALink.h"
#include "afxdialogex.h"


BEGIN_MESSAGE_MAP( CDlgTALink::CMyEdit, CEdit )
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void CDlgTALink::CMyEdit::OnLButtonDown( UINT nFlags, CPoint point )
{
	__super::OnLButtonDown( nFlags, point );
	SetSel( 0, -1 );
}

BEGIN_MESSAGE_MAP( CDlgTALink::CMyExtEdit, CExtNumEdit )
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

void CDlgTALink::CMyExtEdit::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( true == m_bEnabled )
	{
		__super::OnLButtonDown( nFlags, point );
		SetSel( 0, -1 );
	}
}

void CDlgTALink::CMyExtEdit::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	if( true == m_bEnabled )
	{
		__super::OnLButtonDblClk( nFlags, point );
	}
}

void CDlgTALink::CMyExtEdit::OnRButtonDown( UINT nFlags, CPoint point )
{
	if( true == m_bEnabled )
	{
		__super::OnRButtonDown( nFlags, point );
	}
}

void CDlgTALink::CMyExtEdit::OnRButtonDblClk( UINT nFlags, CPoint point )
{
	if( true == m_bEnabled )
	{
		__super::OnRButtonDblClk( nFlags, point );
	}
}

void CDlgTALink::CMyExtEdit::OnMButtonDown( UINT nFlags, CPoint point )
{
	if( true == m_bEnabled )
	{
		__super::OnMButtonDown( nFlags, point );
	}
}

void CDlgTALink::CMyExtEdit::OnMButtonDblClk( UINT nFlags, CPoint point )
{
	if( true == m_bEnabled )
	{
		__super::OnMButtonDblClk( nFlags, point );
	}
}

BOOL CDlgTALink::CMyExtEdit::OnSetCursor( CWnd *pWnd, UINT nHitTest, UINT message )
{
	if( true == m_bEnabled )
	{
		return __super::OnSetCursor( pWnd, nHitTest, message );
	}
	else
	{
		return TRUE;
	}
}

IMPLEMENT_DYNAMIC( CDlgTALink, CDialogEx )

CDlgTALink::CDlgTALink( CWnd *pParent )
	: CDialogEx( CDlgTALink::IDD, pParent )
{
	m_bBlockEnHandlers = false;
	m_pValve = NULL;
	m_dSetting = 0.0;
	m_dFlow = -1.0;
	m_dOutput = -1.0;
	m_dKvComputed = -1.0;
	m_dDpComputed = -1.0;
	m_bOutputError = false;
	m_eLastInput = LastInput::NotYet;
	m_pCurrentTALinkProduct = NULL;
	m_pclWaterChar = TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData();
}

CDlgTALink::~CDlgTALink()
{
	if( m_vecTALinkList.size() > 0 )
	{
		for( int iLoop = 0; iLoop < (int)m_vecTALinkList.size(); iLoop++ )
		{
			delete m_vecTALinkList[iLoop];
		}

		m_vecTALinkList.clear();
	}
}

BEGIN_MESSAGE_MAP( CDlgTALink, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOVALVETYPE, OnSelChangeComboValveType )
	ON_CBN_SELCHANGE( IDC_COMBOVALVEFAMILY, OnSelChangeComboValveFamily )
	ON_CBN_SELCHANGE( IDC_COMBOVALVE, OnSelChangeComboValve )
	ON_CBN_SELCHANGE( IDC_COMBOTALINK, OnSelChangeComboTALink )
	ON_EN_CHANGE( IDC_EDITSETTING, &OnEnChangeEditSetting )
	ON_EN_CHANGE( IDC_EDITFLOW, &OnEnChangeEditFlow )
	ON_EN_CHANGE( IDC_EDITOUTPUT, &OnEnChangeEditOutput )
	ON_BN_CLICKED( IDOK, &OnBnClickedOk )
	ON_BN_CLICKED( IDC_MFCCHOICEBUTTON, &OnClickedMFCChoiceButton )
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgTALink::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOVALVETYPE, m_ComboValveType );
	DDX_Control( pDX, IDC_COMBOVALVEFAMILY, m_ComboValveFamily );
	DDX_Control( pDX, IDC_COMBOVALVE, m_ComboValve );
	DDX_Control( pDX, IDC_COMBOTALINK, m_ComboTALink );
	DDX_Control( pDX, IDC_EDITSETTING, m_EditSetting );
	DDX_Control( pDX, IDC_STATICKVVALUE, m_EditKv );
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow );
	DDX_Control( pDX, IDC_EDITOUTPUT, m_EditOutput );
	DDX_Control( pDX, IDC_STATICDPVALUE, m_EditDp );
	DDX_Control( pDX, IDC_STATICDPMAXVALUE, m_EditDpMax );
	DDX_Control( pDX, IDC_MFCCHOICEBUTTON, m_MFCChoiceButton );
	DDX_Control( pDX, IDOK, m_ButtonOK );
}

BOOL CDlgTALink::OnInitDialog()
{
	// Do the default initialization.
	CDialogEx::OnInitDialog();

	// Initialization of window text.
	CString str = TASApp.LoadLocalizedString( IDS_DLGTALINK_CAPTION );
	SetWindowText(str);

	// Initialization of static text.
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_GROUPVALVETEXT );
	GetDlgItem( IDC_GROUPVALVE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICVALVETYPE );
	GetDlgItem( IDC_STATICVALVETYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICVALVEFAMILY );
	GetDlgItem( IDC_STATICVALVEFAMILY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICVALVE );
	GetDlgItem( IDC_STATICVALVE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICSETTING );
	GetDlgItem( IDC_STATICSETTING )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	m_ButtonOK.SetWindowText( str );

	// For Kv we need to check what is the current unit (Kv or Cv).
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	GetNameOf( pUnitDB->GetDefaultUnit( _C_KVCVCOEFF ), tcName );
	GetDlgItem( IDC_STATICKVTEXT )->SetWindowText( tcName );

	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_GROUPTALINKTEXT );
	GetDlgItem( IDC_GROUPTALINK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICTALINK );
	GetDlgItem( IDC_STATICTALINKPRODUCT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICOUTPUT );
	GetDlgItem( IDC_STATICOUTPUT )->SetWindowText( str );

	// Set the text for the flow unit.
	// Remark: Output signal unit is set when combo is filled.
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICFLOW ) + _T(" (");
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	str += CString( tcName ) + _T(")");
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );

	// Set the text for the Dp unit.
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICDP ) + _T(" (");
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	str += CString( tcName ) + _T(")");
	GetDlgItem( IDC_STATICDP )->SetWindowText( str );

	// Set the text for the Dp max unit.
	str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICDPMAX ) + _T(" (");
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	str += CString( tcName ) + _T(")");
	GetDlgItem( IDC_STATICDPMAX )->SetWindowText( str );

	m_EditDp.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );
	m_EditDp.EnableMouseEvent( false );

	_FillComboValveType();
	OnSelChangeComboValveType();

	_InitializeTALinkList();
	_FillComboTALink();
	OnSelChangeComboTALink();

	// No text at starting.
	m_EditKv.SetWindowText( _T("") );
	m_EditFlow.SetWindowText( _T("") );
	m_EditOutput.SetWindowText( _T("") );
	m_EditDp.SetWindowText( _T("") );

	OnClickedMFCChoiceButton();

	// Set default focus on setting.
	m_EditSetting.SetFocus();
	
	return TRUE;
}

void CDlgTALink::OnOK()
{
	// Don't call base class! (it's to avoid to loose focus when pressing Escape or Enter key).
}

void CDlgTALink::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

BOOL CDlgTALink::PreTranslateMessage( MSG *pMsg )
{
	BOOL bReturn = 0;

	// If it's not enter or escape key, we can transmit message.
	if( WM_KEYDOWN != pMsg->message || VK_RETURN != pMsg->wParam )
	{
		bReturn = CDialogEx::PreTranslateMessage( pMsg );
	}

	return bReturn;
}

void CDlgTALink::OnSelChangeComboValveType()
{
	_FillComboValveFamily();
	OnSelChangeComboValveFamily();
}

void CDlgTALink::OnSelChangeComboValveFamily()
{
	_FillComboValve();
	OnSelChangeComboValve();
}

void CDlgTALink::OnSelChangeComboValve()
{
	IDPTR ValveIDPtr = m_ComboValve.GetCBCurSelIDPtr();
	
	if( NULL == ValveIDPtr.MP )
	{
		return;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( ValveIDPtr.MP );

	if( NULL == pTAP )
	{
		return;
	}

	CDB_ValveCharacteristic *pValveChar = (CDB_ValveCharacteristic *)pTAP->GetValveCharDataPointer();

	if( NULL == pValveChar )
	{
		return;
	}

	// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
	// to be treated when it arrives in base class.
	m_bBlockEnHandlers = true;
	m_pValve = pTAP;

	if( false == pTAP->IsKvSignalEquipped() )
	{
		GetDlgItem( IDC_STATICSETTING )->ShowWindow( SW_SHOW );
		m_EditSetting.ShowWindow( SW_SHOW );

		TCHAR name[_MAXCHARS];
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		GetNameOf( pUnitDB->GetDefaultUnit( _C_KVCVCOEFF ), name );
		GetDlgItem( IDC_STATICKVTEXT )->SetWindowText( name );

		CString str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICSETTING );
		
		double dOpeningMax = pValveChar->GetOpeningMax();

		if( -1.0 != dOpeningMax )
		{
			str += CString( _T(" (") ) + TASApp.LoadLocalizedString( IDS_DLGTALINK_MAX );
			str += CString( _T(" ") ) + pValveChar->GetSettingString( dOpeningMax ) + _T(")");
		}

		GetDlgItem( IDC_STATICSETTING )->SetWindowText( str );

		if( m_dSetting > 0.0 )
		{
			if( m_dSetting > dOpeningMax )
			{
				m_dSetting = dOpeningMax;
			}

			str = pValveChar->GetSettingString( m_dSetting );
			m_EditSetting.SetWindowText( str );
		}
		else
		{
			m_EditSetting.SetWindowText( _T("") );
		}
	}
	else
	{
		GetDlgItem( IDC_STATICSETTING )->ShowWindow( SW_HIDE );
		m_EditSetting.ShowWindow( SW_HIDE );

		// Show Kv (or Cv) signal static text.
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		CString str = TASApp.LoadLocalizedString( ( false == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) ) ? IDS_DLGTALINK_KVSIGNAL : IDS_DLGTALINK_CVSIGNAL );
		GetDlgItem( IDC_STATICKVTEXT )->SetWindowText( str );

		// Show Kv (or Cv) value.
		m_EditKv.SetWindowText( WriteCUDouble( _C_KVCVCOEFF, pTAP->GetKvSignal(), false, 6, 6 ) );
	}

	_Calculate( ComeFrom::EditValve );
	m_bBlockEnHandlers = false;
}

void CDlgTALink::OnSelChangeComboTALink()
{
	int iIndex = m_ComboTALink.GetCurSel();

	if( iIndex < 0 )
	{
		return;
	}

	TALinkProduct *pTALinkProduct = m_vecTALinkList[iIndex];

	if( NULL == pTALinkProduct )
	{
		return;
	}

	// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
	// to be treated when it arrives in base class.
	m_bBlockEnHandlers = true;
	m_pCurrentTALinkProduct = pTALinkProduct;

	// Set the text for the output signal unit.
	CString str = TASApp.LoadLocalizedString( IDS_DLGTALINK_STATICOUTPUT ) + _T(" (");
	CString strUnit;

	switch( pTALinkProduct->eUnit )
	{
		case TLUnit::Volt:
			str += TASApp.LoadLocalizedString( IDS_DLGTALINK_TALINKUNITVOLT ) + _T(")");
			break;

		case TLUnit::A:
			str += TASApp.LoadLocalizedString( IDS_DLGTALINK_TALINKUNITMA ) + _T(")");
			break;
	}
	
	GetDlgItem( IDC_STATICOUTPUT )->SetWindowText( str );

	// Verify current output value.
	if( m_dOutput != -1.0 && ( m_dOutput < m_pCurrentTALinkProduct->dLowOutputValue || m_dOutput > m_pCurrentTALinkProduct->dHighOutputValue ) )
	{
		m_dOutput = ( m_dOutput < m_pCurrentTALinkProduct->dLowOutputValue ) ? m_pCurrentTALinkProduct->dLowOutputValue : m_pCurrentTALinkProduct->dHighOutputValue;
		m_EditOutput.SetWindowText( WriteDouble( m_dOutput, 2, 2 ) );
	}

	// Change Dp max value.
	m_EditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_pCurrentTALinkProduct->dHighDpValue ) );

	_Calculate( ComeFrom::EditTALink );
	m_bBlockEnHandlers = false;
}

void CDlgTALink::OnClickedMFCChoiceButton()
{
	m_bBlockEnHandlers = true;

	if( LastInput::Flow == m_eLastInput )
	{
		m_MFCChoiceButton.SetImage( IDB_LEFTARROWGREEN );

		if( m_dFlow <= 0.0 && m_dOutput <= 0.0 )
		{
			m_EditFlow.SetWindowText( _T("") );
		}
		
		// 'm_EditFlow' control is a CExtNumEdit. When it is set in read only, then background is gray. To get background color for dialog, call ::GetSysColor( COLOR_3DFACE )!!
		// It is needed to call 'SetBackColor' to force default solid brush to have same color to paint background (see CExtNumEdit::SetBackColor).
		m_EditFlow.SetReadOnly( TRUE );
		m_EditFlow.ModifyStyle( WS_TABSTOP, 0 );
		m_EditFlow.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditFlow.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );
		m_EditFlow.EnableMouseEvent( false );

		m_EditOutput.EnableWindow( TRUE );
		m_EditOutput.ModifyStyle( 0, WS_TABSTOP );
		m_EditOutput.SetFocus();
		m_EditOutput.SetSel( 0, -1 );
		m_eLastInput = LastInput::OutputSignal;
	}
	else
	{
		m_MFCChoiceButton.SetImage( IDB_RIGHTARROWGREEN );

		if( m_dFlow <= 0.0 && m_dOutput <= 0.0 )
		{
			m_EditOutput.SetWindowText( _T("") );
		}

		m_EditFlow.SetReadOnly( FALSE );
		m_EditFlow.ModifyStyle( 0, WS_TABSTOP );
		m_EditFlow.SetTextColor( ::GetSysColor( COLOR_WINDOWTEXT ) );
		m_EditFlow.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditFlow.EnableMouseEvent( true );

		m_EditFlow.SetFocus();
		m_EditFlow.SetSel( 0, -1 );
		m_EditOutput.EnableWindow( FALSE );
		m_EditOutput.ModifyStyle( WS_TABSTOP, 0 );
		m_eLastInput = LastInput::Flow;
	}

	m_bBlockEnHandlers = false;
}

void CDlgTALink::OnEnChangeEditSetting()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditSetting )
	{
		// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
		// to be treated when it arrives in base class.
		m_bBlockEnHandlers = true;

		CDB_ValveCharacteristic *pValveChar = (CDB_ValveCharacteristic *)m_pValve->GetValveCharDataPointer();

		if( NULL == pValveChar )
		{
			ASSERT_RETURN;
		}

		double dSetting = -1.0;
		double dOpeningMax = pValveChar->GetOpeningMax();
		
		switch( ReadDouble( m_EditSetting, &dSetting ) )
		{
			case ReadDoubleReturn_enum::RD_OK:
				if( dOpeningMax > 0.0 && dSetting > dOpeningMax )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_SETTINGTOHIGH );
					m_dSetting = dOpeningMax;
					m_EditSetting.SetSel( 0, -1 );
					m_EditSetting.ReplaceSel( pValveChar->GetSettingString( m_dSetting ) );
					m_EditSetting.SetSel( 0, -1 );
					m_EditSetting.SetFocus();
				}
				else if( dSetting < 0.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
					m_dSetting = -1.0;
					m_EditSetting.SetSel( 0, -1 );
					m_EditSetting.ReplaceSel( _T("") );
					m_EditSetting.SetFocus();
				}
				else
				{
					m_dSetting = dSetting;
				}

				break;

			case ReadDoubleReturn_enum::RD_NOT_NUMBER:
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
				m_dSetting = -1.0;
				m_EditSetting.SetSel( 0, -1 );
				m_EditSetting.ReplaceSel( _T("") );
				m_EditSetting.SetFocus();
				break;

			case ReadDoubleReturn_enum::RD_EMPTY:
				m_dSetting = -1.0;
				m_EditSetting.SetSel( 0, -1 );
				m_EditSetting.ReplaceSel( _T("") );
				m_EditSetting.SetFocus();
				break;
		}

		_Calculate( ComeFrom::EditSetting );
		m_bBlockEnHandlers = false;
	}
}

void CDlgTALink::OnEnChangeEditFlow()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditFlow )
	{
		// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
		// to be treated when it arrives in base class.
		m_bBlockEnHandlers = true;
		double dFlow = -1.0;

		switch( ReadCUDouble( _U_FLOW, m_EditFlow, &dFlow ) )
		{
			case ReadDoubleReturn_enum::RD_OK:
		
				if( dFlow < 0.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
					m_dFlow = 0.0;
					m_EditFlow.SetSel( 0, -1 );
					m_EditFlow.ReplaceSel( _T("") );
					m_EditFlow.SetFocus();
				}
				else
				{
					m_dFlow = dFlow;
				}

				break;

			case ReadDoubleReturn_enum::RD_NOT_NUMBER:
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
				m_dFlow = -1.0;
				m_EditFlow.SetSel( 0, -1 );
				m_EditFlow.ReplaceSel( _T("") );
				m_EditFlow.SetFocus();
				break;

			case ReadDoubleReturn_enum::RD_EMPTY:
				m_dFlow = -1.0;
				m_EditFlow.SetSel( 0, -1 );
				m_EditFlow.ReplaceSel( _T("") );
				m_EditFlow.SetFocus();
				break;
		}

		_Calculate( ComeFrom::EditFlow );
		m_bBlockEnHandlers = false;
	}
}

void CDlgTALink::OnEnChangeEditOutput()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditOutput )
	{
		// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
		// to be treated when it arrives in base class.
		m_bBlockEnHandlers = true;
		double dOutput = -1.0;

		switch( ReadDouble( m_EditOutput, &dOutput ) )
		{
			case ReadDoubleReturn_enum::RD_OK:
				// Because we do computing at each user input, it's not possible to verify the low limit. For example, if user
				// has chosen TALink 4-20mA and want to input 20mA, it will first input 2. But 2 is below 4mA and UI will display an
				// error message. We do not want that.
				if( dOutput > m_pCurrentTALinkProduct->dHighOutputValue )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_INVALID_OUTPUT_VALUE );
					m_dOutput = m_pCurrentTALinkProduct->dHighOutputValue;
					m_EditOutput.SetSel( 0, -1 );
					m_EditOutput.ReplaceSel( WriteDouble( m_dOutput, 2, 2 ) );
					m_EditOutput.SetSel( 0, -1 );
					m_EditOutput.SetFocus();

				}
				else if( dOutput < 0.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
					m_dOutput = -1.0;
					m_EditOutput.SetSel( 0, -1 );
					m_EditOutput.ReplaceSel( _T("") );
					m_EditOutput.SetFocus();
				}
				else
				{
					m_dOutput = dOutput;
				}

				break;

			case ReadDoubleReturn_enum::RD_NOT_NUMBER:
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
				m_dOutput = -1.0;
				m_EditOutput.SetSel( 0, -1 );
				m_EditOutput.ReplaceSel( _T("") );
				m_EditOutput.SetFocus();
				break;

			case ReadDoubleReturn_enum::RD_EMPTY:
				m_dOutput = -1.0;
				m_EditOutput.SetSel( 0, -1 );
				m_EditOutput.ReplaceSel( _T("") );
				m_EditOutput.SetFocus();
				break;
		}

		_Calculate( ComeFrom::EditOutput );
		m_bBlockEnHandlers = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgTALink::_InitializeTALinkList()
{
	TALinkProduct *pTALinkProduct = new TALinkProduct();

	if( NULL == pTALinkProduct )
	{
		return;
	}

	pTALinkProduct->eUnit = TLUnit::Volt;
	pTALinkProduct->dLowDpValue = 0.0;
	pTALinkProduct->dHighDpValue = 40000.0;
	pTALinkProduct->dLowOutputValue = 0.0;
	pTALinkProduct->dHighOutputValue = 10.0;
	pTALinkProduct->strName = _T("TA Link 0-10V (52 010-004)");
	m_vecTALinkList.push_back( pTALinkProduct );

	pTALinkProduct = new TALinkProduct();

	if( NULL == pTALinkProduct )
	{
		return;
	}

	pTALinkProduct->eUnit = TLUnit::Volt;
	pTALinkProduct->dLowDpValue = 0.0;
	pTALinkProduct->dHighDpValue = 100000.0;
	pTALinkProduct->dLowOutputValue = 0.0;
	pTALinkProduct->dHighOutputValue = 10.0;
	pTALinkProduct->strName = _T("TA Link 0-10V (52 010-010)");
	m_vecTALinkList.push_back( pTALinkProduct );

	pTALinkProduct = new TALinkProduct();

	if( NULL == pTALinkProduct )
	{
		return;
	}

	pTALinkProduct->eUnit = TLUnit::A;
	pTALinkProduct->dLowDpValue = 0.0;
	pTALinkProduct->dHighDpValue = 100000.0;
	pTALinkProduct->dLowOutputValue = 4;
	pTALinkProduct->dHighOutputValue = 20;
	pTALinkProduct->strName = _T("TA Link 4-20mA (52 110-010)");
	m_vecTALinkList.push_back( pTALinkProduct );
}

void CDlgTALink::_FillComboValveType( CString ValveTypeID )
{
	CRankEx TempValveTypeList;
	TASApp.GetpTADB()->GetBVTypeList( &TempValveTypeList );

	// Loop on all valves to exclude type that doesn't contain valve with Dp signal.
	_string strTypeName;
	LPARAM lpTypeStringID;
	CRankEx ValveTypeList;
	
	for( BOOL bContinue = TempValveTypeList.GetFirst( strTypeName, lpTypeStringID ); TRUE == bContinue; bContinue = TempValveTypeList.GetNext( strTypeName, lpTypeStringID ) )
	{
		CRankEx ValveList;
		CDB_StringID *pTypeID = dynamic_cast<CDB_StringID*>( (CData *)lpTypeStringID );
		ASSERT( NULL != pTypeID );

		if( NULL == pTypeID )
		{
			continue;
		}
		
		TASApp.GetpTADB()->GetBVList( &ValveList, pTypeID->GetIDPtr().ID, _T(""), _T(""), _T(""), _T("") );
		
		_string strValveName;
		LPARAM lpTAProduct;

		for( BOOL bContinue2 = ValveList.GetFirst( strValveName, lpTAProduct ); TRUE == bContinue2; bContinue2 = ValveList.GetNext( strValveName, lpTAProduct ) )
		{
			CDB_TAProduct *pTAP = (CDB_TAProduct *)lpTAProduct;
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}

			// As soon we found a valve that has Dp signal capabilities, we can stop and insert type in the list.
			if( CDB_TAProduct::eMeasurableData::emdNone != pTAP->GetMeasurableData() )
			{
				ValveTypeList.Add( pTypeID->GetString(), _ttoi( pTypeID->GetIDstr() ) - 1, lpTypeStringID );
				break;
			}
		}

		ValveList.PurgeAll();
	}

	TempValveTypeList.PurgeAll();
	m_ComboValveType.FillInCombo( &ValveTypeList, ValveTypeID, 0 );
}

void CDlgTALink::_FillComboValveFamily( CString strValveFamilyID )
{
	CRankEx TempValveFamilyList;
	TASApp.GetpTADB()->GetBVFamilyList( &TempValveFamilyList, m_ComboValveType.GetCBCurSelIDPtr().ID );

	// Loop on all valves to exclude family that doesn't contain valve with Dp signal.
	_string strFamilyName;
	LPARAM lpFamilyStringID;
	CRankEx ValveFamilyList;

	for( BOOL bContinue = TempValveFamilyList.GetFirst( strFamilyName, lpFamilyStringID ); TRUE == bContinue; bContinue = TempValveFamilyList.GetNext( strFamilyName, lpFamilyStringID ) )
	{
		CRankEx ValveList;
		CDB_StringID *pFamilyID = dynamic_cast<CDB_StringID*>( (CData *)lpFamilyStringID );
		ASSERT( NULL != pFamilyID );
		
		if( NULL == pFamilyID )
		{
			continue;
		}
		
		TASApp.GetpTADB()->GetBVList( &ValveList, m_ComboValveType.GetCBCurSelIDPtr().ID, pFamilyID->GetIDPtr().ID, _T(""), _T(""), _T("") );
		
		_string strValveName;
		LPARAM lpTAProduct;

		for( BOOL bContinue2 = ValveList.GetFirst( strValveName, lpTAProduct ); TRUE == bContinue2; bContinue2 = ValveList.GetNext( strValveName, lpTAProduct ) )
		{
			CDB_TAProduct *pTAP = (CDB_TAProduct *)lpTAProduct;
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}

			// As soon we found a valve that has Dp signal capabilities, we can stop and insert family in the list.
			if( CDB_TAProduct::eMeasurableData::emdNone != pTAP->GetMeasurableData() )
			{
				double dKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL ) );
				ValveFamilyList.Add( pFamilyID->GetString(), dKey, lpFamilyStringID );
				break;
			}
		}

		ValveList.PurgeAll();
	}
	
	TempValveFamilyList.PurgeAll();
	m_ComboValveFamily.FillInCombo( &ValveFamilyList, strValveFamilyID, 0 );
}

void CDlgTALink::_FillComboValve( CString strValveID )
{
	CRankEx TempValveList;
	TASApp.GetpTADB()->GetBVList( &TempValveList, m_ComboValveType.GetCBCurSelIDPtr().ID, m_ComboValveFamily.GetCBCurSelIDPtr().ID, _T(""), _T(""), _T("") );
	
	// Loop on list to take only valves that has Dp signal capabilities.
	_string str;
	LPARAM itemdata;
	CRankEx ValveList;
	CDB_TAProduct *pMatchTAP = NULL;

	for( BOOL bContinue = TempValveList.GetFirst( str, itemdata ); TRUE == bContinue; bContinue = TempValveList.GetNext( str, itemdata ) )
	{
		CDB_TAProduct *pTAP = (CDB_TAProduct *)itemdata;
		ASSERT( NULL != pTAP );

		if( NULL == pTAP )
		{
			continue;
		}

		if( CDB_TAProduct::eMeasurableData::emdNone != pTAP->GetMeasurableData() )
		{
			if( true == TASApp.GetpTADB()->CheckIfCharactAlreadyExist( &ValveList, pTAP, pMatchTAP ) )
			{
				CData *pValveChar = pTAP->GetValveCharDataPointer();

				if( NULL != pValveChar )
				{
					ValveList.Add( pTAP->GetName(), (double)pTAP->GetSortingKey(), (LPARAM)( pTAP->GetIDPtr().MP ) );
					pMatchTAP = pTAP;
				}
			}
		}
	}
	
	TempValveList.PurgeAll();
	m_ComboValve.FillInCombo( &ValveList, strValveID, 0 );
}

void CDlgTALink::_FillComboTALink()
{
	// At now we hard code 3 TALink products (see .h for remarks)
	for( int iLoop = 0; iLoop < (int)m_vecTALinkList.size(); iLoop++ )
	{
		m_ComboTALink.AddString( m_vecTALinkList[iLoop]->strName );
	}
	
	m_ComboTALink.SetCurSel( 0 );
	m_pCurrentTALinkProduct = m_vecTALinkList[0];
}

void CDlgTALink::_Calculate( int iComeFrom )
{
	// If user has not yet enter flow or output signal, we do any computing.
	if( LastInput::NotYet == m_eLastInput )
	{
		return;
	}

	if( true == m_bOutputError )
	{
		m_EditFlow.SetTextColor( _BLACK );
		m_EditDp.SetTextDisabledColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_bOutputError = false;
	}

	if( LastInput::Flow == m_eLastInput )
	{
		_CalculateOutput();
	}
	else
	{
		_CalculateFlow();
	}

	switch( iComeFrom )
	{
		case ComeFrom::EditValve:
		case ComeFrom::EditSetting:
		case ComeFrom::EditFlow:
		case ComeFrom::EditOutput:
			_CalculateKv();
			break;

		case ComeFrom::EditTALink:
		default:
			break;
	}
}

void CDlgTALink::_CalculateKv()
{
	if( false == m_pValve->IsKvSignalEquipped() )
	{
		m_dKvComputed = DBL_MAX;

		if( m_dDpComputed > 0.0 )
		{
			m_dKvComputed = CalcKv( m_dFlow, m_dDpComputed, m_pclWaterChar->GetDens() );
		}

		if( DBL_MAX == m_dKvComputed )
		{
			m_dKvComputed = -1.0;
		}

		// Display Kv.
		m_EditKv.SetWindowText( ( m_dKvComputed >= 0.0 ) ? WriteCUDouble( _C_KVCVCOEFF, m_dKvComputed, false ) : _T("") );
	}
}

void CDlgTALink::_CalculateOutput()
{
	// Compute Dp.
	m_dDpComputed = -1.0;

	if( m_dFlow > 0.0 && m_dSetting > 0.0 )
	{
		if( false == m_pValve->IsKvSignalEquipped() )
		{
			CDB_ValveCharacteristic *pValveChar = (CDB_ValveCharacteristic *)m_pValve->GetValveCharDataPointer();

			if( NULL != pValveChar )
			{
				pValveChar->GetValveDp( m_dFlow, &m_dDpComputed, m_dSetting, m_pclWaterChar->GetDens(), m_pclWaterChar->GetKinVisc() );
			}
		}
		else
		{
			if( -1.0 != m_pValve->GetKvSignal() )
			{
				m_dDpComputed = CalcDp( m_dFlow, m_pValve->GetKvSignal(), m_pclWaterChar->GetDens() );
			}
		}
	}

	// Display Dp.
	m_EditDp.SetWindowText( ( m_dDpComputed >= 0.0 ) ? WriteCUDouble( _U_DIFFPRESS, m_dDpComputed, false ) : _T("") );

	// To compute Output, we have ( Output - OutputMin ) / ( OutputMax - OutputMin ) = ( Dp - DpMin ) / ( DpMax - DpMin )
	// Output = ( Dp - DpMin ) / ( DpMax - DpMin ) * ( OutputMax - OutputMin ) + OutputMin
	m_dOutput = -1.0;

	if( m_dDpComputed >= 0.0 )
	{
		m_dOutput = ( m_dDpComputed - m_pCurrentTALinkProduct->dLowDpValue ) / ( m_pCurrentTALinkProduct->dHighDpValue - m_pCurrentTALinkProduct->dLowDpValue );
		m_dOutput *= ( m_pCurrentTALinkProduct->dHighOutputValue - m_pCurrentTALinkProduct->dLowOutputValue );
		m_dOutput += m_pCurrentTALinkProduct->dLowOutputValue;
	}

	// Display output.
	CString str = _T("");
	
	if( m_dOutput >= 0.0 )
	{
		if( m_dOutput > m_pCurrentTALinkProduct->dHighOutputValue )
		{
			m_bOutputError = true;
		}
		else
		{
			str = WriteDouble( m_dOutput, 2, 2 );
		}
	}

	m_EditOutput.SetWindowText( str );
	
	if( true == m_bOutputError )
	{
		m_EditFlow.SetTextColor( _RED );
		m_EditDp.SetTextDisabledColor( _RED );
	}
}

void CDlgTALink::_CalculateFlow()
{
	// To compute Dp, we have ( Output - OutputMin ) / ( OutputMax - OutputMin ) = ( Dp - DpMin ) / ( DpMax - DpMin )
	// Dp = ( Output - OutputMin ) * ( DpMax - DpMin ) / ( OutputMax - OutputMin ) + DpMin
	m_dDpComputed = ( m_dOutput - m_pCurrentTALinkProduct->dLowOutputValue ) * ( m_pCurrentTALinkProduct->dHighDpValue - m_pCurrentTALinkProduct->dLowDpValue );
	m_dDpComputed /= ( m_pCurrentTALinkProduct->dHighOutputValue - m_pCurrentTALinkProduct->dLowOutputValue );
	m_dDpComputed += m_pCurrentTALinkProduct->dLowDpValue;

	// Display Dp.
	m_EditDp.SetWindowText( ( m_dDpComputed >= 0.0 ) ? WriteCUDouble( _U_DIFFPRESS, m_dDpComputed, true ) : _T("") );

	// Compute Flow.
	m_dFlow = -1.0;

	if( m_dDpComputed > 0.0 && m_dSetting > 0.0 )
	{
		if( false == m_pValve->IsKvSignalEquipped() )
		{
			CDB_ValveCharacteristic *pValveChar = (CDB_ValveCharacteristic *)m_pValve->GetValveCharDataPointer();

			if( NULL != pValveChar )
			{
				pValveChar->GetValveQ( &m_dFlow, m_dDpComputed, m_dSetting, m_pclWaterChar->GetDens(), m_pclWaterChar->GetKinVisc() );
			}
		}
		else
		{
			if( -1.0 != m_pValve->GetKvSignal() )
			{
				m_dFlow = CalcqT( m_pValve->GetKvSignal(), m_dDpComputed, m_pclWaterChar->GetDens() );
			}
		}
	}
	else if( 0.0 == m_dDpComputed )
	{
		m_dFlow = 0.0;
	}

	// Display Flow.
	m_EditFlow.SetWindowText( ( m_dFlow >= 0.0 ) ? WriteCUDouble( _U_FLOW, m_dFlow, false, 6, 6 ) : _T("") );
}
