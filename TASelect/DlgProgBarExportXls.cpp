// DlgProgBarExportXls.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "SheetHMCalc.h"
#include "DlgProgBarExportXls.h"

// CDlgProgBarExportXls dialog

IMPLEMENT_DYNAMIC( CDlgProgBarExportXls, CDlgProgressBar )

CDlgProgBarExportXls::CDlgProgBarExportXls( CWnd* pParent, CString Title )
	: CDlgProgressBar( pParent, Title )
{
	m_fCanExport = false;
	m_pclStopEvent = NULL;
	m_pclThreadStopped = NULL;
	m_pThread = NULL;
	m_pSheetHM = NULL;
	m_pTab = NULL;
	m_iMaxTabSheet = 1000;		// Limitation of number spread tabs.
	m_iTabOffset = 0;
	m_fCancel = false;
	m_pclSuspendEvent = NULL;
	m_pclResumeEvent = NULL;
	m_pclThreadSuspended = NULL;
}

CDlgProgBarExportXls::~CDlgProgBarExportXls()
{
	if( NULL != m_pclStopEvent )
		delete m_pclStopEvent;
	if( NULL != m_pclSuspendEvent)
		delete m_pclSuspendEvent;
	if( NULL != m_pclResumeEvent )
		delete m_pclResumeEvent;
	if( NULL != m_pclThreadStopped )
		delete m_pclThreadStopped;
	if( NULL != m_pclThreadSuspended )
		delete m_pclThreadSuspended;
}

void CDlgProgBarExportXls::SetParamForExport( CSSheet* pSheetHM, CTable* pTab, int iTabOffset )
{
	m_pSheetHM = pSheetHM;
	m_pTab = pTab;
	m_iMaxTabSheet = iTabOffset;
}

UINT CDlgProgBarExportXls::ThreadInitSheetModules( LPVOID pParam )
{
	CDlgProgBarExportXls* pclDlgProgBarExportXls = (CDlgProgBarExportXls*)pParam;
	if( NULL == pclDlgProgBarExportXls )
		return -1;

	if( true == pclDlgProgBarExportXls->_InitSheetModules( pclDlgProgBarExportXls->m_pSheetHM, pclDlgProgBarExportXls->m_pTab ) )
		pclDlgProgBarExportXls->m_fCanExport = true;

	// Test if thread has received a stop event.
	DWORD dwReturn = WaitForSingleObject( pclDlgProgBarExportXls->m_pclStopEvent->m_hObject, 0 );

	// If user has not asked to stop thread...
	if( dwReturn != WAIT_OBJECT_0 )
	{
		// Post a message to close the dialog.
		::PostMessage( pclDlgProgBarExportXls->GetSafeHwnd(), WM_CLOSE, 0, 0 );
	}

	// Signal that thread is stopped.
	pclDlgProgBarExportXls->m_pclThreadStopped->SetEvent();

	return 0;
}

BEGIN_MESSAGE_MAP( CDlgProgBarExportXls, CDlgProgressBar )
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CDlgProgBarExportXls::OnInitDialog()
{
	if( FALSE == CDlgProgressBar::OnInitDialog() )
		return FALSE;
	
	// Create an event to stop the thread when needed.
	m_pclStopEvent = new CEvent( FALSE, TRUE );
	if( NULL == m_pclStopEvent )
		return FALSE;
	m_pclStopEvent->ResetEvent();

	// To pause thread when asking to user if he wants well stop current operation.
	m_pclSuspendEvent = new CEvent( FALSE, TRUE );
	if( NULL == m_pclSuspendEvent )
		return FALSE;
	m_pclSuspendEvent->ResetEvent();

	// Allow to resume a thread that has been suspended.
	m_pclResumeEvent = new CEvent( FALSE, TRUE );
	if( NULL == m_pclResumeEvent )
		return FALSE;
	m_pclResumeEvent->ResetEvent();

	// Create an event to allow thread to specify when it has finished.
	m_pclThreadStopped = new CEvent( FALSE, TRUE );
	if( NULL == m_pclThreadStopped )
		return FALSE;
	m_pclThreadStopped->ResetEvent();

	// Create an event to allow thread to specify when it has suspended its operations.
	m_pclThreadSuspended = new CEvent( FALSE, TRUE );
	if( NULL == m_pclThreadSuspended )
		return FALSE;
	m_pclThreadSuspended->ResetEvent();

	// Create the thread and start it.
	m_pThread = AfxBeginThread( &ThreadInitSheetModules, (LPVOID)this );
	if( NULL == m_pThread )
		return FALSE;
	
	return TRUE;
}

void CDlgProgBarExportXls::OnClose()
{
	if( true == _StopThread() )
		CDlgProgressBar::OnClose();
}

void CDlgProgBarExportXls::OnCancel()
{
	if( true == _StopThread () )
		CDlgProgressBar::OnCancel();
}

