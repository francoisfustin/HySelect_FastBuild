//
// DlgPipeDp.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TASelect.h"
#include "Global.h"
#include "Utilities.h"
#include "Hydronic.h"
#include "DlgWaterChar.h"
#include "SSheet.h"
#include "RViewDescription.h"
#include "DlgPipeDp.h"
#include <algorithm>		// For std::sort



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP( CDlgPipeDp, CDialogEx )
	ON_BN_CLICKED( IDC_BUTTONMODWATER, OnButtonModWater )
	ON_EN_CHANGE( IDC_EDITFLOW, OnChangeEditFlow )
	ON_CBN_SELCHANGE( IDC_COMBOPIPENAME, OnSelChangeComboPipeName )
END_MESSAGE_MAP()

CDlgPipeDp::CDlgPipeDp( CWnd *pParent )
	: CDialogEx( CDlgPipeDp::IDD, pParent )
{
	m_pTADS = NULL;
	m_pTechParam = NULL;
	m_dFlow = 0.0;
}

CDlgPipeDp::~CDlgPipeDp()
{
	if( 1 == m_ViewDescription.GetSheetNumber() )
	{
		CSheetDescription* pclSheetDescription = m_ViewDescription.GetTopSheetDescription();
		CSSheet* pclSSheet = pclSheetDescription->GetSSheetPointer();
		pclSSheet->Detach();
		m_ViewDescription.RemoveAllSheetDescription( true );
		delete pclSSheet;
	}
}

void CDlgPipeDp::Display()
{
	m_pTADS = TASApp.GetpTADS();
	m_pTechParam =  m_pTADS->GetpTechParams();
}

void CDlgPipeDp::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow);
	DDX_Control( pDX, IDC_COMBOPIPENAME, m_ComboPipeName);
	DDX_Control( pDX, IDC_BUTTONMODWATER, m_ButtonModWater);
	DDX_Control( pDX, IDC_GROUPPIPE, m_GroupPipe);
	DDX_Control( pDX, IDC_GROUPWATER, m_GroupWater);
	DDX_Control( pDX, IDC_GROUPQ, m_GroupQ);
}

BOOL CDlgPipeDp::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGPIPEDP_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPIPEDP_STATICFLOW );
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPIPEDP_STATICPIPENAME );
	GetDlgItem( IDC_STATICPIPENAME )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );

	// Add icons to Q, pipe and water groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupQ.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
		m_GroupPipe.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pipe );
		m_GroupWater.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Water );
	}

	// Add bitmap to "Water" button.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonModWater.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_ModifyWater ) );
	}

	// Create a ToolTipCtrl and add a tool tip for "Water" button.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTMODWAT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONMODWATER ), TTstr );
	
	// Set the text for the static controls displaying units.
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Set the text for the flow units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), name );
	SetDlgItemText( IDC_STATICQUNIT, name );
	
	// Can not calculate for Change-over
	if( ChangeOver == TASApp.GetpTADS()->GetpTechParams()->GetProductSelectionApplicationType() )
	{
		TASApp.GetpTADS()->GetpTechParams()->SetProductSelectionApplicationType( Heating );
		CWaterChar *pWC = TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData();
		*pWC = *( TASApp.GetpTADS()->GetpTechParams()->GetDefaultISHeatingWC() );
	}

	// Save what is the current project type.
	m_eCurrentProjectType = m_pTADS->GetpTechParams()->GetProductSelectionApplicationType();

	// Build new water characteristic strings.
	m_clWaterChar = *( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() );
	CString str1, str2;
	m_clWaterChar.BuildWaterStrings( str1, str2 );

	// Set the text for the first static.
	SetDlgItemText( IDC_STATICFLUID1, str1 );
	
	// Set the text for the second static.
	SetDlgItemText( IDC_STATICFLUID2, str2 );

	// Fill pipe name combo.
	_FillComboPipe();

	_InitializeSSheet();

	return TRUE;
}

void CDlgPipeDp::OnOK() 
{
	PREVENT_ENTER_KEY
	
	CDialogEx::OnOK();
}

