#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "utilities.h"
#include "DlgKvSeries.h"
#include "DlgKvSeriesTabDzeta.h"

CDlgKvSeriesTabDzeta::CDlgKvSeriesTabDzeta( CWnd *pParent )
	: CDialogExt( CDlgKvSeriesTabDzeta::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
	m_dDzeta = 0.0;
	m_dIntDiam = 0.0;
}

void CDlgKvSeriesTabDzeta::ResetAll()
{
	m_dIntDiam = 0;
	m_dDzeta = 0;
	m_EditDzeta.SetWindowText( _T("") );
	m_EditIntDiameter.SetWindowText( _T("") );
	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

double CDlgKvSeriesTabDzeta::GetIntDiam()
{
	if( BST_CHECKED == m_CheckInternalDiameter.GetCheck() )							// Edit Box Internal Diameter checked
	{
		return CDimValue::CUtoSI( _U_DIAMETER, m_dIntDiam );
	}

	int iItem = m_ComboPipeSize.GetCurSel();

	if( iItem < 0 )
	{
		return -1;
	}

	LPARAM lp = m_ComboPipeSize.GetItemData( iItem );		// Found selected pipe and take pointer on Table
	
	if( NULL == lp )
	{
		return -1;
	}
	
	return ( ( (CDB_Pipe *)lp )->GetIntDiameter() );
}

void CDlgKvSeriesTabDzeta::SetCheckBox( bool bState )
{
	m_CheckInternalDiameter.SetCheck( ( true == bState ) ? BST_CHECKED : BST_UNCHECKED );
	m_EditIntDiameter.EnableWindow( ( true == bState ) ? TRUE : FALSE );
	m_ComboPipeName.EnableWindow( ( true == bState ) ? FALSE: TRUE );
	m_ComboPipeSize.EnableWindow( ( true == bState ) ? FALSE: TRUE );

	if( true == bState )
	{
		m_EditIntDiameter.SetWindowText( _T("") );
		m_EditIntDiameter.SetFocus();
	}
	else
	{
		OnCbnSelChangePipeSize();
	}
}

void CDlgKvSeriesTabDzeta::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECK, m_CheckInternalDiameter );
	DDX_Control( pDX, IDC_GROUPPIPE, m_GroupPipe );
	DDX_Control( pDX, IDC_COMBOPIPENAME, m_ComboPipeName );
	DDX_Control( pDX, IDC_COMBOPIPESIZE, m_ComboPipeSize );
	DDX_Control( pDX, IDC_EDITDZETA, m_EditDzeta );
	DDX_Control( pDX, IDC_EDITINTDIAMETER, m_EditIntDiameter );
	DDX_Control( pDX, IDC_STATICUNIT, m_StaticUnit );
}

BEGIN_MESSAGE_MAP( CDlgKvSeriesTabDzeta, CDialogExt )
	ON_CBN_SELCHANGE( IDC_COMBOPIPENAME, OnCbnSelChangePipeName )
	ON_BN_CLICKED( IDC_CHECK, OnCheck)
	ON_EN_CHANGE( IDC_EDITDZETA, OnEnChangeDzeta )
	ON_EN_KILLFOCUS( IDC_EDITDZETA, OnEnKillFocusDzeta )
	ON_EN_CHANGE( IDC_EDITINTDIAMETER, OnEnChangeInternalDiameter )
	ON_EN_KILLFOCUS( IDC_EDITINTDIAMETER, OnEnKillFocusInternalDiameter )
	ON_CBN_SELCHANGE( IDC_COMBOPIPESIZE, OnCbnSelChangePipeSize )
END_MESSAGE_MAP()

BOOL CDlgKvSeriesTabDzeta::OnInitDialog() 
{
	CDialogExt::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGDZETA_STATICDZETA );
	GetDlgItem( IDC_STATICDZETA )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGDZETA_CHECK );
	m_CheckInternalDiameter.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGDZETA_STATICPIPENAME );
	GetDlgItem( IDC_STATICPIPENAME )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGDZETA_STATICPIPESIZE );
	GetDlgItem( IDC_STATICPIPESIZE )->SetWindowText( str );

	// Initialize units for each edit box.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetUnit( _U_DIAMETER, pUnitDB->GetDefaultUnitIndex( _U_DIAMETER ) ), tcName );
	m_StaticUnit.SetWindowText( tcName );	

	// Initialize Check box into DialogDzeta.
	SetCheckBox( true );
	
	// Initialize Pipe Icon.
	m_GroupPipe.SetBckgndColor( _WHITE_DLGBOX );
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupPipe.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pipe );
	}

	return TRUE;
}

// Used to capture Enter and Esc Key .....
void CDlgKvSeriesTabDzeta::OnCancel() 
{
}

void CDlgKvSeriesTabDzeta::OnOK() 
{
}

