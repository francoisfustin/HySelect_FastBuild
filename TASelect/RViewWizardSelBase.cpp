#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "RViewWizardSelBase.h"


CRViewWizardSelBase::CRViewWizardSelBase( CMainFrame::RightViewList eRightViewWizardID )
	:  CFormViewEx( IDD )
{
	m_pNotificationHandlers = NULL;
	m_eRViewID = eRightViewWizardID;
}

void CRViewWizardSelBase::OnInputChange( int iErrorCode )
{
	if( NULL != m_pNotificationHandlers )
	{
		m_pNotificationHandlers->OnRViewWizSelPMInputChange( ( 0 == iErrorCode ) ? false : true );
	}
}

bool CRViewWizardSelBase::IsEditingMode()
{
	bool bReturn = false;

	if( NULL != m_pNotificationHandlers )
	{
		bReturn = m_pNotificationHandlers->OnRViewWizSelPMIsEditingMode();
	}

	return bReturn;
}

void CRViewWizardSelBase::RegisterNotificationHandler( CRViewWizardSelBase::INotificationHandler *pclHandler )
{
	m_pNotificationHandlers = pclHandler;
}

void CRViewWizardSelBase::UnregisterNotificationHandler( CRViewWizardSelBase::INotificationHandler *pclHandler )
{
	m_pNotificationHandlers = NULL;
}

BOOL CRViewWizardSelBase::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext )
{
	// Create the form view.
	if( FALSE == CFormViewEx::Create( lpszClassName, lpszWindowName, dwStyle | WS_HSCROLL | WS_VSCROLL , rect, pParentWnd, nID, pContext ) )
	{
		return FALSE;
	}

	return TRUE;
}

void CRViewWizardSelBase::SetFocusW( bool fFirst )
{
	// TODO
}

void CRViewWizardSelBase::VerifyFluidCharacteristic( bool &bEnable, CString &strMsg )
{
	if( NULL != m_pNotificationHandlers )
	{
		m_pNotificationHandlers->OnRViewWizSelPMVerifyFluidCharacteristics( bEnable, strMsg );
	}
}

BEGIN_MESSAGE_MAP( CRViewWizardSelBase, CFormViewEx )
	ON_MESSAGE( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, OnSwitchNormalAdvancedMode )
	ON_MESSAGE( WM_USER_RIGHTVIEWWIZPM_BACKNEXT, OnButtonBackNextClicked )
END_MESSAGE_MAP()

LRESULT CRViewWizardSelBase::OnSwitchNormalAdvancedMode( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pNotificationHandlers )
	{
		m_pNotificationHandlers->OnRViewWizSelPMSwitchNormalAdvancedMode( (int)wParam );
	}

	return 0;
}

LRESULT CRViewWizardSelBase::OnButtonBackNextClicked( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pNotificationHandlers )
	{
		if( WM_RVWPMBN_Back == wParam )
		{
			m_pNotificationHandlers->OnRViewWizSelPMButtonBackClicked();
		}
		else if( WM_RVWPMBN_Next == wParam )
		{
			m_pNotificationHandlers->OnRViewWizSelPMButtonNextClicked();
		}
	}

	return 0;
}

void CRViewWizardSelBase::OnInitialUpdate( )
{
	CFormViewEx::OnInitialUpdate();
	m_pNotificationHandlers = NULL;
}
