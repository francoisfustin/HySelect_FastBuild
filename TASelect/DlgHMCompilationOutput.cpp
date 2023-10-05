// DlgHMCompilationOutput.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "HydroMod.h"
#include "DlgOutput.h"
#include "DlgHMCompilationOutput.h"
#include "ToolsDockablePane.h"
#include "DlgLeftTabProject.h"
#include "RViewHMCalc.h"

IMPLEMENT_DYNAMIC( CDlgHMCompilationOutput, CDlgOutput )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgHMCompilationOutput::CHMInterface class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDlgHMCompilationOutput::CHMInterface::CHMInterface()
{
	m_pDlgHMCompilationOutput = NULL;
	m_pclHMMessage = NULL;
	Init();
}

CDlgHMCompilationOutput::CHMInterface::~CHMInterface()
{
	if( m_pclHMMessage != NULL )
		delete m_pclHMMessage;
}

bool CDlgHMCompilationOutput::CHMInterface::Init( void )
{
	if( m_pDlgHMCompilationOutput != NULL )
		return true;
	if( NULL == pMainFrame || NULL == pMainFrame->GetpToolsDockablePane() )
		return false;
	m_pDlgHMCompilationOutput = pMainFrame->GetpToolsDockablePane()->GetpDlgHMCompilationOutput();
	if( NULL == m_pDlgHMCompilationOutput )
		return false;
	if( NULL == m_pclHMMessage )
		m_pclHMMessage = new CHMMessage( m_pDlgHMCompilationOutput );
	else
		m_pclHMMessage->Clear();
	m_pDlgOutput = m_pDlgHMCompilationOutput;
	return true;
}

bool CDlgHMCompilationOutput::CHMInterface::FillAndSendMessage( CString strMessage, MessageType eMessageType, CDS_HydroMod* pclHydroMod, CString strSolution, CString strArg1, CString strArg2, CString strArg3, int iOutputID )
{
	bool fReturn = false;
	if( true == Init() )
		fReturn = m_pclHMMessage->FillAndSendMessage( strMessage, eMessageType, pclHydroMod, strSolution, strArg1, strArg2, strArg3, iOutputID );
	return fReturn;
}