void CDlgPipeDp::OnButtonModWater() 
{
	// Display the dialog.
	CDlgWaterChar dlg;
	dlg.Display( NULL, CDlgWaterChar::DlgWaterChar_ForTools, true, m_eCurrentProjectType );

	// If the user has simply changed the application type, or has changed something in the current application type.
	if( m_eCurrentProjectType != dlg.GetCurrentApplicationType() || true == dlg.IsSomethingChanged( dlg.GetCurrentApplicationType() ) )
	{
		// Save the new user choice to force it if we call again the 'CDlgWaterChar' dialog.
		m_eCurrentProjectType = dlg.GetCurrentApplicationType();

		double dDT = 0.0;
		dlg.GetWaterChar( dlg.GetCurrentApplicationType(), m_clWaterChar, dDT );

		// Update the 2 Water statics because WM_USER_WATERCHANGE message does not reach the dialog.
		CString str1;
		CString str2;
		m_clWaterChar.BuildWaterStrings( str1, str2 );

		// Set the text for the first static.
		SetDlgItemText( IDC_STATICFLUID1, str1 );
	
		// Set the text for the second static.
		SetDlgItemText( IDC_STATICFLUID2, str2 );

		// Update all fields.
		_UpdateAll();
	}
}

void CDlgPipeDp::OnChangeEditFlow() 
{
	if( GetFocus() != &m_EditFlow )
	{
		return; 
	}

	CString str;

	switch( ReadDouble( m_EditFlow, &m_dFlow ) ) 
	{
		case RD_EMPTY:
			m_dFlow = 0.0;
			_UpdateAll();
			break;

		case RD_NOT_NUMBER:
			m_EditFlow.GetWindowText( str );
			
			if( str != _T(".") && str != _T(",") && str.Right( 1 ) != _T("e") && str.Right( 2 ) != _T("e+") && str.Right( 2 ) != _T("e-") )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditFlow.SetWindowText( _T("") );
			}
			
			break;

		case RD_OK:
			m_dFlow = CDimValue::CUtoSI( _U_FLOW, m_dFlow );
			// Update all fields
			_UpdateAll();
			break;
	};
}

void CDlgPipeDp::OnSelChangeComboPipeName() 
{
	// Update all fields
	_UpdateAll();
}

void CDlgPipeDp::_FillComboPipe( LPCTSTR ptcsPipe )
{
	// Fill the pipe name combo box.
	CRank PipeList;
	TASApp.GetpTADB()->GetPipeSerieList( &PipeList );
	m_ComboPipeName.ResetContent();
	PipeList.Transfer( &m_ComboPipeName );

	// Set the Pipe Name selection
	if( NULL != ptcsPipe && ( m_ComboPipeName.FindStringExact( -1, ptcsPipe ) != CB_ERR ) )
	{
		m_ComboPipeName.SelectString( -1, ptcsPipe );
	}
	else
	{
		m_ComboPipeName.SetCurSel( m_ComboPipeName.GetCount() ? 0 : -1 );		
	}
}

LPCTSTR CDlgPipeDp::_GetSelPipeName()
{
	if( CB_ERR == m_ComboPipeName.GetCurSel() )
	{
		return _T("");
	}

	static CString str;

	ASSERT( NULL != m_ComboPipeName.GetCount() );
	m_ComboPipeName.GetLBText( m_ComboPipeName.GetCurSel(), str );
	
	return str;
}

bool CDlgPipeDp::_GetSelPipeIDPtr( IDPTR &PipeFamIDPtr )
{
	CString str = _GetSelPipeName();

	// Find pointer on pipe series table.
	CTable *pTab = TASApp.GetpPipeDB()->GetPipeTab();
	ASSERT( NULL != pTab );

	IDPTR IDPtr = pTab->GetFirst( CLASS( CTable ) );
	ASSERT( _T('\0') != *IDPtr.ID );

	while( _T('\0') != *IDPtr.ID && ( (CTable*)IDPtr.MP )->GetName() != str )
	{
		IDPtr = pTab->GetNext();
	}
	
	ASSERT( _T('\0') != *IDPtr.ID );
	PipeFamIDPtr = IDPtr;

	return true;
}

