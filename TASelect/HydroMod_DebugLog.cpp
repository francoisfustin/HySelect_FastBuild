#include "stdafx.h"

#include "TASelect.h"
#include "HydroMod.h"
#include "HydroMod_DebugLog.h"


#ifdef _DEBUG

CDS_HydroMod_DebugLog::CDS_HydroMod_DebugLog()
{
	m_strFileName = CDS_HYDROMOD_COMPUTEALLLOG_DEFAULTFILENAME;
	m_strTabIndent = _T("");
	m_iTabLength = CString( CDS_HYDROMOD_COMPUTEALLLOG_TABULATION ).GetLength();
	m_bActivated = false;
}

CDS_HydroMod_DebugLog::~CDS_HydroMod_DebugLog()
{
	ComputeAllLog_CloseLogfile();
}

void CDS_HydroMod_DebugLog::ComputeAllLog_Activate( bool bActivate )
{
	if( false == bActivate )
	{
		ComputeAllLog_CloseLogfile();
	}

	m_bActivated = bActivate;
}

void CDS_HydroMod_DebugLog::ComputeAllLog_SetLogFileName( CString strLogFileName )
{
	m_strFileName = strLogFileName;
}

void CDS_HydroMod_DebugLog::ComputeAllLog_OpenLogfile()
{
	if( false == m_bActivated )
	{
		return;
	}

	CFileException fileExecption;
	
	if( FALSE == m_clFile.Open( m_strFileName, CFile::modeWrite | CFile::modeCreate, &fileExecption ) )
	{
		TCHAR szCause[255];
		fileExecption.GetErrorMessage( szCause, 255 );
		OutputDebugStringW( szCause );
	}
}

void CDS_HydroMod_DebugLog::ComputeAllLog_CloseLogfile()
{
	if( false == m_bActivated )
	{
		return;
	}

	if( CFile::hFileNull != m_clFile.m_hFile )
	{
		m_clFile.Close();
	}
	
	m_strTabIndent = _T("");
}

void CDS_HydroMod_DebugLog::ComputeAllLog_ChangeTabIndent( TabIndentFct eTabIndentFct )
{
	if( false == m_bActivated )
	{
		return;
	}

	_TabIndent( eTabIndentFct );
}

void CDS_HydroMod_DebugLog::ComputeAllLog_WriteString( TabIndentFct eTabIndentFct, CString strText, ... )
{
	if( false == m_bActivated )
	{
		return;
	}

	if( CFile::hFileNull == m_clFile.m_hFile )
	{
		return;
	}

	if( TIF_Pre == ( eTabIndentFct & TIF_Pre ) )
	{
		_TabIndent( eTabIndentFct );
	}

	va_list arglist;
	va_start( arglist, strText );
	CString strAllText;
	strAllText.FormatV( strText, arglist );
	va_end( arglist );

	CString strLine;

	if( TIF_NoTab != ( eTabIndentFct & TIF_NoTab ) )
	{
		strLine.Format( _T("%s%s"), m_strTabIndent, strAllText );
	}
	else
	{
		strLine = strAllText;
	}
	
	USES_CONVERSION;
	CStringA strLineA = W2A( strLine );

	m_clFile.Write( strLineA, strLine.GetLength() );

	if( TIF_Post == ( eTabIndentFct & TIF_Post ) )
	{
		_TabIndent( eTabIndentFct );
	}
}

void CDS_HydroMod_DebugLog::ComputeAllLog_WriteVariable( VariableType eVariableType, CString strVariableName, LPARAM lpParam )
{
	if( false == m_bActivated )
	{
		return;
	}

	CString strValue;

	if( VT_Bool == eVariableType )
	{
		strValue = ( true == *( (bool*)lpParam ) ) ? _T("true") : _T("false");
	}
	else if( VT_Int == eVariableType )
	{
		strValue.Format( _T("%i"), *( (int*)lpParam ) );
	}
	else if( VT_Double == eVariableType )
	{
		strValue.Format( _T("%f"), *( (double*)lpParam ) );
	}
	else if( VT_CString == eVariableType )
	{
		strValue = *( (CString*)lpParam );
	}

	CString strLine = strVariableName + _T(" = ") + strValue + _T("\r\n");
	ComputeAllLog_WriteString( TIF_Nothing, strLine );
}

void CDS_HydroMod_DebugLog::ComputeAllLog_WriteBypassModuleList( CDS_HydroMod *pRootHM, std::multimap<int,CDS_HydroMod *> *pBypassModuleMap )
{
	if( false == m_bActivated )
	{
		return;
	}

	if( CFile::hFileNull == m_clFile.m_hFile || NULL == pRootHM || NULL == pBypassModuleMap )
	{
		return;
	}

	ComputeAllLog_WriteString( TIF_PoI, _T("Start of bypass module list.\r\n") );

	CString strLine;

	for( std::multimap<int,CDS_HydroMod *>::iterator iter = pBypassModuleMap->begin(); iter != pBypassModuleMap->end(); iter++ )
	{
		if( INT_MAX != iter->first )
		{
			int iLevel = (int)( iter->first / 10000 );
			int iPos = (int)( iter->first % 10000 );
			strLine.Format( _T("Level: %3i - Pos: %3i - Module name: %s.\r\n"), iLevel, iPos, iter->second->GetHMName() );
		}
		else
		{
			strLine.Format( _T("Root module name: %s.\r\n"), iter->second->GetHMName() );
		}

		ComputeAllLog_WriteString( TIF_Nothing, strLine );
	}

	ComputeAllLog_WriteString( TIF_PrD, _T("End of bypass module list.\r\n\r\n") );
}

void CDS_HydroMod_DebugLog::ComputeAllLog_WriteModulatingCvList( CDS_HydroMod *pRootHM, std::vector<CDS_HydroMod *> *pModCVVector )
{
	if( false == m_bActivated )
	{
		return;
	}

	if( CFile::hFileNull == m_clFile.m_hFile || NULL == pRootHM || NULL == pModCVVector )
	{
		return;
	}

	ComputeAllLog_WriteString( TIF_PoI, _T("Start of modulating CV list.\r\n") );

	CString strLine;

	for( std::vector<CDS_HydroMod *>::iterator iter = pModCVVector->begin(); iter != pModCVVector->end(); iter++ )
	{
		strLine.Format( _T("Module name: %s - CV name: %s.\r\n"), (*iter)->GetHMName(), (*iter)->GetpCV()->GetCvIDPtr().ID );
		ComputeAllLog_WriteString( TIF_Nothing, strLine );
	}

	ComputeAllLog_WriteString( TIF_PrD, _T("End of modulating CV list.\r\n\r\n") );
}

void CDS_HydroMod_DebugLog::_TabIndent( TabIndentFct eTabIndentFct )
{
	switch( eTabIndentFct & TIF_IDMask )
	{
		case TIF_Nothing:
			break;

		case TIF_Inc:
			m_strTabIndent.Append( CDS_HYDROMOD_COMPUTEALLLOG_TABULATION );
			break;

		case TIF_Dec:
			
			if( m_strTabIndent.GetLength() > ( m_iTabLength - 1 ) )
			{
				m_strTabIndent = m_strTabIndent.Left( m_strTabIndent.GetLength() - m_iTabLength );
			}

			break;
	}
}

#endif
