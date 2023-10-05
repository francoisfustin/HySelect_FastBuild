#include "stdafx.h"
#include "MainFrm.h"
#include "TASelect.h"
#include "Global.h"
#include "DlgLTtabctrl.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabInfo.h"

#include "RViewInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRViewInfo *pRViewInfo = NULL;

IMPLEMENT_DYNCREATE( CRViewInfo, CHtmlView )

CRViewInfo::~CRViewInfo()
{
	Stop();
	pRViewInfo = NULL;
}

void CRViewInfo::DisplayPage( LPCTSTR ptcURL )
{
	// Variable
	CString strURL;

	// Verify the existence of the url.
	// If not, display the Qrs_none catalog.
	if( _T('\0') == *ptcURL )
	{
		CString strQrsNoneFolder = TASApp.GetDocumentsFolderForHelp();
		CString strQrsNonePath = strQrsNoneFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _HELP_DIRECTORY ) + TASApp.GetLanguage() + CString( _T("\\") );
		strQrsNonePath += _T("Qrs_none.htm");
		strURL = strQrsNonePath;
	}
	else 
	{
		strURL = ptcURL;
	}
	
	// Check the existence of this file
	CFileFind finder;
	BOOL bFind = finder.FindFile( strURL );

	// test if substitute file exist
	if( TRUE == bFind )
	{
		//BUG KB130860: FindFirstFile() Does Not Handle Wildcard (?) Correctly		
		// Verify the finded file has the same length.
		// Problem : TBV-C???.pdf will find
		// TBV-CM_EN.pdf and not TBV-C_EN.pdf.
		bool bSameLength = false;
		int iTempLength = strURL.GetLength();
		CString URLExact = strURL;
		URLExact.Remove('?');

		while( TRUE == bFind )
		{
			bFind = finder.FindNextFile();
			strURL = finder.GetFilePath();

			if( iTempLength == strURL.GetLength() || 0 == strURL.Compare( URLExact ) )
			{
				break;
			}
		}
	}
	
	// Start navigation.
	strURL.MakeLower();
		
	if( m_strOriginalPage != strURL )
	{
		m_strOriginalPage = strURL;
		SetStatusBar( TRUE );
		Navigate2( m_strOriginalPage );
	}
}

void CRViewInfo::OnInitialUpdate() 
{
	pRViewInfo = this;
	Navigate2( m_strOriginalPage );
}

BEGIN_MESSAGE_MAP( CRViewInfo, CHtmlView )
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CRViewInfo::OnPaint() 
{
	CPaintDC dc( this );
	
	// Do not call CHtmlView::OnPaint() for painting messages
}
