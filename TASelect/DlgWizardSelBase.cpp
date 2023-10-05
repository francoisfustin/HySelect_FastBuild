#include "stdafx.h"
#include <errno.h>

#include "Taselect.h"
#include "MainFrm.h"

#include "RViewSSelSS.h"

#include "DlgSelectionBase.h"
#include "DlgWizardSelBase.h"

CDlgWizardSelBase::CDlgWizardSelBase( CWizardSelectionParameters &clWizardSelParams, UINT nID, CWnd *pParent )
		: CDlgSelectionBase( clWizardSelParams, nID, pParent )
{
	m_pclWizardSelParams = &clWizardSelParams;
}

void CDlgWizardSelBase::VerifyModificationMode()
{
	if( NULL == m_pclWizardSelParams )
	{
		ASSERT_RETURN;
	}

	// Object still exist?
	m_pclWizardSelParams->m_SelIDPtr = m_pclWizardSelParams->m_pTADS->Get( m_pclWizardSelParams->m_SelIDPtr.ID );

	if( NULL == m_pclWizardSelParams->m_SelIDPtr.MP )
	{
		m_pclWizardSelParams->m_bEditModeRunning = false;
	}
	else
	{
		m_pclWizardSelParams->m_bEditModeRunning = true;
	}
}

void CDlgWizardSelBase::SetModificationMode( bool bFlag )
{
	if( NULL == m_pclWizardSelParams )
	{
		ASSERT_RETURN;
	}

	if( false == bFlag )
	{
		m_pclWizardSelParams->m_SelIDPtr = _NULL_IDPTR;
	}

	VerifyModificationMode();
}

void CDlgWizardSelBase::SaveSelectionParameters( void )
{
	if( NULL == m_pclWizardSelParams || NULL == m_pclWizardSelParams->m_pTADS || NULL == m_pclWizardSelParams->m_pTADS->GetpWizardSelParameter() )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::SaveSelectionParameters();

	CDS_WizardSelParameter *pclWizardSelParameters = m_pclWizardSelParams->m_pTADS->GetpWizardSelParameter();
	pclWizardSelParameters->SetPipeSeriesID( m_pclWizardSelParams->m_strPipeSeriesID );
	pclWizardSelParameters->SetPipeSizeID( m_pclWizardSelParams->m_strPipeID );
}

void CDlgWizardSelBase::ActivateLeftTabDialog()
{
	if( NULL == m_pclWizardSelParams || NULL == m_pclWizardSelParams->m_pTADS || NULL == m_pclWizardSelParams->m_pTADS->GetpWizardSelParameter() )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::ActivateLeftTabDialog();

	// Reinitialization pipes accordingly to the current context.
	CDS_WizardSelParameter *pclCDSWizardSelParameters = m_pclWizardSelParams->m_pTADS->GetpWizardSelParameter();
	m_pclWizardSelParams->m_strPipeSeriesID = pclCDSWizardSelParameters->GetPipeSeriesID( m_pclWizardSelParams );
	m_pclWizardSelParams->m_strPipeID = pclCDSWizardSelParameters->GetPipeSizeID();
}

void CDlgWizardSelBase::ClearAll( void )
{
	CDlgSelectionBase::ClearAll();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgWizardSelBase, CDlgSelectionBase )
END_MESSAGE_MAP()

BOOL CDlgWizardSelBase::OnInitDialog()
{
	CDlgSelectionBase::OnInitDialog();

	return TRUE;
}

LRESULT CDlgWizardSelBase::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgSelectionBase::OnNewDocument( wParam, lParam );

	return 0;
}

LRESULT CDlgWizardSelBase::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	CDlgSelectionBase::OnWaterChange( wParam, lParam );
	return 0;
}
