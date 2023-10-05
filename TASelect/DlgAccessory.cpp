#include "stdafx.h"
#include "TASelect.h"
#include "utilities.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"

#include "DlgAccessory.h"

IMPLEMENT_DYNAMIC( CDlgAccessory, CDialogEx )

CDlgAccessory::CDlgAccessory( CTADatabase::FilterSelection eFilterSelection, CWnd* pParent )
	: CDialogEx( CDlgAccessory::IDD, pParent )
{
	m_eFilterSelection = eFilterSelection;
	m_pHM = NULL;
	m_pPipe = NULL;
	m_pfChangeDone = NULL;
	m_pTADB = TASApp.GetpTADB();
	m_RectStatic1 = CRect( 0, 0, 0, 0 );
	
	// By default the check box is set.
	m_fCheckBox = true;

	// Load members variable strings.
	m_strQrefDPref = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICQREFDPREF );
	m_strKV = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICKV );
	m_strCV = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICCV );
	m_strZeta = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICZETA );
	m_strPipe = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_EQUIVPIPELENGTH );
	m_iSelItem = -1;
	m_pSavedPipe = NULL;
	m_dQRef = 0.0;
	m_dDpRef = 0.0;
	m_dKv = 0.0;
	m_dCv = 0.0;
	m_dZeta = 0.0;
	m_dLength = 0.0;
	m_IDPTRPipe = _NULL_IDPTR;
}

CDlgAccessory::~CDlgAccessory()
{
	if( NULL != m_pSavedPipe )
		delete m_pSavedPipe;
}

INT_PTR CDlgAccessory::Display( CDS_HydroMod *pHM, CPipes *pPipe, bool* pfChangeDone )
{
	m_pHM = pHM;
	m_pPipe = pPipe;
	m_pfChangeDone = pfChangeDone;
	m_pSavedPipe = new CPipes( pHM, pPipe->GetLocate() );
	m_pPipe->Copy( m_pSavedPipe );
	return DoModal();
}

BEGIN_MESSAGE_MAP( CDlgAccessory, CDialogEx )
	ON_NOTIFY( NM_CLICK, IDC_LIST, OnNMClickList )
	ON_CBN_SELCHANGE( IDC_COMBOACCESSTYPE, OnCbnSelChangeAccessoryType )
	ON_EN_KILLFOCUS( IDC_EDIT1, OnEnKillFocusEdit1 )
	ON_EN_KILLFOCUS( IDC_EDIT2, OnEnKillFocusEdit2 )
	ON_BN_CLICKED( IDC_CHECK, OnBnClickedCheck )
	ON_CBN_SELCHANGE( IDC_COMBOPIPESERIES, OnCbnSelChangePipeSeries )
	ON_CBN_SELCHANGE( IDC_COMBOPIPESIZE, OnCbnSelChangePipeSize )
	ON_BN_CLICKED( IDC_BUTTONADD, OnBnClickedAdd )
	ON_BN_CLICKED( IDC_BUTTONMODIFY, OnBnClickedModify )
	ON_BN_CLICKED( IDC_BUTTONREMOVE, OnBnClickedRemove )
	ON_BN_CLICKED( IDOK, OnBnClickedOk )
	ON_BN_CLICKED( IDCANCEL, OnBnClickedCancel )
END_MESSAGE_MAP()

void CDlgAccessory::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICGROUPBOX, m_GroupPipe );
	DDX_Control( pDX, IDC_LIST, m_List );
	DDX_Control( pDX, IDC_STATICTOTALDPRESULT, m_StaticTotalDp );
	DDX_Control( pDX, IDC_ACCESSBOX, m_GroupAccessory );
	DDX_Control( pDX, IDC_COMBOACCESSTYPE, m_ComboAccessType );
	DDX_Control( pDX, IDC_EDITDESCRIPTION, m_EditDescription );
	DDX_Control( pDX, IDC_EDIT1, m_Edit1 );
	DDX_Control( pDX, IDC_EDIT2, m_Edit2 );
	DDX_Control( pDX, IDC_EDITNUMBERTOADD, m_EditNumberToAdd );
	DDX_Control( pDX, IDC_SPINNUMBERTOADD, m_SpinNumberToAdd );
	DDX_Control( pDX, IDC_CHECK, m_CheckBox );
	DDX_Control( pDX, IDC_COMBOPIPESERIES, m_ComboPipeSeries );
	DDX_Control( pDX, IDC_COMBOPIPESIZE, m_ComboPipeSize );
	DDX_Control( pDX, IDC_BUTTONADD, m_ButtonAdd );
	DDX_Control( pDX, IDC_BUTTONMODIFY, m_ButtonModify );
	DDX_Control( pDX, IDC_BUTTONREMOVE, m_ButtonRemove );
}