bool CDlgHMCompilationOutput::CHMInterface::FillAndSendMessage( int iMessageID, MessageType eMessageType, CDS_HydroMod* pclHydroMod, CString strSolution, CString strArg1, CString strArg2, CString strArg3, int iOutputID )
{
	bool fReturn = false;
	if( true == Init() )
		fReturn = m_pclHMMessage->FillAndSendMessage( iMessageID, eMessageType, pclHydroMod, strSolution, strArg1, strArg2, strArg3, iOutputID );
	return fReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgHMCompilationOutput class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgHMCompilationOutput::CDlgHMCompilationOutput()
	: CDlgOutput( true, DLGHMCOID )
{
	m_mapCounters[MessageType::Error] = 0;
	m_mapCounters[MessageType::Warning] = 0;
	m_mapCounters[MessageType::Info] = 0;
}

void CDlgHMCompilationOutput::SendMessage( int iMessageID, MessageType eMessageType, CDS_HydroMod* pclHydroMod, CString strSolution, CString strArg1, CString strArg2, CString strArg3, int iOutputID )
{
	if( -1 == iOutputID )
		iOutputID = m_iOutputID;
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
		return;

	CHMMessage* pclMessage = new CHMMessage( this );
	if( NULL == pclMessage )
		return;

	pclMessage->FillAndSendMessage( iMessageID, eMessageType, pclHydroMod, strSolution, strArg1, strArg2, strArg3, iOutputID );
	delete pclMessage;
}

void CDlgHMCompilationOutput::SendMessage( CHMMessage* pclHMMessage, int iOutputID )
{
	if( -1 == iOutputID )
		iOutputID = m_iOutputID;
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
		return;

	if( NULL == pclHMMessage )
		return;

	// Before sending, add pre job if it's a comment message type.
	if( MessageType::Comment == pclHMMessage->GetMessageType() )
	{
		// Specify that we want a span before displaying message.
		pclHMMessage->AddPreJob( new DlgOutputHelper::CPreJobSpan( ColumnID::HydroModName, ColumnID::Solution ) );

		// Move the 'Problem' column content to the 'HydroModName' column.
		DlgOutputHelper::CCellText* pclCellProblem = dynamic_cast<DlgOutputHelper::CCellText*>( pclHMMessage->GetCellByID( ColumnID::Problem ) );
		if( pclCellProblem != NULL )
			pclHMMessage->SetCellText( ColumnID::HydroModName, pclCellProblem->GetText(), true );
	}

	bool fIsRedrawBlocked = IsRedrawBlocked( iOutputID );
	if( false == fIsRedrawBlocked )
		BlockRedraw( iOutputID );

	if( true == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SendMessage( pclHMMessage, &m_clFilterList ) )
	{
		switch( pclHMMessage->GetMessageType() )
		{
			case MessageType::Error:
			case MessageType::Warning:
			case MessageType::Info:
				m_mapCounters[pclHMMessage->GetMessageType()]++;
				_UpdateButtonCaption();
				break;
			default:
				break;
		}
	}

	if( false == fIsRedrawBlocked )
		ReleaseRedraw( iOutputID );
}

void CDlgHMCompilationOutput::OnHeaderButtonClicked( int iButtonID, CVButton::State eState )
{
	bool fIsRedrawBlocked = IsRedrawBlocked();
	if( false == fIsRedrawBlocked )
		m_pclCurrentDlgOutputSpreadContainer->BlockRedraw();

	DlgOutputHelper::CCellFilterList* pclCellFilter = m_clFilterList.GetCellFilter( iButtonID );
	switch( iButtonID )
	{
		case DLGHMCO_BUTTONID_ERROR:
		case DLGHMCO_BUTTONID_WARNING:
		case DLGHMCO_BUTTONID_INFO:
			if( pclCellFilter != NULL )
			{
				if( CVButton::State::Pressed == eState )
					pclCellFilter->SetFlagActive( false );
				else if( CVButton::State::Unpressed == eState )
					pclCellFilter->SetFlagActive( true );

				if( m_pclCurrentDlgOutputSpreadContainer != NULL )
					m_pclCurrentDlgOutputSpreadContainer->ApplyFilter( &m_clFilterList );
			}
			break;

		case DLGHMCO_BUTTONID_CLEAR:
			ClearOutput();
			break;

		default:
			break;
	}

	if( false == fIsRedrawBlocked )
		m_pclCurrentDlgOutputSpreadContainer->ReleaseRedraw();
}

void CDlgHMCompilationOutput::OnOutputSpreadContainerCellDblClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam* pVecParamList )
{
	CDlgOutput::OnOutputSpreadContainerCellClicked( iOutputID, iColumnID, lRow, ptMouse, pVecParamList );
	
	if( NULL != pDlgLeftTabProject )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)( (*pVecParamList)[0].GetParam() ) );
		if( NULL != pHM )
		{
			CDS_HydroMod *pPHM = pHM;
			if( false == pHM->IsaModule() )
				pPHM = dynamic_cast<CDS_HydroMod *>( pHM->GetIDPtr().PP );

			pDlgLeftTabProject->ResetTreeAndSelectHM( pPHM );

			if( NULL != pRViewHMCalc && NULL != pRViewHMCalc->GetSheetHMCalc() )
			{
				pRViewHMCalc->GetSheetHMCalc()->UnSelectMultipleRows();
				pRViewHMCalc->GetSheetHMCalc()->SelectRow( pHM );
			}
		}
	}
}

