#include "stdafx.h"


#include "mainfrm.h"
#include "taselectdoc.h"
#include "Utilities.h"
#include "DlgLeftTabSelManager.h"
#include "DlgSpecActReformatFilterTabFiles.h"

CDlgSpecActReformatFilterTabFiles::CDlgSpecActReformatFilterTabFiles( CWnd *pParent )
	: CDialogExt( CDlgSpecActReformatFilterTabFiles::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
}

BEGIN_MESSAGE_MAP( CDlgSpecActReformatFilterTabFiles, CDialogExt )
	ON_BN_CLICKED( IDC_BUTTONFILTERTABPATH, &OnBnClickedOpenFilterTabPath )
	ON_BN_CLICKED( IDC_BUTTONSTARTREFORMAT, &OnBnClickedStartReformat )
	ON_BN_CLICKED( IDC_BUTTONCLEAR, &OnBnClickedClear )
END_MESSAGE_MAP()

void CDlgSpecActReformatFilterTabFiles::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFILTERTABPATH, m_EditFilterTabPath );
	DDX_Control( pDX, IDC_LIST, m_List );
}

BOOL CDlgSpecActReformatFilterTabFiles::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CString str = TASApp.GetProfileString( _T("Testing"), _T("ReformatFilterTabFiles"), _T("") );
	m_EditFilterTabPath.SetWindowText( str );

	m_List.ModifyStyle( LBS_SORT, 0 );

	return TRUE;
}

void CDlgSpecActReformatFilterTabFiles::OnBnClickedOpenFilterTabPath()
{
	HRESULT hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );

	if( FAILED( hr ) )
	{
		return;

	}

	CString str;
	m_EditFilterTabPath.GetWindowText( str );

	IFileOpenDialog *pFileDialog;
	hr = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>( &pFileDialog ) );

	if( FAILED( hr ) )
	{
		return;
	}
	
	pFileDialog->SetOptions( FOS_PICKFOLDERS );

	hr = pFileDialog->Show( NULL );

	if( FAILED( hr ) )
	{
		return;
	}

	IShellItem *pItem;
	hr = pFileDialog->GetResult( &pItem );

	if( FAILED( hr ) )
	{
		return;
	}

	PWSTR pszFilePath;
	hr = pItem->GetDisplayName( SIGDN_FILESYSPATH, &pszFilePath );

	if( FAILED( hr ) )
	{
		return;
	}

	m_EditFilterTabPath.SetWindowText( pszFilePath );
	TASApp.WriteProfileString( _T("Testing"), _T("ReformatFilterTabFiles"), pszFilePath );

	CoTaskMemFree( pszFilePath );
	pItem->Release();
	pFileDialog->Release();
	CoUninitialize();
}

void CDlgSpecActReformatFilterTabFiles::OnBnClickedStartReformat()
{
	CString strFilterTabFolder;
	m_EditFilterTabPath.GetWindowText( strFilterTabFolder );

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Start...") ) );

	int iDepth = 0;
	_FindFile( strFilterTabFolder, iDepth );

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Finished.") ) );
}

void CDlgSpecActReformatFilterTabFiles::OnBnClickedClear()
{
	m_List.ResetContent();
}

void CDlgSpecActReformatFilterTabFiles::_FindFile( CString strPath, int &iDepth )
{
	CFileFind finder;

	CString strPattern = strPath + _T("\\*.*");
	BOOL bWorking = finder.FindFile( strPattern );

	while( bWorking )
	{
		bWorking = finder.FindNextFile();

		if( TRUE == finder.IsDots() )
		{
			continue;
		}

		if( TRUE == finder.IsDirectory() )
		{
			_FindFile( finder.GetFilePath(), ++iDepth );
		}

		CString strFileName = finder.GetFileName();
		strFileName.MakeLower();

		CPath clPath( strFileName );
		CString strExtension = clPath.SplitPath( CPath::ePathFields::epfExt );
		strExtension.MakeLower();
		
		if( iDepth > 0 && -1 != strFileName.Find( _T("filtertab"), 0 ) && -1 != strExtension.Find( _T(".txt" ), 0 ) )
		{
			_ReformatTabFilterFile( finder.GetFilePath() );

			iDepth--;
			finder.Close();
			return;
		}
	}

	finder.Close();
}