bool CDlgProgBarExportXls::_StopThread( void )
{
	bool fCanCallBaseClass = false;
	
	// If thread is not yet stopped...
	DWORD dwReturn = WaitForSingleObject( m_pclThreadStopped->m_hObject, 0 );
	if( dwReturn != WAIT_OBJECT_0 )
	{
		// Immediately suspend the thread.
		m_pclSuspendEvent->SetEvent();
		
		// Wait if thread is well suspended. Perhaps the thread has finished its work.
		// This is why we need to wait on both events.
		HANDLE arhEvent[2];
		arhEvent[0] = m_pclThreadSuspended->m_hObject;
		arhEvent[1] = m_pclThreadStopped->m_hObject;
		while( WAIT_TIMEOUT == ( dwReturn = WaitForMultipleObjects( 2, arhEvent, FALSE, 10 ) ) )
		{
			TASApp.PumpMessages();
		}
		m_pclThreadSuspended->ResetEvent();

		// Ask user confirmation.
		CString str = TASApp.LoadLocalizedString( IDS_PROGBAR_EXPORTXLS );
		if( IDYES == AfxMessageBox( str, MB_YESNO ) )
		{
			// Try to stop only if thread is well suspended.
			if( WAIT_OBJECT_0 == dwReturn )
			{
				m_pclStopEvent->SetEvent();
				// Wait end of the thread.
				while( WAIT_TIMEOUT == WaitForSingleObject( m_pclThreadStopped->m_hObject, 10 ) ) 
				{
					TASApp.PumpMessages();
				}
			}
			// In both cases (we force thread to stop or thread was already stopped) user doesn't want
			// to export.
			m_fCanExport = false;
			fCanCallBaseClass = true;
		}
		else
		{
			// If user want to continue, we signal the thread that it can resume its job.
			m_pclResumeEvent->SetEvent();
		}
	}
	else
		fCanCallBaseClass = true;
	return fCanCallBaseClass;
}

bool CDlgProgBarExportXls::_InitSheetModules( CSSheet* pSheetHM, CTable* pTab )
{
	TASApp.PumpMessages();
	ASSERT( pTab );
	if( NULL == pTab )
		return false;

	// Test if the user want to cancel the exportation or suspend it.
	HANDLE arhEvent[2];
	arhEvent[0] = m_pclStopEvent->m_hObject;
	arhEvent[1] = m_pclSuspendEvent->m_hObject;
	DWORD dwReturn = WaitForMultipleObjects( 2, arhEvent, FALSE, 0 );
	switch( dwReturn )
	{
		// Stop.
		case WAIT_OBJECT_0:
			return false;
			break;

		// Suspend.
		case WAIT_OBJECT_0 + 1:
			{
				// Signal that thread has well received the suspend event.
				m_pclSuspendEvent->ResetEvent();
				m_pclThreadSuspended->SetEvent();

				// Now we must wait stop or resume event.
				arhEvent[0] = m_pclStopEvent->m_hObject;
				arhEvent[1] = m_pclResumeEvent->m_hObject;
				while( WAIT_TIMEOUT == WaitForMultipleObjects( 2, arhEvent, FALSE, 10 ) )
				{
					TASApp.PumpMessages();
				}
				switch( dwReturn )
				{
					// Stop.
					case WAIT_OBJECT_0:
						// User wants to abort exporting. We stop the thread.
						// Remark: don't reset the stop event. As we are in a recursive method, we have to
						//         stop on the 'WaitForMultipleObjects' at the entry of the method.
						return false;
						break;

					// Resume.
					case WAIT_OBJECT_0 + 1:
						// Finally user doesn't want to stop. We resume the thread.
						m_pclResumeEvent->ResetEvent();
						break;
				}
				break;
			}
	}

	CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>(pTab);
	if( NULL != pHM )
	{
		( (CSheetHMCalc*)pSheetHM )->Init( pHM, false, m_iTabOffset, true );
		
		m_iTabOffset++;
		UpdateProgress( m_iTabOffset + 1, TASApp.LoadLocalizedString(IDS_PROGBAR_EXPORTNAME) + _T(" ") + pHM->GetHMName() );

		// If the number of modules exceed the maximum allowed by spread --> don't show the last ones. An AfxMessageBox 
		// is created to prevent the user.
		int iSheetNbr = ( (CSheetHMCalc*)pSheetHM )->SheetDescription::SLast;
		if( m_iTabOffset > ( m_iMaxTabSheet / iSheetNbr ) )
			return false;
	}

	// Create a map to sort the full level.
	std::map<unsigned int, CDS_HydroMod *> sortmap;
	for( IDPTR IDptr = pTab->GetFirst(); NULL != *IDptr.ID; IDptr = pTab->GetNext() )
	{
		CDS_HydroMod* pCHM = dynamic_cast<CDS_HydroMod*>( IDptr.MP );			ASSERT( pCHM );
		if( true == pCHM->IsaModule() )
			sortmap.insert( std::pair<unsigned int, CDS_HydroMod *>( pCHM->GetPos(), pCHM ) );
	}

	std::map<unsigned int, CDS_HydroMod *>::const_iterator sortmapCIt;
	for( sortmapCIt = sortmap.begin(); sortmapCIt != sortmap.end(); sortmapCIt++ )
	{
		if( false == _InitSheetModules( pSheetHM, sortmapCIt->second ) )
			return false;
	}

	return true;	
}