void CDlgPipeDp::_UpdateAll()
{
	IDPTR FamIDPtr;
		
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetTopSheetDescription();
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	pclSSheet->SetBool( SSB_REDRAW, FALSE );
	pclSSheet->SetMaxRows( RD_PipeResult_Unit );

	int iBestPipeRow = -1;

	if( m_dFlow > 0.0 && true == _GetSelPipeIDPtr( FamIDPtr ) )
	{
		CTable *pPipeTable = (CTable *)( TASApp.GetpPipeDB()->Get( FamIDPtr.ID ).MP );

		if( NULL == pPipeTable )
		{
			return;
		}
	
		// Get fluid char.
		double dRho = m_clWaterChar.GetDens();
		double dNu = m_clWaterChar.GetKinVisc();
	
		std::vector<rPipe> vecPipeList;

		for( IDPTR IDPtr = pPipeTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pPipeTable->GetNext() )
		{
			CDB_Pipe *pPipe = dynamic_cast<CDB_Pipe*>( IDPtr.MP );

			if( NULL == pPipe )
			{
				continue;
			}

			rPipe Pipe;
			Pipe.fBest = false;
			Pipe.strPipeName = pPipe->GetName();
			Pipe.dDiameter = pPipe->GetIntDiameter();
			Pipe.dLinearDp = CalcPipeDp( m_dFlow, dRho, dNu, pPipe->GetRoughness(), Pipe.dDiameter );
			Pipe.dVelocity = m_dFlow / (0.785398163398 * Pipe.dDiameter * Pipe.dDiameter);
		
			double dRe = Pipe.dVelocity * Pipe.dDiameter / dNu;
			CString str;
			
			if( dRe > 3500.0 )
			{
				Pipe.strFlowRegime = TASApp.LoadLocalizedString( IDS_TURBULENT );
			}
			else if( dRe > 2300.0 )
			{
				Pipe.strFlowRegime = TASApp.LoadLocalizedString( IDS_INTERMEDIARY );
			}
			else
			{
				Pipe.strFlowRegime = TASApp.LoadLocalizedString( IDS_LAMINAR );
			}

			vecPipeList.push_back( Pipe );
		}

		// Just to be sure.
		if( 0 == (int)vecPipeList.size() )
		{
			return;
		}

		// Sort list.
		std::sort( vecPipeList.begin(), vecPipeList.end(), CDlgPipeDp::_ComparePipe );

		// Initialize some variables.
		double dVelocityTarget = m_pTechParam->GetPipeTargVel();
		double dVelocityMax = m_pTechParam->GetPipeMaxVel();
		double dLinearDpTarget = m_pTechParam->GetPipeTargDp();
		double dLinearDpMax = m_pTechParam->GetPipeMaxDp();
		int	iVelocityTargetUsed = m_pTechParam->GetPipeVtargUsed();
	
		// Search for best score
		double dScore;
		double dBestScore = DBL_MAX;
		unsigned int uiBestPipePos = vecPipeList.size()-1;
		const int nPipeNeighbours = uiBestPipePos/2;//3;
		
		for( unsigned int uiCurrentPipe = 0; uiCurrentPipe < vecPipeList.size(); uiCurrentPipe++ )
		{
			double dLinearDp = vecPipeList[uiCurrentPipe].dLinearDp;
			double dVelocity = vecPipeList[uiCurrentPipe].dVelocity;
		
			if( (dVelocity <= dVelocityMax ) && ( dLinearDp <= dLinearDpMax ) )
			{
				dScore = ( dLinearDp - dLinearDpTarget ) * ( dLinearDp - dLinearDpTarget ) / ( ( dLinearDpTarget + 1.0e-6 ) * ( dLinearDpTarget + 1.0e-6 ) );
				
				if( iVelocityTargetUsed )
				{
					dScore += ( dVelocity - dVelocityTarget ) * ( dVelocity - dVelocityTarget ) / ( ( dVelocityTarget + 1.0e-6 ) * ( dVelocityTarget + 1.0e-6 ) );
				}
			
				if( dScore < dBestScore )
				{
					uiBestPipePos = uiCurrentPipe;
					dBestScore = dScore;
				}
			}
		}
	
		// Check upper and lower bounds to display.
		unsigned int uiMinPipePos = ( uiBestPipePos > (unsigned int)nPipeNeighbours ) ? uiBestPipePos - nPipeNeighbours : 0;
		unsigned int uiMaxPipePos = min( (int)uiBestPipePos + nPipeNeighbours, (int)vecPipeList.size() - 1 );

		// "Mark" the item corresponding to the best pipe.
		if( dBestScore != DBL_MAX )
		{
			vecPipeList[uiBestPipePos].fBest = true;
		}
	
		// Add items to spread.
		long lRow = RD_PipeResult_FirstAvailRow;
		int iReturn = -1;

		for( int iCurrentPipe = uiMaxPipePos; iCurrentPipe >= (int)uiMinPipePos; iCurrentPipe-- )
		{
			// Add this row.
			// 'true' to specify that this row can't be selected.
			pclSheetDescription->AddRows( 1, false );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

			if( true == vecPipeList[iCurrentPipe].fBest )
			{
				pclSSheet->SetPictureCellWithID( IDB_GREENARROWTS, CD_PipeResult_Icon, lRow, CSSheet::PictureCellType::Bitmap );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
				iBestPipeRow = lRow;
			}
		
			pclSSheet->SetStaticText( CD_PipeResult_Name, lRow, vecPipeList[iCurrentPipe].strPipeName );
			pclSSheet->SetStaticText( CD_PipeResult_LinearDp, lRow,  WriteCUDouble( _U_LINPRESSDROP, vecPipeList[iCurrentPipe].dLinearDp, false ) );
			pclSSheet->SetStaticText( CD_PipeResult_Velocity, lRow,  WriteCUDouble( _U_VELOCITY, vecPipeList[iCurrentPipe].dVelocity, false ) );
			pclSSheet->SetStaticText( CD_PipeResult_FlowRegime, lRow, vecPipeList[iCurrentPipe].strFlowRegime );

			pclSSheet->SetCellBorder( CD_PipeResult_Name, lRow, CD_PipeResult_FlowRegime, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );
	
			lRow++;
		}
	}
		
	pclSSheet->SetFreeze( pclSSheet->GetMaxCols(), RD_PipeResult_Unit );
	pclSSheet->ShowCell(1,iBestPipeRow,SS_SHOW_CENTERLEFT );
	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CDlgPipeDp::_InitializeSSheet( void )
{
	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget 
	// to call 'Detach' before deleting 'CSSheet'.
	CSSheet *pclSSheet = new CSSheet();
	pclSSheet->Attach( GetDlgItem( IDC_SPREAD )->GetSafeHwnd() );
	pclSSheet->Reset();

	CSheetDescription *pclSheetDescription = m_ViewDescription.AddSheetDescription( SD_PipeResult, -1, pclSSheet, CPoint( 0, 0 ) );
	if( NULL == pclSheetDescription )
		return;
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, TRUE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_PipeResult_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase row height.
	pclSSheet->SetRowHeight( RD_PipeResult_FirstRow, 8 );
	pclSSheet->SetRowHeight( RD_PipeResult_ColName, 12 );
	pclSSheet->SetRowHeight( RD_PipeResult_Unit, 12 );

	// Initialize.
	pclSheetDescription->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );

	// Add columns.
	pclSheetDescription->AddColumn( CD_PipeResult_FirstColumn, 1 );
	pclSheetDescription->AddColumn( CD_PipeResult_Icon, 2 );
	pclSheetDescription->AddColumn( CD_PipeResult_Name, 11 );
	pclSheetDescription->AddColumn( CD_PipeResult_LinearDp, 8 );
	pclSheetDescription->AddColumn( CD_PipeResult_Velocity, 8 );
	pclSheetDescription->AddColumn( CD_PipeResult_FlowRegime, 16 );

	// Row name.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)8 );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_GRAY_MED );
	pclSSheet->SetStaticText( CD_PipeResult_Name, RD_PipeResult_ColName, IDS_DLGPIPEDP_SSHEETPIPE );
	pclSSheet->SetStaticText( CD_PipeResult_LinearDp, RD_PipeResult_ColName, IDS_DLGPIPEDP_SSHEETLINDP );
	pclSSheet->SetStaticText( CD_PipeResult_Velocity, RD_PipeResult_ColName, IDS_DLGPIPEDP_SSHEETV );
	pclSSheet->SetStaticText( CD_PipeResult_FlowRegime, RD_PipeResult_ColName, IDS_DLGPIPEDP_SSHEETFLOWREGIME );

	// Row unit.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_PipeResult_Name, RD_PipeResult_Unit, _T("") );
	pclSSheet->SetStaticText( CD_PipeResult_LinearDp, RD_PipeResult_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_PipeResult_Velocity, RD_PipeResult_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	pclSSheet->SetStaticText( CD_PipeResult_FlowRegime, RD_PipeResult_Unit, _T("") );

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_PipeResult_Name, RD_PipeResult_Unit, CD_PipeResult_FlowRegime, RD_PipeResult_Unit, true, SS_BORDERTYPE_BOTTOM );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
}

bool CDlgPipeDp::_ComparePipe( rPipe i, rPipe j )
{
	return( i.dDiameter < j.dDiameter );
}