void CDlgSpecActReformatFilterTabFiles::_ReformatTabFilterFile( CString strFilePath )
{
	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strFilePath ) );

	CString strErrorMsg;
	CTUProdSelFileHelper clFilerTabFile;
	CTUProdSelFileHelper clFilerTabFileOut;

	if( false == clFilerTabFile.IsFileExist( strFilePath ) )
	{
		strErrorMsg.Format( _T("File '%' doesn't exist."), strFilePath );
		m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strErrorMsg ) );
		return;
	}

	if( TU_PRODSELECT_ERROR_OK != clFilerTabFile.OpenFile( strFilePath, _T("r") ) )
	{
		strErrorMsg.Format( _T("Can't open the '%' file."), strFilePath );
		m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strErrorMsg ) );
		return;
	}

	CString strOutputFile( strFilePath );
	strOutputFile.Insert( strOutputFile.ReverseFind( _T( '.' ) ), _T("-Formatted") );

	if( TU_PRODSELECT_ERROR_OK != clFilerTabFileOut.OpenFile( strOutputFile, _T("w") ) )
	{
		strErrorMsg.Format( _T("Can't open the '%' file."), strOutputFile );
		m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strErrorMsg ) );
		return;
	}

	std::map<int, std::vector<int>> mapAllFilters;

	// For 'CData::CheckFilter' for edition.
	std::vector<int> vecSeparatorPos = { 29, 52, 75, 82 };
	mapAllFilters[(int)vecSeparatorPos.size() + 1] = vecSeparatorPos;

	// For 'CData::CheckFilter' for deleting.
	vecSeparatorPos = { 29, 52, 75 };
	mapAllFilters[(int)vecSeparatorPos.size() + 1] = vecSeparatorPos;

	// For 'CDB_Product::CheckFilter'.
	// ClassName; Table; ID; EDIT; FAM_ID; VERS_ID; a,h ; m1,i1; Priority
	vecSeparatorPos = { 29, 52, 75, 82, 105, 125, 131, 156 };
	mapAllFilters[(int)vecSeparatorPos.size() + 1] = vecSeparatorPos;

	// For 'CDB_TAProduct::CheckFilter'.
	// ClassName; Table; ID; EDIT; FAM_ID; CONN_ID; VERS_ID; <=PN_ID; a,h; m1,i1; Priority; -d IDBROL
	vecSeparatorPos = { 29, 52, 75, 82, 105, 125, 143, 153, 159, 183, 194 };
	mapAllFilters[(int)vecSeparatorPos.size() + 1] = vecSeparatorPos;

	// For 'CDB_Set::CheckFilter'.
	vecSeparatorPos = { 29, 52, 75, 82, 105 };
	mapAllFilters[(int)vecSeparatorPos.size() + 1] = vecSeparatorPos;

	CString strLine;
	CString strPreviousLine;
	std::vector<CString> vecTokens;
	std::vector<CString> vecPreviousTokens;
	bool bCanContinue = true;
	bool bGroupStarted = false;
	bool bClassNameDetected = false;
	int iGroupID = -1;
	int iLine = 1;
	CString strFormatted;

	do
	{
		bool bWriteNow = false;
		bool bFormat = false;
		bool bProcessLine = false;
		UINT uiErrorCode = clFilerTabFile.ReadOneLine( strLine, false );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return;
		}

		bool bComment = false;
		bool bEmpty = false;
		CString strFirstToken = _T("");
		strLine.Trim();

		if( false == strLine.IsEmpty() )
		{
			if( TU_PRODSELECT_ERROR_OK != clFilerTabFile.SplitOneLineMultiValues( strLine, vecTokens, _T(";"), false ) )
			{
				strErrorMsg.Format( _T("Problem when trying to split the line %i."), iLine );
				m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strErrorMsg ) );
				return;
			}

			strFirstToken = vecTokens[0];
			strFirstToken.MakeLower();

			bComment = ( _T('/') == vecTokens[0].GetAt( 0 ) && _T('/') == vecTokens[0].GetAt( 1 ) ) ? true : false;

			if( false == bClassNameDetected )
			{
				if( -1 != strFirstToken.Find( _T("classname") ) )
				{
					// 'ClassName' detected, we need the next line to check if is well the start of a new group or
					// if it's only the comment in the start of the file.
					bGroupStarted = false;
					bClassNameDetected = true;
				}
			}
			else
			{
				if( -1 != strFirstToken.Find( _T("//-") ) )
				{
					// If line containing the 'ClassName' string is followed by the line "//--------" then it is the
					// start of a new group.
					bGroupStarted = true;

					// Group has started. Format the line "// ClassName ; ; ; ".
					iGroupID = (int)vecPreviousTokens.size();

					if( 0 == mapAllFilters.count( iGroupID ) )
					{
						strErrorMsg.Format( _T("'ClassName' definition at line %i is not recognized."), iLine - 1 );
						m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strErrorMsg ) );
						return;
					}

					if( false == _FormatString( vecPreviousTokens, mapAllFilters.at( iGroupID ), strFormatted ) )
					{
						strErrorMsg.Format( _T("Error when trying to format the line %i."), iLine - 1 );
						m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strErrorMsg ) );
						return;
					}

					clFilerTabFileOut.WriteOneLine( strFormatted, false );
					bClassNameDetected = false;
				}
				else
				{
					// The line following the previous one with 'ClassName' is not starting by "//-". Thus the previous
					// line was well a comment in the start of the file.
					clFilerTabFileOut.WriteOneLine( strPreviousLine, false );
					bClassNameDetected = false;
				}
			}
		}
		else
		{
			bEmpty = true;
		}

		if( false == bClassNameDetected )
		{
			if( true == bEmpty )
			{
				clFilerTabFileOut.WriteOneLine( _T(""), false );
			}
			else if( true == bComment )
			{
				clFilerTabFileOut.WriteOneLine( strLine, false );
			}
			else
			{
				if( false == _FormatString( vecTokens, mapAllFilters.at( iGroupID ), strFormatted ) )
				{
					strErrorMsg.Format( _T("Error when trying to format the line %i."), iLine );
					m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strErrorMsg ) );
					return;
				}

				clFilerTabFileOut.WriteOneLine( strFormatted, false );
			}
		}
		
		strPreviousLine = strLine;
		vecPreviousTokens = vecTokens;
		iLine++;
	}while( true == bCanContinue );
}