void CDlgKvSeriesTabDzeta::OnCbnSelChangePipeName() 
{
	m_ComboPipeSize.ResetContent();

	if( 0 == m_ComboPipeName.GetCount() )
	{
		return;
	}

	//Found selected Item and take pointer on Table.
	LPARAM lp = m_ComboPipeName.GetItemData( m_ComboPipeName.GetCurSel() );

	if( NULL == lp)
	{
		return;
	}
	
	IDPTR IDPtr = ( (CTable *)lp )->GetIDPtr();
	ASSERT( _T('\0') != *IDPtr.ID );

	//Insert all pipe dimension for current pipe name
	int iIndex;
	CTable *pTab = (CTable *)( IDPtr.MP );
	
	for( IDPtr = pTab->GetFirst( CLASS( CDB_Pipe ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		iIndex = m_ComboPipeSize.AddString( (TCHAR *)( (CDB_Pipe *)IDPtr.MP )->GetName() );
		m_ComboPipeSize.SetItemData( iIndex, (LPARAM)IDPtr.MP );
	}

	m_ComboPipeSize.SetCurSel( 0 );
}

void CDlgKvSeriesTabDzeta::OnCheck() 
{
	SetCheckBox( ( BST_CHECKED == m_CheckInternalDiameter.GetCheck() ) ? true : false );
}

void CDlgKvSeriesTabDzeta::OnEnChangeDzeta() 
{
	if( GetFocus() != &m_EditDzeta )
	{
		return; 
	}

	CString str;
	
	switch( ReadDouble( m_EditDzeta, &m_dDzeta ) )
	{
		case RD_EMPTY:
			m_dDzeta = 0.0;
			break;

		case RD_NOT_NUMBER:
			m_EditDzeta.GetWindowText( str );

			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) )
			{
				return;
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}
			
			m_EditDzeta.SetWindowText( _T("") );
			break;

		case RD_OK:
			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabDzeta::OnEnKillFocusDzeta() 
{
	double dValue;

	switch( ReadDouble( m_EditDzeta, &dValue ) )
	{
		case RD_EMPTY:
			break;

		case RD_OK:

			if( dValue < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditDzeta.SetFocus();
			}
			else
			{
				m_EditDzeta.SetWindowText( WriteDouble( dValue, 4, 1 ) );
			}

			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabDzeta::OnEnChangeInternalDiameter() 
{
	if( GetFocus() != &m_EditIntDiameter )
	{
		return; 
	}

	CString str;

	switch( ReadDouble( m_EditIntDiameter, &m_dIntDiam ) )
	{
		case RD_EMPTY:
			m_dIntDiam = 0.0;
			break;

		case RD_NOT_NUMBER:
			m_EditIntDiameter.GetWindowText( str );

			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) ) 
			{
				return;
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}

			m_EditIntDiameter.SetWindowText( _T("") );
			break;

		case RD_OK:
			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabDzeta::OnEnKillFocusInternalDiameter() 
{
	double dValue;

	switch( ReadDouble( m_EditIntDiameter, &dValue ) )
	{
		case RD_EMPTY:
			break;

		case RD_OK:
			
			if( dValue < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditIntDiameter.SetFocus();
			}
			else
			{
				m_EditIntDiameter.SetWindowText( WriteDouble( dValue, 4, 1 ) );
			}

			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabDzeta::OnCbnSelChangePipeSize() 
{
	double dDi = GetIntDiam();		// Internal Diameter in SI

	if( dDi < 0.0 )
	{
		return;
	}
	
	m_EditIntDiameter.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIAMETER, dDi ), 4, 1 ) );
	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

// Fill in of the pipe Combo's
void CDlgKvSeriesTabDzeta::_FillComboPipes( LPCTSTR ptcPipe, LPCTSTR ptcSize )
{
	// Fill the Pipe Name Combo box.
	CRank PipeList;
	TASApp.GetpTADB()->GetPipeSerieList( &PipeList );

	m_ComboPipeName.ResetContent();
	PipeList.Transfer( &m_ComboPipeName );

	// Set the 'Pipe Name' selection.
	if( NULL != ptcPipe && ( m_ComboPipeName.FindStringExact( -1, ptcPipe ) != CB_ERR ) )
	{
		m_ComboPipeName.SelectString( -1, ptcPipe );
	}
	else
	{
		m_ComboPipeName.SetCurSel( m_ComboPipeName.GetCount() ? 0 : -1 );
	}

	// Fill the Pipe Size Combo box.
	_FillComboSize();

	// Set the Pipe Size selection.
	if( NULL != ptcSize && ( m_ComboPipeSize.FindStringExact( -1, ptcSize ) != CB_ERR ) )
	{
		m_ComboPipeSize.SelectString( -1, ptcSize );
	}
	else
	{
		m_ComboPipeSize.SetCurSel( 0 );
	}
}

void CDlgKvSeriesTabDzeta::_FillComboSize()
{
	OnCbnSelChangePipeName();
}
