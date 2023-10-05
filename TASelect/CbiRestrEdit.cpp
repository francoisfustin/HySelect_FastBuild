#include "stdafx.h"
#include "Imm.h"
#include "CbiRestrEdit.h"
#include "TASelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CCbiRestrEdit::CCbiRestrEdit()
{
}

CCbiRestrEdit::~CCbiRestrEdit()
{
}

BEGIN_MESSAGE_MAP( CCbiRestrEdit, CEdit )
	ON_MESSAGE( WM_IME_STARTCOMPOSITION, OnImeStartComposition )
	ON_CONTROL_REFLECT( EN_CHANGE, OnCBIRestEditChange )
	ON_WM_CHAR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCbiRestrEdit message handlers

LRESULT CCbiRestrEdit::OnImeStartComposition( WPARAM wParam, LPARAM lParam )
{
	TASApp.AfxLocalizeMessageBox( AFXMSG_CBICHARRESTRICTION );
	return true;
}

void CCbiRestrEdit::OnCBIRestEditChange() 
{
	if( NULL == m_hWnd )
		return;
	
	CString str; 
	GetWindowText( str );
	
	// Return if the project is not TA-CBI compatible.
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( false == pPrjParam->IsCompatibleCBI() )
	{
		return;
	}

	// If input is not compatible to the CBI mode...
	if( false == CCbiRestrString::CheckCBICharSet( &str ) )
	{
		SetWindowText( str );
		TASApp.AfxLocalizeMessageBox( AFXMSG_CBICHARRESTRICTION );
		PostMessage( WM_KEYDOWN, VK_END, 0 );
		SetFocus();  
	}
}