bool CDlgSpecActReformatFilterTabFiles::_FormatString( std::vector<CString> &vecTokens, std::vector<int> &vecSeparatorPos, CString &strFormatted )
{
	strFormatted = _T("");

	if( (int)vecTokens.size() != (int)vecSeparatorPos.size() + 1 )
	{
		CString strMsg;
		strMsg.Format( _T("Number of tokens are incorrect (Must be %i)."), (int)vecSeparatorPos.size() );
		m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strMsg ) );
		return false;
	}

	for( int i = 0; i < (int)vecTokens.size(); i++ )
	{
		CString strToken = vecTokens.at( i );
		strToken.Trim();

		if( 0 == i )
		{
			strFormatted = strToken;
		}
		else
		{
			strFormatted += _T(" ") + strToken;
		}

		// Verify length.
		if( i < (int)vecTokens.size() - 1 )
		{
			int iMaxLength;

			if( 0 == i )
			{
				iMaxLength = vecSeparatorPos.at( i ) - 1;
			}
			else
			{
				iMaxLength = vecSeparatorPos.at( i ) - vecSeparatorPos.at( i - 1 ) - 2;
			}

			int iTokenLength = strToken.GetLength();
			
			if( iTokenLength > iMaxLength )
			{
				CString strMsg;
				strMsg.Format( _T("Tokens '%s' is too big at this position (%i)."), strToken, i );
				m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strMsg ) );
				return false;
			}

			// Add spaces.
			for( int j = 0; j < iMaxLength - iTokenLength; j++ )
			{
				strFormatted += _T(" ");
			}
		}

		if( i < (int)vecTokens.size() - 1 )
		{
			strFormatted += _T(";");
		}
	}

	return true;
}
