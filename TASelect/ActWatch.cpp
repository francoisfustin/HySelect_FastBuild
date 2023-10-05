// ActWatch.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "ActWatch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CActivationWatcher

IMPLEMENT_DYNCREATE(CActivationWatcher, CWinThread)

CActivationWatcher::CActivationWatcher()
{
}

CActivationWatcher::~CActivationWatcher()
{
}

BOOL CActivationWatcher::InitInstance()
{
	CSyncObject* pEvents[2];
	pEvents[1] = TASApp.GetpInstanceEvent();
	pEvents[0] = TASApp.GetpShutdownEvent();

	DWORD dwSignalled;
	CMultiLock Locker(pEvents, 2);

	while (1)
	{
		dwSignalled = Locker.Lock(INFINITE, FALSE);
		TRACE(_T("dwSignalled is %d\n"), dwSignalled);

		// is it Shutdown?
		if ((dwSignalled-WAIT_OBJECT_0) == 0)
			break;

		// is it Instance?
		if ((dwSignalled-WAIT_OBJECT_0) == 1)
		{
			CWnd MainWindow;
			if (MainWindow.Attach(TASApp.m_pMainWnd->m_hWnd))
			{
				CWnd* pWnd = MainWindow.GetLastActivePopup();
				pWnd->SetForegroundWindow();
				MainWindow.Detach();
				OnIdle(0);
			}
			else
				::MessageBeep(0);
		}
	}

	// never do ExitInstance() or Run()
	return FALSE;
}

BEGIN_MESSAGE_MAP(CActivationWatcher, CWinThread)
	//{{AFX_MSG_MAP(CActivationWatcher)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CActivationWatcher message handlers