BOOL CDlgAccessory::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Get the rect of the IDC_STATICEDIT1 because the width can change if we select the Zeta coefficient.
	GetDlgItem( IDC_STATICEDIT1 )->GetWindowRect( &m_RectStatic1 );

	// String initialization.
	CString str;

	if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_pPipe->GetLocate() || CDS_HydroMod::eHMObj::eDistributionReturnPipe == m_pPipe->GetLocate() )
	{
		if( CDS_HydroMod::ReturnType::Direct == m_pHM->GetReturnType() )
		{
			// For DIRECT return mode, we use only one distribution supply pipe like before (with 2 connection singularities).
			str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DISTPIPE );
			m_GroupPipe.SetWindowText( str );
			str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CAPTIONDISTRIBUTION );
		}
		else
		{
			// For REVERSE return mode, we use separated distribution pipes: one for supply and one for return.
			if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_pPipe->GetLocate() )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DISTSUPPLYPIPE );
				m_GroupPipe.SetWindowText( str );
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CAPTIONDISTRIBUTIONSUPPLY );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DISTRETURNPIPE );
				m_GroupPipe.SetWindowText( str );
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CAPTIONDISTRIBUTIONRETURN );
			}
		}
	}
	else if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == m_pPipe->GetLocate() )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CIRCPIPE );
		m_GroupPipe.SetWindowText( str );
		str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CAPTIONPRIMARY );
	}
	else if( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe == m_pPipe->GetLocate() )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CAPTIONSECONDARY );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CAPTION );
	}

	// Set caption text.
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_ACCESSORIES );
	GetDlgItem( IDC_STATICACCESS )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_ACCESSINPUT );
	m_GroupAccessory.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_TYPE );
	GetDlgItem( IDC_STATIC )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICDESCRIPTION );
	GetDlgItem( IDC_STATICDESCRIPTION )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICNUMBERTOADD );
	GetDlgItem( IDC_STATICNUMBERTOADD )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_USEEQUIVPIPELENGTH );
	GetDlgItem( IDC_CHECK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_BUTTONADD );
	m_ButtonAdd.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_BUTTONMODIFY );
	m_ButtonModify.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_BUTTONREMOVE );
	m_ButtonRemove.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );

	m_ComboAccessType.AddString( m_strQrefDPref );
	m_ComboAccessType.AddString( m_strZeta );
	m_ComboAccessType.AddString( m_strPipe );
	m_ComboAccessType.AddString( m_strKV );
	m_ComboAccessType.AddString( m_strCV );

	m_ComboAccessType.SetCurSel( 0 );
	OnCbnSelChangeAccessoryType();

	m_SpinNumberToAdd.SetBuddy( GetDlgItem( IDC_EDITNUMBERTOADD ) );
	m_SpinNumberToAdd.SetRange( 1, 10 );
	m_SpinNumberToAdd.SetPos( 1 );

	// ListCtrl.
	ListView_SetExtendedListViewStyleEx( m_List.m_hWnd, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );
	CDC *pDC = ( (CWnd *)this )->GetDC();
	CSize size = pDC->GetTextExtent( _T("999") );
	m_List.DeleteAllItems();
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_HEADERNBR );
	m_List.InsertColumn( eColHeader::Nb, (LPCTSTR)str, LVCFMT_CENTER, size.cx, eColHeader::Nb );

	size = pDC->GetTextExtent( _T("MMMMMMMMMMMMMMMM") );
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICDESCRIPTION );
	m_List.InsertColumn( eColHeader::Desc, str, LVCFMT_CENTER, size.cx, eColHeader::Desc );

	size = pDC->GetTextExtent( _T("Generic Steel Pipes; 15(1/2I); Eq length=500m") );
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_VALUE );
	m_List.InsertColumn( eColHeader::Value, str, LVCFMT_CENTER, size.cx, eColHeader::Value );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	size = pDC->GetTextExtent( _T("5000mmH2O") );
	str = TASApp.LoadLocalizedString( IDS_DP );
	str += CString( _T(" (") ) + GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() + CString( _T(")") );
	m_List.InsertColumn( eColHeader::Dp, str, LVCFMT_CENTER, size.cx, eColHeader::Dp );

	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	( (CWnd *)this)->ReleaseDC( pDC );
	
	RefreshListCtrl();

	if( true == m_pHM->GetpPrjParam()->IsFreezed() )
	{
		m_ButtonAdd.ShowWindow( SW_HIDE );
		m_ButtonModify.ShowWindow( SW_HIDE );
		m_ButtonRemove.ShowWindow( SW_HIDE );
	}

	// Fill informations about the pipe.
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_PIPESERIES );
	str += _T(": ");

	if( true == m_pPipe->IsCompletelyDefined() )
	{
		str += m_pPipe->GetPipeSeries()->GetName();
	}

	GetDlgItem( IDC_STATICDISTPIPESERIES )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_PIPESIZE );
	str += _T(": ");

	if( true == m_pPipe->IsCompletelyDefined() )
	{
		str += dynamic_cast<CDB_Pipe*>( m_pPipe->GetIDPtr().MP )->GetName();
	}

	GetDlgItem( IDC_STATICDISTPIPESIZE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_FLOW );
	str += _T(": ");
	str += WriteCUDouble( _U_FLOW, m_pPipe->GetRealQ(), true );
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );

	// Hide unused windows.
	GetDlgItem( IDC_CHECK)->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_STATICPIPESERIES)->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_STATICPIPESIZE)->ShowWindow( SW_HIDE );
	m_ComboPipeSeries.ShowWindow( SW_HIDE );
	m_ComboPipeSize.ShowWindow( SW_HIDE );

	m_CheckBox.SetCheck( BST_CHECKED );
	m_fCheckBox = true;

	// By default, 'Add', 'Modify' and 'Remove' buttons are disabled.
	//m_ButtonAdd.EnableWindow( FALSE );
	m_ButtonModify.EnableWindow( FALSE );
	m_ButtonRemove.EnableWindow( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAccessory::OnNMClickList( NMHDR *pNMHDR, LRESULT *pResult )
{
	LVHITTESTINFO ht = { 0 };
	DWORD dwPos = GetMessagePos();
	ht.pt.x= ( (int)(short)LOWORD( dwPos ) );			//GET_X_LPARAM(dwpos);
	ht.pt.y= ( (int)(short)HIWORD( dwPos ) );			//GET_Y_LPARAM(dwpos);

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );

	UINT uFlags = 0;
	m_iSelItem = m_List.HitTest( ht.pt, &uFlags );

	if( m_iSelItem >= 0 )
	{
		// Refresh editors.
		CPipes::CSingularity Singularity;
		m_pPipe->GetSingularity( (short)m_List.GetItemData( m_iSelItem ), &Singularity );
		CString str( _T("") );
		CString strCompair( _T("") );
		
		if( Singularity.m_nID > 0 )
		{
			m_EditDescription.EnableWindow( TRUE );
			m_ButtonRemove.EnableWindow( TRUE );
			m_ButtonModify.EnableWindow( TRUE );
			
			switch( Singularity.m_eDpType )
			{
				case CDS_HydroMod::eDpType::Kv:
					
					for( int i = 0; i < m_ComboAccessType.GetCount(); i++ )
					{
						m_ComboAccessType.GetLBText( i, strCompair );
						
						if( strCompair == m_strKV )
						{
							m_ComboAccessType.SetCurSel( i );
						}
					}
					
					OnCbnSelChangeAccessoryType();
					str =WriteDouble( Singularity.m_uDpVal.Kv, 3, 2, 1 );
					m_Edit1.SetWindowText( str );
					break;
			
				case CDS_HydroMod::eDpType::Cv:
					
					for( int i = 0; i < m_ComboAccessType.GetCount(); i++ )
					{
						m_ComboAccessType.GetLBText( i, strCompair );
						
						if( strCompair == m_strCV )
						{
							m_ComboAccessType.SetCurSel( i );
						}
					}
					
					OnCbnSelChangeAccessoryType();
					str = WriteDouble( Singularity.m_uDpVal.Cv, 3, 2, 1 );
					m_Edit1.SetWindowText( str );
					break;
			
				case CDS_HydroMod::eDpType::dzeta:
					
					for( int i = 0; i  <m_ComboAccessType.GetCount(); i++ )
					{
						m_ComboAccessType.GetLBText( i, strCompair );

						if( strCompair == m_strZeta )
						{
							m_ComboAccessType.SetCurSel( i );
						}
					
					}
					
					OnCbnSelChangeAccessoryType();
					str = WriteDouble(Singularity.m_uDpVal.sDzeta.dzeta, 3, 2, true);
					m_Edit1.SetWindowText( str );
					break;
			
				case CDS_HydroMod::eDpType::QDpref:
					
					for( int i = 0; i < m_ComboAccessType.GetCount(); i++ )
					{
						m_ComboAccessType.GetLBText( i, strCompair );

						if( strCompair == m_strQrefDPref )
						{
							m_ComboAccessType.SetCurSel( i );
						}
					}
					
					OnCbnSelChangeAccessoryType();
					str = WriteCUDouble( _U_FLOW, Singularity.m_uDpVal.sQDpRef.Qref );
					m_Edit1.SetWindowText( str );
					str = WriteCUDouble( _U_DIFFPRESS, Singularity.m_uDpVal.sQDpRef.Dpref );
					m_Edit2.SetWindowText( str );
					break;
			
				case CDS_HydroMod::eDpType::Pipelength:
					// Verify if the item is the same as in the pipe.
					// If it is, gray the CComboBox and check the CheckBox.
					if( _NULL_IDPTR == Singularity.GetSingulIDPtr() || 0 == IDcmp( Singularity.GetSingulIDPtr().ID, m_pPipe->GetIDPtr().ID ) )
					{
						m_CheckBox.SetCheck( BST_CHECKED );
						m_fCheckBox = true;
					}
					else
					{
						m_CheckBox.SetCheck( BST_UNCHECKED );
						m_fCheckBox = false;
					}
					
					for( int i = 0; i < m_ComboAccessType.GetCount(); i++ )
					{
						m_ComboAccessType.GetLBText( i, strCompair );
						
						if( strCompair == m_strPipe )
						{
							m_ComboAccessType.SetCurSel( i );
						}
					}
					
					OnCbnSelChangeAccessoryType();
					
					CTable *pTab = NULL;
					
					if( _NULL_IDPTR == Singularity.GetSingulIDPtr() )
					{
						if( true == m_pPipe->IsCompletelyDefined() )
						{
							pTab = dynamic_cast<CTable*>( m_pPipe->GetIDPtr().PP );
						}
					}
					else
					{
						pTab = dynamic_cast<CTable*>( Singularity.GetSingulIDPtr().PP );
					}
					
					if( NULL != pTab )
					{
						for( int i = 0; i < m_ComboPipeSeries.GetCount(); i++ )
						{
							m_ComboPipeSeries.GetLBText( i, strCompair );
							
							if( strCompair == pTab->GetName() )
							{
								m_ComboPipeSeries.SetCurSel( i );
							}
						}
					}
					
					OnCbnSelChangePipeSeries();
					
					CDB_Pipe *pPipe = NULL;
					
					if( _NULL_IDPTR == Singularity.GetSingulIDPtr() )
					{
						pPipe = dynamic_cast<CDB_Pipe*>( m_pPipe->GetIDPtr().MP );
					}
					else
					{
						pPipe = dynamic_cast<CDB_Pipe*>( Singularity.GetSingulIDPtr().MP );
					}
					
					if( NULL != pPipe )
					{
						for( int i = 0; i < m_ComboPipeSize.GetCount(); i++ )
						{
							m_ComboPipeSize.GetLBText( i, strCompair );

							if( strCompair == pPipe->GetName() )
							{
								m_ComboPipeSize.SetCurSel( i );
							}
						}
					}
					
					OnCbnSelChangePipeSize();
					str = WriteCUDouble( _U_LENGTH, Singularity.m_uDpVal.Length );
					m_Edit1.SetWindowText( str );
					break;
			};
			
			m_EditDescription.SetWindowText( Singularity.GetDescription() );
		}
		else
		{
			m_EditDescription.SetWindowText( _T("") );
			m_ButtonModify.EnableWindow( FALSE );
			m_ButtonRemove.EnableWindow( FALSE );
		}
	}
	else
	{
		// No need of 'Modifiy' and 'Remove' buttons when there is no item selected.
		m_ButtonModify.EnableWindow( FALSE );
		m_ButtonRemove.EnableWindow( FALSE );
	}

	*pResult = 0;
}