bool CDlgHMCompilationOutput::ClearOutput( bool fAlsoHeader, int iOutputID )
{
	if( false == CDlgOutput::ClearOutput( fAlsoHeader, iOutputID ) )
		return false;
	m_mapCounters[MessageType::Error] = 0;
	m_mapCounters[MessageType::Warning] = 0;
	m_mapCounters[MessageType::Info] = 0;
	_UpdateButtonCaption();
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CDlgHMCompilationOutput::OnInitDialog()
{
	if( FALSE == CDlgOutput::OnInitDialog() )
		return FALSE;

	CString strCaption;
	
	// Allocate enough space to display at least 9999 messages.
	FormatString( strCaption, IDS_HMCOMPILOUTPUT_ERRORS, _T("0000") );
	AddTopPanelPushButton( DLGHMCO_BUTTONID_ERROR, CVButton::State::Pressed, strCaption, IDI_OUTPUTBOXERROR, BS_ICON, true );
	FormatString( strCaption, IDS_HMCOMPILOUTPUT_ERRORS, _T("0") );
	UpdateTopPanelButtonCaption( DLGHMCO_BUTTONID_ERROR, strCaption );
	
	FormatString( strCaption, IDS_HMCOMPILOUTPUT_WARNINGS, _T("0000") );
	AddTopPanelPushButton( DLGHMCO_BUTTONID_WARNING, CVButton::State::Pressed, strCaption, IDI_OUTPUTBOXWARNING, BS_ICON, true );
	FormatString( strCaption, IDS_HMCOMPILOUTPUT_WARNINGS, _T("0") );
	UpdateTopPanelButtonCaption( DLGHMCO_BUTTONID_WARNING, strCaption );
	
	FormatString( strCaption, IDS_HMCOMPILOUTPUT_INFOS, _T("0000") );
	AddTopPanelPushButton( DLGHMCO_BUTTONID_INFO, CVButton::State::Pressed, strCaption, IDI_OUTPUTBOXINFO, BS_ICON, true );
	FormatString( strCaption, IDS_HMCOMPILOUTPUT_INFOS, _T("0") );
	UpdateTopPanelButtonCaption( DLGHMCO_BUTTONID_INFO, strCaption );
	
	AddTopPanelButton( DLGHMCO_BUTTONID_CLEAR, IDS_HMCOMPILOUTPUT_CLEAR, -1, -1, false, CSize( 80, 20 ) );
		
	// For debug purpose.
// 	AddTopPanelButton( DLGHMCO_BUTTONID_SCROLLLASTLINE, _T("Scroll"), -1, -1, true );
// 	AddTopPanelButton( DLGHMCO_BUTTONID_RELEASEOUTPUT, _T("Release"), -1, -1, true );
// 	AddTopPanelButton( DLGHMCO_BUTTONID_WRITEPACKET, _T("Packet"), -1, -1, true );
//	AddTopPanelButton( DLGHMCO_BUTTONID_WRITEMESSAGE, _T("Message"), -1, -1, true );

	m_pclCurrentDlgOutputSpreadContainer->AutomaticColumnResizing( true );

	// Defines the columns.
	DlgOutputHelper::CColDef clDataColDef;
	clDataColDef.SetAllParams( DlgOutputHelper::CColDef::Bitmap, 2, -1, -1, DlgOutputHelper::CColDef::AutoResizeDisabled, DlgOutputHelper::CColDef::Visible, 
		DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionEnabled, DlgOutputHelper::CColDef::BlockSelectionDisabled, 
		DlgOutputHelper::CColDef::RowSelectionDisabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );
	m_pclCurrentDlgOutputSpreadContainer->AddColumnDefinition( ColumnID::Icon, clDataColDef );

	clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	clDataColDef.SetWidth( 20.0 );
	clDataColDef.SetAutoResized( true );
	m_pclCurrentDlgOutputSpreadContainer->AddColumnDefinition( ColumnID::HydroModName, clDataColDef );

	clDataColDef.SetWidth( 78.0 );
	m_pclCurrentDlgOutputSpreadContainer->AddColumnDefinition( ColumnID::Problem, clDataColDef );

	clDataColDef.SetWidth( 2.0 );
	m_pclCurrentDlgOutputSpreadContainer->AddColumnDefinition( ColumnID::Solution, clDataColDef );

	clDataColDef.SetContentType( DlgOutputHelper::CColDef::Param );
	clDataColDef.SetWidth( 0.0 );
	clDataColDef.SetAutoResized( false );
	clDataColDef.SetVisible( false );
	clDataColDef.SetMouseEventFlag( false );
	clDataColDef.SetSelectionFlag( false );
	m_pclCurrentDlgOutputSpreadContainer->AddColumnDefinition( ColumnID::Parameter, clDataColDef );
	m_pclCurrentDlgOutputSpreadContainer->SetColumnAutoResizeMode( true );

	// Defines the filters.
	DlgOutputHelper::CCellFilterList* pclCellFilter = new DlgOutputHelper::CCellFilterList();
	pclCellFilter->AddCellFilter( new DlgOutputHelper::CCellBitmap( ColumnID::Icon, IDI_OUTPUTBOXERROR ), DlgOutputHelper::TestOperator::equal );
	m_clFilterList.AddFilter( DLGHMCO_BUTTONID_ERROR, pclCellFilter );

	pclCellFilter = new DlgOutputHelper::CCellFilterList();
	pclCellFilter->AddCellFilter( new DlgOutputHelper::CCellBitmap( ColumnID::Icon, IDI_OUTPUTBOXWARNING ), DlgOutputHelper::TestOperator::equal );
	m_clFilterList.AddFilter( DLGHMCO_BUTTONID_WARNING, pclCellFilter );

	pclCellFilter = new DlgOutputHelper::CCellFilterList();
	pclCellFilter->AddCellFilter( new DlgOutputHelper::CCellBitmap( ColumnID::Icon, IDI_OUTPUTBOXINFO ), DlgOutputHelper::TestOperator::equal );
	m_clFilterList.AddFilter( DLGHMCO_BUTTONID_INFO, pclCellFilter );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgHMCompilationOutput::OnDlgOutputAdded( int iOutputID ,CDlgOutputSpreadContainer* pclDlgOutputSpreadContainer )
{
	if( NULL != pclDlgOutputSpreadContainer )
	{
		pclDlgOutputSpreadContainer->EnableWindow( TRUE );
		pclDlgOutputSpreadContainer->ShowWindow( SW_SHOW );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgHMCompilationOutput::_UpdateButtonCaption( void )
{
	for( int iLoop = MessageType::First; iLoop < MessageType::Last; iLoop++ )
	{
		if( m_mapCounters.count( iLoop ) > 0 )
		{
			CString strNumber;
			strNumber.Format( _T("%u"), m_mapCounters[iLoop] );
			CString str;
			switch( iLoop )
			{
				case MessageType::Error:
					FormatString( str, IDS_HMCOMPILOUTPUT_ERRORS, strNumber );
					UpdateTopPanelButtonCaption( DLGHMCO_BUTTONID_ERROR, str );
					break;

				case MessageType::Warning:
					FormatString( str, IDS_HMCOMPILOUTPUT_WARNINGS, strNumber );
					UpdateTopPanelButtonCaption( DLGHMCO_BUTTONID_WARNING, str );
					break;

				case MessageType::Info:
					FormatString( str, IDS_HMCOMPILOUTPUT_INFOS, strNumber );
					UpdateTopPanelButtonCaption( DLGHMCO_BUTTONID_INFO, str );
					break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgHMCompilationOutput::CMessage class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgHMCompilationOutput::CHMMessage::CHMMessage( CDlgHMCompilationOutput* pDlgHMCompilationOutput )
{
	m_eMessageType = MessageType::Undefined;
	m_pDlgHMCompilationOutput = pDlgHMCompilationOutput;
}

CDlgHMCompilationOutput::CHMMessage &CDlgHMCompilationOutput::CHMMessage::operator=( CDlgHMCompilationOutput::CHMMessage &clHMessage )
{
	Copy( &clHMessage );
	return *this;
}

void CDlgHMCompilationOutput::CHMMessage::FillMessage( CString strMessage, MessageType eMessageType, CDS_HydroMod* pclHydroMod, CString strSolution, CString strArg1, CString strArg2, CString strArg3 )
{
	SetMessageString( strMessage );
	SetMessageType( eMessageType );
	SetHydroModPointer( pclHydroMod );
	SetSolutionString( strSolution );
	SetArguments( strArg1, strArg2, strArg3 );
}

void CDlgHMCompilationOutput::CHMMessage::FillMessage( int iMessageID, MessageType eMessageType, CDS_HydroMod* pclHydroMod, CString strSolution, CString strArg1, CString strArg2, CString strArg3 )
{
	SetMessageID( iMessageID );
	SetMessageType( eMessageType );
	SetHydroModPointer( pclHydroMod );
	SetSolutionString( strSolution );
	SetArguments( strArg1, strArg2, strArg3 );
}

bool CDlgHMCompilationOutput::CHMMessage::FillAndSendMessage( CString strMessage, MessageType eMessageType, CDS_HydroMod* pclHydroMod, CString strSolution, CString strArg1, CString strArg2, CString strArg3, int iOutputID )
{
	FillMessage( strMessage, eMessageType, pclHydroMod, strSolution, strArg1, strArg2, strArg3 );
	return SendMessage( iOutputID );
}

bool CDlgHMCompilationOutput::CHMMessage::FillAndSendMessage( int iMessageID, MessageType eMessageType, CDS_HydroMod* pclHydroMod, CString strSolution, CString strArg1, CString strArg2, CString strArg3, int iOutputID )
{
	FillMessage( iMessageID, eMessageType, pclHydroMod, strSolution, strArg1, strArg2, strArg3 );
	return SendMessage( iOutputID );
}

bool CDlgHMCompilationOutput::CHMMessage::SendMessage( int iOutputID )
{
	bool fReturn = false;
	if( m_pDlgHMCompilationOutput != NULL )
	{
		m_pDlgHMCompilationOutput->SendMessage( this, iOutputID );
		fReturn = true;
	}
	return fReturn;
}

void CDlgHMCompilationOutput::CHMMessage::SetMessageType( MessageType eMessageType )
{
	int iBitmapID = 0;
	switch( eMessageType )
	{
		case MessageType::Error:
			iBitmapID = IDI_OUTPUTBOXERROR;
			break;

		case MessageType::Warning:
			iBitmapID = IDI_OUTPUTBOXWARNING;
			break;

		case MessageType::Info:
			iBitmapID = IDI_OUTPUTBOXINFO;
			break;
	}
	DlgOutputHelper::CCellBitmap* pclCellMessageType = dynamic_cast<DlgOutputHelper::CCellBitmap*>( GetCellByID( ColumnID::Icon ) );
	if( pclCellMessageType != NULL )
		pclCellMessageType->SetBitmapID( iBitmapID );
	else
	{
		pclCellMessageType = new DlgOutputHelper::CCellBitmap( ColumnID::Icon, iBitmapID );
		m_vecCellList.push_back( pclCellMessageType );
	}
	m_eMessageType = eMessageType;
}

void CDlgHMCompilationOutput::CHMMessage::SetHydroModPointer( CDS_HydroMod* pclHydroMod )
{
	if( pclHydroMod != NULL )
	{
		DlgOutputHelper::CCellText* pclCellName = dynamic_cast<DlgOutputHelper::CCellText*>( GetCellByID( ColumnID::HydroModName ) );
		if( pclCellName != NULL )
			pclCellName->SetText( pclHydroMod->GetHMName() );
		else
		{
			pclCellName = new DlgOutputHelper::CCellText( ColumnID::HydroModName, pclHydroMod->GetHMName() );
			m_vecCellList.push_back( pclCellName );
		}
		
		DlgOutputHelper::CCellParam* pclCellParam = dynamic_cast<DlgOutputHelper::CCellParam*>( GetCellByID( ColumnID::Parameter ) );
		if( pclCellParam != NULL )
			pclCellParam->SetParam( (LPARAM)pclHydroMod );
		else
		{
			pclCellParam = new DlgOutputHelper::CCellParam( ColumnID::Parameter, (LPARAM)pclHydroMod );
			m_vecCellList.push_back( pclCellParam );
		}
	}
}

void CDlgHMCompilationOutput::CHMMessage::SetMessageString( CString strMessage )
{
	DlgOutputHelper::CCellText* pclCellProblem = dynamic_cast<DlgOutputHelper::CCellText*>( GetCellByID( ColumnID::Problem ) );
	if( pclCellProblem != NULL )
		pclCellProblem->SetText( strMessage );
	else
	{
		pclCellProblem = new DlgOutputHelper::CCellText( ColumnID::Problem, strMessage );
		m_vecCellList.push_back( pclCellProblem );
	}
}

void CDlgHMCompilationOutput::CHMMessage::SetMessageID( int iMessageID )
{
	SetMessageString( TASApp.LoadLocalizedString( iMessageID ) );
}

void CDlgHMCompilationOutput::CHMMessage::SetSolutionString( CString strSolution )
{
	DlgOutputHelper::CCellText* pclCellSolution = dynamic_cast<DlgOutputHelper::CCellText*>( GetCellByID( ColumnID::Solution ) );
	if( pclCellSolution != NULL )
		pclCellSolution->SetText( strSolution );
	else
	{
		pclCellSolution = new DlgOutputHelper::CCellText( ColumnID::Solution, strSolution );
		m_vecCellList.push_back( pclCellSolution );
	}
}

void CDlgHMCompilationOutput::CHMMessage::SetSolutionID( int iSolutionID )
{
	SetSolutionString( TASApp.LoadLocalizedString( iSolutionID ) );
}

void CDlgHMCompilationOutput::CHMMessage::SetArguments( CString strArg1, CString strArg2, CString strArg3 )
{
	DlgOutputHelper::CCellText* pclCellProblem = dynamic_cast<DlgOutputHelper::CCellText*>( GetCellByID( ColumnID::Problem ) );
	if( NULL == pclCellProblem || true == pclCellProblem->GetText().IsEmpty() )
		return;
	
	CString strComplete = _T("");
	FormatString( strComplete, pclCellProblem->GetText(), strArg1, strArg2, strArg3 );
	pclCellProblem->SetText( strComplete );
}

void CDlgHMCompilationOutput::CHMMessage::Copy( CHMMessage* pclHMMessage )
{
	if( NULL == pclHMMessage )
		return;
	CMessageBase::Copy( pclHMMessage );
	m_eMessageType = pclHMMessage->GetMessageType();
	m_pDlgHMCompilationOutput = pclHMMessage->GetpDlgHMCompilationOutput();
}
