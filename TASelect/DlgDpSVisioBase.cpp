#include "stdafx.h"
#include "DpSVisioIncludes.h"

CDlgDpSVisioBase::CDlgDpSVisioBase( UINT nIDTemplate, CDlgDpSVisio *pParent )
	: CDialogEx( nIDTemplate, (CWnd *)pParent )
{
	m_pParent = pParent;
}

BOOL CDlgDpSVisioBase::PreTranslateMessage( MSG *pMsg )
{
	if( WM_KEYDOWN == pMsg->message )
    {
        if( VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam )
        {
            return TRUE;                // Do not process further
        }
    }

    return CWnd::PreTranslateMessage( pMsg );
}

void CDlgDpSVisioBase::OnOK()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ENTER] key).
}

void CDlgDpSVisioBase::OnCancel()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ESCAPE] key).
}