void CDlgAccessory::OnCbnSelChangeAccessoryType()
{
	// Initialize members variables.
	m_dQRef = 0.0;
	m_dDpRef = 0.0;
	m_dKv = 0.0;
	m_dCv = 0.0;
	m_dZeta = 0.0;
	m_dLength = 0.0;

	// Get the current selection.
	CString str( _T("") );
	CString strStatic( _T("") );

	if( CB_ERR != m_ComboAccessType.GetCurSel() )
	{
		m_ComboAccessType.GetLBText( m_ComboAccessType.GetCurSel(), str );
	}

	// Show/Hide Edit2.
	int iQrefDPrefSelection = SW_HIDE;

	if( str == m_strQrefDPref )
	{
		iQrefDPrefSelection = SW_SHOW;
	}

	m_Edit2.ShowWindow( iQrefDPrefSelection );
	GetDlgItem( IDC_STATICEDIT2 )->ShowWindow( iQrefDPrefSelection );
	GetDlgItem( IDC_STATICUNIT2 )->ShowWindow( iQrefDPrefSelection );

	// Show/Hide ComboBox and EditBox.
	int iPipeSelection = SW_HIDE;
	
	if( str == m_strPipe )
	{
		iPipeSelection = SW_SHOW;
	}

	GetDlgItem( IDC_STATICPIPESERIES )->ShowWindow( iPipeSelection );
	GetDlgItem( IDC_STATICPIPESIZE )->ShowWindow( iPipeSelection );
	GetDlgItem( IDC_CHECK )->ShowWindow( iPipeSelection );
	m_ComboPipeSeries.ShowWindow( iPipeSelection );
	m_ComboPipeSize.ShowWindow( iPipeSelection );
	m_EditDescription.EnableWindow( TRUE );

	// Show StaticUnit1 only in case of Q/Dp Ref 
	// or Pipe Equiv Length.
	if( str == m_strQrefDPref || str == m_strPipe )
	{
		GetDlgItem( IDC_STATICUNIT1 )->ShowWindow( SW_SHOW );
	}
	else
	{
		GetDlgItem( IDC_STATICUNIT1 )->ShowWindow( SW_HIDE );
	}

	// Resize the Static text in case of zeta because many information need to be written.
	CRect rectAccessType( 0, 0, 0, 0 );
	m_ComboAccessType.GetWindowRect( &rectAccessType );
	
	if( str == m_strZeta )
	{
		GetDlgItem( IDC_STATICEDIT1 )->SetWindowPos( NULL, 0, 0, rectAccessType.Width(), m_RectStatic1.Height(), SWP_NOMOVE | SWP_NOZORDER );
	}
	else
	{
		GetDlgItem( IDC_STATICEDIT1 )->SetWindowPos( NULL, 0, 0, m_RectStatic1.Width(), m_RectStatic1.Height(), SWP_NOMOVE | SWP_NOZORDER );
	}

	// Initialize what is written on the EditBox.
	m_EditDescription.SetWindowText( _T("") );
	m_Edit1.SetWindowText( _T("") );
	m_Edit2.SetWindowText( _T("") );

	// Units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Manage the selection.
	if( str == m_strQrefDPref )
	{
		// Change names for the statics.
		strStatic = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICQREF );
		GetDlgItem( IDC_STATICEDIT1 )->SetWindowText( strStatic );
		strStatic = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICDPREF );
		GetDlgItem( IDC_STATICEDIT2 )->SetWindowText( strStatic );

		// Units.
		GetDlgItem( IDC_STATICUNIT1 )->SetWindowText( GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
		GetDlgItem( IDC_STATICUNIT2 )->SetWindowText( GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	}
	else if( str == m_strKV )
	{
		// Change names for the static.
		strStatic = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICKV );
		GetDlgItem( IDC_STATICEDIT1 )->SetWindowText( strStatic );
	}
	else if( str == m_strCV )
	{
		// Change names for the static.
		strStatic = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICCV );
		GetDlgItem( IDC_STATICEDIT1 )->SetWindowText( strStatic );
	}
	else if( str == m_strZeta )
	{
		// Change names for the static.
		str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICZETA );
		GetDlgItem( IDC_STATICEDIT1 )->SetWindowText( str );
	}
	else if( str == m_strPipe )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICLENGTH );
		GetDlgItem( IDC_STATICEDIT1 )->SetWindowText( str );
		GetDlgItem( IDC_STATICUNIT1 )->SetWindowText( GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
		
		// Verify that the pipe is completely define, otherwise do not allow to check the check box.
		if( false == m_pPipe->IsCompletelyDefined() )
		{
			m_fCheckBox = false;
			m_CheckBox.SetCheck( BST_UNCHECKED );
			m_CheckBox.EnableWindow( FALSE );
		}
		
		// Fill the ComboBox.
		FillComboPipeSeries();
		FillComboPipeSize();
	}
}

