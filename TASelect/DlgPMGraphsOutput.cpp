#include "stdafx.h"
#include <algorithm>
#include "ATLBase.h"
#include "AtlConv.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "HydroMod.h"
#include "DlgOutput.h"
#include "DlgPMGraphsOutput.h"
#include "ToolsDockablePane.h"
#include "DlgLeftTabProject.h"
#include "RViewHMCalc.h"

IMPLEMENT_DYNAMIC( CDlgPMGraphsOutput, CDlgOutput )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgPMGraphsOutput::CInterfaceBSO class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDlgPMGraphsOutput::CPMInterface::CPMInterface()
	: CBaseInterface()
{
	m_pDlgPMGraphsOutput = NULL;
	Init();
}

bool CDlgPMGraphsOutput::CPMInterface::Init( void )
{
	if( NULL != m_pDlgPMGraphsOutput )
	{
		return true;
	}

	if( NULL == pMainFrame || NULL == pMainFrame->GetpToolsDockablePane() )
	{
		return false;
	}

	m_pDlgPMGraphsOutput = pMainFrame->GetpToolsDockablePane()->GetpDlgPMGraphsOutput();

	if( NULL == m_pDlgPMGraphsOutput )
	{
		return false;
	}

	m_pDlgOutput = m_pDlgPMGraphsOutput;
	return true;
}

bool CDlgPMGraphsOutput::CPMInterface::AddBitmap( HBITMAP hBitmap )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgPMGraphsOutput->AddBitmap( hBitmap );
	}

	return bReturn;
}

bool CDlgPMGraphsOutput::CPMInterface::AddTChartDialog( CDialogEx *pclTChartDialog )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgPMGraphsOutput->AddTChartDialog( pclTChartDialog );
	}

	return bReturn;
}

bool CDlgPMGraphsOutput::CPMInterface::GetCurrentHeight( long *plHeight )
{
	bool bReturn = false;

	if( NULL == plHeight )
	{
		return false;
	}

	if( true == Init() )
	{
		*plHeight = m_pDlgPMGraphsOutput->GetCurrentHeight();
		bReturn = true;
	}

	return bReturn;
}

CWnd *CDlgPMGraphsOutput::CPMInterface::GetCWndPMGraphsOutput( void )
{
	CWnd *pWnd = NULL;

	if( true == Init() )
	{
		pWnd = (CWnd *)m_pDlgPMGraphsOutput;
	}

	return pWnd;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgPMGraphsOutput class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgPMGraphsOutput::CDlgPMGraphsOutput()
	: CDlgOutput( true, DLGPMOID, false )
{
	m_pclBitmapContainer = NULL;
	m_hBitmap = NULL;
	m_pclTChart = NULL;
	m_pclTChartDialog = NULL;
	m_iCurrentX = 0;
	m_lCurrentHeight = 0;
}

CDlgPMGraphsOutput::~CDlgPMGraphsOutput()
{
	ClearOutput();
}

bool CDlgPMGraphsOutput::AddBitmap( HBITMAP hBitmap )
{
	if( NULL == hBitmap )
	{
		return false;
	}

	if( NULL != m_pclBitmapContainer )
	{
		m_pclBitmapContainer->DestroyWindow();
		delete m_pclBitmapContainer;
		m_pclBitmapContainer = NULL;
	}

	if( NULL != m_hBitmap )
	{
		DeleteObject( m_hBitmap );
		m_hBitmap = NULL;
	}

	m_pclBitmapContainer = new CStatic();

	if( NULL == m_pclBitmapContainer )
	{
		return false;
	}

	BITMAP bm;
	::GetObject( hBitmap, sizeof( bm ), &bm );

	if( FALSE == m_pclBitmapContainer->Create( _T( "" ), SS_BITMAP | SS_CENTERIMAGE, CRect( 0, 0, bm.bmWidth, bm.bmHeight ), this ) )
	{
		delete m_pclBitmapContainer;
	}

	int iY = DLGPMMARGIN;

	if( 0 != m_lCurrentHeight && bm.bmHeight < m_lCurrentHeight )
	{
		iY += ( m_lCurrentHeight - bm.bmHeight ) / 2;
	}
	else
	{
		m_lCurrentHeight =bm.bmHeight;
	}

	m_pclBitmapContainer->SetBitmap( hBitmap );
	m_pclBitmapContainer->SetWindowPos( NULL, m_iCurrentX + DLGPMMARGIN, iY, -1, -1, SWP_NOACTIVATE | SWP_NOSIZE );
	m_pclBitmapContainer->ShowWindow( TRUE );

	m_hBitmap = hBitmap;

	m_iCurrentX += ( DLGPMMARGIN + bm.bmWidth );

	return true;
}

bool CDlgPMGraphsOutput::AddTChartDialog( CDialogEx *pclTChartDialog )
{
	if( NULL == pclTChartDialog || NULL == pclTChartDialog->GetSafeHwnd() )
	{
		return false;
	}

	if( NULL != m_pclTChartDialog )
	{
		m_pclTChartDialog->DestroyWindow();
		delete m_pclTChartDialog;
		m_pclTChartDialog = NULL;
	}

	CRect rect;
	pclTChartDialog->GetClientRect( &rect );
	int iY = DLGPMMARGIN;

	if( 0 != m_lCurrentHeight && rect.Height() < m_lCurrentHeight )
	{
		iY += ( m_lCurrentHeight - rect.Height() ) / 2;
	}
	else
	{
		m_lCurrentHeight = rect.Height();
	}

	pclTChartDialog->SetWindowPos( NULL, m_iCurrentX + DLGPMMARGIN, iY, -1, -1, SWP_NOACTIVATE | SWP_NOSIZE );
	pclTChartDialog->ShowWindow( TRUE );

	m_pclTChartDialog = pclTChartDialog;

	m_iCurrentX += ( DLGPMMARGIN + rect.Width() );

	return true;
}

long CDlgPMGraphsOutput::GetCurrentHeight( void )
{
	return m_lCurrentHeight + ( 2 * DLGPMMARGIN );
}

bool CDlgPMGraphsOutput::ClearOutput( bool bAlsoHeader, int iOutputID )
{
	// No need to call base class because for PM Graph we don't have the spread container.

	m_iCurrentX = 0;

	if( NULL != m_pclBitmapContainer )
	{
		m_pclBitmapContainer->DestroyWindow();
		delete m_pclBitmapContainer;
		m_pclBitmapContainer = NULL;
	}

	if( NULL != m_hBitmap )
	{
		DeleteObject( m_hBitmap );
		m_hBitmap = NULL;
	}

	if( NULL != m_pclTChart )
	{
		m_pclTChart->DestroyWindow();
		delete m_pclTChart;
		m_pclTChart = NULL;
	}

	if( NULL != m_pclTChartDialog )
	{
		m_pclTChartDialog->DestroyWindow();
		delete m_pclTChartDialog;
		m_pclTChartDialog = NULL;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CDlgPMGraphsOutput::OnInitDialog()
{
	if( FALSE == CDlgOutput::OnInitDialog() )
	{
		return FALSE;
	}

	return TRUE;
}

void CDlgPMGraphsOutput::OnDlgOutputAdded( int iOutputID, CDlgOutputSpreadContainer *pclDlgOutputSpreadContainer )
{
	if( NULL != pclDlgOutputSpreadContainer )
	{
		pclDlgOutputSpreadContainer->EnableWindow( TRUE );
		pclDlgOutputSpreadContainer->ShowWindow( SW_SHOW );
	}
}