void CDlgAccessory::OnEnKillFocusEdit1()
{
	double dValue;
	switch( ReadDouble( m_Edit1, &dValue ) )
	{
		case RD_EMPTY:
			break;

		case RD_OK:
			if( dValue < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_Edit1.SetFocus();
			}
			else
			{
				// Get the current selection.
				CString str( _T("") );

				if( CB_ERR != m_ComboAccessType.GetCurSel() )
				{
					m_ComboAccessType.GetLBText( m_ComboAccessType.GetCurSel(), str );
				}

				if( str == m_strQrefDPref )
				{
					m_dQRef = CDimValue::CUtoSI( _U_FLOW, dValue );
					m_Edit1.SetWindowText( WriteCUDouble( _U_FLOW, m_dQRef ) );
					m_ButtonAdd.EnableWindow( ( m_dQRef > 0.0 && m_dDpRef > 0.0 ) ? TRUE : FALSE );
				}
				else if( str == m_strZeta )
				{
					m_dZeta = dValue;
					m_Edit1.SetWindowText( WriteDouble( m_dZeta, 3, 2, true ) );
					m_ButtonAdd.EnableWindow( ( m_dZeta > 0.0 ) ? TRUE : FALSE );
				}
				else if( str == m_strPipe )
				{
					m_dLength = CDimValue::CUtoSI( _U_LENGTH, dValue );
					m_Edit1.SetWindowText( WriteCUDouble( _U_LENGTH, m_dLength ) );
					m_ButtonAdd.EnableWindow( ( m_dLength > 0.0 ) ? TRUE : FALSE );
				}
				else if( str == m_strKV )
				{
					m_dKv = dValue;
					m_Edit1.SetWindowText( WriteDouble( m_dKv, 2, 0, true ) );
					m_ButtonAdd.EnableWindow( ( m_dKv > 0.0 ) ? TRUE : FALSE );
				}
				else if( str == m_strCV )
				{
					m_dCv = dValue;
					m_Edit1.SetWindowText( WriteDouble( m_dCv, 2, 0, true ) );
					m_ButtonAdd.EnableWindow( ( m_dCv > 0.0 ) ? TRUE : FALSE );
				}
			}
			break;
	
		case RD_NOT_NUMBER:
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_Edit1.SetFocus();
			break;
	}
}

void CDlgAccessory::OnEnKillFocusEdit2()
{
	double dValue;
	switch( ReadDouble( m_Edit2, &dValue ) )
	{
		case RD_EMPTY:
			break;

		case RD_OK:
			if( dValue < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_Edit2.SetFocus();
			}
			else
			{	
				m_dDpRef = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
				m_Edit2.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_dDpRef ) );
				m_ButtonAdd.EnableWindow( ( m_dQRef > 0.0 && m_dDpRef > 0.0 ) ? TRUE : FALSE );
			}
			break;

		case RD_NOT_NUMBER:
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_Edit2.SetFocus();
			break;
	}

}

void CDlgAccessory::OnBnClickedCheck()
{
	m_fCheckBox = ( BST_CHECKED == m_CheckBox.GetCheck() ) ? true : false;
	FillComboPipeSeries();
	FillComboPipeSize();
}

void CDlgAccessory::OnCbnSelChangePipeSeries()
{
	FillComboPipeSize();
}

void CDlgAccessory::OnCbnSelChangePipeSize()
{
	LPARAM lp;
	lp = m_ComboPipeSize.GetItemData( m_ComboPipeSize.GetCurSel() );
	if( NULL == lp )
		return;
	m_IDPTRPipe = ( (CTable *)lp )->GetIDPtr();
}

void CDlgAccessory::OnBnClickedAdd()
{
	CString str;
	m_EditDescription.GetWindowText( str );

	// Get the current selection.
	CString strSelection( _T("") );

	if( CB_ERR != m_ComboAccessType.GetCurSel() )
	{
		m_ComboAccessType.GetLBText( m_ComboAccessType.GetCurSel(), strSelection );
	}

	int iNumberToAdd = m_SpinNumberToAdd.GetPos();

	if( strSelection == m_strQrefDPref )
	{
		OnEnKillFocusEdit1();
		OnEnKillFocusEdit2();
		
		if( m_dDpRef > 0.0  && m_dQRef > 0.0 )
		{
			for( int iLoop = 0; iLoop < iNumberToAdd; iLoop++ )
			{
				m_pPipe->AddAccessorySingularity( str, CDS_HydroMod::eDpType::QDpref, m_dDpRef, m_dQRef );
			}
		}
	}
	else if( strSelection == m_strZeta )
	{
		OnEnKillFocusEdit1();

		if( m_dZeta > 0.0 )
		{
			for( int iLoop = 0; iLoop < iNumberToAdd; iLoop++ )
			{
				m_pPipe->AddAccessorySingularity( str, CDS_HydroMod::eDpType::dzeta, m_dZeta );
			}
		}
	}
	else if( strSelection == m_strPipe )
	{
		OnEnKillFocusEdit1();
		
		if( m_dLength > 0.0 )
		{
			if( true == m_fCheckBox )
			{
				for( int iLoop = 0; iLoop < iNumberToAdd; iLoop++ )
				{
					m_pPipe->AddPipeSingularity( str,_NULL_IDPTR, m_dLength );
				}
			}
			else
			{
				for( int iLoop = 0; iLoop < iNumberToAdd; iLoop++ )
				{
					m_pPipe->AddPipeSingularity( str, m_IDPTRPipe, m_dLength );
				}
			}
		}
	}
	else if( strSelection == m_strKV )
	{
		OnEnKillFocusEdit1();
		
		if( m_dKv > 0.0 )
		{
			for( int iLoop = 0; iLoop < iNumberToAdd; iLoop++ )
			{
				m_pPipe->AddAccessorySingularity( str, CDS_HydroMod::eDpType::Kv, m_dKv );
			}
		}
	}
	else if( strSelection == m_strCV )
	{
		OnEnKillFocusEdit1();
		
		if( m_dCv > 0.0 )
		{
			for( int iLoop = 0; iLoop < iNumberToAdd; iLoop++ )
			{
				m_pPipe->AddAccessorySingularity( str, CDS_HydroMod::eDpType::Cv, m_dCv );
			}
		}
	}

	RefreshListCtrl();
}

void CDlgAccessory::OnBnClickedModify()
{
	// Get the current selection.
	CString str( _T("") );
	
	if( CB_ERR != m_ComboAccessType.GetCurSel() )
	{
		m_ComboAccessType.GetLBText( m_ComboAccessType.GetCurSel(), str );
	}

	if( m_iSelItem < 0 )
	{
		return;
	}
	
	if( str == m_strQrefDPref )
	{
		OnEnKillFocusEdit1();
		OnEnKillFocusEdit2();

		if( 0.0 == m_dDpRef || 0.0 == m_dQRef )
		{
			return;
		}
	}
	else if( str == m_strZeta )
	{
		OnEnKillFocusEdit1();

		if( 0.0 == m_dZeta )
		{
			return;
		}
	}
	else if( str == m_strPipe )
	{
		OnEnKillFocusEdit1();

		if( 0.0 == m_dLength )
		{
			return;
		}
	}
	else if( str == m_strKV )
	{
		OnEnKillFocusEdit1();

		if( 0.0 == m_dKv )
		{
			return;
		}
	}
	else if( str == m_strCV )
	{
		OnEnKillFocusEdit1();

		if( 0.0 == m_dCv )
		{
			return;
		}
	}

	CPipes::CSingularity Singularity;
	m_pPipe->GetSingularity( (short)m_List.GetItemData( m_iSelItem ), &Singularity );
	
	OnBnClickedRemove();
	OnBnClickedAdd();
	
	for( int i = 0; i < m_List.GetItemCount(); i++ )
	{
		if( Singularity.m_nID == (short)m_List.GetItemData( i ) )
		{
			m_List.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );
			m_iSelItem = i;
			break;
		}
	}
}

void CDlgAccessory::OnBnClickedRemove()
{
	if( m_iSelItem > -1 )
	{
		m_pPipe->RemoveSingularity( (short)m_List.GetItemData( m_iSelItem ) );
		RefreshListCtrl();
	}
}

void CDlgAccessory::OnBnClickedOk()
{
	PREVENT_ENTER_KEY

	if( NULL != m_pfChangeDone )
	{
		*m_pfChangeDone = !m_pPipe->Compare( m_pSavedPipe );
	}

	CDialogEx::OnOK();
}

void CDlgAccessory::OnBnClickedCancel()
{
	// Restore pipe as it was before changing anything.
	m_pSavedPipe->Copy( m_pPipe );
	CDialogEx::OnCancel();
}

void CDlgAccessory::RefreshListCtrl()
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	m_List.DeleteAllItems();
	
	LVITEM lvItem;
	CString str, str1;
	CPipes::CSingularity Singularity;
	int iPos = m_pPipe->GetFirstSingularity( &Singularity );
	int n = 0;
	
	for( int i = 0; i < m_pPipe->GetSingularityCount(); i++ )
	{
		bool bConnect = ( SINGULARITY_CONNECT_ID == Singularity.m_nID && 0 != _tcscmp( Singularity.GetSingulIDPtr().ID, _T("SINGUL_NONE") ) );

		if( Singularity.m_nID > 0 || true == bConnect )
		{
			lvItem.mask = LVIF_TEXT;
			str.Format( _T("%u"), n + 1 );
			lvItem.iItem = i;
			lvItem.iSubItem = eColHeader::Nb; 
			lvItem.pszText = (TCHAR *)(LPCTSTR)str; 
			lvItem.lParam = 0;
			int j = m_List.InsertItem( &lvItem );

			// Fill 'Description' column.
			CString str = Singularity.GetDescription();
			if( true == bConnect )
			{
				CString strDescription;
				m_pPipe->GetConnectDescription( strDescription );
				FormatString( str, IDS_DLGACCESSORY_LISTCONNECTION, strDescription );
			}

			m_List.SetItemText( j, eColHeader::Desc, str );
			
			// Fill 'Value' column.
			double dValue = 0.0;
			CString str1, str2, str3;

			switch( Singularity.m_eDpType )
			{
				case CDS_HydroMod::eDpType::Dp:
					dValue = Singularity.m_uDpVal.Dp;
					str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DP );
					str1 = str + WriteCUDouble( _U_DIFFPRESS, dValue ) + GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str();
					break;

				case CDS_HydroMod::eDpType::Cv:
					dValue = Singularity.m_uDpVal.Cv;
					str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CV );
					str1 = str + WriteDouble( dValue, 2, 0, true );
					break;

				case CDS_HydroMod::eDpType::Kv:
					dValue = Singularity.m_uDpVal.Kv;
					str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_KV );
					str1 = str + WriteDouble( dValue, 2, 0, true );
					break;

				case CDS_HydroMod::eDpType::dzeta:
					dValue = Singularity.m_uDpVal.sDzeta.dzeta;
					str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_ZETA );
					str1 = str + WriteDouble( dValue, 3, 2, true );
					break;

				case CDS_HydroMod::eDpType::Pipelength:
					dValue = m_pPipe->GetSingularityDp( &Singularity );
					str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DP );
					str = str + WriteCUDouble( _U_DIFFPRESS, dValue,true );
					
					if( _NULL_IDPTR == Singularity.GetSingulIDPtr() )
					{
						if( true == m_pPipe->IsCompletelyDefined() )
							str1 = m_pPipe->GetIDPtr().PP->GetName();
					}
					else
					{
						str1 = Singularity.GetSingulIDPtr().PP->GetName();
					}

					str1 += _T("; ");
					
					if( _NULL_IDPTR == Singularity.GetSingulIDPtr() )
					{
						if( true == m_pPipe->IsCompletelyDefined() )
						{
							str1 += dynamic_cast<CDB_Pipe*>( m_pPipe->GetIDPtr().MP )->GetName();
						}
					}
					else
					{
						str1 += dynamic_cast<CDB_Pipe*>( Singularity.GetSingulIDPtr().MP )->GetName();
					}

					str1 += _T("; ");
					str1 += TASApp.LoadLocalizedString( IDS_DLGACCESSORY_LENGTH );
					str1 += WriteCUDouble( _U_LENGTH, Singularity.m_uDpVal.Length, true );
					break;
				
				case CDS_HydroMod::eDpType::QDpref:
					dValue = Singularity.m_uDpVal.sQDpRef.Dpref;
					str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DPREF );
					str1 = str + WriteCUDouble( _U_DIFFPRESS, dValue, true ) + _T("; ");
					str = str1;
					dValue = Singularity.m_uDpVal.sQDpRef.Qref;
					str2 = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_QREF );
					str1 = str + str2 + WriteCUDouble( _U_FLOW, dValue, true );
					break;
			}
			m_List.SetItemText( j, eColHeader::Value, str1 );
			
			// Fill 'Dp' column.
			str = WriteCUDouble( _U_DIFFPRESS, m_pPipe->GetSingularityDp( &Singularity ) );
			m_List.SetItemText( j, eColHeader::Dp, str );
			m_List.SetItemData( j, Singularity.m_nID );
			n++;
		}
		iPos = m_pPipe->GetNextSingularity( iPos, &Singularity );
	}
	
	// 'true' to also include pressure drop on connect singularity.
	double dTotalDp = m_pPipe->GetSingularityTotalDp( true );
	if( dTotalDp > 0.0 )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_STATICTOTALDP );
		str1 = str + _T(" ") + WriteCUDouble( _U_DIFFPRESS, dTotalDp, true );
		m_StaticTotalDp.SetWindowText( str1 );
	}
	else
		m_StaticTotalDp.SetWindowText( _T("") );
	m_iSelItem = -1;
}

void CDlgAccessory::FillComboPipeSeries()
{
	CRank PipeList;

	// Reset the content;
	m_ComboPipeSeries.ResetContent();

	// Get the PipeSerie's list.
	m_pTADB->GetPipeSerieList( &PipeList, m_eFilterSelection );
	
	// Transfer the list to the combobox.
	PipeList.Transfer( &m_ComboPipeSeries );
	
	// If the check box is checked and the distribution pipe or circuit pipe are completely defined: 
	// use current pipe series.
	if( true == m_fCheckBox && true == m_pPipe->IsCompletelyDefined() )
	{
		m_ComboPipeSeries.EnableWindow( FALSE );
	}
	else
	{
		m_ComboPipeSeries.EnableWindow( TRUE );
	}

	// Select the current pipe series used.
	if( true == m_pPipe->IsCompletelyDefined() )
	{
		CTable *pTabSerie = m_pPipe->GetPipeSeries();
		CString strCompair;

		for( int i = 0; i < m_ComboPipeSeries.GetCount(); i++ )
		{
			m_ComboPipeSeries.GetLBText( i, strCompair );

			if( strCompair == pTabSerie->GetName() )
			{
				m_ComboPipeSeries.SetCurSel( i );
			}
		}
	}
	else
	{
		m_ComboPipeSeries.SetCurSel( 0 );
	}
}

void CDlgAccessory::FillComboPipeSize()
{
	m_ComboPipeSize.ResetContent();

	if( 0 == m_ComboPipeSeries.GetCount() )
	{
		return;
	}

	CRank List;
	CTable *pTab = ( CTable * )m_ComboPipeSeries.GetItemData( m_ComboPipeSeries.GetCurSel() );

	TASApp.GetpTADB()->GetPipeSizeList( &List, pTab, CTADatabase::FilterSelection::ForHMCalc );
	List.Transfer( &m_ComboPipeSize );
	
	// If the check box is checked : use current pipe series.
	if( true == m_fCheckBox && true == m_pPipe->IsCompletelyDefined() )
	{
		m_ComboPipeSeries.EnableWindow( FALSE );
		m_ComboPipeSize.EnableWindow( FALSE );
	}
	else
	{
		m_ComboPipeSeries.EnableWindow( TRUE );
		m_ComboPipeSize.EnableWindow( TRUE );
	}

	// Set the current selection to 0 by default.
	if( false == m_fCheckBox)
	{
		m_ComboPipeSize.SetCurSel( 0 );
	}

	// Select the current pipe series used.
	CString strCompair;
	
	if( true == m_pPipe->IsCompletelyDefined() )
	{
		for( int i = 0; i < m_ComboPipeSize.GetCount(); i++ )
		{
			m_ComboPipeSize.GetLBText( i, strCompair );

			if( strCompair == dynamic_cast<CDB_Pipe*>( m_pPipe->GetIDPtr().MP )->GetName() )
			{
				m_ComboPipeSize.SetCurSel( i );
			}
		}
	}
	
	OnCbnSelChangePipeSize();
}
