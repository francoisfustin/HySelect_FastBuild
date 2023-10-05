#include "stdafx.h"
#include "atlpath.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"
#include "Utilities.h"

#include "SelectPM.h"
#include "RViewSSelSS.h"
#include "DlgSelectionBase.h"
#include "DlgBatchSelBase.h"
#include "ProductSelectionParameters.h"
#include "TUProductSelectionHelper.h"

#ifdef DEBUG


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to ease access to a file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUProdSelFileHelper::CTUProdSelFileHelper()
{
	m_pfFile = NULL;
	m_iLineNumber = 0;
}

CTUProdSelFileHelper::~CTUProdSelFileHelper()
{
	CloseFile();
}

bool CTUProdSelFileHelper::IsFileExist( CString strFileName )
{
	CPathT<CString>clPath( strFileName );
	return ( TRUE == clPath.FileExists() ) ? true : false;
}

UINT CTUProdSelFileHelper::OpenFile( CString strFileName, CString strMode )
{
	// To be sure.
	CloseFile();

	if( true == strFileName.IsEmpty() )
	{
		return TU_ERROR_FILEHELPER_OPENFILE_FILENAMEEMPTY;
	}

	if( true == strMode.IsEmpty() )
	{
		return TU_ERROR_FILEHELPER_OPENFILE_MODEEMPTY;
	}

	// Try to open the file.
	m_pfFile = _tfopen( strFileName, strMode );

	if( NULL == m_pfFile )
	{
		return TU_ERROR_FILEHELPER_OPENFILE_CANTOPEN;
	}

	return TU_PRODSELECT_ERROR_OK;
}

void CTUProdSelFileHelper::CloseFile()
{
	if( NULL != m_pfFile )
	{
		fclose( m_pfFile );
		m_pfFile = NULL;
	}
}

UINT CTUProdSelFileHelper::ReadOneLine( CString &strLine, bool bForProdSelTest )
{
	if( NULL == m_pfFile )
	{
		return TU_ERROR_FILEHELPER_READLINE_FILENOTOPENED;
	}

	TCHAR tcLine[1024];
	bool bStop = false;

	do 
	{
		if( NULL == _fgetts( tcLine, 1024, m_pfFile ) )
		{
			if( 0 != feof( m_pfFile ) )
			{
				fclose( m_pfFile);
				m_pfFile = NULL;
				return TU_ERROR_FILEHELPER_READLINE_UNEXPECTEDEOF;
			}
			else
			{
				fclose( m_pfFile);
				m_pfFile = NULL;
				return TU_ERROR_FILEHELPER_READLINE_ERRORINFILE;
			}
		}

		m_iLineNumber++;
		strLine = tcLine;
		strLine.Trim( _T("\r\n") );

		if( true == bForProdSelTest )
		{
			//strLine.MakeLower();

			if( false == strLine.IsEmpty() && _T('#') != strLine.GetAt( 0 ) )
			{
				bStop = true;
			}
		}
		else
		{
			bStop = true;
		}
	}while( false == bStop );

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelFileHelper::ReadOneLineKeyValue( CString &strLine, CString &strKey, CString &strValue, CString strToken, bool bLowerKey, bool bLowerValue )
{
	TU_FILEHELPER_READLINE( (*this), strLine );
	m_iLineNumber++;

	int iCurPos = 0;
	strKey = strLine.Tokenize( strToken, iCurPos );

	if( true == strKey.IsEmpty() )
	{
		return TU_ERROR_FILEHELPER_READLINE_BADTOKEN;
	}

	strKey.Trim();

	if( true == bLowerKey )
	{
		strKey.MakeLower();
	}

	// 'strValue' can be NULL.
	strValue = strLine.Tokenize( strToken, iCurPos );
	strValue.Trim();

	if( true == bLowerValue )
	{
		strValue.MakeLower();
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelFileHelper::ReadOneLineMultiValues( CString &strLine, std::vector<CString> &vecStrings, CString strToken, bool bForProdSelTest )
{
	TU_FILEHELPER_READLINE( (*this), strLine );
	m_iLineNumber++;

	return SplitOneLineMultiValues( strLine, vecStrings, strToken, bForProdSelTest );
}

UINT CTUProdSelFileHelper::WriteOneLine( CString strLine, bool bForProdSelTest )
{
	if( NULL == m_pfFile )
	{
		return TU_ERROR_FILEHELPER_WRITELINE_FILENOTOPENED;
	}

	if( false == bForProdSelTest )
	{
		strLine += _T("\n");
	}

	if( _ftprintf( m_pfFile, strLine ) < 0 )
	{
		CloseFile();
		return TU_ERROR_FILEHELPER_WRITELINE_WRITEERROR;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelFileHelper::WriteOneLineArgText( CString strLine, CString strArg, bool bForProdSelTest )
{
	if( NULL == m_pfFile )
	{
		return TU_ERROR_FILEHELPER_WRITELINE_FILENOTOPENED;
	}

	if( false == bForProdSelTest )
	{
		strLine += _T("\n");
	}

	if( _ftprintf( m_pfFile, strLine, strArg ) < 0 )
	{
		CloseFile();
		return TU_ERROR_FILEHELPER_WRITELINE_WRITEERROR;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelFileHelper::WriteOneLineArgFloat( CString strLine, double fValue, bool bForProdSelTest )
{
	if( NULL == m_pfFile )
	{
		return TU_ERROR_FILEHELPER_WRITELINE_FILENOTOPENED;
	}

	if( false == bForProdSelTest )
	{
		strLine += _T("\n");
	}

	if( _ftprintf( m_pfFile, strLine, fValue ) < 0 )
	{
		CloseFile();
		return TU_ERROR_FILEHELPER_WRITELINE_WRITEERROR;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelFileHelper::SplitOneLineMultiValues( CString &strLine, std::vector<CString> &vecStrings, CString strToken, bool bForProdSelTest )
{
	vecStrings.clear();

	int iPrevPos = 0;

	if( _T(';') == strLine.GetAt( 0 ) )
	{
		vecStrings.push_back( _T("") );
		iPrevPos = 1;
	}

	int iCurrentPos = strLine.Find( strToken, iPrevPos );

	while( -1 != iCurrentPos )
	{
		CString strValue = _T("");

		if( iCurrentPos > iPrevPos )
		{
			strValue = strLine.Mid( iPrevPos, iCurrentPos - iPrevPos );
		}

		strValue.Trim();

		if( true == bForProdSelTest )
		{
			strValue.MakeLower();
		}

		vecStrings.push_back( strValue );
		iPrevPos = iCurrentPos + 1;
		iCurrentPos = strLine.Find( strToken, iPrevPos );
	}

	if( _T(';') == strLine.GetAt( strLine.GetLength() - 1 ) )
	{
		vecStrings.push_back( _T("") );
	}
	else
	{
		CString strValue = strLine.Right( strLine.GetLength() - iPrevPos ).Trim();

		if( true == bForProdSelTest )
		{
			strValue.MakeLower();
		}

		vecStrings.push_back( strValue );
	}

	return TU_PRODSELECT_ERROR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to help to do test unit on product selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUProdSelHelper::CTUProdSelHelper( CProductSelelectionParameters &clProductSelectionParams )
{
	m_pclProdSelParams = &clProductSelectionParams;
}

UINT CTUProdSelHelper::SetpTADB( CTADatabase *pTADB )
{
	if( NULL == m_pclProdSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	if( NULL == pTADB )
	{
		return -1;
	}

	m_pclProdSelParams->m_pTADB = pTADB;
	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::SetpTADS( CTADatastruct *pTADS )
{
	if( NULL == m_pclProdSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	if( NULL == pTADS )
	{
		return -1;
	}

	m_pclProdSelParams->m_pTADS = pTADS;
	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::SetpUserDB( CUserDatabase *pUserDB )
{
	if( NULL == m_pclProdSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	if( NULL == pUserDB )
	{
		return -1;
	}

	m_pclProdSelParams->m_pUserDB = pUserDB;
	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::SetpPipeDB( CPipeUserDatabase *pPipeDB )
{
	if( NULL == m_pclProdSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	if( NULL == pPipeDB )
	{
		return -1;
	}

	m_pclProdSelParams->m_pPipeDB = pPipeDB;
	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::ReadInputs( CTUProdSelFileHelper &clTUFileHelper )
{
	m_mapInputs.clear();
	m_mapInputsWithCase.clear();

	CString strLine;
	TU_FILEHELPER_READLINE( clTUFileHelper, strLine );

	if( 0 != strLine.CompareNoCase( _T("StartInputs") ) )
	{
		return TU_ERROR_HELPER_READINPUTS_PRODSEL_BADSTART;
	}

	bool bStop = false;

	do 
	{
		CString strKey;
		CString strValue;
		UINT uiErrorCode = clTUFileHelper.ReadOneLineKeyValue( strLine, strKey, strValue, _T("="), true, false );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
		else if( 0 == strLine.CompareNoCase( _T("EndInputs" ) ) )
		{
			bStop = true;
		}
		else
		{
			m_mapInputsWithCase[strKey] = strValue; // For String ID
			m_mapInputs[strKey] = strValue.MakeLower();
		}

	}while( false == bStop );

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::InterpreteInputs()
{
	if( NULL == m_pclProdSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	// The 'm_WC.m_AdditFamID' variable.
	// Remark: not mandatory. By default = "WATER_ADDIT".
	if( 0 != m_mapInputs.count( _T("wcadditfamilyid" ) ) )
	{
		m_pclProdSelParams->m_WC.SetAdditFamID( m_mapInputsWithCase[_T("wcadditfamilyid" )] );
	}
	
	// The 'm_WC.m_AdditID' variable.
	// Remark: not mandatory. By default = "WATER".
	if( 0 != m_mapInputs.count( _T("wcadditid" ) ) )
	{
		m_pclProdSelParams->m_WC.SetAdditID( m_mapInputsWithCase[_T("wcadditid" )] );
	}

	// The 'm_WC.m_dTemp' variable.
	// Remark: not mandatory. By default = '20.0'.
	if( 0 != m_mapInputs.count( _T("wctemp" ) ) )
	{
		m_pclProdSelParams->m_WC.SetTemp( _wtof( m_mapInputs[_T("wctemp" )] ) );
	}

	// The 'm_dPcWeight' variable.
	// Remark: not mandatory. By default = '0.0'.
	if( 0 != m_mapInputs.count( _T("wcpcweight" ) ) )
	{
		m_pclProdSelParams->m_WC.SetPcWeight( _wtof( m_mapInputs[_T("wcpcweight" )] ) );
	}
	
	// To update other variable inside the 'CWaterChar' object.
	m_pclProdSelParams->m_WC.UpdateFluidData( m_pclProdSelParams->m_WC.GetTemp(), m_pclProdSelParams->m_WC.GetPcWeight() );

	// The 'm_strPipeSeriesID' variable.
	// Remark: not mandatory. By default = "STEEL_GEN".
	if( 0 != m_mapInputs.count( _T("pipeseriesid" ) ) )
	{
		m_pclProdSelParams->m_strPipeSeriesID = m_mapInputsWithCase[_T("pipeseriesid" )];
	}

	// The 'm_strPipeID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("pipeid" ) ) )
	{
		m_pclProdSelParams->m_strPipeID = m_mapInputsWithCase[_T("pipeid" )];
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::VerifyInputs()
{
	if( NULL == m_pclProdSelParams || NULL == m_pclProdSelParams->m_pTADB || NULL == m_pclProdSelParams->m_pTADS )
	{
		ASSERTA_RETURN( -1 );
	}

	if( _NULL_IDPTR == m_pclProdSelParams->m_pTADB->Get( m_pclProdSelParams->m_WC.GetAdditFamID() ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDITFAMIDINVALID;
	}

	if( _NULL_IDPTR == m_pclProdSelParams->m_pTADB->Get( m_pclProdSelParams->m_WC.GetAdditID() ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDITIDINVALID;
	}

	double dTemp = m_pclProdSelParams->m_WC.GetTemp();
	CWaterChar::eFluidRetCode eFluidError = m_pclProdSelParams->m_WC.CheckFluidData( dTemp );

	if( CWaterChar::efrcFluidOk != eFluidError )
	{
		if( CWaterChar::efrcTemperatureTooLow == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERTEMPTOOLOW;
		}
		else if( CWaterChar::efrcTempTooHigh == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERTEMPTOOHIGH;
		}
		else if( CWaterChar::efrcAdditiveTooHigh == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDTOOHIGH;
		}
	}

	if( true == m_pclProdSelParams->m_strPipeSeriesID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_PIPESERIESIDEMPTY;
	}
	else if( _NULL_IDPTR == m_pclProdSelParams->m_pTADB->Get( m_pclProdSelParams->m_strPipeSeriesID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_PIPESERIESIDINVALID;
	}

	if( false == m_pclProdSelParams->m_strPipeID.IsEmpty() 
			&& _NULL_IDPTR == m_pclProdSelParams->m_pTADB->Get( m_pclProdSelParams->m_strPipeID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_PIPEIDINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::ReadOutputs( CTUProdSelFileHelper &clTUFileHelper, bool bUpdate )
{
	m_vecOutputValues.clear();

	CString strLine;
	TU_FILEHELPER_READLINE( clTUFileHelper, strLine );

	if( 0 != strLine.CompareNoCase( _T("StartOutputs") ) )
	{
		return TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BADSTART;
	}

	bool bStop = false;
	std::vector<CString> vecStrings;

	do
	{
		UINT uiErrorCode = clTUFileHelper.ReadOneLineMultiValues( strLine, vecStrings, _T(";") );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
		else if( 0 == strLine.CompareNoCase( _T("EndOutputs" ) ) )
		{
			bStop = true;
		}
		else
		{
			if( false == bUpdate )
			{
				if( GetOutputTitlesNbr( &vecStrings ) != vecStrings.size() )
				{
					return TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BADNBRINOUTPUT;
				}
			}

			m_vecOutputValues.push_back( vecStrings );
		}

	}while( false == bStop );

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	CString strString = pclProdSelParams->m_WC.GetAdditFamID();
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("WCAdditFamilyID = %s\n"), strString );

	strString = pclProdSelParams->m_WC.GetAdditID();
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("WCAdditID = %s\n"), strString );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("WCTemp = %.13g\n"), pclProdSelParams->m_WC.GetTemp() );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("WCPCWeight = %.13g\n"), pclProdSelParams->m_WC.GetPcWeight() );

	strString = pclProdSelParams->m_strPipeSeriesID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("PipeSeriesID = %s\n"), strString );

	strString = pclProdSelParams->m_strPipeID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("PipeID = %s\n"), strString );

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelHelper::WriteVecResultInFile( CTUProdSelFileHelper &clTUFileHelper )
{
	CString strLine = _T("");
	CString strSpace = _T(" ");
	CString strSep = _T(";");

	for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
	{
		strLine += ( ( false == m_vecResultData[iLoop].IsEmpty() ) ? m_vecResultData[iLoop] : strSpace );

		if( iLoop < (int)m_vecResultData.size() - 1 )
		{
			strLine += strSep;
		}
	}
	
	strLine += _T("\n");
	TU_FILEHELPER_WRITELINE( clTUFileHelper, strLine );

	return TU_PRODSELECT_ERROR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on individual selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSelHelper::CTUIndSelHelper( CIndividualSelectionParameters &clIndSelParams )
	: CTUProdSelHelper( clIndSelParams )
{
	m_pclIndSelParams = &clIndSelParams;
}

UINT CTUIndSelHelper::InterpreteInputs()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	UINT uiErrorCode = CTUProdSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_eFlowOrPowerDT' variable.
	// Remark: not mandatory. By default = 'flowmode'.
	if( 0 != m_mapInputs.count( _T("floworpowerdt" ) ) )
	{
		if( 0 == m_mapInputs[_T("floworpowerdt")].Compare( _T("flowmode") ) )
		{
			m_pclIndSelParams->m_eFlowOrPowerDTMode = CDS_SelProd::efdFlow;
		}
		else if( 0 == m_mapInputs[_T("floworpowerdt")].Compare( _T("powerdtmode") ) )
		{
			m_pclIndSelParams->m_eFlowOrPowerDTMode = CDS_SelProd::efdPower;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_FLOWORPOWERDTBAD;
		}
	}

	// The 'm_dFlow' variable.
	// Remark: mandatory if 'floworpowerdt' = 'flowmode'.
	if( 0 != m_mapInputs.count( _T("flow" ) ) )
	{
		m_pclIndSelParams->m_dFlow = _wtof( m_mapInputs[_T("flow")] );
	}
	
	// The 'm_dPowerMax' variable.
	// Remark: mandatory if 'floworpowerdt' = 'powerdtmode'.
	if( 0 != m_mapInputs.count( _T("power" ) ) )
	{
		m_pclIndSelParams->m_dPower = _wtof( m_mapInputs[_T("power")] );
	}

	// The 'm_dDT' variable.
	// Remark: mandatory if 'floworpowerdt' = 'powerdtmode'.
	if( 0 != m_mapInputs.count( _T("dt" ) ) )
	{
		m_pclIndSelParams->m_dDT = _wtof( m_mapInputs[_T("dt")] );
	}

	// The 'm_bDpEnabled' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("dpenabled" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpenabled")].Compare( _T("true") ) || 0 == m_mapInputs[_T("dpenabled")].Compare( _T("1") ) )
		{
			m_pclIndSelParams->m_bDpEnabled = true;
		}
		else if( 0 == m_mapInputs[_T("dpenabled")].Compare( _T("false") ) || 0 == m_mapInputs[_T("dpenabled")].Compare( _T("0") ) )
		{
			m_pclIndSelParams->m_bDpEnabled = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_DPENABLEBAD;
		}
	}

	// The 'm_dDp' variable.
	// Remark: mandatory if 'dpenabled' = 'true'.
	if( 0 != m_mapInputs.count( _T("dp" ) ) )
	{
		m_pclIndSelParams->m_dDp = _wtof( m_mapInputs[_T("dp")] );
	}

	// The 'm_strComboTypeID' variable.
	// Remark: mandatory for BV, PIBCV, DPCBCV, and CV not for other (By default "").
	if( 0 != m_mapInputs.count( _T("combotypeid") ) )
	{
		m_pclIndSelParams->m_strComboTypeID = m_mapInputsWithCase[_T("combotypeid")];
	}

	// The 'm_strComboFamilyID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("combofamilyid") ) )
	{
		m_pclIndSelParams->m_strComboFamilyID = m_mapInputsWithCase[_T("combofamilyid")];
	}

	// The 'm_strComboMaterialID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("combomaterialid") ) )
	{
		m_pclIndSelParams->m_strComboMaterialID = m_mapInputsWithCase[_T("combomaterialid")];
	}

	// The 'm_strComboConnectID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("comboconnectid") ) )
	{
		m_pclIndSelParams->m_strComboConnectID = m_mapInputsWithCase[_T("comboconnectid")];
	}

	// The 'm_strComboVersionID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("comboversionid") ) )
	{
		m_pclIndSelParams->m_strComboVersionID = m_mapInputsWithCase[_T("comboversionid")];
	}

	// The 'm_strComboPNID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("combopnid") ) )
	{
		m_pclIndSelParams->m_strComboPNID = m_mapInputsWithCase[_T("combopnid")];
	}

	// The 'm_bOnlyForSet' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("onlyforset" ) ) )
	{
		if( 0 == m_mapInputs[_T("onlyforset")].Compare( _T("true") ) || 0 == m_mapInputs[_T("onlyforset")].Compare( _T("1") ) )
		{
			m_pclIndSelParams->m_bOnlyForSet = true;
		}
		else if( 0 == m_mapInputs[_T("onlyforset")].Compare( _T("false") ) || 0 == m_mapInputs[_T("onlyforset")].Compare( _T("0") ) )
		{
			m_pclIndSelParams->m_bOnlyForSet = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_ONLYFORSETBAD;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelHelper::VerifyInputs()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	UINT uiErrorCode = CTUProdSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}
	
	// HYS-1375 : For TA-6-way valve don't check flow, power, dt yet.
	CIndSel6WayValveParams *pclParam6wayValve = dynamic_cast<CIndSel6WayValveParams *>( m_pclIndSelParams );

	if( NULL == pclParam6wayValve )
	{
		if( CDS_SelProd::efdFlow == m_pclIndSelParams->m_eFlowOrPowerDTMode && m_pclIndSelParams->m_dFlow <= 0.0 )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_FLOWNOTVALID;
		}
		else if( CDS_SelProd::efdPower == m_pclIndSelParams->m_eFlowOrPowerDTMode )
		{
			if( m_pclIndSelParams->m_dPower <= 0.0 )
			{
				return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_POWERNOTVALID;
			}
			else if( m_pclIndSelParams->m_dDT <= 0.0 )
			{
				return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_DTNOTVALID;
			}

			double dQ = CalcqFromPDT( m_pclIndSelParams->m_dPower, m_pclIndSelParams->m_dDT, m_pclIndSelParams->m_WC.GetDens(),
				m_pclIndSelParams->m_WC.GetSpecifHeat() );

			if( dQ <= 0.0 )
			{
				return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_CANTCOMPUTEFLOW;
			}

			m_pclIndSelParams->m_dFlow = dQ;
		}
	}

	if( true == m_pclIndSelParams->m_bDpEnabled && m_pclIndSelParams->m_dDp <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_DPNOTVALID;
	}

	if( false == m_pclIndSelParams->m_strComboTypeID.IsEmpty() 
			&& _NULL_IDPTR == m_pclIndSelParams->m_pTADB->Get( m_pclIndSelParams->m_strComboTypeID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_TYPEIDINVALID;
	}

	if( false == m_pclIndSelParams->m_strComboFamilyID.IsEmpty() 
			&& _NULL_IDPTR == m_pclIndSelParams->m_pTADB->Get( m_pclIndSelParams->m_strComboFamilyID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_FAMILYIDINVALID;
	}

	if( false == m_pclIndSelParams->m_strComboMaterialID.IsEmpty() 
			&& _NULL_IDPTR == m_pclIndSelParams->m_pTADB->Get( m_pclIndSelParams->m_strComboMaterialID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_MATERIALIDINVALID;
	}

	if( false == m_pclIndSelParams->m_strComboConnectID.IsEmpty() 
			&& _NULL_IDPTR == m_pclIndSelParams->m_pTADB->Get( m_pclIndSelParams->m_strComboConnectID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_CONNECTIDINVALID;
	}

	if( false == m_pclIndSelParams->m_strComboVersionID.IsEmpty() 
			&& _NULL_IDPTR == m_pclIndSelParams->m_pTADB->Get( m_pclIndSelParams->m_strComboVersionID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_VERSIONIDINVALID;
	}

	if( false == m_pclIndSelParams->m_strComboPNID.IsEmpty() 
			&& _NULL_IDPTR == m_pclIndSelParams->m_pTADB->Get( m_pclIndSelParams->m_strComboPNID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_PNIDINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	UINT uiErrorCode = CTUProdSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper);

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndividualSelectionParameters *pclIndSelParams = dynamic_cast<CIndividualSelectionParameters *>( pclProdSelParams );

	CString strString = ( CDS_SelProd::efdFlow == pclIndSelParams->m_eFlowOrPowerDTMode ) ? _T("FlowMode") : _T("PowerDTMode");
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("FlowOrPowerDT = %s\n"), strString );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("Flow = %.13g\n"), pclIndSelParams->m_dFlow );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("Power = %.13g\n"), pclIndSelParams->m_dPower );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("DT = %.13g\n"), pclIndSelParams->m_dDT );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpEnabled = %s\n"), ( false == pclIndSelParams->m_bDpEnabled ) ? _T("false") : _T("true") );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("Dp = %.13g\n"), pclIndSelParams->m_dDp );

	strString = pclIndSelParams->m_strComboTypeID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboTypeID = %s\n"), strString );

	strString = pclIndSelParams->m_strComboFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboFamilyID = %s\n"), strString );

	strString = pclIndSelParams->m_strComboMaterialID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboMaterialID = %s\n"), strString );

	strString = pclIndSelParams->m_strComboConnectID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboConnectID = %s\n"), strString );

	strString = pclIndSelParams->m_strComboVersionID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboVersionID = %s\n"), strString );

	strString = pclIndSelParams->m_strComboPNID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboPNID = %s\n"), strString );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("OnlyForSet = %s\n"), ( false == pclIndSelParams->m_bOnlyForSet ) ? _T("false") : _T("true") );

	return TU_PRODSELECT_ERROR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for regulating valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUIndSelBVHelper::CTUIndSelBVHelper() 
	: CTUIndSelHelper( m_clIndSelBVParams ) 
{
	m_clIndSelBVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	m_clIndSelBVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Valve computed setting") );
	m_vecOutputTitles.push_back( _T("Valve displayed setting") );
	m_vecOutputTitles.push_back( _T("Valve Dp") );
	m_vecOutputTitles.push_back( _T("Valve Kvsignal") );
	m_vecOutputTitles.push_back( _T("Valve Dp at full opening") );
	m_vecOutputTitles.push_back( _T("Valve Dp at half opening") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelBVHelper::~CTUIndSelBVHelper()
{
	if( NULL != m_clIndSelBVParams.m_pclSelectBvList )
	{
		delete m_clIndSelBVParams.m_pclSelectBvList;
		m_clIndSelBVParams.m_pclSelectBvList = NULL;
	}
}

UINT CTUIndSelBVHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_clIndSelBVParams.m_strComboTypeID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBV_TYPEIDMISSING;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelBVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_NOINPUT;
	}

	if( NULL != m_clIndSelBVParams.m_pclSelectBvList )
	{
		delete m_clIndSelBVParams.m_pclSelectBvList;
	}

	m_clIndSelBVParams.m_pclSelectBvList = new CSelectList();

	if( NULL == m_clIndSelBVParams.m_pclSelectBvList || NULL == m_clIndSelBVParams.m_pclSelectBvList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_CANTCREATESELECTLIST;
	}

	m_clIndSelBVParams.m_BvList.PurgeAll();

	m_clIndSelBVParams.m_pTADB->GetBVList( 
			&m_clIndSelBVParams.m_BvList,
			(LPCTSTR)m_clIndSelBVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelBVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelBVParams.m_strComboMaterialID, 
			(LPCTSTR)m_clIndSelBVParams.m_strComboConnectID, 
			(LPCTSTR)m_clIndSelBVParams.m_strComboVersionID,
			m_clIndSelBVParams.m_eFilterSelection );

	m_clIndSelBVParams.m_pclSelectBvList->GetSelectPipeList()->SelectPipes( &m_clIndSelBVParams, m_clIndSelBVParams.m_dFlow );

	int iDevFound = m_clIndSelBVParams.m_pclSelectBvList->GetManBvList( &m_clIndSelBVParams );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_RESULTNBRNOTSAME;
	}

	int iLoopResultInInputFile = 0;

	for( CSelectedValve *pclSelectedValve = m_clIndSelBVParams.m_pclSelectBvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_clIndSelBVParams.m_pclSelectBvList->GetNext<CSelectedValve>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelBVParams, pclSelectedValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelBVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = BV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CIndSelBVParams *pclIndSelBVParams = dynamic_cast<CIndSelBVParams *>( pclProdSelParams );

	if( NULL != pclIndSelBVParams->m_pclSelectBvList )
	{
		int iResultCount = 0;

		for( CSelectedValve *pclSelectedValve = pclIndSelBVParams->m_pclSelectBvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
				pclSelectedValve = pclIndSelBVParams->m_pclSelectBvList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSelBVParams, pclSelectedValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelBVHelper::_PrepareResultData( CIndSelBVParams *pclIndSelBVParams, CSelectedValve *pclSelectedValve, int iResultCount )
{
	m_vecResultData.clear();
	CDB_TAProduct *pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

	if( NULL == pclTAProduct ) 
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );
	
	// Valve ID.
	m_vecResultData.push_back( pclTAProduct->GetIDPtr().ID );
	
	// Valve name.
	m_vecResultData.push_back( pclTAProduct->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclTAProduct->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclTAProduct->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclTAProduct->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclTAProduct->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclTAProduct->GetSize() );
		
	// Valve computed setting.
	strValue.Format( _T("%g"), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_ValveCharacteristic *pclValveCharacteristic = pclTAProduct->GetValveCharacteristic();
	
	if( NULL != pclValveCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclValveCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclValveCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Pressure drop through the valve.
	strValue.Format( _T("%g"), pclSelectedValve->GetDp() );
	m_vecResultData.push_back( strValue );

	// Kv signal.
	strValue = _T("");

	if( true == pclTAProduct->IsKvSignalEquipped() )
	{
		strValue.Format( _T("%g"), pclSelectedValve->GetDpSignal() );
	}

	m_vecResultData.push_back( strValue );

	// Pressure drop when valve fully opened.
	strValue.Format( _T("%g"), pclSelectedValve->GetDpFullOpen() );
	m_vecResultData.push_back( strValue );

	// Pressure drop when valve is half opened.
	strValue = _T("");

	if( false == pclIndSelBVParams->m_bDpEnabled || -1.0 == pclSelectedValve->GetDp() )
	{
		strValue.Format( _T("%g"), pclSelectedValve->GetDpHalfOpen() );
	}

	m_vecResultData.push_back( strValue );

	// Temperature range of the valve.
	m_vecResultData.push_back( pclTAProduct->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelBVParams );
	pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList()->GetMatchingPipe( pclTAProduct->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on individual selection for for control valves, balancing and control valves, 
// pressure independent balancing and control valves and combined Dp controller, balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSelCtrlBaseHelper::CTUIndSelCtrlBaseHelper( CIndSelCtrlParamsBase &clIndSelCtrlBaseParams )
	: CTUIndSelHelper( clIndSelCtrlBaseParams )
{
	m_pclIndSelCtrlBaseParams = &clIndSelCtrlBaseParams;
}

UINT CTUIndSelCtrlBaseHelper::InterpreteInputs()
{
	if( NULL == m_pclIndSelCtrlBaseParams )
	{
		ASSERTA_RETURN( -1 );
	}

	UINT uiErrorCode = CTUIndSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_eCV2WW3' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("cv2w3w") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CV2W3WMISSING;
	}

	if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("cv2w") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::CV2W;
	}
	else if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("cv3w") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::CV3W;
	}
	else if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("cv4w") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::CV4W;
	}
	else if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("cv6w") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::CV6W;
	}
	else if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("unknown") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::LastCV2W3W;
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CV2W3WBAD;
	}

	// The 'm_eCvCtrlType' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("cvctrltype") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CVCTRLTYPEMISSING;
	}

	if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("no") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCvCtrlType = CDB_ControlProperties::eCvNU;
	}
	else if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("proportional") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCvCtrlType = CDB_ControlProperties::eCvProportional;
	}
	else if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("onoff") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCvCtrlType = CDB_ControlProperties::eCvOnOff;
	}
	else if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("3points") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCvCtrlType = CDB_ControlProperties::eCv3point;
	}
	else if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("unknown") ) )
	{
		m_pclIndSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::LastCV2W3W;
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CVCTRLTYPEBAD;
	}

	// The 'm_strActuatorPowerSupplyID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("comboactuatorpowersupplyid") ) )
	{
		m_pclIndSelCtrlBaseParams->m_strActuatorPowerSupplyID = m_mapInputsWithCase[_T("comboactuatorpowersupplyid")];
	}

	// The 'm_strActuatorInputSignalID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("comboactuatorinputsignalid") ) )
	{
		m_pclIndSelCtrlBaseParams->m_strActuatorInputSignalID = m_mapInputsWithCase[_T("comboactuatorinputsignalid")];
	}

	// The 'm_iActuatorFailSafeFunction' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("actuatorfailsafefunction") ) )
	{
		if( 0 == m_mapInputs[_T("actuatorfailsafefunction")].Compare( _T("no") ) )
		{
			m_pclIndSelCtrlBaseParams->m_iActuatorFailSafeFunction = 0;
		}
		else if( 0 == m_mapInputs[_T("actuatorfailsafefunction")].Compare( _T("yes") ) )
		{
			m_pclIndSelCtrlBaseParams->m_iActuatorFailSafeFunction = 1;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_ACTUATORFAILSAFEFCTBAD;
		}
	}

	// The 'm_eActuatorDRPFunction' variable for default return position.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("comboactuatordefaultreturnpos") ) )
	{
		if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos")].Compare( _T("no") ) )
		{
			m_pclIndSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfNone;
		}
		else if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos")].Compare( _T("closing") ) )
		{
			m_pclIndSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfClosing;
		}
		else if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos")].Compare( _T("opening") ) )
		{
			m_pclIndSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfOpening;
		}
		else if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos")].Compare( _T("configurable") ) )
		{
			m_pclIndSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfOpeningOrClosing;
		}
		else if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos")].Compare( _T("unknown") ) )
		{
			m_pclIndSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfAll;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_ACTUATORFAILSAFEFCTBAD;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelCtrlBaseHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( false == m_pclIndSelCtrlBaseParams->m_strActuatorPowerSupplyID.IsEmpty() 
			&& _NULL_IDPTR == m_pclIndSelCtrlBaseParams->m_pTADB->Get( m_pclIndSelCtrlBaseParams->m_strActuatorPowerSupplyID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELCTRLBASE_ACTPOWERSUPPLYIDINVALID;
	}

	if( false == m_pclIndSelCtrlBaseParams->m_strActuatorInputSignalID.IsEmpty() 
			&& _NULL_IDPTR == m_pclIndSelCtrlBaseParams->m_pTADB->Get( m_pclIndSelCtrlBaseParams->m_strActuatorInputSignalID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELCTRLBASE_ACTINPUTSIGNALIDINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelCtrlBaseHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	UINT uiErrorCode = CTUIndSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSelCtrlParamsBase *pclIndSelCtrlBaseParams = dynamic_cast<CIndSelCtrlParamsBase *>( pclProdSelParams );

	if( CDB_ControlProperties::CV2W == pclIndSelCtrlBaseParams->m_eCV2W3W )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = CV2w\n") );
	}
	else if( CDB_ControlProperties::CV3W == pclIndSelCtrlBaseParams->m_eCV2W3W )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = CV3w\n") );
	}
	else if( CDB_ControlProperties::CV4W == pclIndSelCtrlBaseParams->m_eCV2W3W )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = CV4w\n") );
	}
	else if( CDB_ControlProperties::CV6W == pclIndSelCtrlBaseParams->m_eCV2W3W )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = CV6w\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = Unknown\n") );
	}

	if( CDB_ControlProperties::eCvNU == pclIndSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = No\n") );
	}
	else if( CDB_ControlProperties::eCvProportional == pclIndSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = Proportional\n") );
	}
	else if( CDB_ControlProperties::eCvOnOff == pclIndSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = OnOff\n") );
	}
	else if( CDB_ControlProperties::eCv3point == pclIndSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = 3points\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = Unknown\n") );
	}

	CString strString = pclIndSelCtrlBaseParams->m_strActuatorPowerSupplyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboActuatorPowerSupplyID = %s\n"), strString );

	strString = pclIndSelCtrlBaseParams->m_strActuatorInputSignalID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboActuatorInputSignalID = %s\n"), strString );

	if( 0 == pclIndSelCtrlBaseParams->m_iActuatorFailSafeFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ActuatorFailSafeFunction = No\n") );
	}
	else if( 1 == pclIndSelCtrlBaseParams->m_iActuatorFailSafeFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ActuatorFailSafeFunction = Yes\n") );
	}

	if( CDB_ControlValve::DRPFunction::drpfNone == pclIndSelCtrlBaseParams->m_eActuatorDRPFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = No\n") );
	}
	else if( CDB_ControlValve::DRPFunction::drpfClosing == pclIndSelCtrlBaseParams->m_eActuatorDRPFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = Closing\n") );
	}
	else if( CDB_ControlValve::DRPFunction::drpfOpening == pclIndSelCtrlBaseParams->m_eActuatorDRPFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = Opening\n") );
	}
	else if( CDB_ControlValve::DRPFunction::drpfOpening == pclIndSelCtrlBaseParams->m_eActuatorDRPFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = Configurable\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = Unknown\n") );
	}
	
	return TU_PRODSELECT_ERROR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUIndSelBCVHelper::CTUIndSelBCVHelper() 
	: CTUIndSelCtrlBaseHelper( m_clIndSelBCVParams )
{
	m_clIndSelBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_BalAndCtrlValve;
	m_clIndSelBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Selection done for a set") );
	m_vecOutputTitles.push_back( _T("Is valve part of a set") );
	m_vecOutputTitles.push_back( _T("Valve rangeability") );
	m_vecOutputTitles.push_back( _T("Valve leakage rate") );
	m_vecOutputTitles.push_back( _T("Valve stroke length") );
	m_vecOutputTitles.push_back( _T("Valve control characteristic") );
	m_vecOutputTitles.push_back( _T("Valve push or pull to close") );
	m_vecOutputTitles.push_back( _T("Valve computed presetting") );
	m_vecOutputTitles.push_back( _T("Valve displayed presetting") );
	m_vecOutputTitles.push_back( _T("Valve Dp") );
	m_vecOutputTitles.push_back( _T("Valve Dp at full opening") );
	m_vecOutputTitles.push_back( _T("Valve Dp at half opening") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelBCVHelper::~CTUIndSelBCVHelper()
{
	if( NULL != m_clIndSelBCVParams.m_pclSelectBCVList )
	{
		delete m_clIndSelBCVParams.m_pclSelectBCVList;
		m_clIndSelBCVParams.m_pclSelectBCVList = NULL;
	}
}

UINT CTUIndSelBCVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_NOINPUT;
	}

	if( NULL != m_clIndSelBCVParams.m_pclSelectBCVList )
	{
		delete m_clIndSelBCVParams.m_pclSelectBCVList;
	}

	m_clIndSelBCVParams.m_pclSelectBCVList = new CSelectBCVList();

	if( NULL == m_clIndSelBCVParams.m_pclSelectBCVList || NULL == m_clIndSelBCVParams.m_pclSelectBCVList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_CANTCREATESELECTLIST;
	}

	m_clIndSelBCVParams.m_pclSelectBCVList->GetSelectPipeList()->SelectPipes( &m_clIndSelBCVParams, m_clIndSelBCVParams.m_dFlow );

	m_clIndSelBCVParams.m_CtrlList.PurgeAll();

	int iValveCount = m_clIndSelBCVParams.m_pTADB->GetTaCVList(	
			&m_clIndSelBCVParams.m_CtrlList,					// List where to saved
			CTADatabase::eForBCv,								// Control valve target (cv, hmcv, picv or bcv)
			false, 												// 'true' returns as soon a result is found
			m_clIndSelBCVParams.m_eCV2W3W, 						// Set way number of valve
			_T(""),							 					// Type ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID,	// Family ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, 	// Body material ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID,	// Connection ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID, 	// Version ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboPNID,		// PN ID
			CDB_ControlProperties::LastCVFUNC, 					// Set the control function (control only, presettable, ...)
			m_clIndSelBCVParams.m_eCvCtrlType,					// Set the control type (on/off, proportional, ...)
			m_clIndSelBCVParams.m_eFilterSelection,
			0,													// DNMin
			INT_MAX,											// DNMax
			false,												// 'true' if it's for hub station.
			NULL,												// 'pProd'.
			m_clIndSelBCVParams.m_bOnlyForSet );				// 'true' if it's only for a set.

	bool bSizeShiftProblem = false;
	int iDevFound = m_clIndSelBCVParams.m_pclSelectBCVList->SelectQ( &m_clIndSelBCVParams, &bSizeShiftProblem );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}
	
	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_RESULTNBRNOTSAME;
	}

	int iLoopResultInInputFile = 0;

	for( CSelectedValve *pclSelectedValve = m_clIndSelBCVParams.m_pclSelectBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_clIndSelBCVParams.m_pclSelectBCVList->GetNext<CSelectedValve>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelBCVParams, pclSelectedValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelBCVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = BCV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}
	
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CIndSelBCVParams *pclIndSelBCVParams = dynamic_cast<CIndSelBCVParams *>( pclProdSelParams );

	if( NULL != pclIndSelBCVParams->m_pclSelectBCVList )
	{
		int iResultCount = 0;

		for( CSelectedValve *pclSelectedValve = pclIndSelBCVParams->m_pclSelectBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
				pclSelectedValve = pclIndSelBCVParams->m_pclSelectBCVList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSelBCVParams, pclSelectedValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelBCVHelper::_PrepareResultData( CIndSelBCVParams *pclIndSelBCVParams, CSelectedValve *pclSelectedValve, int iResultCount )
{
	m_vecResultData.clear();
	CDB_ControlValve *pclBalancingAndControlValve = dynamic_cast<CDB_ControlValve *>( pclSelectedValve->GetpData() );

	if( NULL == pclBalancingAndControlValve ) 
	{
		return;
	}
		
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );
	
	// Valve ID.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetIDPtr().ID );
	
	// Valve name.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetSize() );

	// Selection done for set?
	strValue.Format( _T("%s"), ( true == pclIndSelBCVParams->m_bOnlyForSet ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Is part of a set?
	strValue.Format( _T("%s"), ( true == pclBalancingAndControlValve->IsPartOfaSet() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Rangeability.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetStrRangeability().c_str() );

	// Leakage rate.
	strValue.Format( _T("%g"), pclBalancingAndControlValve->GetLeakageRate() * 100.0 );
	m_vecResultData.push_back( strValue );

	// Stroke.
	strValue.Format( _T("%g"), pclBalancingAndControlValve->GetStroke() );
	m_vecResultData.push_back( strValue );

	// Control characteristic.
	if( CDB_ControlProperties::Linear == pclBalancingAndControlValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("Linear") );
	}
	else if( CDB_ControlProperties::EqualPc == pclBalancingAndControlValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("EqualPc") );
	}
	else
	{
		m_vecResultData.push_back( _T("NotCharacterized") );
	}

	// Push or pull to close.
	if( CDB_ControlProperties::PushToClose == pclBalancingAndControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PushToClose") );
	}
	else if( CDB_ControlProperties::PullToClose == pclBalancingAndControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PullToClose") );
	}
	else if( CDB_ControlProperties::Undef == pclBalancingAndControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("Undef") );
	}

	// Valve computed presetting.
	strValue.Format( _T("%g"), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_ValveCharacteristic *pclValveCharacteristic = pclBalancingAndControlValve->GetValveCharacteristic();
	
	if( NULL != pclValveCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclValveCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclValveCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Dp.
	strValue.Format( _T("%g"), pclSelectedValve->GetDp() );
	m_vecResultData.push_back( strValue );

	// Dp full opening.
	strValue = _T("");

	if( false == pclIndSelBCVParams->m_bDpEnabled || pclIndSelBCVParams->m_dDp <= 0.0 )
	{
		strValue.Format( _T("%g"), pclSelectedValve->GetDpFullOpen() );
	}

	m_vecResultData.push_back( strValue );

	// Dp half opening.
	strValue = _T("");

	if( false == pclIndSelBCVParams->m_bDpEnabled || pclIndSelBCVParams->m_dDp <= 0.0 )
	{
		strValue.Format( _T("%g"), pclSelectedValve->GetDpHalfOpen() );
	}

	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelBCVParams );
	pclIndSelBCVParams->m_pclSelectBCVList->GetSelectPipeList()->GetMatchingPipe( pclBalancingAndControlValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for pure control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSelCVHelper::CTUIndSelCVHelper()
	: CTUIndSelCtrlBaseHelper( m_clIndSelCVParams )
{
	m_clIndSelCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_ControlValve;
	m_clIndSelCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Selection done for a set") );
	m_vecOutputTitles.push_back( _T("Is valve part of a set") );
	m_vecOutputTitles.push_back( _T("Valve Kvs") );
	m_vecOutputTitles.push_back( _T("Valve Dp") );
	m_vecOutputTitles.push_back( _T("Valve rangeability") );
	m_vecOutputTitles.push_back( _T("Valve leakage rate") );
	m_vecOutputTitles.push_back( _T("Valve stroke length") );
	m_vecOutputTitles.push_back( _T("Valve control characteristic") );
	m_vecOutputTitles.push_back( _T("Valve push or pull to close") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelCVHelper::~CTUIndSelCVHelper()
{
	if( NULL != m_clIndSelCVParams.m_pclSelectCVList )
	{
		delete m_clIndSelCVParams.m_pclSelectCVList;
		m_clIndSelCVParams.m_pclSelectCVList = NULL;
	}
}

UINT CTUIndSelCVHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_bIsGroupKvsOrDpChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("groupkvsordpchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("groupkvsordpchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("groupkvsordpchecked")].Compare( _T("1") ) )
		{
			m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked = true;
		}
		else if( 0 == m_mapInputs[_T("groupkvsordpchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("groupkvsordpchecked")].Compare( _T("0") ) )
		{
			m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCV_GROUPKVSORDPBAD;
		}
	}

	// The 'm_eRadioKvsOrDp' variable.
	// Remark: mandatory if 'groupkvsordpchecked' = 'true'.
	if( 0 != m_mapInputs.count( _T("radiokvsordp" ) ) )
	{
		if( 0 == m_mapInputs[_T("radiokvsordp")].Compare( _T("kvs") ) )
		{
			m_clIndSelCVParams.m_eRadioKvsOrDp = CDS_SSelCv::KvsOrDp::Kvs;
		}
		else if( 0 == m_mapInputs[_T("radiokvsordp")].Compare( _T("dp") ) )
		{
			m_clIndSelCVParams.m_eRadioKvsOrDp = CDS_SSelCv::KvsOrDp::Dp;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCV_KVSORDPBAD;
		}
	}

	// The 'm_dKvs' variable.
	// Remark: mandatory if 'groupkvsordpchecked' = 'true' and 'radiokvsordp' = 'kvs'.
	if( 0 != m_mapInputs.count( _T("kvs" ) ) )
	{
		m_clIndSelCVParams.m_dKvs = _wtof( m_mapInputs[_T("kvs")] );
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelCVHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked && CDS_SSelCv::KvsOrDp::Kvs == m_clIndSelCVParams.m_eRadioKvsOrDp
			&& m_clIndSelCVParams.m_dKvs <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_KVSNOTVALID;
	}

	if( true == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked && CDS_SSelCv::KvsOrDp::Dp == m_clIndSelCVParams.m_eRadioKvsOrDp
			&& m_clIndSelCVParams.m_dDp <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_DPNOTVALID;
	}

	if( true == m_clIndSelCVParams.m_strComboTypeID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_TYPEIDMISSING;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelCVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_NOINPUT;
	}

	if( NULL != m_clIndSelCVParams.m_pclSelectCVList )
	{
		delete m_clIndSelCVParams.m_pclSelectCVList;
	}

	m_clIndSelCVParams.m_pclSelectCVList = new CSelectCVList();

	if( NULL == m_clIndSelCVParams.m_pclSelectCVList || NULL == m_clIndSelCVParams.m_pclSelectCVList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_CANTCREATESELECTLIST;
	}

	m_clIndSelCVParams.m_pclSelectCVList->GetSelectPipeList()->SelectPipes( &m_clIndSelCVParams, m_clIndSelCVParams.m_dFlow );
	
	m_clIndSelCVParams.m_CtrlList.PurgeAll();

	m_clIndSelCVParams.m_pTADB->GetTaCVList(	
			&m_clIndSelCVParams.m_CtrlList,						// List where to saved
			CTADatabase::eForSSelCv,							// Control valve target (cv, hmcv, picv or bcv)
			false,												// 'true' returns as soon a result is found
			m_clIndSelCVParams.m_eCV2W3W,						// Set way number of valve
			(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,		// Type ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID,		// Family ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, 	// Body material ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,	// Connection ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID, 	// Version ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboPNID,			// PN ID
			CDB_ControlProperties::LastCVFUNC,					// Set the control function (control only, presettable, ...)
			m_clIndSelCVParams.m_eCvCtrlType,					// Set the control type (on/off, proportional, ...)
			m_clIndSelCVParams.m_eFilterSelection,
			0,													// DNMin
			INT_MAX,											// DNMax
			false,												// 'true' if it's for hub station.
			NULL,												// 'pProd'.
			m_clIndSelCVParams.m_bOnlyForSet );					// 'true' if it's only for a set.

	bool bSizeShiftProblem = false;
	int iDevFound = 0;

	if( m_clIndSelCVParams.m_dKvs > 0.0 )
	{
		iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectCvKvs( &m_clIndSelCVParams, &bSizeShiftProblem );
	}
	else if( m_clIndSelCVParams.m_dDp > 0.0 )
	{
		iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectCvDp( &m_clIndSelCVParams, &bSizeShiftProblem );
	}
	else
	{
		iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectQ( &m_clIndSelCVParams, &bSizeShiftProblem );
	}

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_RESULTNBRNOTSAME;
	}

	int iLoopResultInInputFile = 0;

	for( CSelectedValve *pclSelectedValve = m_clIndSelCVParams.m_pclSelectCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_clIndSelCVParams.m_pclSelectCVList->GetNext<CSelectedValve>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelCVParams, pclSelectedValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelCVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = CV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSelCVParams *pclIndSelCVParams = dynamic_cast<CIndSelCVParams *>( pclProdSelParams );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("GroupKvsOrDpChecked = %s\n"), ( false == pclIndSelCVParams->m_bIsGroupKvsOrDpChecked ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("RadioKvsOrDp = %s\n"), ( CDS_SSelCv::KvsOrDp::Kvs == pclIndSelCVParams->m_eRadioKvsOrDp ) ? _T("Kvs") : _T("Dp") );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("Kvs = %.13g\n"), pclIndSelCVParams->m_dKvs );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );

	if( NULL != pclIndSelCVParams->m_pclSelectCVList )
	{
		int iResultCount = 0;

		for( CSelectedValve *pclSelectedValve = pclIndSelCVParams->m_pclSelectCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
				pclSelectedValve = pclIndSelCVParams->m_pclSelectCVList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSelCVParams, pclSelectedValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelCVHelper::_PrepareResultData( CIndSelCVParams *pclIndSelCVParams, CSelectedValve *pclSelectedValve, int iResultCount )
{
	m_vecResultData.clear();
	CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclSelectedValve->GetpData() );

	if( NULL == pclControlValve ) 
	{
		return;
	}
	
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclControlValve->GetIDPtr().ID );
	
	// Valve name.
	m_vecResultData.push_back( pclControlValve->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclControlValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclControlValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclControlValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclControlValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclControlValve->GetSize() );

	// Selection done for set?
	strValue.Format( _T("%s"), ( true == pclIndSelCVParams->m_bOnlyForSet ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Is part of a set?
	strValue.Format( _T("%s"), ( true == pclControlValve->IsPartOfaSet() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Kvs.
	strValue.Format( _T("%g"), pclControlValve->GetKvs() );
	m_vecResultData.push_back( strValue );

	// Dp.
	double dDp = CalcDp( pclIndSelCVParams->m_dFlow, pclControlValve->GetKvs(), pclIndSelCVParams->m_WC.GetDens() );
	strValue.Format( _T("%g"), dDp );
	m_vecResultData.push_back( strValue );

	// Rangeability.
	m_vecResultData.push_back( pclControlValve->GetStrRangeability().c_str() );

	// Leakage rate.
	strValue.Format( _T("%g"), pclControlValve->GetLeakageRate() * 100.0 );
	m_vecResultData.push_back( strValue );

	// Stroke.
	strValue.Format( _T("%.13g"), pclControlValve->GetStroke() );
	m_vecResultData.push_back( strValue );

	// Control characteristic.
	if( CDB_ControlProperties::Linear == pclControlValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("Linear") );
	}
	else if( CDB_ControlProperties::EqualPc == pclControlValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("EqualPc") );
	}
	else
	{
		m_vecResultData.push_back( _T("NotCharacterized") );
	}

	// Push or pull to close.
	if( CDB_ControlProperties::PushToClose == pclControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PushToClose") );
	}
	else if( CDB_ControlProperties::PullToClose == pclControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PullToClose") );
	}
	else if( CDB_ControlProperties::Undef == pclControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("Undef") );
	}

	// Temperature range.
	m_vecResultData.push_back( pclControlValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelCVParams );
	pclIndSelCVParams->m_pclSelectCVList->GetSelectPipeList()->GetMatchingPipe( pclControlValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for Dp controller valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSelDpCHelper::CTUIndSelDpCHelper()
	: CTUIndSelHelper( m_clIndSelDpCParams )
{
	m_clIndSelDpCParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_DpController;
	m_clIndSelDpCParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Selection done for a set") );
	m_vecOutputTitles.push_back( _T("Is valve part of a set") );
	m_vecOutputTitles.push_back( _T("Valve Dpmin") );
	m_vecOutputTitles.push_back( _T("Valve Dpl range") );
	m_vecOutputTitles.push_back( _T("Is Dpmax checkbox checked?") );
	m_vecOutputTitles.push_back( _T("Valve Dp max") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelDpCHelper::~CTUIndSelDpCHelper()
{
	if( NULL != m_clIndSelDpCParams.m_pclSelectDpCList )
	{
		delete m_clIndSelDpCParams.m_pclSelectDpCList;
		m_clIndSelDpCParams.m_pclSelectDpCList = NULL;
	}
}

UINT CTUIndSelDpCHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUIndSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_bIsGroupDpbranchOrKvChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("groupdpbranchorkvchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("groupdpbranchorkvchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("groupdpbranchorkvchecked")].Compare( _T("1") ) )
		{
			m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked = true;
		}
		else if( 0 == m_mapInputs[_T("groupdpbranchorkvchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("groupdpbranchorkvchecked")].Compare( _T("0") ) )
		{
			m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_GROUPDPBRANCHORKVBAD;
		}
	}

	// The 'm_bIsDpMaxChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("dpmaxchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("1") ) )
		{
			m_clIndSelDpCParams.m_bIsDpMaxChecked = true;
		}
		else if( 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("0") ) )
		{
			m_clIndSelDpCParams.m_bIsDpMaxChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_DPMAXCHECKEDBAD;
		}
	}

	// The 'm_dDpBranch' variable.
	// Remark: mandatory if 'groupdpbranchorkvchecked' = 'true' and 'dpstabilizedon' = 'branch'.
	if( 0 != m_mapInputs.count( _T("dpbranch" ) ) )
	{
		m_clIndSelDpCParams.m_dDpBranch = _wtof( m_mapInputs[_T("dpbranch")] );
	}

	// The 'm_dDpMax' variable.
	// Remark: mandatory if 'dpmaxchecked' = 'true'.
	if( 0 != m_mapInputs.count( _T("dpmax" ) ) )
	{
		m_clIndSelDpCParams.m_dDpMax = _wtof( m_mapInputs[_T("dpmax")] );
	}

	// The 'm_dKv' variable.
	// Remark: mandatory if 'groupdpbranchorkvchecked' = 'true' and 'dpstabilizedon' = 'controlvalve'.
	if( 0 != m_mapInputs.count( _T("kv" ) ) )
	{
		m_clIndSelDpCParams.m_dKv = _wtof( m_mapInputs[_T("kv")] );
	}

	// The 'm_eDpStab' variable.
	// Remark: not mandatory. By default = 'branch'.
	if( 0 != m_mapInputs.count( _T("dpstabilizedon" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpstabilizedon")].Compare( _T("branch") ) )
		{
			m_clIndSelDpCParams.m_eDpStab = eDpStab::DpStabOnBranch;
		}
		else if( 0 == m_mapInputs[_T("dpstabilizedon")].Compare( _T("controlvalve") ) )
		{
			m_clIndSelDpCParams.m_eDpStab = eDpStab::DpStabOnCV;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_DPSTABBAD;
		}
	}

	// The 'm_eDpCLoc' variable.
	// Remark: not mandatory. By default = 'downstream'.
	if( 0 != m_mapInputs.count( _T("dpclocalization" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpclocalization")].Compare( _T("downstream") ) )
		{
			m_clIndSelDpCParams.m_eDpCLoc = eDpCLoc::DpCLocDownStream;
		}
		else if( 0 == m_mapInputs[_T("dpclocalization")].Compare( _T("upstream") ) )
		{
			m_clIndSelDpCParams.m_eDpCLoc = eDpCLoc::DpCLocUpStream;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_DPCLOCBAD;
		}
	}

	// The 'm_eMvLoc' variable.
	// Remark: not mandatory. By default = 'primary'.
	if( 0 != m_mapInputs.count( _T("mvlocalization" ) ) )
	{
		if( 0 == m_mapInputs[_T("mvlocalization")].Compare( _T("primary") ) )
		{
			m_clIndSelDpCParams.m_eMvLoc = eMvLoc::MvLocPrimary;
		}
		else if( 0 == m_mapInputs[_T("mvlocalization")].Compare( _T("secondary") ) )
		{
			m_clIndSelDpCParams.m_eMvLoc = eMvLoc::MvLocSecondary;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_MVLOCBAD;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelDpCHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked && eDpStab::DpStabOnBranch == m_clIndSelDpCParams.m_eDpStab
			&& m_clIndSelDpCParams.m_dDpBranch <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_DPBRANCHINVALID;
	}

	if( true == m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked && eDpStab::DpStabOnCV == m_clIndSelDpCParams.m_eDpStab
			&& m_clIndSelDpCParams.m_dKv <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_KVINVALID;
	}

	if( true == m_clIndSelDpCParams.m_bIsDpMaxChecked && m_clIndSelDpCParams.m_dDpMax <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_DPMAXINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelDpCHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_NOINPUT;
	}

	if( NULL != m_clIndSelDpCParams.m_pclSelectDpCList )
	{
		delete m_clIndSelDpCParams.m_pclSelectDpCList;
	}

	m_clIndSelDpCParams.m_pclSelectDpCList = new CSelectDpCList();

	if( NULL == m_clIndSelDpCParams.m_pclSelectDpCList || NULL == m_clIndSelDpCParams.m_pclSelectDpCList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_CANTCREATESELECTLIST;
	}

	m_clIndSelDpCParams.m_pclSelectDpCList->GetSelectPipeList()->SelectPipes( &m_clIndSelDpCParams, m_clIndSelDpCParams.m_dFlow );

	m_clIndSelDpCParams.m_DpCList.PurgeAll();

	m_clIndSelDpCParams.m_pTADB->GetDpCList( 
			&m_clIndSelDpCParams.m_DpCList,
			m_clIndSelDpCParams.m_eDpCLoc,
			(LPCTSTR)m_clIndSelDpCParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelDpCParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelDpCParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelDpCParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelDpCParams.m_strComboVersionID,
			(LPCTSTR)m_clIndSelDpCParams.m_strComboPNID,
			m_clIndSelDpCParams.m_eFilterSelection );

	bool bValidFound = false;

	int iDevFound = m_clIndSelDpCParams.m_pclSelectDpCList->SelectDpC( &m_clIndSelDpCParams, bValidFound );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_RESULTNBRNOTSAME;
	}

	int iLoopResultInInputFile = 0;

	for( CSelectedValve *pclSelectedValve = m_clIndSelDpCParams.m_pclSelectDpCList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_clIndSelDpCParams.m_pclSelectDpCList->GetNext<CSelectedValve>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelDpCParams, pclSelectedValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelDpCHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = DpC\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSelDpCParams *pclIndSelDpCParams = dynamic_cast<CIndSelDpCParams *>( pclProdSelParams );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("GroupDpBranchOrKvChecked = %s\n"), ( false == pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpMaxChecked = %s\n"), ( false == pclIndSelDpCParams->m_bIsDpMaxChecked ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("DpBranch = %.13g\n"), pclIndSelDpCParams->m_dDpBranch );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("DpMax = %.13g\n"), pclIndSelDpCParams->m_dDpMax );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("Kv = %.13g\n"), pclIndSelDpCParams->m_dKv );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpStabilizedOn = %s\n"), ( eDpStab::DpStabOnBranch == pclIndSelDpCParams->m_eDpStab ) ? _T("Branch") : _T("ControlValve") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpCLocalization = %s\n"), ( eDpCLoc::DpCLocDownStream == pclIndSelDpCParams->m_eDpCLoc ) ? _T("DownStream") : _T("UpStream") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("MvLocalization = %s\n"), ( eMvLoc::MvLocPrimary == pclIndSelDpCParams->m_eMvLoc ) ? _T("Primary") : _T("Secondary") );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	if( NULL != pclIndSelDpCParams->m_pclSelectDpCList )
	{
		int iResultCount = 0;

		for( CSelectedValve *pclSelectedValve = pclIndSelDpCParams->m_pclSelectDpCList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
				pclSelectedValve = pclIndSelDpCParams->m_pclSelectDpCList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSelDpCParams, pclSelectedValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelDpCHelper::_PrepareResultData( CIndSelDpCParams *pclIndSelDpCParams, CSelectedValve *pclSelectedValve, int iResultCount )
{
	m_vecResultData.clear();
	CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclSelectedValve->GetpData() );

	if( NULL == pclDpController ) 
	{
		return;
	}
	
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclDpController->GetIDPtr().ID );
	
	// Valve name.
	m_vecResultData.push_back( pclDpController->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclDpController->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclDpController->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclDpController->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclDpController->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclDpController->GetSize() );

	// Selection done for set?
	strValue.Format( _T("%s"), ( true == pclIndSelDpCParams->m_bOnlyForSet ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Is part of a set?
	strValue.Format( _T("%s"), ( true == pclDpController->IsPartOfaSet() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Dp min.
	strValue.Format( _T("%g"), pclSelectedValve->GetDpMin() );
	m_vecResultData.push_back( strValue );

	// Dpl range.
	strValue.Format( _T("[%g-%g]"), pclDpController->GetDplmin(), pclDpController->GetDplmax() );
	m_vecResultData.push_back( strValue );

	// Is Dp max checkbox checked.
	strValue.Format( _T("%s"), ( true == pclIndSelDpCParams->m_bIsDpMaxChecked ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Dp max.
	strValue.Format( _T("%g"), pclDpController->GetDpmax() );
	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclDpController->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelDpCParams );
	pclIndSelDpCParams->m_pclSelectDpCList->GetSelectPipeList()->GetMatchingPipe( pclDpController->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for combined Dp controller, balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSelDpCBCVHelper::CTUIndSelDpCBCVHelper()
	:CTUIndSelCtrlBaseHelper( m_clIndSelDpCBCVParams )
{
	m_clIndSelDpCBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve;
	m_clIndSelDpCBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Selection done for a set") );
	m_vecOutputTitles.push_back( _T("Is valve part of a set") );
	m_vecOutputTitles.push_back( _T("Valve computed presetting") );	
	m_vecOutputTitles.push_back( _T("Valve displayed presetting") );	
	m_vecOutputTitles.push_back( _T("Valve rangeability") );
	m_vecOutputTitles.push_back( _T("Valve leakage rate") );
	m_vecOutputTitles.push_back( _T("Valve Dpmin") );
	m_vecOutputTitles.push_back( _T("Valve Dpl range") );
	m_vecOutputTitles.push_back( _T("Valve stroke length") );
	m_vecOutputTitles.push_back( _T("Valve control characteristic") );
	m_vecOutputTitles.push_back( _T("Valve push or pull to close") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelDpCBCVHelper::~CTUIndSelDpCBCVHelper()
{
	if( NULL != m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList )
	{
		delete m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList;
		m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList = NULL;
	}
}

UINT CTUIndSelDpCBCVHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_bIsGroupDpToStabilizeChecked' variable.
	// HY-957: doesn't exist anymore. Force for the moment to 'true'.
	m_clIndSelDpCBCVParams.m_bIsGroupDpToStabilizeChecked = true;

	// The 'm_dDpToStabilize' variable.
	// Remark: mandatory if 'groupdptostabilizechecked' = 'true'.
	if( 0 != m_mapInputs.count( _T("dptostabilize" ) ) )
	{
		m_clIndSelDpCBCVParams.m_dDpToStabilize = _wtof( m_mapInputs[_T("dptostabilize")] );
	}

	// The 'm_bIsWithSTSChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("withstschecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("withstschecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("withstschecked")].Compare( _T("1") ) )
		{
			m_clIndSelDpCBCVParams.m_bIsWithSTSChecked = true;
		}
		else if( 0 == m_mapInputs[_T("withstschecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("withstschecked")].Compare( _T("0") ) )
		{
			m_clIndSelDpCBCVParams.m_bIsWithSTSChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPCBCV_WITHSTSBAD;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelDpCBCVHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_clIndSelDpCBCVParams.m_bIsGroupDpToStabilizeChecked && m_clIndSelDpCBCVParams.m_dDpToStabilize <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPCBCV_DPLINVALID;
	}

	if( true == m_clIndSelDpCBCVParams.m_strComboTypeID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPCBCV_TYPEIDMISSING;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelDpCBCVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_NOINPUT;
	}

	if( NULL != m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList )
	{
		delete m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList;
	}

	m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList = new CSelectDpCBCVList();

	if( NULL == m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList || NULL == m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_CANTCREATESELECTLIST;
	}

	m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->GetSelectPipeList()->SelectPipes( &m_clIndSelDpCBCVParams, m_clIndSelDpCBCVParams.m_dFlow );

	m_clIndSelDpCBCVParams.m_CtrlList.PurgeAll();

	m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(
			&m_clIndSelDpCBCVParams.m_CtrlList,
			CTADatabase::eForDpCBCV,
			false,
			m_clIndSelDpCBCVParams.m_eCV2W3W,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboVersionID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboPNID,
			CDB_ControlProperties::LastCVFUNC,
			m_clIndSelDpCBCVParams.m_eCvCtrlType,
			m_clIndSelDpCBCVParams.m_eFilterSelection,
			0,
			INT_MAX,
			false,
			NULL,
			m_clIndSelDpCBCVParams.m_bOnlyForSet );

	bool bSizeShiftProblem = false;
	bool bValidFound = false;
	int iDevFound = m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->SelectDpCBCV( &m_clIndSelDpCBCVParams, &bValidFound, &bSizeShiftProblem );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_RESULTNBRNOTSAME;
	}

	int iLoopResultInInputFile = 0;

	for( CSelectedValve *pclSelectedValve = m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->GetNext<CSelectedValve>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelDpCBCVParams, pclSelectedValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelDpCBCVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = DpCBCV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSelDpCBCVParams *pclIndSelDpCBCVParams = dynamic_cast<CIndSelDpCBCVParams *>( pclProdSelParams );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("DpToStabilize = %.13g\n"), pclIndSelDpCBCVParams->m_dDpToStabilize );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("WithSTSChecked = %s\n"), ( false == pclIndSelDpCBCVParams->m_bIsWithSTSChecked ) ? _T("false") : _T("true") );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	if( NULL != pclIndSelDpCBCVParams->m_pclSelectDpCBCVList )
	{
		int iResultCount = 0;

		for( CSelectedValve *pclSelectedValve = pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
				pclSelectedValve = pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSelDpCBCVParams, pclSelectedValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelDpCBCVHelper::_PrepareResultData( CIndSelDpCBCVParams *pclIndSelDpCBCVParams, CSelectedValve *pclSelectedValve,
		int iResultCount )
{
	m_vecResultData.clear();
	CDB_DpCBCValve *pclCombinedDpCAndBCValve = dynamic_cast<CDB_DpCBCValve *>( pclSelectedValve->GetpData() );

	if( NULL == pclCombinedDpCAndBCValve ) 
	{
		return;
	}
	
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetIDPtr().ID );
	
	// Valve name.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetSize() );

	// Selection done for set?
	strValue.Format( _T("%s"), ( true == pclIndSelDpCBCVParams->m_bOnlyForSet ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Is part of a set?
	strValue.Format( _T("%s"), ( true == pclCombinedDpCAndBCValve->IsPartOfaSet() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Valve presetting.
	strValue.Format( _T("%g"), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_DpCBCVCharacteristic *pclDpCBCVCharacteristic = pclCombinedDpCAndBCValve->GetDpCBCVCharacteristic();
	
	if( NULL != pclDpCBCVCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclDpCBCVCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclDpCBCVCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Rangeability.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetStrRangeability().c_str() );

	// Leakage rate.
	strValue.Format( _T("%g"), pclCombinedDpCAndBCValve->GetLeakageRate() * 100.0 );
	m_vecResultData.push_back( strValue );

	// Dp minimum.
	// Get pressure drop over the pressure part of the valve.
	double dDpp = pclCombinedDpCAndBCValve->GetDppmin( pclIndSelDpCBCVParams->m_dFlow, pclIndSelDpCBCVParams->m_WC.GetDens() );
		
	// Get pressure drop over the control part of the valve.
	double dDpc = pclCombinedDpCAndBCValve->GetDpc( pclIndSelDpCBCVParams->m_dFlow, pclIndSelDpCBCVParams->m_dDpToStabilize );

	double dDpMin = dDpp + dDpc;
	strValue.Format( _T("%g"), dDpMin );
	m_vecResultData.push_back( strValue );

	// Dpl range.
	strValue.Format( _T("[%g-%g]"), pclCombinedDpCAndBCValve->GetDplmin(), pclCombinedDpCAndBCValve->GetDplmax( pclIndSelDpCBCVParams->m_dFlow, pclIndSelDpCBCVParams->m_WC.GetDens() ) );
	m_vecResultData.push_back( strValue );

	// Stroke.
	strValue.Format( _T("%g"), pclCombinedDpCAndBCValve->GetStroke() );
	m_vecResultData.push_back( strValue );

	// Control characteristic.
	if( CDB_ControlProperties::Linear == pclCombinedDpCAndBCValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("Linear") );
	}
	else if( CDB_ControlProperties::EqualPc == pclCombinedDpCAndBCValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("EqualPc") );
	}
	else
	{
		m_vecResultData.push_back( _T("NotCharacterized") );
	}

	// Push or pull to close.
	if( CDB_ControlProperties::PushToClose == pclCombinedDpCAndBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PushToClose") );
	}
	else if( CDB_ControlProperties::PullToClose == pclCombinedDpCAndBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PullToClose") );
	}
	else if( CDB_ControlProperties::Undef == pclCombinedDpCAndBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("Undef") );
	}

	// Temperature range.
	m_vecResultData.push_back( pclCombinedDpCAndBCValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelDpCBCVParams );
	pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetSelectPipeList()->GetMatchingPipe( pclCombinedDpCAndBCValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for pressure independent balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSelPIBCVHelper::CTUIndSelPIBCVHelper()
	: CTUIndSelCtrlBaseHelper( m_clIndSelPIBCVParams )
{
	m_clIndSelPIBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_PressureIndepCtrlValve;
	m_clIndSelPIBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Selection done for a set") );
	m_vecOutputTitles.push_back( _T("Is valve part of a set") );
	m_vecOutputTitles.push_back( _T("Valve computed presetting") );	
	m_vecOutputTitles.push_back( _T("Valve displayed presetting") );	
	m_vecOutputTitles.push_back( _T("Valve rangeability") );
	m_vecOutputTitles.push_back( _T("Valve leakage rate") );
	m_vecOutputTitles.push_back( _T("Valve Dpmin") );
	m_vecOutputTitles.push_back( _T("Valve stroke length") );
	m_vecOutputTitles.push_back( _T("Valve control characteristic") );
	m_vecOutputTitles.push_back( _T("Valve push or pull to close") );
	m_vecOutputTitles.push_back( _T("Is Dpmax checkbox checked?") );
	m_vecOutputTitles.push_back( _T("Valve Dp max") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelPIBCVHelper::~CTUIndSelPIBCVHelper()
{
	if( NULL != m_clIndSelPIBCVParams.m_pclSelectPIBCVList )
	{
		delete m_clIndSelPIBCVParams.m_pclSelectPIBCVList;
		m_clIndSelPIBCVParams.m_pclSelectPIBCVList = NULL;
	}
}

UINT CTUIndSelPIBCVHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_bIsDpMaxChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("dpmaxchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("1") ) )
		{
			m_clIndSelPIBCVParams.m_bIsDpMaxChecked = true;
		}
		else if( 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("0") ) )
		{
			m_clIndSelPIBCVParams.m_bIsDpMaxChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPIBCV_DPMAXCHECKEDBAD;
		}
	}

	// The 'm_dDpMax' variable.
	// Remark: mandatory if 'dpmaxchecked' = 'true'.
	if( 0 != m_mapInputs.count( _T("dpmax" ) ) )
	{
		m_clIndSelPIBCVParams.m_dDpMax = _wtof( m_mapInputs[_T("dpmax")] );
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelPIBCVHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_clIndSelPIBCVParams.m_bIsDpMaxChecked && m_clIndSelPIBCVParams.m_dDpMax <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_DPMAXINVALID;
	}

	if( true == m_clIndSelPIBCVParams.m_strComboTypeID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_TYPEIDMISSING;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelPIBCVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_NOINPUT;
	}

	if( NULL != m_clIndSelPIBCVParams.m_pclSelectPIBCVList )
	{
		delete m_clIndSelPIBCVParams.m_pclSelectPIBCVList;
	}

	m_clIndSelPIBCVParams.m_pclSelectPIBCVList = new CSelectPICVList();

	if( NULL == m_clIndSelPIBCVParams.m_pclSelectPIBCVList || NULL == m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_CANTCREATESELECTLIST;
	}

	m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetSelectPipeList()->SelectPipes( &m_clIndSelPIBCVParams, m_clIndSelPIBCVParams.m_dFlow );

	m_clIndSelPIBCVParams.m_CtrlList.PurgeAll();

	m_clIndSelPIBCVParams.m_pTADB->GetTaCVList(
			&m_clIndSelPIBCVParams.m_CtrlList,							// List where to saved
			CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			m_clIndSelPIBCVParams.m_eCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboVersionID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboPNID,
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelPIBCVParams.m_eFilterSelection,
			0,															// DNMin
			INT_MAX,													// DNMax
			false,														// 'true' if it's for hub station.
			NULL,														// 'pProd'.
			m_clIndSelPIBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.
										 
	bool bSizeShiftProblem = false;
	int iDevFound = m_clIndSelPIBCVParams.m_pclSelectPIBCVList->SelectQ( &m_clIndSelPIBCVParams, &bSizeShiftProblem );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_RESULTNBRNOTSAME;
	}

	int iLoopResultInInputFile = 0;

	for( CSelectedValve *pclSelectedValve = m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetNext<CSelectedValve>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelPIBCVParams, pclSelectedValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelPIBCVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = PIBCV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSelPIBCVParams *pclIndSelPIBCVParams = dynamic_cast<CIndSelPIBCVParams *>( pclProdSelParams );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpMaxChecked = %s\n"), ( false == pclIndSelPIBCVParams->m_bIsDpMaxChecked ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("DpMax = %.13g\n"), pclIndSelPIBCVParams->m_dDpMax );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );

	if( NULL != pclIndSelPIBCVParams->m_pclSelectPIBCVList )
	{
		int iResultCount = 0;

		for( CSelectedValve *pclSelectedValve = pclIndSelPIBCVParams->m_pclSelectPIBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
				pclSelectedValve = pclIndSelPIBCVParams->m_pclSelectPIBCVList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSelPIBCVParams, pclSelectedValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelPIBCVHelper::_PrepareResultData( CIndSelPIBCVParams *pclIndSelPIBCVParams, CSelectedValve *pclSelectedValve,
		int iResultCount )
{
	m_vecResultData.clear();
	CDB_PIControlValve *pclPIBCValve = dynamic_cast<CDB_PIControlValve *>( pclSelectedValve->GetpData() );

	if( NULL == pclPIBCValve ) 
	{
		return;
	}
	
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclPIBCValve->GetIDPtr().ID );
	
	// Valve name.
	m_vecResultData.push_back( pclPIBCValve->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclPIBCValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclPIBCValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclPIBCValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclPIBCValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclPIBCValve->GetSize() );

	// Selection done for set?
	strValue.Format( _T("%s"), ( true == pclIndSelPIBCVParams->m_bOnlyForSet ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Is part of a set?
	strValue.Format( _T("%s"), ( true == pclPIBCValve->IsPartOfaSet() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Valve computed presetting.
	strValue.Format( _T("%g"), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_PICVCharacteristic *pclPIBCVCharacteristic = pclPIBCValve->GetPICVCharacteristic();
	
	if( NULL != pclPIBCVCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclPIBCVCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclPIBCVCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Rangeability.
	m_vecResultData.push_back( pclPIBCValve->GetStrRangeability().c_str() );

	// Leakage rate.
	strValue.Format( _T("%g"), pclPIBCValve->GetLeakageRate() * 100.0 );
	m_vecResultData.push_back( strValue );

	// DP min.
	strValue.Format( _T("%g"), pclSelectedValve->GetDpMin() );
	m_vecResultData.push_back( strValue );

	// Stroke.
	strValue.Format( _T("%g"), pclPIBCValve->GetStroke() );
	m_vecResultData.push_back( strValue );

	// Control characteristic.
	if( CDB_ControlProperties::Linear == pclPIBCValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("Linear") );
	}
	else if( CDB_ControlProperties::EqualPc == pclPIBCValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("EqualPc") );
	}
	else
	{
		m_vecResultData.push_back( _T("NotCharacterized") );
	}

	// Push or pull to close.
	if( CDB_ControlProperties::PushToClose == pclPIBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PushToClose") );
	}
	else if( CDB_ControlProperties::PullToClose == pclPIBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PullToClose") );
	}
	else if( CDB_ControlProperties::Undef == pclPIBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("Undef") );
	}

	// Is Dp max checkbox checked.
	strValue.Format( _T("%s"), ( true == pclIndSelPIBCVParams->m_bIsDpMaxChecked ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Dp max.
	strValue.Format( _T("%g"), pclPIBCValve->GetDpmax() );
	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclPIBCValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelPIBCVParams );
	pclIndSelPIBCVParams->m_pclSelectPIBCVList->GetSelectPipeList()->GetMatchingPipe( pclPIBCValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for pressure maintenance.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSelPMHelper::CTUIndSelPMHelper()
	: CTUProdSelHelper( m_clIndSelPMParams )
{
	m_clIndSelPMParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_ExpansionVessel;
	m_clIndSelPMParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_mapLayoutName[CDB_Product::eiUndefined] = _T("Undefined");
	m_mapLayoutName[CDB_Product::eilWallHanged] = _T("Wall hanged");
	m_mapLayoutName[CDB_Product::eilVesselTop] = _T("Vessel top");
	m_mapLayoutName[CDB_Product::eilFloorStanding] = _T("Floor standing");
	m_mapLayoutName[CDB_Product::eilFloorStandingNextTo] = _T("Floor standing next to");
	m_mapLayoutName[CDB_Product::eilFloorStandingInFront] = _T("Floor standing in front");
	m_mapLayoutName[CDB_Product::eilIntegrated] = _T("Integrated");

	m_vecOutputComputedDataTitles.push_back( _T("Result number") );
	m_vecOutputComputedDataTitles.push_back( _T("Line type") );
	m_vecOutputComputedDataTitles.push_back( _T("Expansion factor") );
	m_vecOutputComputedDataTitles.push_back( _T("Expansion volume") );
	m_vecOutputComputedDataTitles.push_back( _T("Water reserve") );
	m_vecOutputComputedDataTitles.push_back( _T("Contraction volume for cooling") );
	m_vecOutputComputedDataTitles.push_back( _T("Vapor pressure") );
	m_vecOutputComputedDataTitles.push_back( _T("Pz") );
	m_vecOutputComputedDataTitles.push_back( _T("Minimum pressure") );
	m_vecOutputComputedDataTitles.push_back( _T("Vessel min. PSV") );
	m_vecOutputComputedDataTitles.push_back( _T("Compresso min. PSV") );
	m_vecOutputComputedDataTitles.push_back( _T("Transfero min. PSV") );
	m_vecOutputComputedDataTitles.push_back( _T("Compresso && Transfero initial pressure") );
	m_vecOutputComputedDataTitles.push_back( _T("Compresso target pressure") );
	m_vecOutputComputedDataTitles.push_back( _T("Transfero target pressure") );
	m_vecOutputComputedDataTitles.push_back( _T("Vessel final pressure") );
	m_vecOutputComputedDataTitles.push_back( _T("Compresso final pressure") );
	m_vecOutputComputedDataTitles.push_back( _T("Transfero final pressure") );
	m_vecOutputComputedDataTitles.push_back( _T("Vessel pressure factor") );
	m_vecOutputComputedDataTitles.push_back( _T("Compresso & Transfero pressure factor") );
	m_vecOutputComputedDataTitles.push_back( _T("Vessel nominal volume") );
	m_vecOutputComputedDataTitles.push_back( _T("Compresso & Transfero nominal volume") );
	m_vecOutputComputedDataTitles.push_back( _T("Compresso & Transfero needed flow rate equalization volume") );

	m_vecOutputExpansionVesselTitles.push_back( _T("Result number") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Line type") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Is price index used?") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Price index") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel ID") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel name") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel quantity") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel nominal volume") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel max. pressure") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel temperature range") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel factory preset pressure") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel diameter") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel height") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel weight") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel max. weight") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel standing") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Vessel connection") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Required preset pressure") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Optimized initial pressure") );
	m_vecOutputExpansionVesselTitles.push_back( _T("Optimized water reserve") );

	m_vecOutputCompressoTitles.push_back( _T("Result number") );
	m_vecOutputCompressoTitles.push_back( _T("Line type") );
	m_vecOutputCompressoTitles.push_back( _T("Is price index used?") );
	m_vecOutputCompressoTitles.push_back( _T("Price index") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso ID") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso name") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso redundancy") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso max. pressure") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso temperature range") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso water make-up") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso max. temp. for water make-up") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso width") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso height") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso depth") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso weight") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso standing") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso power") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso supply voltage") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso sound pressure level") );
	m_vecOutputCompressoTitles.push_back( _T("Compresso integrated vessel ID") );

	m_vecOutputTransferoTitles.push_back( _T("Result number") );
	m_vecOutputTransferoTitles.push_back( _T("Line type") );
	m_vecOutputTransferoTitles.push_back( _T("Is price index used?") );
	m_vecOutputTransferoTitles.push_back( _T("Price index") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero ID") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero name") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero redundancy") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero max. pressure") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero temperature range") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero water make-up") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero max. temp. for water make-up") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero width") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero height") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero depth") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero weight") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero standing") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero power") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero supply voltage") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero sound pressure level") );
	m_vecOutputTransferoTitles.push_back( _T("Transfero integrated vessel ID") );

	m_vecOutputVentoTitles.push_back( _T("Result number") );
	m_vecOutputVentoTitles.push_back( _T("Line type") );
	m_vecOutputVentoTitles.push_back( _T("Is price index used?") );
	m_vecOutputVentoTitles.push_back( _T("Price index") );
	m_vecOutputVentoTitles.push_back( _T("Vento ID") );
	m_vecOutputVentoTitles.push_back( _T("Vento name") );
	m_vecOutputVentoTitles.push_back( _T("Vento quantity") );
	m_vecOutputVentoTitles.push_back( _T("Vento redundancy") );
	m_vecOutputVentoTitles.push_back( _T("Vento max. pressure") );
	m_vecOutputVentoTitles.push_back( _T("Vento temperature range") );
	m_vecOutputVentoTitles.push_back( _T("Vento water make-up") );
	m_vecOutputVentoTitles.push_back( _T("Vento max. temp. for water make-up") );
	m_vecOutputVentoTitles.push_back( _T("Vento width") );
	m_vecOutputVentoTitles.push_back( _T("Vento height") );
	m_vecOutputVentoTitles.push_back( _T("Vento depth") );
	m_vecOutputVentoTitles.push_back( _T("Vento weight") );
	m_vecOutputVentoTitles.push_back( _T("Vento standing") );
	m_vecOutputVentoTitles.push_back( _T("Vento electrical power") );
	m_vecOutputVentoTitles.push_back( _T("Vento supply voltage") );
	m_vecOutputVentoTitles.push_back( _T("Vento system volume") );
	m_vecOutputVentoTitles.push_back( _T("Vento sound pressure level") );
	m_vecOutputVentoTitles.push_back( _T("Vento working pressure range") );
	m_vecOutputVentoTitles.push_back( _T("Vento flow water make-up") );
	m_vecOutputVentoTitles.push_back( _T("Vento IP") );

	m_vecOutputPlenoTitles.push_back( _T("Result number") );
	m_vecOutputPlenoTitles.push_back( _T("Line type") );
	m_vecOutputPlenoTitles.push_back( _T("Is price index used?") );
	m_vecOutputPlenoTitles.push_back( _T("Price index") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno ID") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno name") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno redundancy") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno max. pressure") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno temperature range") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno width") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno height") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno depth") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno weight") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno standing") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno electrical power") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno supply voltage") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno Kvs") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno sound pressure level") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno working pressure range") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno flow water make-up") );
	m_vecOutputPlenoTitles.push_back( _T("Pleno IP") );
}

UINT CTUIndSelPMHelper::SetpTADB( CTADatabase *pTADB )
{
	CTUProdSelHelper::SetpTADB( pTADB );
	m_clPMInputUser.SetpTADB( pTADB );
	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelPMHelper::SetpTADS( CTADatastruct *pTADS )
{
	CTUProdSelHelper::SetpTADS( pTADS );
	m_clPMInputUser.SetpTADS( pTADS );
	return TU_PRODSELECT_ERROR_OK;
}

CTUIndSelPMHelper::~CTUIndSelPMHelper()
{
	if( NULL != m_clIndSelPMParams.m_pclSelectPMList )
	{
		delete m_clIndSelPMParams.m_pclSelectPMList;
		m_clIndSelPMParams.m_pclSelectPMList = NULL;
	}
}

UINT CTUIndSelPMHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUProdSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	m_clPMInputUser.SetWC( m_clIndSelPMParams.m_WC );

	// The 'CPMInputUser::m_eApplicationType' variable.
	// Remark: not mandatory. By default = 'heating'.
	if( 0 != m_mapInputs.count( _T("applicationtype" ) ) )
	{
		if( 0 == m_mapInputs[_T("applicationtype")].Compare( _T("heating") ) )
		{
			m_clPMInputUser.SetApplicationType( ProjectType::Heating );
		}
		else if( 0 == m_mapInputs[_T("applicationtype")].Compare( _T("cooling") ) )
		{
			m_clPMInputUser.SetApplicationType( ProjectType::Cooling );
		}
		else if( 0 == m_mapInputs[_T("applicationtype")].Compare( _T("solar") ) )
		{
			m_clPMInputUser.SetApplicationType( ProjectType::Solar );
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_APPTYPEBAD;
		}
	}

	// The 'CPMInputUser::m_strPressureMaintenanceTypeID' variable.
	// Remark: not mandatory. By default = 'PMT_ALL_TAB'.
	if( 0 != m_mapInputs.count( _T("pmtypeid") ) )
	{
		m_clPMInputUser.SetPressureMaintenanceTypeID( m_mapInputsWithCase[_T("pmtypeid")] );
	}

	// The 'CPMInputUser::m_strWaterMakeUpTypeID' variable.
	// Remark: not mandatory. By default = 'WMUP_TYPE_NONE'.
	if( 0 != m_mapInputs.count( _T("watermakeuptypeid") ) )
	{
		m_clPMInputUser.SetWaterMakeUpTypeID( m_mapInputsWithCase[_T("watermakeuptypeid")] );
	}

	// The 'CPMInputUser::m_iDegassingChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("degassingchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("degassingchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("degassingchecked")].Compare( _T("1") ) )
		{
			m_clPMInputUser.SetDegassingChecked( BST_CHECKED );
		}
		else if( 0 == m_mapInputs[_T("degassingchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("degassingchecked")].Compare( _T("0") ) )
		{
			m_clPMInputUser.SetDegassingChecked( BST_UNCHECKED );
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_DEGASSINGCHECKEDBAD;
		}
	}

	// The 'CPMInputUser::m_strNormID' variable.
	// Remark: not mandatory. By default = 'PM_NORM_EN12828'.
	if( 0 != m_mapInputs.count( _T("normid") ) )
	{
		m_clPMInputUser.SetNormID( m_mapInputsWithCase[_T("normid")] );
	}

	// The 'CPMInputUser::m_dSystemVolume' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("systemvolume" ) ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_SYSVOLMISSING;
	}

	m_clPMInputUser.SetSystemVolume( _wtof( m_mapInputs[_T("systemvolume")] ) );

	// The 'CPMInputUser::m_dSolarCollectorVolume' variable.
	// Remark: mandatory if 'ApplicationType' = 'Solar'.
	if( 0 != m_mapInputs.count( _T("solarcontent" ) ) )
	{
		m_clPMInputUser.SetSolarCollectorVolume( _wtof( m_mapInputs[_T("solarcontent")] ) );
	}

	// The 'CPMInputUser::m_dInstalledPower' variable.
	// Remark: not mandatory. By default = '0.0'.
	if( 0 != m_mapInputs.count( _T("installedpower" ) ) )
	{
		m_clPMInputUser.SetInstalledPower( _wtof( m_mapInputs[_T("installedpower")] ) );
	}

	// The 'CPMInputUser::m_dStaticHeight' variable.
	// Remark: not mandatory. By default = '0.0'.
	if( 0 != m_mapInputs.count( _T("staticheight" ) ) )
	{
		m_clPMInputUser.SetStaticHeight( _wtof( m_mapInputs[_T("staticheight")] ) );
	}

	// The 'CPMInputUser::m_dPzChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("pzchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("pzchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("pzchecked")].Compare( _T("1") ) )
		{
			m_clPMInputUser.SetPzChecked( BST_CHECKED );
		}
		else if( 0 == m_mapInputs[_T("pzchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("pzchecked")].Compare( _T("0") ) )
		{
			m_clPMInputUser.SetPzChecked( BST_UNCHECKED );
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_DEGASSINGCHECKEDBAD;
		}

	}
	// The 'CPMInputUser::m_dPz' variable.
	// Remark: mandatory if 'pzchecked' = 'true'.
	if( 0 != m_mapInputs.count( _T("pz" ) ) )
	{
		m_clPMInputUser.SetPz( _wtof( m_mapInputs[_T("pz")] ) );
	}

	// The 'CPMInputUser::m_dSafetyPressValve' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("psvs" ) ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_PSVSMISSING;
	}

	m_clPMInputUser.SetSafetyValveResponsePressure( _wtof( m_mapInputs[_T("psvs")] ) );

	// The 'CPMInputUser::m_dSafetyTempLimiter' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("safetytemplimiter" ) ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_SFTYTEMPLIMITERMISSING;
	}

	m_clPMInputUser.SetSafetyTempLimiter( _wtof( m_mapInputs[_T("safetytemplimiter")] ) );

	// The 'CPMInputUser::m_dMaxTemperature' variable.
	// Remark: mandatory if 'ApplicationType' = 'Cooling'.
	if( 0 != m_mapInputs.count( _T("maxtemperature" ) ) )
	{
		m_clPMInputUser.SetMaxTemperature( _wtof( m_mapInputs[_T("maxtemperature")] ) );
	}

	// The 'CPMInputUser::m_dSupplyTemperature' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("supplytemperature" ) ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_SUPPLYTEMPMISSING;
	}

	m_clPMInputUser.SetSupplyTemperature( _wtof( m_mapInputs[_T("supplytemperature")] ) );

	// The 'CPMInputUser::m_dReturnTemperature' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("returntemperature" ) ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_RETURNTEMPMISSING;
	}

	m_clPMInputUser.SetReturnTemperature( _wtof( m_mapInputs[_T("returntemperature")] ) );

	// The 'CPMInputUser::m_dMinTemperature' variable.
	// Remark: mandatory if 'ApplicationType' = 'Heating' or 'Solar'.
	if( 0 != m_mapInputs.count( _T("mintemperature" ) ) )
	{
		m_clPMInputUser.SetMinTemperature( _wtof( m_mapInputs[_T("mintemperature")] ) );
	}

	// The 'CPMInputUser::m_fCheckMinTemperature' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("mintemperaturechecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("mintemperaturechecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("mintemperaturechecked")].Compare( _T("1") ) )
		{
			m_clPMInputUser.SetCheckMinTemperature( true );
		}
		else if( 0 == m_mapInputs[_T("mintemperaturechecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("mintemperaturechecked")].Compare( _T("0") ) )
		{
			m_clPMInputUser.SetCheckMinTemperature( false );
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_MINTEMPCHECKEDBAD;
		}
	}

	// The 'CPMInputUser::m_dFillTemperature' variable.
	// Remark: mandatory if 'ApplicationType' = 'Cooling'.
	if( 0 != m_mapInputs.count( _T("filltemperature" ) ) )
	{
		m_clPMInputUser.SetFillTemperature( _wtof( m_mapInputs[_T("filltemperature")] ) );
	}

	// The 'CPMInputUser::m_ePressOn' variable.
	// Remark: not mandatory. By default = 'PumpSuction'.
	if( 0 != m_mapInputs.count( _T("pressurisationon" ) ) )
	{
		if( 0 == m_mapInputs[_T("pressurisationon")].Compare( _T("pumpsuction") ) )
		{
			m_clPMInputUser.SetPressOn( PressurON::poPumpSuction );
		}
		else if( 0 == m_mapInputs[_T("pressurisationon")].Compare( _T("pumpdischarge") ) )
		{
			m_clPMInputUser.SetPressOn( PressurON::poPumpDischarge );
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_PRESSONBAD;
		}
	}

	// The 'CPMInputUser::m_dPumpHead' variable.
	// Remark: mandatory if 'PressurisationOn' = 'PumpDischarge'.
	if( 0 != m_mapInputs.count( _T("pumphead" ) ) )
	{
		m_clPMInputUser.SetPumpHead( _wtof( m_mapInputs[_T("pumphead")] ) );
	}

	// The 'CPMInputUser::m_dMaxWidth' variable.
	// Remark: not mandatory. By default '0.0'.
	if( 0 != m_mapInputs.count( _T("maxwidth" ) ) )
	{
		m_clPMInputUser.SetMaxWidth( _wtof( m_mapInputs[_T("maxwidth")] ) );
	}

	// The 'CPMInputUser::m_dMaxHeight' variable.
	// Remark: not mandatory. By default '0.0'.
	if( 0 != m_mapInputs.count( _T("maxheight" ) ) )
	{
		m_clPMInputUser.SetMaxHeight( _wtof( m_mapInputs[_T("maxheight")] ) );
	}

	// The 'CPMInputUser::m_dDegassingMaxTempConnectPoint' variable.
	// Remark: mandatory if 'PMTypeID' = 'PMT_NONE_TAB'.
	if( 0 != m_mapInputs.count( _T("degassingmaxtempconnectionpoint" ) ) )
	{
		m_clPMInputUser.SetDegassingMaxTempConnectPoint( _wtof( m_mapInputs[_T("degassingmaxtempconnectionpoint")] ) );
	}

	// The 'CPMInputUser::m_dDegassingPressureConnectPoint' variable.
	// Remark: mandatory if 'PMTypeID' = 'PMT_NONE_TAB'.
	if( 0 != m_mapInputs.count( _T("degassingpressureconnectionpoint" ) ) )
	{
		m_clPMInputUser.SetDegassingPressureConnectPoint( _wtof( m_mapInputs[_T("degassingpressureconnectionpoint")] ) );
	}

	// The 'CPMInputUser::m_dWaterMakeUpNetworkPN' variable.
	// Remark: mandatory if 'WaterMakeUpTypeID' != 'WMUP_TYPE_NONE'.
	if( 0 != m_mapInputs.count( _T("watermakeupnetworkpn" ) ) )
	{
		m_clPMInputUser.SetWaterMakeUpNetworkPN( _wtof( m_mapInputs[_T("watermakeupnetworkpn")] ) );
	}

	// The 'CPMInputUser::m_dWaterMakeUpWaterTemp' variable.
	// Remark: mandatory if 'WaterMakeUpTypeID' != 'WMUP_TYPE_NONE'.
	if( 0 != m_mapInputs.count( _T("watermakeupwatertemp" ) ) )
	{
		m_clPMInputUser.SetWaterMakeUpWaterTemp( _wtof( m_mapInputs[_T("watermakeupwatertemp")] ) );
	}

	// The 'CPMInputUser::m_dWaterMakeUpHardness' variable.
	// Remark: mandatory if 'WaterMakeUpTypeID' != 'WMUP_TYPE_NONE'.
	if( 0 != m_mapInputs.count( _T("watermakeupwaterhardness" ) ) )
	{
		m_clPMInputUser.SetWaterMakeUpWaterHardness( _wtof( m_mapInputs[_T("watermakeupwaterhardness")] ) );
	}

	// The 'CPMInputUser::CPMWQPrefs' variable.
	CString arstrPMWQPrefs[] = { _T("generalcombinedinonedevice"), _T("generalredundancypumpcomp"), _T("generalredundancytecbox"), 
			_T("compressointernalcoating"), _T("compressoexternalair"), _T("pumpdegassingcoolingversion"), _T("watermakeupbreaktank"), _T("watermakeupdutystandby") };

	for( int iLoop = CPMWQPrefs::PMWQP_First; iLoop < CPMWQPrefs::PMWQP_Last; iLoop++ )
	{
		if( 0 != m_mapInputs.count( arstrPMWQPrefs[iLoop] ) )
		{
			if( 0 == m_mapInputs[arstrPMWQPrefs[iLoop]].Compare( _T("true") ) || 0 == m_mapInputs[arstrPMWQPrefs[iLoop]].Compare( _T("1") ) )
			{
				m_clPMInputUser.GetPMWQSelectionPreferences()->SetChecked( (CPMWQPrefs::PMWQPreference)iLoop, true );
			}
			else if( 0 == m_mapInputs[arstrPMWQPrefs[iLoop]].Compare( _T("false") ) || 0 == m_mapInputs[arstrPMWQPrefs[iLoop]].Compare( _T("0") ) )
			{
				m_clPMInputUser.GetPMWQSelectionPreferences()->SetChecked( (CPMWQPrefs::PMWQPreference)iLoop, false );
			}
			else
			{
				return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_GENCOMBINONEDEVICEBAD + iLoop;
			}
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelPMHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUProdSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( m_clPMInputUser.GetSystemVolume() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_SYSVOLINVALID;
	}

	if( ProjectType::Solar == m_clPMInputUser.GetApplicationType() && m_clPMInputUser.GetSolarCollectorVolume() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_SOLCONTINVALID;
	}

	if( BST_CHECKED == m_clPMInputUser.GetPzChecked() && m_clPMInputUser.GetPz() < 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_PZINVALID;
	}

	if( ProjectType::Cooling == m_clPMInputUser.GetApplicationType() && m_clPMInputUser.GetMaxTemperature() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_MAXTEMPINVALID;
	}

	if( true == m_clPMInputUser.GetCheckMinTemperature() 
			&& ( ProjectType::Heating == m_clPMInputUser.GetApplicationType() || ProjectType::Solar == m_clPMInputUser.GetApplicationType() )
			&& m_clPMInputUser.GetMinTemperature() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_MINTEMPINVALID;
	}

	if(  m_clPMInputUser.GetFillTemperature() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_FILLTEMPINVALID;
	}

	if( PressurON::poPumpDischarge == m_clPMInputUser.GetPressOn() && m_clPMInputUser.GetPumpHead() < 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_PUMPHEADINVALID;
	}

	if( 0 == m_clPMInputUser.GetPressureMaintenanceTypeID().CompareNoCase( _T("pmt_none_tab") )
			&& m_clPMInputUser.GetDegassingMaxTempConnectPoint() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_DEGMAXTEMPATCONPTINVALID;
	}

	if( 0 == m_clPMInputUser.GetPressureMaintenanceTypeID().CompareNoCase( _T("pmt_none_tab") )
			&& m_clPMInputUser.GetDegassingPressureConnectPoint() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_DEGPRESSCONPTINVALID;
	}

	if( 0 != m_clPMInputUser.GetWaterMakeUpTypeID().CompareNoCase( _T("wmup_type_none") )
			&& m_clPMInputUser.GetWaterMakeUpNetworkPN() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_WMUPNETPNINVALID;
	}

	if( 0 != m_clPMInputUser.GetWaterMakeUpTypeID().CompareNoCase( _T("wmup_type_none") )
			&& m_clPMInputUser.GetWaterMakeUpWaterTemp() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_WMUPTEMPINVALID;
	}

	if( 0 != m_clPMInputUser.GetWaterMakeUpTypeID().CompareNoCase( _T("wmup_type_none") )
			&& m_clPMInputUser.GetWaterMakeUpWaterHardness() <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_WMUPHARDINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelPMHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_NOINPUT;
	}

	if( NULL != m_clIndSelPMParams.m_pclSelectPMList )
	{
		delete m_clIndSelPMParams.m_pclSelectPMList;
	}

	m_clIndSelPMParams.m_pclSelectPMList = new CSelectPMList();

	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_CANTCREATESELECTLIST;
	}

	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->CopyFrom( &m_clPMInputUser );

	// Remark: The 'm_pTADB' and 'm_pTADS' variables in the 'm_pclSelectPMList' classes has not yet been initialized.
	m_clIndSelPMParams.m_pclSelectPMList->SetpTADB( m_clIndSelPMParams.m_pTADB );
	m_clIndSelPMParams.m_pclSelectPMList->SetpTADS( m_clIndSelPMParams.m_pTADS );

	m_clIndSelPMParams.m_pclSelectPMList->Select();

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	CPMInputUser *pclInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();

	bool bIsPMTypeAll = false;

	if( CPMInputUser::MT_All == pclInputUser->GetPressureMaintenanceType() )
	{
		bIsPMTypeAll = true;
	}

	int iLoopResultInInputFile = 0;

	if( true == pclInputUser->IsPressurisationSystemExist() )
	{
		// Compare computed data.
		_PrepareComputedDataResultData( &m_clIndSelPMParams, iLoopResultInInputFile );
		
		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputComputedDataTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;

		// Compare expansion vessel if exist.
		if( true == bIsPMTypeAll 
				|| CPMInputUser::MT_ExpansionVessel == pclInputUser->GetPressureMaintenanceType() )
		{
			for( CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_ExpansionVessel );
					NULL != pclSelectedProduct; pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_ExpansionVessel ) )
			{
				CSelectedVssl *pclSelectedExpansionVessel = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

				if( NULL == pclSelectedExpansionVessel )
				{
					continue;
				}
			
				_PrepareExpansionVesselResultData( &m_clIndSelPMParams, pclSelectedExpansionVessel, iLoopResultInInputFile );
				
				// Now just compare between results and input file.
				for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
				{
					if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
					{
						strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
								m_vecOutputExpansionVesselTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

						return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_RESULTDIFF;
					}
				}

				iLoopResultInInputFile++;
			}
		}

		// Compare expansion vessel with membrane if exist.
		if( true == bIsPMTypeAll 
				|| CPMInputUser::MT_ExpansionVesselWithMembrane == pclInputUser->GetPressureMaintenanceType() )
		{
			for( CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_ExpansionVesselMembrane ); 
					NULL != pclSelectedProduct; pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_ExpansionVesselMembrane ) )
			{
				CSelectedVssl *pclSelectedExpansionVessel = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

				if( NULL == pclSelectedExpansionVessel )
				{
					continue;
				}
			
				_PrepareExpansionVesselResultData( &m_clIndSelPMParams, pclSelectedExpansionVessel, iLoopResultInInputFile );
				
				// Now just compare between results and input file.
				for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
				{
					if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
					{
						strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
								m_vecOutputExpansionVesselTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

						return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_RESULTDIFF;
					}
				}

				iLoopResultInInputFile++;
			}
		}

		// Compare Compresso if exist.
		if( true == bIsPMTypeAll 
				|| CPMInputUser::MT_WithCompressor == pclInputUser->GetPressureMaintenanceType() )
		{
			for( CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Compresso ); 
					NULL != pclSelectedProduct; pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Compresso ) )
			{
				CSelectedCompresso *pclSelectedTecBoxCompresso = dynamic_cast<CSelectedCompresso *>( pclSelectedProduct );

				if( NULL == pclSelectedTecBoxCompresso )
				{
					continue;
				}
			
				_PrepareCompressoResultData( &m_clIndSelPMParams, pclSelectedTecBoxCompresso, iLoopResultInInputFile );
				
				// Now just compare between results and input file.
				for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
				{
					if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
					{
						strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
								m_vecOutputCompressoTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

						return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_RESULTDIFF;
					}
				}

				iLoopResultInInputFile++;
			}
		}

		// Compare Transfero if exist.
		if( true == bIsPMTypeAll 
				|| CPMInputUser::MT_WithPump == pclInputUser->GetPressureMaintenanceType() )
		{
			for( CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Transfero ); 
					NULL != pclSelectedProduct; pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Transfero ) )
			{
				CSelectedTransfero *pclSelectedTecBoxTransfero = dynamic_cast<CSelectedTransfero *>( pclSelectedProduct );

				if( NULL == pclSelectedTecBoxTransfero )
				{
					continue;
				}
			
				_PrepareTransferoResultData( &m_clIndSelPMParams, pclSelectedTecBoxTransfero, iLoopResultInInputFile );
				
				// Now just compare between results and input file.
				for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
				{
					if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
					{
						strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
								m_vecOutputTransferoTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

						return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_RESULTDIFF;
					}
				}

				iLoopResultInInputFile++;
			}
		}
	}
	else
	{
		for( CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Vento );
				NULL != pclSelectedProduct; pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Vento ) )
		{
			CSelectedVento *pclSelectedTBVento = dynamic_cast<CSelectedVento *>( pclSelectedProduct );

			if( NULL == pclSelectedTBVento )
			{
				continue;
			}

			_PrepareVentoResultData( &m_clIndSelPMParams, pclSelectedTBVento, iLoopResultInInputFile );
			
			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
							m_vecOutputVentoTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

					return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_RESULTDIFF;
				}
			}

			iLoopResultInInputFile++;
		}

		for( CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Pleno );
				NULL != pclSelectedProduct; pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Pleno ) )
		{
			_PreparePlenoResultData( &m_clIndSelPMParams, pclSelectedProduct, iLoopResultInInputFile );
			
			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
							m_vecOutputPlenoTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

					return TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_RESULTDIFF;
				}
			}

			iLoopResultInInputFile++;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelPMHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = PM\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUProdSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSelPMParams *pclIndSelPMParams = dynamic_cast<CIndSelPMParams *>( pclProdSelParams );
	CPMInputUser *pclInputUser = pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	
	if( ProjectType::Heating == pclInputUser->GetApplicationType() )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ApplicationType = Heating\n") );
	}
	else if( ProjectType::Cooling == pclInputUser->GetApplicationType() )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ApplicationType = Cooling\n") );
	}
	else if( ProjectType::Solar == pclInputUser->GetApplicationType() )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ApplicationType = Solar\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ApplicationType = Unknown\n") );
	}

	CString strString = pclInputUser->GetPressureMaintenanceTypeID();
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("PMTypeID = %s\n"), strString.MakeUpper() );

	strString = pclInputUser->GetWaterMakeUpTypeID();
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("WaterMakeUpTypeID = %s\n"), strString.MakeUpper() );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DegassingChecked = %s\n"), ( BST_UNCHECKED == pclInputUser->GetDegassingChecked() ) ? _T("false") : _T("true") );

	strString = pclInputUser->GetNormID();
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("NormID = %s\n"), strString.MakeUpper() );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("SystemVolume = %.13g\n"), pclInputUser->GetSystemVolume() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("SolarContent = %.13g\n"), pclInputUser->GetSolarCollectorVolume() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("InstalledPower = %.13g\n"), pclInputUser->GetInstalledPower() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("StaticHeight = %.13g\n"), pclInputUser->GetStaticHeight() );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("PzChecked = %s\n"), ( BST_UNCHECKED == pclInputUser->GetPzChecked() ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("Pz = %.13g\n"), pclInputUser->GetPz() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("PSVS = %.13g\n"), pclInputUser->GetSafetyValveResponsePressure() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("SafetyTempLimiter = %.13g\n"), pclInputUser->GetSafetyTempLimiter() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("MaxTemperature = %.13g\n"), pclInputUser->GetMaxTemperature() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("SupplyTemperature = %.13g\n"), pclInputUser->GetSupplyTemperature() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("ReturnTemperature = %.13g\n"), pclInputUser->GetReturnTemperature() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("MinTemperature = %.13g\n"), pclInputUser->GetMinTemperature() );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("MinTemperatureChecked = %s\n"), ( false == pclInputUser->GetCheckMinTemperature() ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("FillTemperature = %.13g\n"), pclInputUser->GetFillTemperature() );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("PressurisationOn = %s\n"), ( PressurON::poPumpSuction == pclInputUser->GetPressOn() ) ? _T("PumpSuction") : _T("PumpDischarge") );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("PumpHead = %.13g\n"), pclInputUser->GetPumpHead() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("MaxWidth = %.13g\n"), pclInputUser->GetMaxWidth() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("MaxHeight = %.13g\n"), pclInputUser->GetMaxHeight() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("DegassingMaxTempConnectionPoint = %.13g\n"), pclInputUser->GetDegassingMaxTempConnectPoint() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("DegassingPressureConnectionPoint = %.13g\n"), pclInputUser->GetDegassingPressureConnectPoint() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("WaterMakeUpNetworkPN = %.13g\n"), pclInputUser->GetWaterMakeUpNetworkPN() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("WaterMakeUpWaterTemp = %.13g\n"), pclInputUser->GetWaterMakeUpWaterTemp() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("WaterMakeUpWaterHardness = %.13g\n"), pclInputUser->GetWaterMakeUpWaterHardness() );

	CPMWQPrefs *pPMWQPrefs = pclInputUser->GetPMWQSelectionPreferences();
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("GeneralCombinedInOneDevice = %s\n"), ( false == pPMWQPrefs->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("GeneralRedundancyPumpComp = %s\n"), ( false == pPMWQPrefs->IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("GeneralRedundancyTecBox = %s\n"), ( false == pPMWQPrefs->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("CompressoInternalCoating = %s\n"), ( false == pPMWQPrefs->IsChecked( CPMWQPrefs::CompressoInternalCoating ) ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("CompressoExternalAir = %s\n"), ( false == pPMWQPrefs->IsChecked( CPMWQPrefs::CompressoExternalAir ) ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("PumpDegassingCoolingVersion = %s\n"), ( false == pPMWQPrefs->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion ) ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("WaterMakeUpBreakTank = %s\n"), ( false == pPMWQPrefs->IsChecked( CPMWQPrefs::WaterMakeupBreakTank ) ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("WaterMakeUpDutyStandBy = %s\n"), ( false == pPMWQPrefs->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy ) ) ? _T("false") : _T("true") );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	if( NULL != pclIndSelPMParams->m_pclSelectPMList )
	{
		bool bIsPMTypeAll = false;

		if( CPMInputUser::MT_All == pclInputUser->GetPressureMaintenanceType() )
		{
			bIsPMTypeAll = true;
		}

		if( true == pclInputUser->IsPressurisationSystemExist() )
		{
			int iResultCount = 0;
			_PrepareComputedDataResultData( pclIndSelPMParams, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}

			if( true == bIsPMTypeAll 
					|| CPMInputUser::MT_ExpansionVessel == pclInputUser->GetPressureMaintenanceType() )
			{
				for( CSelectedPMBase *pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_ExpansionVessel ); NULL != pclSelectedProduct; 
						pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_ExpansionVessel ) )
				{
					CSelectedVssl *pclSelectedExpansionVessel = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

					if( NULL == pclSelectedExpansionVessel )
					{
						continue;
					}
			
					_PrepareExpansionVesselResultData( pclIndSelPMParams, pclSelectedExpansionVessel, iResultCount++ );
					WriteVecResultInFile( clTUFileHelper );
				}
			}

			if( true == bIsPMTypeAll 
					|| CPMInputUser::MT_ExpansionVesselWithMembrane == pclInputUser->GetPressureMaintenanceType() )
			{
				for( CSelectedPMBase *pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_ExpansionVesselMembrane ); 
						NULL != pclSelectedProduct; pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_ExpansionVesselMembrane ) )
				{
					CSelectedVssl *pclSelectedExpansionVessel = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

					if( NULL == pclSelectedExpansionVessel )
					{
						continue;
					}
			
					_PrepareExpansionVesselResultData( pclIndSelPMParams, pclSelectedExpansionVessel, iResultCount++ );
					WriteVecResultInFile( clTUFileHelper );
				}
			}

			if( true == bIsPMTypeAll 
					|| CPMInputUser::MT_WithCompressor == pclInputUser->GetPressureMaintenanceType() )
			{
				for( CSelectedPMBase *pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Compresso ); 
						NULL != pclSelectedProduct; pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Compresso ) )
				{
					CSelectedCompresso *pclSelectedTecBoxCompresso = dynamic_cast<CSelectedCompresso *>( pclSelectedProduct );

					if( NULL == pclSelectedTecBoxCompresso )
					{
						continue;
					}
			
					_PrepareCompressoResultData( pclIndSelPMParams, pclSelectedTecBoxCompresso, iResultCount++ );
					WriteVecResultInFile( clTUFileHelper );
				}
			}

			if( true == bIsPMTypeAll 
					|| CPMInputUser::MT_WithPump == pclInputUser->GetPressureMaintenanceType() )
			{
				for( CSelectedPMBase *pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Transfero ); 
						NULL != pclSelectedProduct; pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Transfero ) )
				{
					CSelectedTransfero *pclSelectedTecBoxTransfero = dynamic_cast<CSelectedTransfero *>( pclSelectedProduct );

					if( NULL == pclSelectedTecBoxTransfero )
					{
						continue;
					}
			
					_PrepareTransferoResultData( pclIndSelPMParams, pclSelectedTecBoxTransfero, iResultCount++ );
					WriteVecResultInFile( clTUFileHelper );
				}
			}
		}
		else
		{
			int iResultCount = 0;

			for( CSelectedPMBase *pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Vento );
				NULL != pclSelectedProduct; pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Vento ) )
			{
				CSelectedVento *pclSelectedTBVento = dynamic_cast<CSelectedVento *>( pclSelectedProduct );

				if( NULL == pclSelectedTBVento )
				{
					continue;
				}

				_PrepareVentoResultData( pclIndSelPMParams, pclSelectedTBVento, iResultCount++ );
				WriteVecResultInFile( clTUFileHelper );
			}

			for( CSelectedPMBase *pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Pleno );
				NULL != pclSelectedProduct; pclSelectedProduct = pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Pleno ) )
			{
				_PreparePlenoResultData( pclIndSelPMParams, pclSelectedProduct, iResultCount++ );
				WriteVecResultInFile( clTUFileHelper );
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelPMHelper::GetOutputTitlesNbr( std::vector<CString> *pvecString )
{
	if( NULL == pvecString )
	{
		ASSERTA_RETURN( 0 );
	}

	int iOutputTitlesNbr = 0;

	if( 0 == pvecString->at( 1 ).CompareNoCase( _T("calculated data") ) )
	{
		iOutputTitlesNbr = (int)m_vecOutputComputedDataTitles.size();
	}
	else if( 0 == pvecString->at( 1 ).CompareNoCase( _T("vessel") ) )
	{
		iOutputTitlesNbr = (int)m_vecOutputExpansionVesselTitles.size();
	}
	else if( 0 == pvecString->at( 1 ).CompareNoCase( _T("compresso") ) )
	{
		iOutputTitlesNbr = (int)m_vecOutputCompressoTitles.size();
	}
	else if( 0 == pvecString->at( 1 ).CompareNoCase( _T("transfero") ) )
	{
		iOutputTitlesNbr = (int)m_vecOutputTransferoTitles.size();
	}
	else if( 0 == pvecString->at( 1 ).CompareNoCase( _T("vento") ) )
	{
		iOutputTitlesNbr = (int)m_vecOutputVentoTitles.size();
	}
	else if( 0 == pvecString->at( 1 ).CompareNoCase( _T("pleno") ) )
	{
		iOutputTitlesNbr = (int)m_vecOutputPlenoTitles.size();
	}

	return iOutputTitlesNbr;
}

void CTUIndSelPMHelper::_PrepareComputedDataResultData( CIndSelPMParams *pclIndSelPMParams, int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclIndSelPMParams || NULL == pclIndSelPMParams->m_pclSelectPMList || NULL == pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclInputUser = pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Line type.
	m_vecResultData.push_back( _T("Calculated data") );

	// Expansion factor.
	strValue.Format( _T("%g"), pclInputUser->GetSystemExpansionCoefficient() );
	m_vecResultData.push_back( strValue );

	// Expansion volume.
	strValue.Format( _T("%g"), pclInputUser->GetSystemExpansionVolume() );
	m_vecResultData.push_back( strValue );

	// Water reserve.
	strValue.Format( _T("%g"), pclInputUser->GetMinimumWaterReserve() );
	m_vecResultData.push_back( strValue );

	// Contraction volume for cooling.
	strValue.Format( _T("%g"), pclInputUser->ComputeContractionVolume() );
	m_vecResultData.push_back( strValue );

	// Vapor pressure.
	strValue.Format( _T("%g"), pclInputUser->GetVaporPressure() );
	m_vecResultData.push_back( strValue );

	// Pz.
	strValue.Format( _T("%g"), pclInputUser->GetPz() );
	m_vecResultData.push_back( strValue );

	// Minimum pressure.
	strValue.Format( _T("%g"), pclInputUser->GetMinimumPressure() );
	m_vecResultData.push_back( strValue );

	// Vessel min PSV.
	strValue.Format( _T("%g"), pclInputUser->GetMinimumRequiredPSVRounded(CPMInputUser::MT_ExpansionVessel));
	m_vecResultData.push_back( strValue );

	// Compresso min PSV.
	strValue.Format( _T("%g"), pclInputUser->GetMinimumRequiredPSVRounded(CPMInputUser::MT_WithCompressor) );
	m_vecResultData.push_back( strValue );

	// Transfero min PSV.
	strValue.Format( _T("%g"), pclInputUser->GetMinimumRequiredPSVRounded(CPMInputUser::MT_WithPump) );
	m_vecResultData.push_back( strValue );

	// Compresso & Transfero minimum initial pressure.
	strValue.Format( _T("%g"), pclInputUser->GetMinimumInitialPressure() );
	m_vecResultData.push_back( strValue );

	// Compresso target pressure.
	strValue.Format( _T("%g"), pclInputUser->GetTargetPressureForTecBox( CDB_TecBox::etbtCompresso ) );
	m_vecResultData.push_back( strValue );

	// Transfero target pressure.
	strValue.Format( _T("%g"), pclInputUser->GetTargetPressureForTecBox( CDB_TecBox::etbtTransfero ) );
	m_vecResultData.push_back( strValue );

	// Vessel final pressure.
	strValue.Format( _T("%g"), pclInputUser->GetFinalPressure() );
	m_vecResultData.push_back( strValue );

	// Compresso final pressure.
	strValue.Format( _T("%g"), pclInputUser->GetFinalPressure( CDB_TecBox::etbtCompresso ) );
	m_vecResultData.push_back( strValue );

	// Transfero final pressure.
	strValue.Format( _T("%g"), pclInputUser->GetFinalPressure( CDB_TecBox::etbtTransfero ) );
	m_vecResultData.push_back( strValue );

	// Vessel pressure factor.
	strValue.Format( _T("%g"), pclInputUser->GetPressureFactor() );
	m_vecResultData.push_back( strValue );

	// Compresso & Transfero pressure factor.
	strValue.Format( _T("%g"), pclInputUser->GetPressureFactor( true ) );
	m_vecResultData.push_back( strValue );

	// Vessel nominal volume.
	strValue.Format( _T("%g"), pclInputUser->GetVesselNominalVolume() );
	m_vecResultData.push_back( strValue );

	// Compresso & Transfero nominal volume.
	strValue.Format( _T("%g"), pclInputUser->GetVesselNominalVolume( true ) );
	m_vecResultData.push_back( strValue );


	// Compresso & Transfero needed flow rate equalization volume.
	strValue.Format( _T("%g"), pclInputUser->GetVD() );
	m_vecResultData.push_back( strValue );
}

void CTUIndSelPMHelper::_PrepareExpansionVesselResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedVssl *pclSelectedVessel,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclIndSelPMParams || NULL == pclIndSelPMParams->m_pclSelectPMList || NULL == pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == pclSelectedVessel || NULL == dynamic_cast<CDB_Vessel *>( pclSelectedVessel->GetpData() ) )
	{
		ASSERT_RETURN;
	}

	CDB_Vessel *pclExpansionVessel = dynamic_cast<CDB_Vessel *>( pclSelectedVessel->GetpData() );
	CPMInputUser *pclInputUser = pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Line type.
	m_vecResultData.push_back( _T("Vessel") );

	// Is price index used?
	strValue.Format( _T("%s"), ( true == TASApp.IsPMPriceIndexUsed() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Price index.
	strValue.Format( _T("%g"), pclSelectedVessel->GetPriceIndex() );
	m_vecResultData.push_back( strValue );

	// Vessel ID.
	m_vecResultData.push_back( pclExpansionVessel->GetIDPtr().ID );

	// Vessel name.
	m_vecResultData.push_back( pclExpansionVessel->GetName() );

	// Vessel quantity.
	strValue.Format( _T("%i"), pclSelectedVessel->GetNbreOfVsslNeeded() );
	m_vecResultData.push_back( strValue );

	// Vessel nominal volume.
	strValue.Format( _T("%g"), pclExpansionVessel->GetNominalVolume() );
	m_vecResultData.push_back( strValue );

	// Vessel max. pressure.
	strValue.Format( _T("%g"), pclExpansionVessel->GetPmaxmax() );
	m_vecResultData.push_back( strValue );

	// Vessel temperature range.
	m_vecResultData.push_back( pclExpansionVessel->GetTempRange() );

	// Vessel factory preset pressure.
	strValue.Format( _T("%g"), pclExpansionVessel->GetFactoryP0() );
	m_vecResultData.push_back( strValue );

	// Vessel diameter.
	strValue.Format( _T("%g"), pclExpansionVessel->GetDiameter() );
	m_vecResultData.push_back( strValue );

	// Vessel height.
	strValue.Format( _T("%g"), pclExpansionVessel->GetHeight() );
	m_vecResultData.push_back( strValue );

	// Vessel weight.
	strValue.Format( _T("%g"), pclExpansionVessel->GetWeight() );
	m_vecResultData.push_back( strValue );

	// Vessel max. weight.
	strValue.Format( _T("%g"), pclInputUser->GetMaxWeight( pclExpansionVessel->GetWeight(), pclExpansionVessel->GetNominalVolume(), pclInputUser->GetMinTemperature() ) );
	m_vecResultData.push_back( strValue );

	// Vessel standing.
	m_vecResultData.push_back( m_mapLayoutName[pclExpansionVessel->GetInstallationLayout()] );

	// Vessel connection.
	m_vecResultData.push_back( pclExpansionVessel->GetConnect() );

	// Required preset pressure.
	strValue.Format( _T("%g"), pclSelectedVessel->GetMinimumInitialPressure() );
	m_vecResultData.push_back( strValue );

	// HYS-1022: 'pa,opt' becomes 'pa' (Initial pressure).
	strValue.Format( _T("%g"), pclSelectedVessel->GetInitialPressure() );
	m_vecResultData.push_back( strValue );

	// HYS-1022: 'Vwr,opt' becomes 'Vwr' (Water reserve).
	strValue.Format( _T("%g"), pclSelectedVessel->GetWaterReserve() );
	m_vecResultData.push_back( strValue );
}

void CTUIndSelPMHelper::_PrepareCompressoResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedCompresso *pclSelectedCompresso,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclIndSelPMParams || NULL == pclIndSelPMParams->m_pclSelectPMList || NULL == pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == pclSelectedCompresso || NULL == dynamic_cast<CDB_TecBox *>( pclSelectedCompresso->GetpData() ) )
	{
		ASSERT_RETURN;
	}

	CDB_TecBox *pclCompresso = dynamic_cast<CDB_TecBox *>( pclSelectedCompresso->GetpData() );
	CPMInputUser *pclInputUser = pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Line type.
	m_vecResultData.push_back( _T("Compresso") );

	// Is price index used?
	strValue.Format( _T("%s"), ( true == TASApp.IsPMPriceIndexUsed() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Price index.
	strValue.Format( _T("%g"), pclSelectedCompresso->GetPriceIndex() );
	m_vecResultData.push_back( strValue );

	// Compresso ID.
	m_vecResultData.push_back( pclCompresso->GetIDPtr().ID );

	// Compresso name.
	m_vecResultData.push_back( pclCompresso->GetName() );

	// Compresso redundancy?
	bool bRedundancy = pclInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	strValue.Format( _T("%s"), ( true == bRedundancy ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Compresso max. pressure.
	strValue.Format( _T("%g"), pclCompresso->GetPmaxmax() );
	m_vecResultData.push_back( strValue );

	// Compresso temperature range.
	m_vecResultData.push_back( pclCompresso->GetTempRange() );

	// Has Compresso function water make-up?
	strValue.Format( _T("%s"), ( true == pclCompresso->IsFctWaterMakeUp() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Compresso max. temperature for water make-up.
	strValue.Format( _T("%g"), pclCompresso->GetTmaxWaterMakeUp() );
	m_vecResultData.push_back( strValue );

	// Compresso width.
	strValue.Format( _T("%g"), pclCompresso->GetWidth() );
	m_vecResultData.push_back( strValue );

	// Compresso height.
	strValue.Format( _T("%g"), pclCompresso->GetHeight() );
	m_vecResultData.push_back( strValue );

	// Compresso depth.
	strValue.Format( _T("%g"), pclCompresso->GetDepth() );
	m_vecResultData.push_back( strValue );

	// Compresso weight.
	strValue.Format( _T("%g"), pclCompresso->GetWeight() );
	m_vecResultData.push_back( strValue );

	// Compresso standing.
	m_vecResultData.push_back( m_mapLayoutName[pclCompresso->GetInstallationLayout()] );

	// Compresso power.
	strValue.Format( _T("%g"), pclCompresso->GetPower() );
	m_vecResultData.push_back( strValue );

	// Compresso supply voltage.
	m_vecResultData.push_back( pclCompresso->GetPowerSupplyStr() );

	// Compresso sound pressure level.
	strValue.Format( _T("%g"), pclCompresso->GetSoundPressureLevel() );
	m_vecResultData.push_back( strValue );

	// Compresso integrated vessel ID.
	m_vecResultData.push_back( pclCompresso->GetIntegratedVesselIDPtr().ID );
}

void CTUIndSelPMHelper::_PrepareTransferoResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedTransfero *pclSelectedTransfero,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclIndSelPMParams || NULL == pclIndSelPMParams->m_pclSelectPMList || NULL == pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == pclSelectedTransfero || NULL == dynamic_cast<CDB_TecBox *>( pclSelectedTransfero->GetpData() ) )
	{
		ASSERT_RETURN;
	}

	CDB_TecBox *pclTransfero = dynamic_cast<CDB_TecBox *>( pclSelectedTransfero->GetpData() );
	CPMInputUser *pclInputUser = pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Line type.
	m_vecResultData.push_back( _T("Transfero") );

	// Is price index used?
	strValue.Format( _T("%s"), ( true == TASApp.IsPMPriceIndexUsed() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Price index.
	strValue.Format( _T("%g"), pclSelectedTransfero->GetPriceIndex() );
	m_vecResultData.push_back( strValue );

	// Transfero ID.
	m_vecResultData.push_back( pclTransfero->GetIDPtr().ID );

	// Transfero name.
	m_vecResultData.push_back( pclTransfero->GetName() );

	// Transfero redundancy?
	bool bRedundancy = pclInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	strValue.Format( _T("%s"), ( true == bRedundancy ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Transfero max. pressure.
	strValue.Format( _T("%g"), pclTransfero->GetPmaxmax() );
	m_vecResultData.push_back( strValue );

	// Transfero temperature range.
	m_vecResultData.push_back( pclTransfero->GetTempRange() );

	// Has Transfero function water make-up?
	strValue.Format( _T("%s"), ( true == pclTransfero->IsFctWaterMakeUp() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Transfero max. temperature for water make-up.
	strValue.Format( _T("%g"), pclTransfero->GetTmaxWaterMakeUp() );
	m_vecResultData.push_back( strValue );

	// Transfero width.
	strValue.Format( _T("%g"), pclTransfero->GetWidth() );
	m_vecResultData.push_back( strValue );

	// Transfero height.
	strValue.Format( _T("%g"), pclTransfero->GetHeight() );
	m_vecResultData.push_back( strValue );

	// Transfero depth.
	strValue.Format( _T("%g"), pclTransfero->GetDepth() );
	m_vecResultData.push_back( strValue );

	// Transfero weight.
	strValue.Format( _T("%g"), pclTransfero->GetWeight() );
	m_vecResultData.push_back( strValue );

	// Transfero standing.
	m_vecResultData.push_back( m_mapLayoutName[pclTransfero->GetInstallationLayout()] );

	// Transfero power.
	strValue.Format( _T("%g"), pclTransfero->GetPower() );
	m_vecResultData.push_back( strValue );

	// Transfero supply voltage.
	m_vecResultData.push_back( pclTransfero->GetPowerSupplyStr() );

	// Transfero sound pressure level.
	strValue.Format( _T("%g"), pclTransfero->GetSoundPressureLevel() );
	m_vecResultData.push_back( strValue );

	// Transfero integrated vessel ID.
	m_vecResultData.push_back( pclTransfero->GetIntegratedVesselIDPtr().ID );
}

void CTUIndSelPMHelper::_PrepareVentoResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedVento *pclSelectedVento,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclIndSelPMParams || NULL == pclIndSelPMParams->m_pclSelectPMList || NULL == pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == pclSelectedVento || NULL == dynamic_cast<CDB_TBPlenoVento *>( pclSelectedVento->GetpData() ) )
	{
		ASSERT_RETURN;
	}

	CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedVento->GetpData() );
	CPMInputUser *pclInputUser = pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Line type.
	m_vecResultData.push_back( _T("Vento") );

	// Price index.
	strValue.Format( _T("%s"), ( true == TASApp.IsPMPriceIndexUsed() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Price index.
	strValue.Format( _T("%g"), pclSelectedVento->GetPriceIndex() );
	m_vecResultData.push_back( strValue );

	// Vento ID.
	m_vecResultData.push_back( pclVento->GetIDPtr().ID );

	// Vento name.
	m_vecResultData.push_back( pclVento->GetName() );

	// Vento quantity.
	strValue.Format( _T("%i"), pclSelectedVento->GetNumberOfVento() );
	m_vecResultData.push_back( strValue );

	// Vento redundancy?
	bool bRedundancy = pclInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	strValue.Format( _T("%s"), ( true == bRedundancy ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Vento max. pressure.
	strValue.Format( _T("%g"), pclVento->GetPmaxmax() );
	m_vecResultData.push_back( strValue );

	// Vento temperature range.
	m_vecResultData.push_back( pclVento->GetTempRange() );

	// Has Vento function water make-up?
	strValue.Format( _T("%s"), ( true == pclVento->IsFctWaterMakeUp() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Vento max. temperature for water make-up.
	strValue.Format( _T("%g"), pclVento->GetTmaxWaterMakeUp() );
	m_vecResultData.push_back( strValue );

	// Vento width.
	strValue.Format( _T("%g"), pclVento->GetWidth() );
	m_vecResultData.push_back( strValue );

	// Vento height.
	strValue.Format( _T("%g"), pclVento->GetHeight() );
	m_vecResultData.push_back( strValue );

	// Vento depth.
	strValue.Format( _T("%g"), pclVento->GetDepth() );
	m_vecResultData.push_back( strValue );

	// Vento weight.
	strValue.Format( _T("%g"), pclVento->GetWeight() );
	m_vecResultData.push_back( strValue );

	// Vento standing.
	m_vecResultData.push_back( m_mapLayoutName[pclVento->GetInstallationLayout()] );

	// Vento electrical power.
	strValue.Format( _T("%g"), pclVento->GetPower() );
	m_vecResultData.push_back( strValue );

	// Vento power supply voltage.
	m_vecResultData.push_back( pclVento->GetPowerSupplyStr() );

	// Vento system volume.
	strValue.Format( _T("%g"), pclVento->GetSystemVolume() );
	m_vecResultData.push_back( strValue );

	// Vento sound pressure level.
	strValue.Format( _T("%g"), pclVento->GetSoundPressureLevel() );
	m_vecResultData.push_back( strValue );

	// Vento working pressure range.
	m_vecResultData.push_back( pclVento->GetWorkingPressureRange().c_str() );

	// Vento flow water make-up.
	strValue.Format( _T("%g"), pclVento->GetFlowWaterMakeUp() );
	m_vecResultData.push_back( strValue );

	// Vento IP.
	m_vecResultData.push_back( pclVento->GetIPxx() );
}

void CTUIndSelPMHelper::_PreparePlenoResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedPMBase *pclSelectedPleno,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclIndSelPMParams || NULL == pclIndSelPMParams->m_pclSelectPMList || NULL == pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == pclSelectedPleno )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclInputUser = pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// It exists now in the DB a Pleno that is in fact a combination of two modules "P BA4 P" + "P ABR5-R". We want to show this product as
	// one product but with specific data for each of both devices.
	bool bWTCombination = ( CSelectedPMBase::ST_WaterTreatmentCombination == pclSelectedPleno->GetSelectedType() ) ? true : false;

	CDB_TBPlenoVento *pclFirstTecBoxPleno = NULL;
	CDB_TBPlenoVento *pclSecondTecBoxPleno = NULL;
	CSelectedWaterTreatmentCombination *pclSelectedWTCombination = NULL;
	CDB_Set *pWTCombination = NULL;
	bool bDoubleInfo = false;
	bool bNoInfo = false;

	if( false == bWTCombination )
	{
		// Only one device.
		pclFirstTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedPleno->GetpData() );
	}
	else
	{
		// Combination of two devices.
		pclSelectedWTCombination = dynamic_cast<CSelectedWaterTreatmentCombination*>( pclSelectedPleno );

		if( NULL == pclSelectedWTCombination )
		{
			return;
		}

		pWTCombination = pclSelectedWTCombination->GetWTCombination();

		if( NULL == pWTCombination )
		{
			return;
		}

		if( NULL != pclSelectedWTCombination->GetSelectedFirst() )
		{
			pclFirstTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedWTCombination->GetSelectedFirst()->GetpData() );
		}

		if( NULL != pclSelectedWTCombination->GetSelectedSecond() )
		{
			pclSecondTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedWTCombination->GetSelectedSecond()->GetpData() );
		}

		// The 'fDoubleInfo' variable is set to 'true' if there is 2 devices for this water treatment product.
		if( NULL != pclFirstTecBoxPleno && NULL != pclSecondTecBoxPleno )
		{
			bDoubleInfo = true;
		}
		
		// It exists also the possibility to choose no water make-up treatment but just the cartridge. In this case we effectively have 
		// an empty object in the DB that give the possibility to the user when clicking on it to choose a cartridge in the deployed sheet.
		if( NULL == pclFirstTecBoxPleno && NULL == pclSecondTecBoxPleno )
		{
			bNoInfo = true;
		}
	}

	// Line type.
	m_vecResultData.push_back( _T("Pleno") );

	// Price index.
	strValue.Format( _T("%s"), ( true == TASApp.IsPMPriceIndexUsed() ) ? _T("true") : _T("false") );
	m_vecResultData.push_back( strValue );

	// Price index.
	strValue.Format( _T("%g"), pclSelectedPleno->GetPriceIndex() );
	m_vecResultData.push_back( strValue );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno ID.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue = pclFirstTecBoxPleno->GetIDPtr().ID;
	}
	else
	{
		// Combination of two devices.

		if( 0 == pWTCombination->GetName().GetLength() )
		{
			// Otherwise we take the name of the first device.
			strValue = pclFirstTecBoxPleno->GetIDPtr().ID;
			
			if( true == bDoubleInfo )
			{
				// And add the name of the second device if exist.
				strValue += CString( _T("/") ) + pclSecondTecBoxPleno->GetIDPtr().ID;
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno name.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue = pclFirstTecBoxPleno->GetName();
	}
	else
	{
		// Combination of two devices.

		if( pWTCombination->GetName().GetLength() > 0 )
		{
			// If the combination has a unique name, we use it.
			std::wstring *pStr = GetpXmlStrTab()->GetIDSStr( pWTCombination->GetName().GetBuffer() );
			
			if( NULL != pStr )
			{
				strValue = pStr->c_str();
			}
			else
			{
				strValue = pWTCombination->GetName();
			}
		}
		else
		{
			// Otherwise we take the name of the first device.
			strValue = pclFirstTecBoxPleno->GetName();
			
			if( true == bDoubleInfo )
			{
				// And add the name of the second device if exist.
				strValue += CString( _T("/") ) + pclSecondTecBoxPleno->GetName();
			}
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno redundancy.
	bool bRedundancy = pclInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	
	if( false == bWTCombination )
	{
		// Only one device.
		strValue = ( false == bRedundancy ) ? _T("false" ) : _T( "true");
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the redundancy of the first device.
			strValue = ( false == bRedundancy ) ? _T("false" ) : _T( "true");

			if( true == bDoubleInfo )
			{
				// And add the redundancy of the second device if exist.
				strValue += _T("/");
				strValue += ( false == bRedundancy ) ? _T("false" ) : _T( "true");
			}

		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno max. pressure.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetPmaxmax() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the Pmaxmax of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetPmaxmax() );

			if( true == bDoubleInfo )
			{
				// And add the Pmaxmax of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetPmaxmax() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}
	
	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno temperature range.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue = pclFirstTecBoxPleno->GetTempRange();
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the temperature range of the first device.
			strValue = pclFirstTecBoxPleno->GetTempRange();

			if( true == bDoubleInfo )
			{
				// And add the temperature range of the second device if exist.
				strValue += _T("/") + pclSecondTecBoxPleno->GetTempRange();
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno width.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetWidth() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the width of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetWidth() );

			if( true == bDoubleInfo )
			{
				// And add the width of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetWidth() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno height.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetHeight() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the height of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetHeight() );

			if( true == bDoubleInfo )
			{
				// And add the height of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetHeight() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno depth.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetDepth() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the depth of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetDepth() );

			if( true == bDoubleInfo )
			{
				// And add the depth of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetDepth() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno weight.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetWeight() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the weight of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetWeight() );

			if( true == bDoubleInfo )
			{
				// And add the weight of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetWeight() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}
	
	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno standing.
	// Only one device. Don't need to show standing for both devices.

	m_vecResultData.push_back( m_mapLayoutName[pclFirstTecBoxPleno->GetInstallationLayout()] );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno electrical power.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetPower() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the power of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetPower() );

			if( true == bDoubleInfo )
			{
				// And add the power of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetPower() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno power supply voltage.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue = pclFirstTecBoxPleno->GetPowerSupplyStr();
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the power supply of the first device.
			strValue = pclFirstTecBoxPleno->GetPowerSupplyStr();

			if( true == bDoubleInfo )
			{
				// And add the power supply of the second device if exist.
				strValue += CString( _T("/") ) + pclSecondTecBoxPleno->GetPowerSupplyStr();
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno Kvs.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetKvs() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the kvs of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetKvs() );

			if( true == bDoubleInfo )
			{
				// And add the kvs of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetKvs() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno sound pressure level.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetSoundPressureLevel() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the sound pressure level of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetSoundPressureLevel() );

			if( true == bDoubleInfo )
			{
				// And add the sound pressure level of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetSoundPressureLevel() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno DPU.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue = pclFirstTecBoxPleno->GetWorkingPressureRange().c_str();
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the DPp of the first device.
			strValue = pclFirstTecBoxPleno->GetWorkingPressureRange().c_str();

			if( true == bDoubleInfo )
			{
				// And add the DPp of the second device if exist.
				strValue += CString( _T("/") ) + pclSecondTecBoxPleno->GetWorkingPressureRange().c_str();
			}
		}
		else
		{
			strValue = _T( "" );
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno QNWM.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetFlowWaterMakeUp() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the 'qNwm' of the first device.
			strValue.Format( _T("%g"), pclFirstTecBoxPleno->GetFlowWaterMakeUp() );

			if( true == bDoubleInfo )
			{
				// And add the 'qNwm' of the second device if exist.
				strValue.Format( _T("%s/%g"), strValue, pclSecondTecBoxPleno->GetFlowWaterMakeUp() );
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno IP.
	if( false == bWTCombination )
	{
		// Only one device.
		strValue = pclFirstTecBoxPleno->GetIPxx();
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the IP of the first device.
			strValue = pclFirstTecBoxPleno->GetIPxx();

			if( true == bDoubleInfo )
			{
				// And add the IP of the second device if exist.
				strValue += _T("/") + pclSecondTecBoxPleno->GetIPxx();
			}
		}
		else
		{
			strValue = _T("");
		}
	}

	m_vecResultData.push_back( strValue );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for separators and air vents.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUIndSelSeparatorHelper::CTUIndSelSeparatorHelper() 
		: CTUIndSelHelper( m_clIndSelSeparatorParams )
{
	m_clIndSelSeparatorParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_Separator;
	m_clIndSelSeparatorParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Separator ID") );
	m_vecOutputTitles.push_back( _T("Separator name") );
	m_vecOutputTitles.push_back( _T("Separator body material") );
	m_vecOutputTitles.push_back( _T("Separator connection") );
	m_vecOutputTitles.push_back( _T("Separator version") );
	m_vecOutputTitles.push_back( _T("Separator PN") );
	m_vecOutputTitles.push_back( _T("Separator size") );
	m_vecOutputTitles.push_back( _T("Separator Dp") );
	m_vecOutputTitles.push_back( _T("Separator temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelSeparatorHelper::~CTUIndSelSeparatorHelper()
{
	if( NULL != m_clIndSelSeparatorParams.m_pclSelectSeparatorList )
	{
		delete m_clIndSelSeparatorParams.m_pclSelectSeparatorList;
		m_clIndSelSeparatorParams.m_pclSelectSeparatorList = NULL;
	}
}

UINT CTUIndSelSeparatorHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_NOINPUT;
	}

	if( NULL != m_clIndSelSeparatorParams.m_pclSelectSeparatorList )
	{
		delete m_clIndSelSeparatorParams.m_pclSelectSeparatorList;
	}

	m_clIndSelSeparatorParams.m_pclSelectSeparatorList = new CSelectSeparatorList();

	if( NULL == m_clIndSelSeparatorParams.m_pclSelectSeparatorList || NULL == m_clIndSelSeparatorParams.m_pclSelectSeparatorList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_CANTCREATESELECTLIST;
	}

	m_clIndSelSeparatorParams.m_pclSelectSeparatorList->GetSelectPipeList()->SelectPipes( &m_clIndSelSeparatorParams, m_clIndSelSeparatorParams.m_dFlow );

	m_clIndSelSeparatorParams.m_SeparatorList.PurgeAll();

	m_clIndSelSeparatorParams.m_pTADB->GetSeparatorList( 
				&m_clIndSelSeparatorParams.m_SeparatorList, 
				(LPCTSTR)m_clIndSelSeparatorParams.m_strComboTypeID, 
				(LPCTSTR)m_clIndSelSeparatorParams.m_strComboFamilyID,
				(LPCTSTR)m_clIndSelSeparatorParams.m_strComboConnectID,
				(LPCTSTR)m_clIndSelSeparatorParams.m_strComboVersionID,
				m_clIndSelSeparatorParams.m_eFilterSelection );

	bool bSizeShiftProblem = false;
	bool bBestFound = false;
	int iDevFound = m_clIndSelSeparatorParams.m_pclSelectSeparatorList->SelectSeparator( &m_clIndSelSeparatorParams, &bSizeShiftProblem, bBestFound );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_RESULTNBRNOTSAME;
	}
	
	int iLoopResultInInputFile = 0;

	for( CSelectedSeparator *pclSelectedSeparator = m_clIndSelSeparatorParams.m_pclSelectSeparatorList->GetFirst<CSelectedSeparator>(); NULL != pclSelectedSeparator; 
			pclSelectedSeparator = m_clIndSelSeparatorParams.m_pclSelectSeparatorList->GetNext<CSelectedSeparator>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelSeparatorParams, pclSelectedSeparator, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelSeparatorHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = Separator\n\n") );
	
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}
	
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CIndSelSeparatorParams *pclIndSelSeparatorParams = dynamic_cast<CIndSelSeparatorParams *>( pclProdSelParams );

	if( NULL != pclIndSelSeparatorParams->m_pclSelectSeparatorList )
	{
		int iResultCount = 0;

		for( CSelectedSeparator *pclSelectedSeparator = pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetFirst<CSelectedSeparator>(); NULL != pclSelectedSeparator; 
				pclSelectedSeparator = pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetNext<CSelectedSeparator>() )
		{
			_PrepareResultData( pclIndSelSeparatorParams, pclSelectedSeparator, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelSeparatorHelper::_PrepareResultData( CIndSelSeparatorParams *pclIndSelSeparatorParams, CSelectedSeparator *pclSelectedSeparator,
		int iResultCount )
{
	m_vecResultData.clear();
	CDB_TAProduct *pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclSelectedSeparator->GetpData() );

	if( NULL == pclTAProduct ) 
	{
		return;
	}
	
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Separator ID.
	m_vecResultData.push_back( pclTAProduct->GetIDPtr().ID );
	
	// Separator name.
	m_vecResultData.push_back( pclTAProduct->GetName() );

	// Separator body material.
	m_vecResultData.push_back( pclTAProduct->GetBodyMaterial() );

	// Separator connection.
	m_vecResultData.push_back( pclTAProduct->GetConnect() );

	// Separator version.
	m_vecResultData.push_back( pclTAProduct->GetVersion() );

	// Separator PN.
	m_vecResultData.push_back( pclTAProduct->GetPN().c_str() );

	// Separator size.
	m_vecResultData.push_back( pclTAProduct->GetSize() );

	// Separator Dp.
	strValue.Format( _T("%g"), pclSelectedSeparator->GetDp() );
	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclTAProduct->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelSeparatorParams );
	pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetSelectPipeList()->GetMatchingPipe( pclTAProduct->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for shutoff valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUIndSelSVHelper::CTUIndSelSVHelper() 
		: CTUIndSelHelper( m_clIndSelSVParams )
{
	m_clIndSelSVParams.m_eProductSubCategory = ProductSubCategory::PSC_ESC_ShutoffValve;
	m_clIndSelSVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Valve Dp") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelSVHelper::~CTUIndSelSVHelper()
{
	if( NULL != m_clIndSelSVParams.m_pclSelectSVList )
	{
		delete m_clIndSelSVParams.m_pclSelectSVList;
		m_clIndSelSVParams.m_pclSelectSVList = NULL;
	}
}

UINT CTUIndSelSVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_NOINPUT;
	}

	if( NULL != m_clIndSelSVParams.m_pclSelectSVList )
	{
		delete m_clIndSelSVParams.m_pclSelectSVList;
	}

	m_clIndSelSVParams.m_pclSelectSVList = new CSelectShutoffList();

	if( NULL == m_clIndSelSVParams.m_pclSelectSVList || NULL == m_clIndSelSVParams.m_pclSelectSVList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_CANTCREATESELECTLIST;
	}

	m_clIndSelSVParams.m_pclSelectSVList->GetSelectPipeList()->SelectPipes( &m_clIndSelSVParams, m_clIndSelSVParams.m_dFlow );

	m_clIndSelSVParams.m_SVList.PurgeAll();

	m_clIndSelSVParams.m_pTADB->GetSvList( 
			&m_clIndSelSVParams.m_SVList,
			(LPCTSTR)m_clIndSelSVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelSVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelSVParams.m_strComboMaterialID, 
			(LPCTSTR)m_clIndSelSVParams.m_strComboConnectID, 
			(LPCTSTR)m_clIndSelSVParams.m_strComboVersionID,
			m_clIndSelSVParams.m_eFilterSelection );

	bool bSizeShiftProblem = false;
	bool bBestFound = false;
	int iDevFound = m_clIndSelSVParams.m_pclSelectSVList->SelectShutoffValve( &m_clIndSelSVParams, &bSizeShiftProblem, bBestFound );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_RESULTNBRNOTSAME;
	}
	int iLoopResultInInputFile = 0;

	for( CSelectedValve *pclSelectedValve = m_clIndSelSVParams.m_pclSelectSVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_clIndSelSVParams.m_pclSelectSVList->GetNext<CSelectedValve>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelSVParams, pclSelectedValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelSVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = SV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}
	
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CIndSelSVParams *pclIndSelSVParams = dynamic_cast<CIndSelSVParams *>( pclProdSelParams );

	if( NULL != pclIndSelSVParams->m_pclSelectSVList )
	{
		int iResultCount = 0;

		for( CSelectedValve *pclSelectedSV = pclIndSelSVParams->m_pclSelectSVList->GetFirst<CSelectedValve>(); NULL != pclSelectedSV; 
				pclSelectedSV = pclIndSelSVParams->m_pclSelectSVList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSelSVParams, pclSelectedSV, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelSVHelper::_PrepareResultData( CIndSelSVParams *pclIndSelSVParams, CSelectedValve *pclSelectedValve,
		int iResultCount )
{
	m_vecResultData.clear();
	CDB_ShutoffValve *pclTAProduct = dynamic_cast<CDB_ShutoffValve *>( pclSelectedValve->GetpData() );

	if( NULL == pclTAProduct ) 
	{
		return;
	}
	
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclTAProduct->GetIDPtr().ID );
	
	// Valve name.
	m_vecResultData.push_back( pclTAProduct->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclTAProduct->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclTAProduct->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclTAProduct->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclTAProduct->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclTAProduct->GetSize() );

	// Valve Dp.
	strValue.Format( _T("%g"), pclSelectedValve->GetDp() );
	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclTAProduct->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelSVParams );
	pclIndSelSVParams->m_pclSelectSVList->GetSelectPipeList()->GetMatchingPipe( pclTAProduct->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for thermostatic regulative valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSelTRVHelper::CTUIndSelTRVHelper()
	: CTUIndSelHelper( m_clIndSelTRVParams )
{
	m_clIndSelTRVParams.m_eProductSubCategory = ProductSubCategory::PSC_TC_ThermostaticValve;
	m_clIndSelTRVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	// For supply valves.
	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Valve computed setting") );
	m_vecOutputTitles.push_back( _T("Valve displayed setting") );
	m_vecOutputTitles.push_back( _T("Valve Dp") );
	m_vecOutputTitles.push_back( _T("Valve Dp full opening") );
	m_vecOutputTitles.push_back( _T("Valve Kv") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );

	// For flow limited control valves.
	m_vecOutputFlowLimitedTitles.push_back( _T("Result number") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve ID") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve name") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve connection") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve version") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve PN") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve size") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve computed setting") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve displayed setting") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve Dpmin") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve flow range") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Valve temperature range") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Pipe size") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputFlowLimitedTitles.push_back( _T("Pipe velocity") );
	
	// For other inserts.
	m_vecOutputOtherInsertTitles.push_back( _T("Result number") );
	m_vecOutputOtherInsertTitles.push_back( _T("Insert Kv") );
	m_vecOutputOtherInsertTitles.push_back( _T("Insert Dp") );
}

CTUIndSelTRVHelper::~CTUIndSelTRVHelper()
{
	if( NULL != m_clIndSelTRVParams.m_pclSelectSupplyValveList )
	{
		delete m_clIndSelTRVParams.m_pclSelectSupplyValveList;
		m_clIndSelTRVParams.m_pclSelectSupplyValveList = NULL;
	}
}

UINT CTUIndSelTRVHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUIndSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_bIsTrvTypePreset' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("istrvtypepreset" ) ) )
	{
		if( 0 == m_mapInputs[_T("istrvtypepreset")].Compare( _T("true") ) || 0 == m_mapInputs[_T("istrvtypepreset")].Compare( _T("1") ) )
		{
			m_clIndSelTRVParams.m_bIsTrvTypePreset = true;
		}
		else if( 0 == m_mapInputs[_T("istrvtypepreset")].Compare( _T("false") ) || 0 == m_mapInputs[_T("istrvtypepreset")].Compare( _T("0") ) )
		{
			m_clIndSelTRVParams.m_bIsTrvTypePreset = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_ISTRVTYPEPRESETBAD;
		}
	}

	// The 'm_eValveType' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("valvetype" ) ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_VALVETYPEMISSING;
	}

	if( 0 == m_mapInputs[_T("valvetype")].Compare( _T("standard") ) )
	{
		m_clIndSelTRVParams.m_eValveType = RadiatorValveType::RVT_Standard;
	}
	else if( 0 == m_mapInputs[_T("valvetype")].Compare( _T("presettable") ) )
	{
		m_clIndSelTRVParams.m_eValveType = RadiatorValveType::RVT_Presettable;
	}
	else if( 0 == m_mapInputs[_T("valvetype")].Compare( _T("withflowlimitation") ) )
	{
		m_clIndSelTRVParams.m_eValveType = RadiatorValveType::RVT_WithFlowLimitation;
	}
	else if( 0 == m_mapInputs[_T("valvetype")].Compare( _T("inserts") ) )
	{
		m_clIndSelTRVParams.m_eValveType = RadiatorValveType::RVT_Inserts;
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_VALVETYPEBAD;
	}

	// The 'm_eInsertType' variable.
	// Remark: mandatory if 'ValveType' = 'Inserts'.
	if( 0 != m_mapInputs.count( _T("inserttype" ) ) )
	{
		if( 0 == m_mapInputs[_T("inserttype")].Compare( _T("heimeier") ) )
		{
			m_clIndSelTRVParams.m_eInsertType = RadiatorInsertType::RIT_Heimeier;
		}
		else if( 0 == m_mapInputs[_T("inserttype")].Compare( _T("bykv") ) )
		{
			m_clIndSelTRVParams.m_eInsertType = RadiatorInsertType::RIT_ByKv;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_INSERTTYPEBAD;
		}
	}

	// The 'm_eReturnValveMode' variable.
	// Remark: mandatory if 'ValveType' = 'Inserts'.
	if( 0 == m_mapInputs.count( _T("returnvalvemode" ) ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_RETURNVALVEMODEMISSING;
	}

	if( 0 == m_mapInputs[_T("returnvalvemode")].Compare( _T("nothing") ) )
	{
		m_clIndSelTRVParams.m_eReturnValveMode = RadiatorReturnValveMode::RRVM_Nothing;
	}
	else if( 0 == m_mapInputs[_T("returnvalvemode")].Compare( _T("other") ) )
	{
		m_clIndSelTRVParams.m_eReturnValveMode = RadiatorReturnValveMode::RRVM_Other;
	}
	else if( 0 == m_mapInputs[_T("returnvalvemode")].Compare( _T("imi") ) )
	{
		m_clIndSelTRVParams.m_eReturnValveMode = RadiatorReturnValveMode::RRVM_IMI;
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_RETURNVALVEMODEBAD;
	}

	// The 'm_bIsThermostaticHead' variable.
	// Remark: not mandatory. By default 'true'.
	if( 0 != m_mapInputs.count( _T("headtype") ) )
	{
		if( 0 == m_mapInputs[_T("headtype")].Compare( _T("thermostatic") ) )
		{
			m_clIndSelTRVParams.m_bIsThermostaticHead = true;
		}
		else if( 0 == m_mapInputs[_T("headtype")].Compare( _T("electroactuator") ) )
		{
			m_clIndSelTRVParams.m_bIsThermostaticHead = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_HEADTYPEBAD;
		}
	}

	// The 'm_dInsertKvValue' variable.
	// Remark: mandatory if 'ValveType' = 'Inserts' and 'InsertType' = 'ByKv'.
	if( 0 != m_mapInputs.count( _T("insertkvvalue" ) ) )
	{
		m_clIndSelTRVParams.m_dInsertKvValue = _wtof( m_mapInputs[_T("insertkvvalue")] );
	}

	// The 'm_strSVTypeID' variable.
	// Remark: mandatory if 'ValveType' = 'Standard', 'Presettable' or 'WithFlowLimitation'.
	if( 0 != m_mapInputs.count( _T("svtypeid") ) )
	{
		m_clIndSelTRVParams.m_strSVTypeID = m_mapInputsWithCase[_T("svtypeid")];
	}

	// The 'm_strComboSVFamilyID' variable.
	// Remark: mandatory if 'ValveType' = 'Standard', 'Presettable' or 'WithFlowLimitation'.
	if( 0 != m_mapInputs.count( _T("combosvfamilyid") ) )
	{
		m_clIndSelTRVParams.m_strComboSVFamilyID = m_mapInputsWithCase[_T("combosvfamilyid")];
	}

	// The 'm_strComboSVConnectID' variable.
	// Remark: not mandatory.
	if( 0 != m_mapInputs.count( _T("combosvconnectid") ) )
	{
		m_clIndSelTRVParams.m_strComboSVConnectID = m_mapInputsWithCase[_T("combosvconnectid")];
	}

	// The 'm_strComboSVVersionID' variable.
	// Remark: not mandatory.
	if( 0 != m_mapInputs.count( _T("combosvversionid") ) )
	{
		m_clIndSelTRVParams.m_strComboSVVersionID = m_mapInputsWithCase[_T("combosvversionid")];
	}

	// The 'm_strComboRVFamilyID' variable.
	// Remark: mandatory if 'ValveType' = 'Standard', 'Presettable' or 'WithFlowLimitation'.
	if( 0 != m_mapInputs.count( _T("comborvfamilyid") ) )
	{
		m_clIndSelTRVParams.m_strComboRVFamilyID = m_mapInputsWithCase[_T("comborvfamilyid")];
	}

	// The 'm_strComboRVConnectID' variable.
	// Remark: not mandatory.
	if( 0 != m_mapInputs.count( _T("comborvconnectid") ) )
	{
		m_clIndSelTRVParams.m_strComboRVConnectID = m_mapInputsWithCase[_T("comborvconnectid")];
	}

	// The 'm_strComboRVVersionID' variable.
	// Remark: not mandatory.
	if( 0 != m_mapInputs.count( _T("comborvversionid") ) )
	{
		m_clIndSelTRVParams.m_strComboRVVersionID = m_mapInputsWithCase[_T("comborvversionid")];
	}

	// The 'm_strComboSVInsertName' variable.
	// Remark: mandatory if 'ValveType' = 'Inserts'.
	if( 0 != m_mapInputs.count( _T("combosvinsertname") ) )
	{
		m_clIndSelTRVParams.m_strComboSVInsertName = m_mapInputs[_T("combosvinsertname")];
	}

	// The 'm_strComboSVInsertFamilyID' variable.
	// Remark: mandatory if 'ValveType' = 'Inserts' and 'InsertType' = 'Heimeier insert'.
	if( 0 != m_mapInputs.count( _T("combosvinsertfamilyid") ) )
	{
		m_clIndSelTRVParams.m_strComboSVInsertFamilyID = m_mapInputsWithCase[_T("combosvinsertfamilyid")];
	}

	// The 'm_strComboSVInsertTypeID' variable.
	// Remark: mandatory if 'ValveType' = 'Inserts' and 'InsertType' = 'Heimeier insert'.
	if( 0 != m_mapInputs.count( _T("combosvinserttypeid") ) )
	{
		m_clIndSelTRVParams.m_strComboSVInsertTypeID = m_mapInputsWithCase[_T("combosvinserttypeid")];
	}

	// The 'm_strComboRVInsertFamilyID' variable.
	// Remark: not mandatory.
	if( 0 != m_mapInputs.count( _T("comborvinsertfamilyid") ) )
	{
		m_clIndSelTRVParams.m_strComboRVInsertFamilyID = m_mapInputsWithCase[_T("comborvinsertfamilyid")];
	}

	// The 'm_strComboActuatorTypeID' variable.
	// Remark: not mandatory. By default "".
	if( 0 != m_mapInputs.count( _T("comboactuatortypeid") ) )
	{
		m_clIndSelTRVParams.m_strComboActuatorTypeID = m_mapInputsWithCase[_T("comboactuatortypeid")];
	}

	// The 'm_strComboActuatorFamilyID' variable.
	// Remark: not mandatory. By default "".
	if( 0 != m_mapInputs.count( _T("comboactuatorfamilyid") ) )
	{
		m_clIndSelTRVParams.m_strComboActuatorFamilyID = m_mapInputsWithCase[_T("comboactuatorfamilyid")];
	}

	// The 'm_strComboActuatorVersionID' variable.
	// Remark: not mandatory. By default "".
	if( 0 != m_mapInputs.count( _T("comboactuatorversionid") ) )
	{
		m_clIndSelTRVParams.m_strComboActuatorVersionID = m_mapInputsWithCase[_T("comboactuatorversionid")];
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelTRVHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( RadiatorValveType::RVT_Inserts == m_clIndSelTRVParams.m_eValveType && RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType
			&& m_clIndSelTRVParams.m_dInsertKvValue <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_KVINVALID;
	}

	if( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType )
	{
		if( _T("") == m_clIndSelTRVParams.m_strSVTypeID )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVTYPEIDEMPTY;
		}
		else if( _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strSVTypeID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVFAMIDINVALID;
		}

		if( _T("") == m_clIndSelTRVParams.m_strComboSVFamilyID )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVFAMIDEMPTY;
		}
		else if( _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboSVFamilyID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVFAMIDINVALID;
		}

		if( false == m_clIndSelTRVParams.m_strComboSVConnectID.IsEmpty()
				&& _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboSVConnectID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVCONIDINVALID;
		}

		if( false == m_clIndSelTRVParams.m_strComboSVVersionID.IsEmpty()
				&& _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboSVVersionID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVVERSIDINVALID;
		}

		if( _T("") == m_clIndSelTRVParams.m_strComboRVFamilyID )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVFAMIDEMPTY;
		}
		else if( _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboRVFamilyID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVFAMIDINVALID;
		}

		if( false == m_clIndSelTRVParams.m_strComboRVConnectID.IsEmpty()
				&& _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboRVConnectID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVCONIDINVALID;
		}

		if( false == m_clIndSelTRVParams.m_strComboRVVersionID.IsEmpty()
				&& _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboRVVersionID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVVERSIDINVALID;
		}
	}
	else
	{
		if( _T("") == m_clIndSelTRVParams.m_strComboSVInsertName )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVINSNAMEEMPTY;
		}
		else if( 0 != m_clIndSelTRVParams.m_strComboSVInsertName.CompareNoCase( CString( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_INSERTHEIMEIER ) ) ) 
				&& 0 != m_clIndSelTRVParams.m_strComboSVInsertName.CompareNoCase( CString( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_OTHERINSERT ) ) ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVINSNAMEINVALID;
		}
		
		if( 0 == m_clIndSelTRVParams.m_strComboSVInsertName.CompareNoCase( CString( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_INSERTHEIMEIER ) ) ) )
		{
			if( _T("") == m_clIndSelTRVParams.m_strComboSVInsertTypeID )
			{
				return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVINSTYPEIDEMPTY;
			}
			else if( _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboSVInsertTypeID ) )
			{
				return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVINSTYPEIDEMPTY;
			}
		}

		if( false == m_clIndSelTRVParams.m_strComboRVInsertFamilyID.IsEmpty()
				&& _NULL_IDPTR == m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboRVInsertFamilyID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVINSFAMIDINVALID;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelTRVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T( "" );

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_NOINPUT;
	}

	if( NULL != m_clIndSelTRVParams.m_pclSelectSupplyValveList )
	{
		delete m_clIndSelTRVParams.m_pclSelectSupplyValveList;
	}

	m_clIndSelTRVParams.m_pclSelectSupplyValveList = new CSelectTrvList();

	if( NULL == m_clIndSelTRVParams.m_pclSelectSupplyValveList || NULL == m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_CANTCREATESELECTLIST;
	}

	m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetSelectPipeList()->SelectPipes( &m_clIndSelTRVParams, m_clIndSelTRVParams.m_dFlow );

	int iSupplyValveFound = 0;
	int iReturnValveFound = 0;
	double dDpOnSupplyValve = 0.0;
	bool bSizeShiftProblem = false;

	// Prepare some variables to facilitate code after.
	CString strSVFamily = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboSVFamilyID : m_clIndSelTRVParams.m_strComboSVInsertFamilyID;
	CString strSVType = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strSVTypeID : m_clIndSelTRVParams.m_strComboSVInsertTypeID;
	CString strSVConnect = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboSVConnectID : _T( "" );
	CString strSVVersion = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboSVVersionID : _T( "" );
	CString strSVPN = _T( "" );
	CString strRVFamily = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboRVFamilyID : m_clIndSelTRVParams.m_strComboRVInsertFamilyID;
	CString strRVType = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? _T( "RVTYPE_RV" ) : _T( "RVTYPE_RV_INSERT" );
	CString strRVConnect = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboRVConnectID : _T( "" );
	CString strRVVersion = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboRVVersionID : _T( "" );

	bool bInsert = ( RadiatorValveType::RVT_Inserts == m_clIndSelTRVParams.m_eValveType ) ? true : false;
	double dMaxDpSV = 0.0;
	double dMaxDpRV = -DBL_MAX;
	bool bDiffDpSV = false;
	bool bDiffDpRV = false;
	// HYS-1002: The bInsertInKv has to be tested when we are in RVT_Inserts mode					
	bool bInsertInKv = false;
	if( bInsert )
	{
		bInsertInKv = ( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType ) ? true : false;
	}
	if( true == m_clIndSelTRVParams.m_bIsTrvTypePreset )
	{
		// TRVTYPE_PRESET or TRVTYPE_INSERT_PRESET.

		// Determine the right Dp.
		if( true == bInsert || RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode )
		{
			iReturnValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectReturnValve( &m_clIndSelTRVParams, (LPCTSTR)strRVType,
				(LPCTSTR)strRVFamily, (LPCTSTR)strRVConnect, (LPCTSTR)strRVVersion, -1.0, true );

			if( 0 != iReturnValveFound )
			{
				// Determine MaxDpRv and if there are different DpRv's.
				// Remark: the bigger DpRv leads to the lowest Dp for the Trv.
				CSelectList *pReturnValveList = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetReturnValveList();

				for( CSelectedBase *pclSelectedProduct = pReturnValveList->GetFirst<CSelectedBase>(); NULL != pclSelectedProduct; pclSelectedProduct = pReturnValveList->GetNext<CSelectedBase>() )
				{
					if( -DBL_MAX != dMaxDpRV )
					{
						if( dMaxDpRV != pclSelectedProduct->GetDp() )
						{
							bDiffDpRV = true;
							dMaxDpRV = max( dMaxDpRV, pclSelectedProduct->GetDp() );
						}
					}
					else
					{
						dMaxDpRV = pclSelectedProduct->GetDp();
					}
				}
			}

			dDpOnSupplyValve = ( true == m_clIndSelTRVParams.m_bDpEnabled ) ? m_clIndSelTRVParams.m_dDp - dMaxDpRV : 0.0;
		}
		else if( RadiatorReturnValveMode::RRVM_Nothing == m_clIndSelTRVParams.m_eReturnValveMode )
		{
			dDpOnSupplyValve = ( true == m_clIndSelTRVParams.m_bDpEnabled ) ? m_clIndSelTRVParams.m_dDp : 0.0;
		}

		m_clIndSelTRVParams.m_SupplyValveList.PurgeAll();

		// Pre-select thermostatic valve ONLY if we are not in 'Other insert' mode.
		if( !( true == bInsert && true == bInsertInKv ) )
		{
			m_clIndSelTRVParams.m_pTADB->GetTrvList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
				(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

			bool bValidFound = false;

			if( ( true == bDiffDpRV && RadiatorReturnValveMode::RRVM_Nothing != m_clIndSelTRVParams.m_eReturnValveMode ) || false == m_clIndSelTRVParams.m_bDpEnabled )
			{
				// With 'fForceFullOpen' set to 'true'.
				iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectSupplyValve( &m_clIndSelTRVParams, dDpOnSupplyValve,
					bValidFound, &bSizeShiftProblem, true );
			}
			else
			{
				// With Dp to be adjusted as determined above from Rv pre-selection.
				iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectSupplyValve( &m_clIndSelTRVParams, dDpOnSupplyValve,
					bValidFound, &bSizeShiftProblem, false );
			}
		}
	}
	else
	{
		// TRVTYPE_NOPRSET, TRVTYPE_INSERT_NOPRESET, TRVTYPE_INSERT_FL or TRVTYPE_FLOWLIMITED.

		if( false == bInsertInKv )
		{
			// Pre-select thermostatic valves.
			m_clIndSelTRVParams.m_SupplyValveList.PurgeAll();

			// Select supply valve with 'fForceFullOpen' set to 'true'.
			bool bValidFound = false;
			dDpOnSupplyValve = ( true == m_clIndSelTRVParams.m_bDpEnabled ) ? m_clIndSelTRVParams.m_dDp : 0.0;
			// HYS-1305 : Add insert with automatic flow limiter and No preset trv insert
			if( false == bInsert )
			{
				if( RadiatorValveType::RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
				{
					m_clIndSelTRVParams.m_pTADB->GetTrvList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
						(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

					iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectSupplyValve( &m_clIndSelTRVParams, dDpOnSupplyValve,
						bValidFound, &bSizeShiftProblem, true );
				}
				else
				{
					m_clIndSelTRVParams.m_pTADB->GetFLCVList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
						(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

					iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectFlowLimitedControlValve( &m_clIndSelTRVParams,
						bValidFound, &bSizeShiftProblem, false );
				}
			}
			else
			{
				if( false == m_clIndSelTRVParams.m_bIsFLCVInsert )
				{
					m_clIndSelTRVParams.m_pTADB->GetTrvInsertList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
						(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

					iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectSupplyValve( &m_clIndSelTRVParams, dDpOnSupplyValve,
						bValidFound, &bSizeShiftProblem, true );
				}
				else
				{
					m_clIndSelTRVParams.m_pTADB->GetFLCVInsertList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
						(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

					iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectFlowLimitedControlValve( &m_clIndSelTRVParams,
						bValidFound, &bSizeShiftProblem, false );
				}
			}
			
			dMaxDpSV = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetMaxDpSupplyValve();
			bDiffDpSV = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetDiffDpSV();

			if( iSupplyValveFound > 0 )
			{
				// Handle return valve according to selected mode.
				if( true == bInsert || RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode )
				{
					// If user wants a pressure drop that is below pressure drop on supply valve, we force full opening on return valve.
					if( true == bDiffDpSV || false == m_clIndSelTRVParams.m_bDpEnabled || m_clIndSelTRVParams.m_dDp <= dMaxDpSV )
					{
						// With 'fForceFullOpen' set to 'true'.
						iReturnValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectReturnValve( &m_clIndSelTRVParams, (LPCTSTR)strRVType,
								(LPCTSTR)strRVFamily,  (LPCTSTR)strRVConnect, (LPCTSTR)strRVVersion, -1.0, true );
					}
					else
					{
						// With Dp to be adjusted as determined above from Trv pre-selection
						double dDpOnReturnValve = m_clIndSelTRVParams.m_dDp - dMaxDpSV;

						iReturnValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectReturnValve( &m_clIndSelTRVParams, (LPCTSTR)strRVType,
								(LPCTSTR)strRVFamily, (LPCTSTR)strRVConnect, (LPCTSTR)strRVVersion, dDpOnReturnValve, false );
					}
				}
			}
		}
		else
		{
			// Here user inputs Kv for the supply insert. Thus, thanks to flow we can deduce Dp on this insert. The Dp here is only on the insert and is not
			// the same as 'm_SelDp' (that is the Dp on both supply and return valves that users input).
			bDiffDpSV = false;
			dMaxDpSV = CalcDp( m_clIndSelTRVParams.m_dFlow, m_clIndSelTRVParams.m_dInsertKvValue, m_clIndSelTRVParams.m_WC.GetDens() );

			// Handle return valve according to selected mode.
			if( true == bInsert || RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode )
			{
				// With 'fForceFullOpen' set to 'true'.
				iReturnValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectReturnValve( &m_clIndSelTRVParams, (LPCTSTR)strRVType, 
						(LPCTSTR)strRVFamily, (LPCTSTR)strRVConnect, (LPCTSTR)strRVVersion, -1.0, true );
			}
		}
	}

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( ( false == bInsert || false == bInsertInKv ) && iSupplyValveFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_RESULTNBRNOTSAME;
	}

	std::vector<CString> *pvecTitleToUser = NULL;

	if( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType || RIT_Heimeier == m_clIndSelTRVParams.m_eInsertType )
	{
		if( RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
		{
			pvecTitleToUser = &m_vecOutputTitles;
		}
		else
		{
			pvecTitleToUser = &m_vecOutputFlowLimitedTitles;
		}
	}
	else
	{
		pvecTitleToUser = &m_vecOutputOtherInsertTitles;
	}

	int iLoopResultInInputFile = 0;

	if( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType || RIT_Heimeier == m_clIndSelTRVParams.m_eInsertType )
	{
		for( CSelectedValve *pclSelectedSupplyValve = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedSupplyValve; 
				pclSelectedSupplyValve = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetNext<CSelectedValve>() )
		{
			// Prepare vector with all value to verify.
			_PrepareResultData( &m_clIndSelTRVParams, pclSelectedSupplyValve, iLoopResultInInputFile );

			if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
			{
				return TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BADEXTRACT;
			}

			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
							pvecTitleToUser->at( iLoop ), m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

					return TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_RESULTDIFF;
				}
			}

			iLoopResultInInputFile++;
		}
	}
	else
	{
		// Prepare vector with all value to verify.
		_PreprareOtherInsertResultData( &m_clIndSelTRVParams, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						pvecTitleToUser->at( iLoop ), m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelTRVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = TRV\n\n") );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSelTRVParams *pclIndSelTRVParams = dynamic_cast<CIndSelTRVParams *>( pclProdSelParams );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("IsTrvTypePreset = %s\n"), ( false == pclIndSelTRVParams->m_bIsTrvTypePreset ) ? _T("false") : _T("true") );

	if( RadiatorValveType::RVT_Standard == pclIndSelTRVParams->m_eValveType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ValveType = Standard\n") );
	}
	else if( RadiatorValveType::RVT_Presettable == pclIndSelTRVParams->m_eValveType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ValveType = Presettable\n") );
	}
	else if( RadiatorValveType::RVT_WithFlowLimitation == pclIndSelTRVParams->m_eValveType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ValveType = WithFlowLimitation\n") );
	}
	else if( RadiatorValveType::RVT_Inserts == pclIndSelTRVParams->m_eValveType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ValveType = Inserts\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ValveType = Unknown\n") );
	}

	if( RadiatorInsertType::RIT_Heimeier == pclIndSelTRVParams->m_eInsertType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("InsertType = Heimeier\n") );
	}
	else if( RadiatorInsertType::RIT_ByKv == pclIndSelTRVParams->m_eInsertType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("InsertType = ByKv\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("InsertType = Unknown\n") );
	}

	if( RadiatorReturnValveMode::RRVM_Nothing == pclIndSelTRVParams->m_eReturnValveMode )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ReturnValveMode = Nothing\n") );
	}
	else if( RadiatorReturnValveMode::RRVM_Other == pclIndSelTRVParams->m_eReturnValveMode )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ReturnValveMode = Other\n") );
	}
	else if( RadiatorReturnValveMode::RRVM_IMI == pclIndSelTRVParams->m_eReturnValveMode )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ReturnValveMode = IMI\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ReturnValveMode = Unknown\n") );
	}

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("HeadType = %s\n"), ( true == pclIndSelTRVParams->m_bIsThermostaticHead ) ? _T("Thermostatic") : _T("ElectroActuator") );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("InsertKvValue = %.13g\n"), pclIndSelTRVParams->m_dInsertKvValue );

	CString strString = pclIndSelTRVParams->m_strSVTypeID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("SVTypeID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboSVFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboSVFamilyID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboSVConnectID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboSVConnectID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboSVVersionID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboSVVersionID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboRVFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboRVFamilyID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboRVConnectID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboRVConnectID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboRVVersionID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboRVVersionID = %s\n"), strString );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboSVInsertName = %s\n"), pclIndSelTRVParams->m_strComboSVInsertName.MakeLower() );

	strString = pclIndSelTRVParams->m_strComboSVInsertFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboSVInsertFamilyID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboSVInsertTypeID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboSVInsertTypeID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboRVInsertFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboRVInsertFamilyID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboActuatorTypeID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboActuatorTypeID = %s\n"), strString );

	strString = pclIndSelTRVParams->m_strComboActuatorFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboActuatorFamilyID = %s\n"), strString );
	
	strString = pclIndSelTRVParams->m_strComboActuatorVersionID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboActuatorVersionID = %s\n"), strString );
	
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );

	if( NULL != pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		if( RadiatorValveType::RVT_Inserts != pclIndSelTRVParams->m_eValveType || RIT_Heimeier == pclIndSelTRVParams->m_eInsertType )
		{
			int iResultCount = 0;

			for( CSelectedValve *pclSelectedSupplyValve = pclIndSelTRVParams->m_pclSelectSupplyValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedSupplyValve; 
					pclSelectedSupplyValve = pclIndSelTRVParams->m_pclSelectSupplyValveList->GetNext<CSelectedValve>() )
			{
				_PrepareResultData( pclIndSelTRVParams, pclSelectedSupplyValve, iResultCount++ );

				uiErrorCode = WriteVecResultInFile( clTUFileHelper );

				if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
				{
					return uiErrorCode;
				}
			}
		}
		else
		{
			_PreprareOtherInsertResultData( pclIndSelTRVParams, 0 );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelTRVHelper::GetOutputTitlesNbr( std::vector<CString> *pvecString )
{
	int iOutputTitlesNbr = 0;

	if( 0 != m_mapInputs.count( _T("valvetype" ) ) )
	{
		if( 0 == m_mapInputs[_T("valvetype")].Compare( _T("standard") ) 
				|| 0 == m_mapInputs[_T("valvetype")].Compare( _T("presettable") ) )
		{
			iOutputTitlesNbr = (int)m_vecOutputTitles.size();
		}
		else if( 0 == m_mapInputs[_T("valvetype")].Compare( _T("withflowlimitation") ) )
		{
			iOutputTitlesNbr = (int)m_vecOutputFlowLimitedTitles.size();
		}
		else if( 0 == m_mapInputs[_T("valvetype")].Compare( _T("inserts") ) )
		{
			if( 0 == m_mapInputs[_T("inserttype")].Compare( _T("heimeier") ) )
			{
				iOutputTitlesNbr = (int)m_vecOutputTitles.size();
			}
			else
			{
				iOutputTitlesNbr = (int)m_vecOutputOtherInsertTitles.size();
			}
		}
	}

	return iOutputTitlesNbr;
}

void CTUIndSelTRVHelper::_PrepareResultData( CIndSelTRVParams *pclIndSelTRVParams, CSelectedValve *pclSelectedValve,
		int iResultCount )
{
	m_vecResultData.clear();

	// If user doesn't want insert or wants insert but Heimeier.
	if( RadiatorValveType::RVT_Inserts != pclIndSelTRVParams->m_eValveType || RIT_Heimeier == pclIndSelTRVParams->m_eInsertType )
	{
		if( RVT_WithFlowLimitation != pclIndSelTRVParams->m_eValveType )
		{
			_PreprareSupplyValveResultData( pclIndSelTRVParams, pclSelectedValve, iResultCount );
		}
		else
		{
			_PreprareSupplyFlowLimitedResultData( pclIndSelTRVParams, pclSelectedValve, iResultCount );
		}
	}
}

void CTUIndSelTRVHelper::_PreprareSupplyValveResultData( CIndSelTRVParams *pclIndSelTRVParams, CSelectedValve *pclSelectedValve,
		int iResultCount )
{
	CDB_ThermostaticValve *pclSupplyValve = dynamic_cast<CDB_ThermostaticValve *>( pclSelectedValve->GetpData() );

	if( NULL == pclSupplyValve )
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclSupplyValve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclSupplyValve->GetName() );

	// Valve connection.
	m_vecResultData.push_back( pclSupplyValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclSupplyValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclSupplyValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclSupplyValve->GetSize() );
	
	// Valve computed setting.
	strValue.Format( _T("%g"), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_ThermoCharacteristic *pclThermoCharacteristic = pclSupplyValve->GetThermoCharacteristic();
	
	if( NULL != pclThermoCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclThermoCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclThermoCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Valve Dp.
	strValue.Format( _T("%g"), pclSelectedValve->GetDp() );
	m_vecResultData.push_back( strValue );

	// Valve Dp full opening.
	strValue.Format( _T("%g"), pclSelectedValve->GetDpFullOpen() );
	m_vecResultData.push_back( strValue );

	// Valve Kv.
	double dKv = -1.0;
	int iDeltaT = ( true == pclIndSelTRVParams->m_bIsThermostaticHead ) ? 2 : 0;

	if( NULL != pclThermoCharacteristic )
	{
		dKv = pclThermoCharacteristic->GetKv( pclSelectedValve->GetH(), iDeltaT );
	}

	strValue.Format( _T("%g"), dKv );
	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclSupplyValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelTRVParams );
	pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList()->GetMatchingPipe( pclSupplyValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

void CTUIndSelTRVHelper::_PreprareSupplyFlowLimitedResultData( CIndSelTRVParams *pclIndSelTRVParams, CSelectedValve *pclSelectedValve,
		int iResultCount )
{
	CDB_FlowLimitedControlValve *pclSupplyValve = dynamic_cast<CDB_FlowLimitedControlValve *>( pclSelectedValve->GetpData() );

	if( NULL == pclSupplyValve )
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclSupplyValve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclSupplyValve->GetName() );

	// Valve connection.
	m_vecResultData.push_back( pclSupplyValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclSupplyValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclSupplyValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclSupplyValve->GetSize() );
	
	// Valve computed setting.
	strValue.Format( _T("%g"), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_FLCVCharacteristic *pclFLCVCharacteristic = pclSupplyValve->GetFLCVCharacteristic();
	
	if( NULL != pclFLCVCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclFLCVCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclFLCVCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Valve Dpmin.
	double dDpmin = -1.0;

	if( NULL != pclFLCVCharacteristic )
	{
		dDpmin = pclFLCVCharacteristic->GetDpmin( pclSelectedValve->GetH() );
	}

	strValue.Format( _T("%g"), dDpmin );
	m_vecResultData.push_back( strValue );

	// Valve flow range.
	strValue = GetDashDotDash();
	
	if( NULL != pclFLCVCharacteristic )
	{
		strValue.Format( _T("%g/%g"), pclFLCVCharacteristic->GetQLFmin(), pclFLCVCharacteristic->GetQNFmax() );
	}

	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclSupplyValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelTRVParams );
	pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList()->GetMatchingPipe( pclSupplyValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

void CTUIndSelTRVHelper::_PreprareOtherInsertResultData( CIndSelTRVParams *pclIndSelTRVParams, int iResultCount )
{
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Insert in Kv.
	strValue.Format( _T("%g"), pclIndSelTRVParams->m_dInsertKvValue );
	m_vecResultData.push_back( strValue );

	// Insert Dp.
	double dFlow = pclIndSelTRVParams->m_dFlow;
	double dKv = pclIndSelTRVParams->m_dInsertKvValue;
	double dSVOtherInsertDp = 0.0;

	if( dFlow > 0.0 && dKv > 0.0 )
	{
		dSVOtherInsertDp = CalcDp( dFlow, dKv, pclIndSelTRVParams->m_WC.GetDens() );
	}

	strValue.Format( _T("%g"), dSVOtherInsertDp );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for safety valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUIndSelSafetyValveHelper::CTUIndSelSafetyValveHelper() 
		: CTUProdSelHelper( m_clIndSelSafetyValveParams )
{
	m_clIndSelSafetyValveParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_SafetyValve;
	m_clIndSelSafetyValveParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("ID") );
	m_vecOutputTitles.push_back( _T("Name") );
	m_vecOutputTitles.push_back( _T("Number") );
	m_vecOutputTitles.push_back( _T("Body material") );
	m_vecOutputTitles.push_back( _T("Set pressure") );
	m_vecOutputTitles.push_back( _T("Medium name") );
	m_vecOutputTitles.push_back( _T("Version") );
	m_vecOutputTitles.push_back( _T("Lifting type") );
	m_vecOutputTitles.push_back( _T("Connection") );
	m_vecOutputTitles.push_back( _T("Size") );
	m_vecOutputTitles.push_back( _T("Power") );
	m_vecOutputTitles.push_back( _T("Power 100%") );
	m_vecOutputTitles.push_back( _T("Temperature range") );
}

CTUIndSelSafetyValveHelper::~CTUIndSelSafetyValveHelper()
{
	if( NULL != m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList )
	{
		delete m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList;
		m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList = NULL;
	}
}

UINT CTUIndSelSafetyValveHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUProdSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_clIndSelSafetyValveParams.m_eSystemApplicationType' variable.
	// Remark: Mandatory.
	// We need here also to write the 'm_eApplicationType' variable in the 'CProductSelectionParameters' base class.
	if( 0 != m_mapInputs.count( _T("systemapplicationtype") ) )
	{
		if( 0 == m_mapInputs[_T("systemapplicationtype" )].Compare( _T( "heating") ) )
		{
			m_clIndSelSafetyValveParams.m_eSystemApplicationType = ProjectType::Heating;
			m_clIndSelSafetyValveParams.m_eApplicationType = ProjectType::Heating;
		}
		else if( 0 == m_mapInputs[_T("systemapplicationtype" )].Compare( _T( "cooling") ) )
		{
			m_clIndSelSafetyValveParams.m_eSystemApplicationType = ProjectType::Cooling;
			m_clIndSelSafetyValveParams.m_eApplicationType = ProjectType::Cooling;
		}
		else if( 0 == m_mapInputs[_T("systemapplicationtype" )].Compare( _T( "solar") ) )
		{
			m_clIndSelSafetyValveParams.m_eSystemApplicationType = ProjectType::Solar;
			m_clIndSelSafetyValveParams.m_eApplicationType = ProjectType::Solar;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_SYSTEMAPPTYPEBAD;
		}
	}

	// The 'm_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID' variable.
	// Remark: mandatory if application type is heating.
	if( ProjectType::Heating == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		if( 0 != m_mapInputs.count( _T("systemheatgeneratortypeid") ) )
		{
			m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID = m_mapInputsWithCase[_T("systemheatgeneratortypeid")];
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_HEATGENTYPEIDEMPTY;
		}
	}

	// The 'm_clIndSelSafetyValveParams.m_strNormID' variable.
	// Remark: mandatory if application type is cooling.
	if( ProjectType::Cooling == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		if( 0 != m_mapInputs.count( _T("normid") ) )
		{
			m_clIndSelSafetyValveParams.m_strNormID = m_mapInputsWithCase[_T("normid")];
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_NORMIDEMPTY;
		}
	}

	// The 'm_clIndSelSafetyValveParams.m_dInstalledPower' variable.
	// Remark: mandatory.
	if( 0 != m_mapInputs.count( _T("installedpower") ) )
	{
		m_clIndSelSafetyValveParams.m_dInstalledPower = _wtof( m_mapInputs[_T("installedpower")] );
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_INSTALLEDPOWEREMPTY;
	}

	// The 'm_clIndSelSafetyValveParams.m_dInstalledCollector' variable.
	// Remark: mandatory if application type is solar.
	if( ProjectType::Solar == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		if( 0 != m_mapInputs.count( _T("installedcollector") ) )
		{
			m_clIndSelSafetyValveParams.m_dInstalledCollector = _wtof( m_mapInputs[_T("installedcollector")] );
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_INSTALLEDCOLLECTOREMPTY;
		}
	}

	// The 'm_clIndSelSafetyValveParams.m_strSafetyValveFamilyID' variable.
	// Remark: Not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("safetyvalvefamilyid") ) )
	{
		m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID = m_mapInputsWithCase[_T("safetyvalvefamilyid")];
	}

	// The 'm_clIndSelSafetyValveParams.m_strSafetyValveConnectionID' variable.
	// Remark: Not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("safetyvalveconnectionid") ) )
	{
		m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID = m_mapInputsWithCase[_T("safetyvalveconnectionid")];
	}

	// The 'm_clIndSelSafetyValveParams.m_dUserSetPressureChoice' variable.
	// Remark: mandatory.
	if( 0 != m_mapInputs.count( _T("usersetpressurechoice") ) )
	{
		m_clIndSelSafetyValveParams.m_dUserSetPressureChoice = _wtof( m_mapInputs[_T("usersetpressurechoice")] );
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_USERSETPRESSURECHOICEEMPTY;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelSafetyValveHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUProdSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( ProjectType::Heating == m_clIndSelSafetyValveParams.m_eSystemApplicationType
			&& false == m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID.IsEmpty()
			&& _NULL_IDPTR == m_clIndSelSafetyValveParams.m_pTADB->Get( m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_HEATGENERATORTYPEIDINVALID;
	}

	if( ProjectType::Cooling == m_clIndSelSafetyValveParams.m_eSystemApplicationType
		&& false == m_clIndSelSafetyValveParams.m_strNormID.IsEmpty()
		&& _NULL_IDPTR == m_clIndSelSafetyValveParams.m_pTADB->Get( m_clIndSelSafetyValveParams.m_strNormID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_NORMIDINVALID;
	}

	if( true == m_clIndSelSafetyValveParams.m_dInstalledPower <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_INSTALLEDPOWERNOVALID;
	}

	if( ProjectType::Solar == m_clIndSelSafetyValveParams.m_eSystemApplicationType
			&& m_clIndSelSafetyValveParams.m_dInstalledCollector <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_INSTALLEDCOLLECTORNOVALID;
	}

	if( false == m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID.IsEmpty()
			&& _NULL_IDPTR == m_clIndSelSafetyValveParams.m_pTADB->Get( m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_FAMILYIDINVALID;
	}

	if( false == m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID.IsEmpty()
		&& _NULL_IDPTR == m_clIndSelSafetyValveParams.m_pTADB->Get( m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_CONNECTIDINVALID;
	}

	if( true == m_clIndSelSafetyValveParams.m_dUserSetPressureChoice <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_USERSETPRESSURENOVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelSafetyValveHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_NOINPUT;
	}

	if( NULL != m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList )
	{
		delete m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList;
	}

	m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList = new CSelectSafetyValveList();

	if( NULL == m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_CANTCREATESELECTLIST;
	}

	m_clIndSelSafetyValveParams.m_SafetyValveList.PurgeAll();

	m_clIndSelSafetyValveParams.m_pTADB->GetSafetyValveList( 
				&m_clIndSelSafetyValveParams.m_SafetyValveList, 
				m_clIndSelSafetyValveParams.m_eSystemApplicationType,
				(LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID, 
				(LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID,
				m_clIndSelSafetyValveParams.m_dUserSetPressureChoice,
				m_clIndSelSafetyValveParams.m_eFilterSelection );

	bool bBestFound = false;
	int iDevFound = m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList->SelectSafetyValve( &m_clIndSelSafetyValveParams, bBestFound );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_RESULTNBRNOTSAME;
	}
	
	int iLoopResultInInputFile = 0;

	for( CSelectedSafetyValve *pclSelectedSafetyValve = m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList->GetFirst<CSelectedSafetyValve>(); NULL != pclSelectedSafetyValve; 
			pclSelectedSafetyValve = m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList->GetNext<CSelectedSafetyValve>() )
	{
		// Prepare vector with all values to verify.
		_PrepareResultData( &m_clIndSelSafetyValveParams, pclSelectedSafetyValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelSafetyValveHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = Safety valve\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUProdSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSelSafetyValveParams *pclIndSelSafetyValveParams = dynamic_cast<CIndSelSafetyValveParams *>( pclProdSelParams );

	if( ProjectType::Heating == pclIndSelSafetyValveParams->m_eSystemApplicationType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("SystemApplicationType = Heating\n") );
	}
	else if( ProjectType::Cooling == pclIndSelSafetyValveParams->m_eSystemApplicationType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("SystemApplicationType = Cooling\n") );
	}
	else if( ProjectType::Solar == pclIndSelSafetyValveParams->m_eSystemApplicationType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("SystemApplicationType = Solar\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("SystemApplicationType = Unknown\n") );
	}

	CString strSystemHeatGeneratorID = ( ProjectType::Heating == pclIndSelSafetyValveParams->m_eSystemApplicationType ) ? pclIndSelSafetyValveParams->m_strSystemHeatGeneratorTypeID : _T( "" );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("SystemHeatGeneratorTypeID = %s\n"), strSystemHeatGeneratorID );

	CString strNormID = ( ProjectType::Cooling == pclIndSelSafetyValveParams->m_eSystemApplicationType ) ? pclIndSelSafetyValveParams->m_strNormID : _T( "" );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("NormID = %s\n"), strNormID );
	
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("InstalledPower = %.13g\n"), pclIndSelSafetyValveParams->m_dInstalledPower );

	double dInstalledCollector = ( ProjectType::Solar == pclIndSelSafetyValveParams->m_eSystemApplicationType ) ? pclIndSelSafetyValveParams->m_dInstalledCollector : 0.0;
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("InstalledCollector = %.13g\n"), dInstalledCollector );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("SafetyValveFamilyID = %s\n"), pclIndSelSafetyValveParams->m_strSafetyValveFamilyID );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("SafetyValveConnectionID = %s\n"), pclIndSelSafetyValveParams->m_strSafetyValveConnectionID );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("UserSetPressureChoice = %.13g\n"), pclIndSelSafetyValveParams->m_dUserSetPressureChoice );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	pclIndSelSafetyValveParams = dynamic_cast<CIndSelSafetyValveParams *>( pclProdSelParams );

	if( NULL != pclIndSelSafetyValveParams->m_pclSelectSafetyValveList )
	{
		int iResultCount = 0;

		for( CSelectedSafetyValve *pclSelectedSafetyValve = pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetFirst<CSelectedSafetyValve>(); NULL != pclSelectedSafetyValve; 
				pclSelectedSafetyValve = pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetNext<CSelectedSafetyValve>() )
		{
			_PrepareResultData( pclIndSelSafetyValveParams, pclSelectedSafetyValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelSafetyValveHelper::_PrepareResultData( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, CSelectedSafetyValve *pclSelectedSafetyValve,
		int iResultCount )
{
	m_vecResultData.clear();
	CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( pclSelectedSafetyValve->GetpData() );

	if( NULL == pclSafetyValve ) 
	{
		return;
	}
	
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Safety valve ID.
	m_vecResultData.push_back( pclSafetyValve->GetIDPtr().ID );
	
	// Safety valve name.
	m_vecResultData.push_back( pclSafetyValve->GetName() );

	// Safety valve needed number.
	strValue.Format( _T("%i"), pclSelectedSafetyValve->GetQuantityNeeded() );
	m_vecResultData.push_back( strValue );

	// Safety valve body material.
	m_vecResultData.push_back( pclSafetyValve->GetBodyMaterial() );

	// Safety valve set pressure.
	strValue.Format( _T("%g"), pclSafetyValve->GetSetPressure() );
	m_vecResultData.push_back( strValue );

	// Safety valve medium name.
	m_vecResultData.push_back( pclSafetyValve->GetMediumName() );

	// Safety valve version.
	m_vecResultData.push_back( pclSafetyValve->GetVersion() );

	// Safety valve lifting type.
	m_vecResultData.push_back( pclSafetyValve->GetLiftingType() );

	// Safety valve connection.
	m_vecResultData.push_back( pclSafetyValve->GetInOutletConnectionString() );

	// Safety valve size.
	m_vecResultData.push_back( pclSafetyValve->GetInOutletSizeString() );

	// Safety valve power.
	strValue.Format( _T("%g"), pclIndSelSafetyValveParams->m_dInstalledPower );
	m_vecResultData.push_back( strValue );

	// Safety valve power 100%.
	strValue.Format( _T("%g"), pclSafetyValve->GetPowerLimit( pclIndSelSafetyValveParams->m_strSystemHeatGeneratorTypeID, pclIndSelSafetyValveParams->m_strNormID ) );
	m_vecResultData.push_back( strValue );

	// Safety valve temperature range.
	m_vecResultData.push_back( pclSafetyValve->GetTempRange() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for TA-6-way control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUIndSel6WayValveHelper::CTUIndSel6WayValveHelper()
	: CTUIndSelCtrlBaseHelper( m_clIndSel6WayValveParams )
{
	m_clIndSel6WayValveParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_6WayControlValve;
	m_clIndSel6WayValveParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	// 6-way valve.
	m_vecOutputTitles.push_back( _T( "Result number" ) );
	m_vecOutputTitles.push_back( _T( "Valve ID" ) );
	m_vecOutputTitles.push_back( _T( "Valve name" ) );
	m_vecOutputTitles.push_back( _T( "Valve body material" ) );
	m_vecOutputTitles.push_back( _T( "Valve connection" ) );
	m_vecOutputTitles.push_back( _T( "Valve version" ) );
	m_vecOutputTitles.push_back( _T( "Valve PN" ) );
	m_vecOutputTitles.push_back( _T( "Valve size" ) );
	m_vecOutputTitles.push_back( _T( "Dp cooling" ) );
	m_vecOutputTitles.push_back( _T( "Dp heating" ) );
	m_vecOutputTitles.push_back( _T( "Valve temperature range" ) );
	m_vecOutputTitles.push_back( _T( "Pipe size" ) );

	// PIBCV eqm.
	m_vecOutputSecondEQMTitles.push_back( _T( "Result number" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "6-way valve ID" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve ID" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve name" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve body material" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve connection" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve version" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve PN" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve size" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve rangeability" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve leakage rate" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve Dpmin" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve stroke length" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve control characteristic" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve push or pull to close" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Is Dpmax checkbox checked?" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve Dp max" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Valve temperature range" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Pipe size" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Pipe linear Dp" ) );
	m_vecOutputSecondEQMTitles.push_back( _T( "Pipe velocity" ) );

	// PIBCV on/off cooling / heating.
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Result number" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve ID" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "6-way valve ID" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve name" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve body material" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve connection" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve version" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve PN" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve size" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve computed presetting" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve displayed presetting" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve rangeability" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve leakage rate" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve Dpmin" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve stroke length" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve control characteristic" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve push or pull to close" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Is Dpmax checkbox checked?" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve Dp max" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Valve temperature range" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Pipe size" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Pipe linear Dp" ) );
	m_vecOutputSecondOnoffPIBCVTitles.push_back( _T( "Pipe velocity" ) );

	// On/off with bv cooling.
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Result number" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve ID" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "6-way valve ID" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve name" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve body material" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve connection" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve version" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve PN" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve size" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve computed setting" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve displayed setting" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve Dp" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve Dp at full opening" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve Dp at half opening" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Valve temperature range" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Pipe size" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Pipe linear Dp" ) );
	m_vecOutpuSecondOnoffBvTitles.push_back( _T( "Pipe velocity" ) );
}

CTUIndSel6WayValveHelper::~CTUIndSel6WayValveHelper()
{
	if( NULL != m_clIndSel6WayValveParams.m_pclSelect6WayValveList )
	{
		delete m_clIndSel6WayValveParams.m_pclSelect6WayValveList;
		m_clIndSel6WayValveParams.m_pclSelect6WayValveList = NULL;
	}
}

UINT CTUIndSel6WayValveHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_CoolingWC.m_dTemp' variable.
	// Remark: not mandatory. By default = '6.0'.
	if( 0 != m_mapInputs.count( _T( "wctempcooling" ) ) )
	{
		m_clIndSel6WayValveParams.GetCoolingWaterChar().SetTemp( _wtof( m_mapInputs[_T( "wctempcooling" )] ) );
	}

	// The 'm_HeatingWC.m_dTemp' variable.
	// Remark: not mandatory. By default = '20.0'.
	if( 0 != m_mapInputs.count( _T( "wctempheating" ) ) )
	{
		m_clIndSel6WayValveParams.GetHeatingWaterChar().SetTemp( _wtof( m_mapInputs[_T( "wctempheating" )] ) );
	}

	// The 'm_CoolingWC.dPcWeight' variable.
	// Remark: not mandatory. By default = '0.0'.
	if( 0 != m_mapInputs.count( _T( "wcpcweightcooling" ) ) )
	{
		m_clIndSel6WayValveParams.GetCoolingWaterChar().SetPcWeight( _wtof( m_mapInputs[_T( "wcpcweighcooling" )] ) );
	}

	// The 'm_HeatingWC.dPcWeight' variable.
	// Remark: not mandatory. By default = '0.0'.
	if( 0 != m_mapInputs.count( _T( "wcpcweightheating" ) ) )
	{
		m_clIndSel6WayValveParams.GetHeatingWaterChar().SetPcWeight( _wtof( m_mapInputs[_T( "wcpcweigheating" )] ) );
	}
	m_clIndSel6WayValveParams.m_CoolingWC.UpdateFluidData( m_clIndSel6WayValveParams.m_CoolingWC.GetTemp(), m_clIndSel6WayValveParams.m_CoolingWC.GetPcWeight() );

	m_clIndSel6WayValveParams.m_HeatingWC.UpdateFluidData( m_clIndSel6WayValveParams.m_HeatingWC.GetTemp(), m_clIndSel6WayValveParams.m_HeatingWC.GetPcWeight() );

	// The 'm_e6WayValveSelectionMode' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T( "selectionmode" ) ) )
	{
		if( 0 == m_mapInputs[_T( "selectionmode" )].Compare(_T("alone" ) ) )
		{
			m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = e6Way_Alone;
		}
		else if( 0 == m_mapInputs[_T( "selectionmode" )].Compare( _T( "eqm" ) ) )
		{
			m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = e6Way_EQMControl;
		}
		else if( 0 == m_mapInputs[_T( "selectionmode" )].Compare( _T( "onoff pibcv" ) ) )
		{
			m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = e6Way_OnOffControlWithPIBCV;
		}
		else if( 0 == m_mapInputs[_T( "selectionmode" )].Compare( _T( "onoff bv" ) ) )
		{
			m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = e6Way_OnOffControlWithSTAD;
		}
	}

	// The 'm_bCheckFastConnection' variable.
	// Remark: mandatory if 'groupkvsordpchecked' = 'true'.
	if( 0 != m_mapInputs.count( _T( "fastelecconnenabled" ) ) )
	{
		if( 0 == m_mapInputs[_T( "fastelecconnenabled" )].Compare( _T( "true") ) )
		{
			m_clIndSel6WayValveParams.m_bCheckFastConnection = true;
		}
		else
		{
			m_clIndSel6WayValveParams.m_bCheckFastConnection = false;
		}
	}

	// The 'm_dCoolingFlow' variable.
	if( 0 != m_mapInputs.count( _T( "flowcooling" ) ) )
	{
		m_clIndSel6WayValveParams.m_dCoolingFlow = _wtof( m_mapInputs[_T( "flowcooling" )] );
	}

	// The 'm_dHeatingFlow' variable.
	if( 0 != m_mapInputs.count( _T( "flowheating" ) ) )
	{
		m_clIndSel6WayValveParams.m_dHeatingFlow = _wtof( m_mapInputs[_T( "flowheating" )] );
	}

	if( 0 != m_mapInputs.count( _T( "powercooling" ) ) )
	{
		m_clIndSel6WayValveParams.m_dCoolingPower = _wtof( m_mapInputs[_T( "powercooling" )] );
	}
	if( 0 != m_mapInputs.count( _T( "powerheating" ) ) )
	{
		m_clIndSel6WayValveParams.m_dHeatingPower = _wtof( m_mapInputs[_T( "powerheating" )] );
	}
	if( 0 != m_mapInputs.count( _T( "dtcooling" ) ) )
	{
		m_clIndSel6WayValveParams.m_dCoolingDT = _wtof( m_mapInputs[_T( "dtcooling" )] );
	}
	if( 0 != m_mapInputs.count( _T( "dtheating" ) ) )
	{
		m_clIndSel6WayValveParams.m_dHeatingDT = _wtof( m_mapInputs[_T( "dtheating" )] );
	}

	// The 'm_strComboConnectID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "6wcomboconnectid" ) ) )
	{
		m_clIndSel6WayValveParams.m_strComboConnectID = m_mapInputsWithCase[_T( "6wcomboconnectid" )];
	}

	// The 'm_strComboVersionID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "6wcomboversionid" ) ) )
	{
		m_clIndSel6WayValveParams.m_strComboVersionID = m_mapInputsWithCase[_T( "6wcomboversionid" )];
	}

	if( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_EQMControl 
		|| m_clIndSel6WayValveParams.m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_OnOffControlWithPIBCV )
	{
		UINT uiErrorCode = InterpretePIBCVInputs( &m_clIndSel6WayValveParams.m_clIndSelPIBCVParams );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
	}
	else if( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_OnOffControlWithSTAD )
	{
		UINT uiErrorCode = InterpreteBvInputs( &m_clIndSel6WayValveParams.m_clIndSelBVParams );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSel6WayValveHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( _NULL_IDPTR == m_clIndSel6WayValveParams.m_pTADB->Get( m_clIndSel6WayValveParams.GetCoolingWaterChar().GetAdditFamID() ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDITFAMIDINVALID;
	}

	if( _NULL_IDPTR == m_clIndSel6WayValveParams.m_pTADB->Get( m_clIndSel6WayValveParams.GetCoolingWaterChar().GetAdditID() ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDITIDINVALID;
	}

	double dTemp = m_clIndSel6WayValveParams.GetCoolingWaterChar().GetTemp();
	CWaterChar::eFluidRetCode eFluidError = m_clIndSel6WayValveParams.GetCoolingWaterChar().CheckFluidData( dTemp );

	if( CWaterChar::efrcFluidOk != eFluidError )
	{
		if( CWaterChar::efrcTemperatureTooLow == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERTEMPTOOLOW;
		}
		else if( CWaterChar::efrcTempTooHigh == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERTEMPTOOHIGH;
		}
		else if( CWaterChar::efrcAdditiveTooHigh == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDTOOHIGH;
		}
	}

	dTemp = m_clIndSel6WayValveParams.GetHeatingWaterChar().GetTemp();
	eFluidError = m_clIndSel6WayValveParams.GetHeatingWaterChar().CheckFluidData( dTemp );

	if( CWaterChar::efrcFluidOk != eFluidError )
	{
		if( CWaterChar::efrcTemperatureTooLow == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERTEMPTOOLOW;
		}
		else if( CWaterChar::efrcTempTooHigh == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERTEMPTOOHIGH;
		}
		else if( CWaterChar::efrcAdditiveTooHigh == eFluidError )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDTOOHIGH;
		}
	}

	if( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode > e6WayValveSelectionMode::e6Way_Last
		|| m_clIndSel6WayValveParams.m_e6WayValveSelectionMode < e6WayValveSelectionMode::e6Way_Alone )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSIXWAYVALVE_INVALIDSELECTIONMODE;
	}

	if( m_clIndSel6WayValveParams.m_bCheckFastConnection == true 
		&& m_clIndSel6WayValveParams.m_e6WayValveSelectionMode != e6WayValveSelectionMode::e6Way_EQMControl )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELSIXWAYVALVE_FASTELECCONNNOTVALID;
	}

	if( CDS_SelProd::efdFlow == m_clIndSel6WayValveParams.m_eFlowOrPowerDTMode
		&& ( m_clIndSel6WayValveParams.m_dCoolingFlow <= 0.0 || m_clIndSel6WayValveParams.m_dHeatingFlow <= 0.0 ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_FLOWNOTVALID;
	}
	else if( CDS_SelProd::efdPower == m_clIndSel6WayValveParams.m_eFlowOrPowerDTMode )
	{
		if( m_clIndSel6WayValveParams.m_dCoolingPower <= 0.0 )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_POWERNOTVALID;
		}
		else if( m_clIndSel6WayValveParams.m_dCoolingDT <= 0.0 )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_DTNOTVALID;
		}
		else if( m_clIndSel6WayValveParams.m_dHeatingPower <= 0.0 )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_POWERNOTVALID;
		}
		else if( m_clIndSel6WayValveParams.m_dHeatingDT <= 0.0 )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_DTNOTVALID;
		}

		double dQ = CalcqFromPDT( m_clIndSel6WayValveParams.m_dCoolingPower, m_clIndSel6WayValveParams.m_dCoolingDT, m_clIndSel6WayValveParams.GetCoolingWaterChar().GetDens(),
			m_clIndSel6WayValveParams.GetCoolingWaterChar().GetSpecifHeat() );

		if( dQ <= 0.0 )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_CANTCOMPUTEFLOW;
		} 
		m_clIndSel6WayValveParams.m_dCoolingFlow = dQ;

		dQ = CalcqFromPDT( m_clIndSel6WayValveParams.m_dHeatingPower, m_clIndSel6WayValveParams.m_dHeatingDT, m_clIndSel6WayValveParams.GetHeatingWaterChar().GetDens(),
			m_clIndSel6WayValveParams.GetHeatingWaterChar().GetSpecifHeat() );

		if( dQ <= 0.0 )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_CANTCOMPUTEFLOW;
		}

		m_clIndSel6WayValveParams.m_dHeatingFlow = dQ;
	}

	if( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_EQMControl
		|| m_clIndSel6WayValveParams.m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_OnOffControlWithPIBCV )
	{
		UINT uiErrorCode = VerifyPIBCVInputs( &m_clIndSel6WayValveParams );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
	}
	else if( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_OnOffControlWithSTAD )
	{
		UINT uiErrorCode = VerifyBVInputs( &m_clIndSel6WayValveParams );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSel6WayValveHelper::LaunchTest( CString& strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T( "" );

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_NOINPUT;
	}

	if( NULL != m_clIndSel6WayValveParams.m_pclSelect6WayValveList )
	{
		delete m_clIndSel6WayValveParams.m_pclSelect6WayValveList;
	}

	m_clIndSel6WayValveParams.m_pclSelect6WayValveList = new CSelect6WayValveList();

	if( NULL == m_clIndSel6WayValveParams.m_pclSelect6WayValveList || NULL == m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_CANTCREATESELECTLIST;
	}

	CWaterChar* pclWaterChar = NULL;
	double dBiggestFlow = 0.0;

	// To size pipe we need the biggest flow but also the corresponding fluid characteristic.
	if( m_clIndSel6WayValveParams.GetCoolingFlow() >= m_clIndSel6WayValveParams.GetHeatingFlow() )
	{
		dBiggestFlow = m_clIndSel6WayValveParams.GetCoolingFlow();
		pclWaterChar = &m_clIndSel6WayValveParams.GetCoolingWaterChar();
	}
	else
	{
		// Normally this case will never appear.
		dBiggestFlow = m_clIndSel6WayValveParams.GetHeatingFlow();
		pclWaterChar = &m_clIndSel6WayValveParams.GetHeatingWaterChar();
	}

	m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetSelectPipeList()->SelectPipes( &m_clIndSel6WayValveParams,
		max( m_clIndSel6WayValveParams.GetCoolingFlow(), m_clIndSel6WayValveParams.GetHeatingFlow() ), true, true, 0, 0, pclWaterChar );

	m_clIndSel6WayValveParams.m_e6WayValveList.PurgeAll();

	m_clIndSel6WayValveParams.m_pTADB->GetTaCVList(
		&m_clIndSel6WayValveParams.m_e6WayValveList,												// List where to saved
		CTADatabase::eFor6WayCV,								// Control valve target (cv, hmcv, picv or bcv)
		false, 													// 'true' returns as soon a result is found
		m_clIndSel6WayValveParams.m_eCV2W3W, 							// Set way number of valve
		_T( "" ),							 						// Type ID
		_T( "" ),			// Family ID
		_T( "" ),		// Body material ID
		(LPCTSTR)m_clIndSel6WayValveParams.m_strComboConnectID,		// Connection ID HYS-1252 : Now we use connectID instead of familyID
		(LPCTSTR)m_clIndSel6WayValveParams.m_strComboVersionID, 		// Version ID
		_T( "" ),				// PN ID
		CDB_ControlProperties::ControlOnly, 						// Set the control function (control only, presettable, ...)
		m_clIndSel6WayValveParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
		m_clIndSel6WayValveParams.m_eFilterSelection );

	bool bSizeShiftProblem = false;
	int iDevFound = 0;

	iDevFound = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->Select6WayValve( &m_clIndSel6WayValveParams, &bSizeShiftProblem );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}
	// Verify flows before display secondary valves
	bool bOK = _VerifyFlows( &m_clIndSel6WayValveParams, m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetFirst<CSelectedValve>() );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	if( true == bOK )
	{
		int iLoopResultInInputFile = 0;
		CSelectedValve* pclFristSelected6WayValve = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetFirst<CSelectedValve>();
		for( CSelectedValve* pclSelectedValve = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
			pclSelectedValve = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetNext<CSelectedValve>() )
		{
			// Prepare vector with all value to verify.
			_PrepareResultData( &m_clIndSel6WayValveParams, pclSelectedValve, iLoopResultInInputFile );

			if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
			{
				return TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_BADEXTRACT;
			}

			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
				{
					strErrorDetails.Format( _T( "Result %i: '%s' are not the same -> Input file: %s - Current result: %s" ), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

					return TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_RESULTDIFF;
				}
			}

			iLoopResultInInputFile++;
		}
		SideDefinition eSideDefinition;
		if( NULL != m_clIndSel6WayValveParams.m_pclSelect6WayValveList )
		{
			if( e6Way_EQMControl == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
			{
				eSideDefinition = BothSide;
				CSelectPICVList* pclSelectedPICvList = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetPICVList( eSideDefinition );
				if( NULL != pclSelectedPICvList )
				{
					for( CSelectedValve* pclSelectedValve = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = pclSelectedPICvList->GetNext<CSelectedValve>() )
					{
						// Prepare vector with all value to verify.
						_PrepareResultDataPIBCV( &m_clIndSel6WayValveParams, pclSelectedValve, pclFristSelected6WayValve, iLoopResultInInputFile, BothSide );

						if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
						{
							return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BADEXTRACT;
						}

						// Now just compare between results and input file.
						for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
						{
							if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
							{
								strErrorDetails.Format( _T( "Result %i: '%s' are not the same -> Input file: %s - Current result: %s" ), iLoopResultInInputFile,
									m_vecOutputSecondEQMTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

								return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_RESULTDIFF;
							}
						}

						iLoopResultInInputFile++;
					}
				}
			}
			else if( e6Way_OnOffControlWithPIBCV == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
			{
				eSideDefinition = CoolingSide;
				CSelectPICVList* pclSelectedPICvList = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetPICVList( eSideDefinition );
				if( NULL != pclSelectedPICvList )
				{
					for( CSelectedValve* pclSelectedValve = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = pclSelectedPICvList->GetNext<CSelectedValve>() )
					{
						// Prepare vector with all value to verify.
						_PrepareResultDataPIBCV( &m_clIndSel6WayValveParams, pclSelectedValve, pclFristSelected6WayValve, iLoopResultInInputFile, eSideDefinition );

						if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
						{
							return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BADEXTRACT;
						}

						// Now just compare between results and input file.
						for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
						{
							if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
							{
								strErrorDetails.Format( _T( "Result %i: '%s' are not the same -> Input file: %s - Current result: %s" ), iLoopResultInInputFile,
									m_vecOutputSecondOnoffPIBCVTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

								return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_RESULTDIFF;
							}
						}

						iLoopResultInInputFile++;
					}
				}
				eSideDefinition = HeatingSide;
				pclSelectedPICvList = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetPICVList( eSideDefinition );
				if( NULL != pclSelectedPICvList )
				{
					for( CSelectedValve* pclSelectedValve = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = pclSelectedPICvList->GetNext<CSelectedValve>() )
					{
						// Prepare vector with all value to verify.
						_PrepareResultDataPIBCV( &m_clIndSel6WayValveParams, pclSelectedValve, pclFristSelected6WayValve, iLoopResultInInputFile, eSideDefinition );

						if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
						{
							return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BADEXTRACT;
						}

						// Now just compare between results and input file.
						for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
						{
							if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
							{
								strErrorDetails.Format( _T( "Result %i: '%s' are not the same -> Input file: %s - Current result: %s" ), iLoopResultInInputFile,
									m_vecOutputSecondOnoffPIBCVTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

								return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_RESULTDIFF;
							}
						}

						iLoopResultInInputFile++;
					}
				}
			}
			else if( e6Way_OnOffControlWithSTAD == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
			{
				eSideDefinition = CoolingSide;
				CSelect6WayValveList* pcl6WayValveList = m_clIndSel6WayValveParams.m_pclSelect6WayValveList;

				// Retrieve the balancing valve list linked to current the 6-way valve.
				CSelectList* pclSelectedBalancingValveList = NULL;
				bool bSizeShiftProblem = false;
				eSideDefinition = CoolingSide;

				if( pcl6WayValveList->SelectBalancingValve( &m_clIndSel6WayValveParams, pclFristSelected6WayValve, eSideDefinition, &bSizeShiftProblem ) > 0 )
				{
					pclSelectedBalancingValveList = pcl6WayValveList->GetBVList( eSideDefinition );
				}
				if( NULL != pclSelectedBalancingValveList )
				{
					for( CSelectedValve* pclSelectedValve = pclSelectedBalancingValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = pclSelectedBalancingValveList->GetNext<CSelectedValve>() )
					{
						// Prepare vector with all value to verify.
						_PrepareResultDataBV( &m_clIndSel6WayValveParams, pclSelectedValve, pclFristSelected6WayValve, iLoopResultInInputFile, eSideDefinition );

						if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
						{
							return TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BADEXTRACT;
						}

						// Now just compare between results and input file.
						for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
						{
							if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
							{
								strErrorDetails.Format( _T( "Result %i: '%s' are not the same -> Input file: %s - Current result: %s" ), iLoopResultInInputFile,
									m_vecOutpuSecondOnoffBvTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

								return TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_RESULTDIFF;
							}
						}

						iLoopResultInInputFile++;
					}
				}
				eSideDefinition = HeatingSide;

				if( pcl6WayValveList->SelectBalancingValve( &m_clIndSel6WayValveParams, pclFristSelected6WayValve, eSideDefinition, &bSizeShiftProblem ) > 0 )
				{
					pclSelectedBalancingValveList = pcl6WayValveList->GetBVList( eSideDefinition );
				}
				if( NULL != pclSelectedBalancingValveList )
				{
					for( CSelectedValve* pclSelectedValve = pclSelectedBalancingValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = pclSelectedBalancingValveList->GetNext<CSelectedValve>() )
					{
						// Prepare vector with all value to verify.
						_PrepareResultDataBV( &m_clIndSel6WayValveParams, pclSelectedValve, pclFristSelected6WayValve, iLoopResultInInputFile, eSideDefinition );

						if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
						{
							return TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BADEXTRACT;
						}

						// Now just compare between results and input file.
						for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
						{
							if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
							{
								strErrorDetails.Format( _T( "Result %i: '%s' are not the same -> Input file: %s - Current result: %s" ), iLoopResultInInputFile,
									m_vecOutpuSecondOnoffBvTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

								return TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_RESULTDIFF;
							}
						}

						iLoopResultInInputFile++;
					}
				}
			}
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSel6WayValveHelper::DropOutSelection( CProductSelelectionParameters* pclProdSelParams, CTUProdSelFileHelper& clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "ProductCategory = TA-6-way valve\n\n" ) );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "StartInputs\n" ) );

	UINT uiErrorCode = CTUIndSelCtrlBaseHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CIndSel6WayValveParams* pclIndSel6WayValveParams = dynamic_cast<CIndSel6WayValveParams*>( pclProdSelParams );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "WCTempCooling = %.13g\n" ), pclIndSel6WayValveParams->GetCoolingWaterChar().GetTemp() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "WCTempHeating = %.13g\n" ), pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "WCPCWeightCooling = %.13g\n" ), pclIndSel6WayValveParams->GetCoolingWaterChar().GetPcWeight() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "WCPCWeightHeating = %.13g\n" ), pclIndSel6WayValveParams->GetHeatingWaterChar().GetPcWeight() );

	if( e6Way_Alone == pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SelectionMode = Alone\n" ) );
	}
	else if( e6Way_EQMControl == pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SelectionMode = EQM\n" ) );
	}
	else if( e6Way_OnOffControlWithPIBCV == pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SelectionMode = Onoff PIBCV\n" ) );
	}
	else if( e6Way_OnOffControlWithSTAD == pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SelectionMode = Onoff BV\n" ) );
	}

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "FastElecConnEnabled = %s\n" ), ( true == pclIndSel6WayValveParams->m_bCheckFastConnection ) ? _T( "true" ) : _T( "false" ) );

	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "FlowCooling = %.13g\n" ), pclIndSel6WayValveParams->GetCoolingFlow() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "FlowHeating = %.13g\n" ), pclIndSel6WayValveParams->GetHeatingFlow() );
	
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "PowerCooling = %.13g\n" ), pclIndSel6WayValveParams->GetCoolingPower() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "DTCooling = %.13g\n" ), pclIndSel6WayValveParams->GetCoolingDT() );	
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "PowerHeating = %.13g\n" ), pclIndSel6WayValveParams->GetHeatingPower() );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "DTHeating = %.13g\n" ), pclIndSel6WayValveParams->GetHeatingDT() );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "6WComboConnectID = %s\n" ), pclIndSel6WayValveParams->m_strComboConnectID );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "6WComboVersionID = %s\n" ), pclIndSel6WayValveParams->m_strComboVersionID );

	if( pclIndSel6WayValveParams->m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_EQMControl
		|| pclIndSel6WayValveParams->m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_OnOffControlWithPIBCV )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "\n\n#StartInputs PIBCV\n\n" ) );
		UINT uiErrorCode = DropOutSelectionPIBCV( &pclIndSel6WayValveParams->m_clIndSelPIBCVParams, clTUFileHelper );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
	}
	else if( pclIndSel6WayValveParams->m_e6WayValveSelectionMode == e6WayValveSelectionMode::e6Way_OnOffControlWithSTAD )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "\n\n#StartInputs BV\n\n" ) );
		UINT uiErrorCode = DropOutSelectionBV( &pclIndSel6WayValveParams->m_clIndSelBVParams, clTUFileHelper );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "EndInputs\n\n" ) );


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "StartOutputs\n" ) );

	int iResultCount = 0;

	if( NULL != pclIndSel6WayValveParams->m_pclSelect6WayValveList )
	{
		for( CSelectedValve* pclSelectedValve = pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
			pclSelectedValve = pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSel6WayValveParams, pclSelectedValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	// Verify flows before display secondary valves
	bool bOK = _VerifyFlows( pclIndSel6WayValveParams, pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>() );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	if( true == bOK )
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Extract all secondary results.
		CSelectPICVList* pclSelectedPICvList = pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( BothSide );


		if( NULL != pclSelectedPICvList && pclIndSel6WayValveParams->m_e6WayValveSelectionMode == e6Way_EQMControl )
		{
			TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#StartOutputs PIBCV\n" ) );

			for( CSelectedValve* pclSelectedValve = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
				pclSelectedValve = pclSelectedPICvList->GetNext<CSelectedValve>() )
			{
				_PrepareResultDataPIBCV( pclIndSel6WayValveParams, pclSelectedValve, pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(), iResultCount++, BothSide );

				uiErrorCode = WriteVecResultInFile( clTUFileHelper );

				if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
				{
					return uiErrorCode;
				}
			}
			TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#EndOutputs PIBCV\n\n" ) );
		}
		else if( pclIndSel6WayValveParams->m_e6WayValveSelectionMode == e6Way_OnOffControlWithPIBCV )
		{
			pclSelectedPICvList = pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( CoolingSide );
			if( NULL != pclSelectedPICvList )
			{
				TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#StartOutputs PIBCV Cooling\n" ) );

				for( CSelectedValve* pclSelectedValve = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
					pclSelectedValve = pclSelectedPICvList->GetNext<CSelectedValve>() )
				{
					_PrepareResultDataPIBCV( pclIndSel6WayValveParams, pclSelectedValve, pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(), iResultCount++, CoolingSide );

					uiErrorCode = WriteVecResultInFile( clTUFileHelper );

					if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
					{
						return uiErrorCode;
					}
				}
				TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#EndOutputs PIBCV Cooling\n\n" ) );
			}

			// heating
			pclSelectedPICvList = pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( HeatingSide );
			if( NULL != pclSelectedPICvList )
			{
				TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#StartOutputs PIBCV Heating\n" ) );

				for( CSelectedValve* pclSelectedValve = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
					pclSelectedValve = pclSelectedPICvList->GetNext<CSelectedValve>() )
				{
					_PrepareResultDataPIBCV( pclIndSel6WayValveParams, pclSelectedValve, pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(), iResultCount++, HeatingSide );

					uiErrorCode = WriteVecResultInFile( clTUFileHelper );

					if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
					{
						return uiErrorCode;
					}
				}
				TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#EndOutputs PIBCV Heating\n\n" ) );
			}
		}
		else if( pclIndSel6WayValveParams->m_e6WayValveSelectionMode == e6Way_OnOffControlWithSTAD )
		{
			CSelect6WayValveList* pcl6WayValveList = pclIndSel6WayValveParams->m_pclSelect6WayValveList;

			// Retrieve the balancing valve list linked to current the 6-way valve.
			CSelectList* pclSelectedBVList = NULL;
			bool bSizeShiftProblem = false;

			if( pcl6WayValveList->SelectBalancingValve( pclIndSel6WayValveParams, pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(), 
				CoolingSide, &bSizeShiftProblem ) > 0 )
			{
				pclSelectedBVList = pcl6WayValveList->GetBVList( CoolingSide );
			}

			if( NULL != pclSelectedBVList )
			{
				TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#StartOutputs BV Cooling\n" ) );

				for( CSelectedValve* pclSelectedValve = pclSelectedBVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
					pclSelectedValve = pclSelectedBVList->GetNext<CSelectedValve>() )
				{
					_PrepareResultDataBV( pclIndSel6WayValveParams, pclSelectedValve, pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(), iResultCount++, CoolingSide );

					uiErrorCode = WriteVecResultInFile( clTUFileHelper );

					if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
					{
						return uiErrorCode;
					}
				}
				TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#EndOutputs BV Cooling\n\n" ) );
			}

			// heating
			// Retrieve the balancing valve list linked to current the 6-way valve.
			pclSelectedBVList = NULL;
			bSizeShiftProblem = false;

			if( pcl6WayValveList->SelectBalancingValve( pclIndSel6WayValveParams, pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(),
				HeatingSide, &bSizeShiftProblem ) > 0 )
			{
				pclSelectedBVList = pcl6WayValveList->GetBVList( HeatingSide );
			}	

			if( NULL != pclSelectedBVList )
			{
				TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#StartOutputs BV Heating\n" ) );

				for( CSelectedValve* pclSelectedValve = pclSelectedBVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
					pclSelectedValve = pclSelectedBVList->GetNext<CSelectedValve>() )
				{
					_PrepareResultDataBV( pclIndSel6WayValveParams, pclSelectedValve, pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(), iResultCount++, HeatingSide );

					uiErrorCode = WriteVecResultInFile( clTUFileHelper );

					if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
					{
						return uiErrorCode;
					}
				}
				TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#EndOutputs BV Heating\n\n" ) );
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "EndOutputs\n\n" ) );

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSel6WayValveHelper::ReadOutputs( CTUProdSelFileHelper& clTUFileHelper, bool bUpdate )
{
	m_vecOutputValues.clear();

	CString strLine;
	TU_FILEHELPER_READLINE( clTUFileHelper, strLine );

	if( 0 != strLine.CompareNoCase( _T( "StartOutputs" ) ) )
	{
		return TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BADSTART;
	}

	bool bStop = false;
	std::vector<CString> vecStrings;

	do
	{
		UINT uiErrorCode = clTUFileHelper.ReadOneLineMultiValues( strLine, vecStrings, _T( ";" ) );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
		else if( 0 == strLine.CompareNoCase( _T( "EndOutputs" ) ) )
		{
			bStop = true;
		}
		else
		{
			if( false == bUpdate )
			{
				if( ( GetOutputTitlesNbr( &vecStrings ) != vecStrings.size() ) && ( vecStrings.size() != m_vecOutputSecondEQMTitles.size() )
					&& ( vecStrings.size() != m_vecOutputSecondOnoffPIBCVTitles.size() ) && ( vecStrings.size() != m_vecOutpuSecondOnoffBvTitles.size() ) )
				{
					return TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BADNBRINOUTPUT;
				}
			}

			m_vecOutputValues.push_back( vecStrings );
		}

	}
	while( false == bStop );

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSel6WayValveHelper::_PrepareResultData( CIndSel6WayValveParams* pclIndSel6wayValveParams, CSelectedValve* pclSelectedValve, int iResultCount )
{
	m_vecResultData.clear();
	CDB_6WayValve* pclControlValve = dynamic_cast<CDB_6WayValve*>( pclSelectedValve->GetpData() );

	if( NULL == pclControlValve )
	{
		return;
	}

	CString strValue;
	strValue.Format( _T( "Result number = %i" ), iResultCount );
	m_vecResultData.push_back( strValue );

	m_vecOutputTitles.push_back( _T( "Valve temperature range" ) );
	m_vecOutputTitles.push_back( _T( "Pipe size" ) );
	// Valve ID.
	m_vecResultData.push_back( pclControlValve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclControlValve->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclControlValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclControlValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclControlValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclControlValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclControlValve->GetSize() );
	
	// Dp in cooling mode.
	double dDp = CalcDp( pclIndSel6wayValveParams->GetCoolingFlow(), pclControlValve->GetKvs(), pclIndSel6wayValveParams->GetCoolingWaterChar().GetDens() );
	strValue.Format( _T( "%g" ), dDp );
	m_vecResultData.push_back( strValue );
	
	// Dp in heating mode.
	dDp = CalcDp( pclIndSel6wayValveParams->GetHeatingFlow(), pclControlValve->GetKvs(), pclIndSel6wayValveParams->GetHeatingWaterChar().GetDens() );
	strValue.Format( _T( "%g" ), dDp );
	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclControlValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSel6wayValveParams );
	pclIndSel6wayValveParams->m_pclSelect6WayValveList->GetSelectPipeList()->GetMatchingPipe( pclControlValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );
}

UINT CTUIndSel6WayValveHelper::InterpretePIBCVInputs( CIndSelPIBCVParams* pclIndSelPIBCVParams )
{
	// The 'm_strComboTypeID' variable.
	// Remark: mandatory for BV, PIBCV, DPCBCV, and CV not for other (By default "").
	if( 0 != m_mapInputs.count( _T( "secondcombotypeid" ) ) )
	{
		pclIndSelPIBCVParams->m_strComboTypeID = m_mapInputsWithCase[_T( "secondcombotypeid" )];
	}

	// The 'm_strComboFamilyID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcombofamilyid" ) ) )
	{
		pclIndSelPIBCVParams->m_strComboFamilyID = m_mapInputsWithCase[_T( "secondcombofamilyid" )];
	}

	// The 'm_strComboMaterialID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcombomaterialid" ) ) )
	{
		pclIndSelPIBCVParams->m_strComboMaterialID = m_mapInputsWithCase[_T( "secondcombomaterialid" )];
	}

	// The 'm_strComboConnectID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcomboconnectid" ) ) )
	{
		pclIndSelPIBCVParams->m_strComboConnectID = m_mapInputsWithCase[_T( "secondcomboconnectid" )];
	}

	// The 'm_strComboVersionID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcomboversionid" ) ) )
	{
		pclIndSelPIBCVParams->m_strComboVersionID = m_mapInputsWithCase[_T( "secondcomboversionid" )];
	}

	// The 'm_strComboPNID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcombopnid" ) ) )
	{
		pclIndSelPIBCVParams->m_strComboPNID = m_mapInputsWithCase[_T( "secondcombopnid" )];
	}

	// The 'm_eCvCtrlType' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T( "secondcvctrltype" ) ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CVCTRLTYPEMISSING;
	}

	if( 0 == m_mapInputs[_T( "secondcvctrltype" )].Compare( _T( "no" ) ) )
	{
		pclIndSelPIBCVParams->m_eCvCtrlType = CDB_ControlProperties::eCvNU;
	}
	else if( 0 == m_mapInputs[_T( "secondcvctrltype" )].Compare( _T( "proportional" ) ) )
	{
		pclIndSelPIBCVParams->m_eCvCtrlType = CDB_ControlProperties::eCvProportional;
	}
	else if( 0 == m_mapInputs[_T( "secondcvctrltype" )].Compare( _T( "onoff" ) ) )
	{
		pclIndSelPIBCVParams->m_eCvCtrlType = CDB_ControlProperties::eCvOnOff;
	}
	else if( 0 == m_mapInputs[_T( "secondcvctrltype" )].Compare( _T( "3points" ) ) )
	{
		pclIndSelPIBCVParams->m_eCvCtrlType = CDB_ControlProperties::eCv3point;
	}
	else if( 0 == m_mapInputs[_T( "secondcvctrltype" )].Compare( _T( "unknown" ) ) )
	{
		pclIndSelPIBCVParams->m_eCV2W3W = CDB_ControlProperties::LastCV2W3W;
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CVCTRLTYPEBAD;
	}

	// The 'm_strActuatorInputSignalID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcomboactuatorinputsignalid" ) ) )
	{
		pclIndSelPIBCVParams->m_strActuatorInputSignalID = m_mapInputsWithCase[_T( "secondcomboactuatorinputsignalid" )];
	}

	// The 'm_bIsDpMaxChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T( "seconddpmaxchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T( "seconddpmaxchecked" )].Compare( _T( "true" ) ) || 0 == m_mapInputs[_T( "seconddpmaxchecked" )].Compare( _T( "1" ) ) )
		{
			pclIndSelPIBCVParams->m_bIsDpMaxChecked = true;
		}
		else if( 0 == m_mapInputs[_T( "seconddpmaxchecked" )].Compare( _T( "false" ) ) || 0 == m_mapInputs[_T( "seconddpmaxchecked" )].Compare( _T( "0" ) ) )
		{
			pclIndSelPIBCVParams->m_bIsDpMaxChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPIBCV_DPMAXCHECKEDBAD;
		}
	}

	// The 'm_dDpMax' variable.
	// Remark: mandatory if 'dpmaxchecked' = 'true'.
	if( 0 != m_mapInputs.count( _T( "seconddpmax" ) ) )
	{
		pclIndSelPIBCVParams->m_dDpMax = _wtof( m_mapInputs[_T( "seconddpmax" )] );
	}
	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSel6WayValveHelper::VerifyPIBCVInputs( CIndSel6WayValveParams* pclIndSel6WayValveParams )
{
	CIndSelPIBCVParams* pclIndSelPIBCVParams = &pclIndSel6WayValveParams->m_clIndSelPIBCVParams;
	if( false == pclIndSelPIBCVParams->m_strComboTypeID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelPIBCVParams->m_strComboTypeID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_TYPEIDINVALID;
	}

	if( false == pclIndSelPIBCVParams->m_strComboFamilyID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelPIBCVParams->m_strComboFamilyID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_FAMILYIDINVALID;
	}

	if( false == pclIndSelPIBCVParams->m_strComboMaterialID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelPIBCVParams->m_strComboMaterialID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_MATERIALIDINVALID;
	}

	if( false == pclIndSelPIBCVParams->m_strComboConnectID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelPIBCVParams->m_strComboConnectID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_CONNECTIDINVALID;
	}

	if( false == pclIndSelPIBCVParams->m_strComboVersionID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelPIBCVParams->m_strComboVersionID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_VERSIONIDINVALID;
	}

	if( false == pclIndSelPIBCVParams->m_strComboPNID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelPIBCVParams->m_strComboPNID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_PNIDINVALID;
	}

	if( false == pclIndSelPIBCVParams->m_strActuatorInputSignalID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelPIBCVParams->m_strActuatorInputSignalID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELCTRLBASE_ACTINPUTSIGNALIDINVALID;
	}

	if( true == pclIndSelPIBCVParams->m_bIsDpMaxChecked && pclIndSelPIBCVParams->m_dDpMax <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_DPMAXINVALID;
	}

	if( true == pclIndSelPIBCVParams->m_strComboTypeID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_TYPEIDMISSING;
	}

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSel6WayValveHelper::_PrepareResultDataPIBCV( CIndSel6WayValveParams* pclIndSel6wayValveParams, CSelectedValve* pclSelectedValve, CSelectedValve* pclSelected6WayValve, 
	int iResultCount, SideDefinition eSideDefinition )
{
	m_vecResultData.clear();
	CDB_PIControlValve* pclPIBCValve = dynamic_cast<CDB_PIControlValve*>( pclSelectedValve->GetpData() );

	if( NULL == pclPIBCValve )
	{
		return;
	}
	CDB_6WayValve* pclControlValve = dynamic_cast<CDB_6WayValve*>( pclSelected6WayValve->GetpData() );

	CString strValue;
	strValue.Format( _T( "Result number = %i" ), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclPIBCValve->GetIDPtr().ID );
	m_vecResultData.push_back( pclControlValve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclPIBCValve->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclPIBCValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclPIBCValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclPIBCValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclPIBCValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclPIBCValve->GetSize() );

	if( pclIndSel6wayValveParams->m_e6WayValveSelectionMode == e6Way_OnOffControlWithPIBCV )
	{
		// Valve computed presetting.
		strValue.Format( _T( "%g" ), pclSelectedValve->GetH() );
		m_vecResultData.push_back( strValue );

		// Valve displayed presetting.
		CDB_PICVCharacteristic *pclPIBCVCharacteristic = pclPIBCValve->GetPICVCharacteristic();
	
		if( NULL != pclPIBCVCharacteristic )
		{
			CString strRounding;
			strRounding.Format( _T("%g"), pclPIBCVCharacteristic->GetSettingRounding() );

			CString strDisplayedSettings = pclPIBCVCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
			strValue.Format( _T("%s"), strDisplayedSettings );
			m_vecResultData.push_back( strValue );
		}
		else
		{
			m_vecResultData.push_back( _T("-") );
		}
	}

	// Rangeability.
	m_vecResultData.push_back( pclPIBCValve->GetStrRangeability().c_str() );

	// Leakage rate.
	strValue.Format( _T( "%g" ), pclPIBCValve->GetLeakageRate() * 100.0 );
	m_vecResultData.push_back( strValue );

	// DP min.
	strValue.Format( _T( "%g" ), pclSelectedValve->GetDpMin() );
	m_vecResultData.push_back( strValue );

	// Stroke.
	strValue.Format( _T( "%g" ), pclPIBCValve->GetStroke() );
	m_vecResultData.push_back( strValue );

	// Control characteristic.
	if( CDB_ControlProperties::Linear == pclPIBCValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T( "Linear" ) );
	}
	else if( CDB_ControlProperties::EqualPc == pclPIBCValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T( "EqualPc" ) );
	}
	else
	{
		m_vecResultData.push_back( _T( "NotCharacterized" ) );
	}

	// Push or pull to close.
	if( CDB_ControlProperties::PushToClose == pclPIBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T( "PushToClose" ) );
	}
	else if( CDB_ControlProperties::PullToClose == pclPIBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T( "PullToClose" ) );
	}
	else if( CDB_ControlProperties::Undef == pclPIBCValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T( "Undef" ) );
	}

	// Is Dp max checkbox checked.
	strValue.Format( _T( "%s" ), ( true == pclIndSel6wayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked ) ? _T( "true" ) : _T( "false" ) );
	m_vecResultData.push_back( strValue );

	// Dp max.
	strValue.Format( _T( "%g" ), pclPIBCValve->GetDpmax() );
	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclPIBCValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSel6wayValveParams);
	if( pclIndSel6wayValveParams->m_e6WayValveSelectionMode == e6Way_EQMControl )
	{
		pclIndSel6wayValveParams->m_pclSelect6WayValveList->GetSelectPipeList()->GetMatchingPipe( pclPIBCValve->GetSizeKey(), selPipe );
	}
	else if( pclIndSel6wayValveParams->m_e6WayValveSelectionMode == e6Way_OnOffControlWithPIBCV )
	{
		pclIndSel6wayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition )->GetSelectPipeList()->GetMatchingPipe( pclPIBCValve->GetSizeKey(), selPipe );
	}

	//pclIndSel6wayValveParams->m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetSelectPipeList()->GetMatchingPipe( pclPIBCValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T( "%g" ), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T( "%g" ), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

UINT CTUIndSel6WayValveHelper::DropOutSelectionPIBCV( CProductSelelectionParameters* pclProdSelParams, CTUProdSelFileHelper& clTUFileHelper )
{

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#ProductCategory = PIBCV\n\n" ) );
	CIndividualSelectionParameters* pclIndSelParams = dynamic_cast<CIndividualSelectionParameters*>( pclProdSelParams );
	CIndSelCtrlParamsBase* pclIndSelCtrlBaseParams = dynamic_cast<CIndSelCtrlParamsBase*>( pclProdSelParams );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#StartInputs\n" ) );

	CString strString = pclIndSelParams->m_strComboTypeID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboTypeID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboFamilyID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboMaterialID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboMaterialID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboConnectID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboConnectID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboVersionID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboVersionID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboPNID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboPNID = %s\n" ), strString );

	if( CDB_ControlProperties::eCvNU == pclIndSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SecondCVCtrlType = No\n" ) );
	}
	else if( CDB_ControlProperties::eCvProportional == pclIndSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SecondCVCtrlType = Proportional\n" ) );
	}
	else if( CDB_ControlProperties::eCvOnOff == pclIndSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SecondCVCtrlType = OnOff\n" ) );
	}
	else if( CDB_ControlProperties::eCv3point == pclIndSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SecondCVCtrlType = 3points\n" ) );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "SecondCVCtrlType = Unknown\n" ) );
	}

	strString = pclIndSelCtrlBaseParams->m_strActuatorInputSignalID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboActuatorInputSignalID = %s\n" ), strString );


	CIndSelPIBCVParams* pclIndSelPIBCVParams = dynamic_cast<CIndSelPIBCVParams*>( pclProdSelParams );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondDpMaxChecked = %s\n" ), ( false == pclIndSelPIBCVParams->m_bIsDpMaxChecked ) ? _T( "false" ) : _T( "true" ) );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T( "SecondDpMax = %.13g\n" ), pclIndSelPIBCVParams->m_dDpMax );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#EndInputs PIBCV\n\n" ) );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSel6WayValveHelper::InterpreteBvInputs( CIndSelBVParams* pclIndSelBVParams )
{
	// The 'm_strComboTypeID' variable.
	// Remark: mandatory for BV, PIBCV, DPCBCV, and CV not for other (By default "").
	if( 0 != m_mapInputs.count( _T( "secondcombotypeid" ) ) )
	{
		pclIndSelBVParams->m_strComboTypeID = m_mapInputsWithCase[_T( "secondcombotypeid" )];
	}

	// The 'm_strComboFamilyID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcombofamilyid" ) ) )
	{
		pclIndSelBVParams->m_strComboFamilyID = m_mapInputsWithCase[_T( "secondcombofamilyid" )];
	}

	// The 'm_strComboMaterialID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcombomaterialid" ) ) )
	{
		pclIndSelBVParams->m_strComboMaterialID = m_mapInputsWithCase[_T( "secondcombomaterialid" )];
	}

	// The 'm_strComboConnectID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcomboconnectid" ) ) )
	{
		pclIndSelBVParams->m_strComboConnectID = m_mapInputsWithCase[_T( "secondcomboconnectid" )];
	}

	// The 'm_strComboVersionID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcomboversionid" ) ) )
	{
		pclIndSelBVParams->m_strComboVersionID = m_mapInputsWithCase[_T( "secondcomboversionid" )];
	}

	// The 'm_strComboPNID' variable.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T( "secondcombopnid" ) ) )
	{
		pclIndSelBVParams->m_strComboPNID = m_mapInputsWithCase[_T( "secondcombopnid" )];
	}
	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSel6WayValveHelper::VerifyBVInputs( CIndSel6WayValveParams* pclIndSel6WayValveParams )
{
	CIndSelBVParams* pclIndSelBVParams = &pclIndSel6WayValveParams->m_clIndSelBVParams;

	if( false == pclIndSelBVParams->m_strComboTypeID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelBVParams->m_strComboTypeID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_TYPEIDINVALID;
	}

	if( false == pclIndSelBVParams->m_strComboFamilyID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelBVParams->m_strComboFamilyID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_FAMILYIDINVALID;
	}

	if( false == pclIndSelBVParams->m_strComboMaterialID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelBVParams->m_strComboMaterialID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_MATERIALIDINVALID;
	}

	if( false == pclIndSelBVParams->m_strComboConnectID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelBVParams->m_strComboConnectID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_CONNECTIDINVALID;
	}

	if( false == pclIndSelBVParams->m_strComboVersionID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelBVParams->m_strComboVersionID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_VERSIONIDINVALID;
	}

	if( false == pclIndSelBVParams->m_strComboPNID.IsEmpty()
		&& _NULL_IDPTR == pclIndSel6WayValveParams->m_pTADB->Get( pclIndSelBVParams->m_strComboPNID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_PNIDINVALID;
	}
	if( true == pclIndSelBVParams->m_strComboTypeID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_INDSELBV_TYPEIDMISSING;
	}
	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSel6WayValveHelper::_PrepareResultDataBV( CIndSel6WayValveParams* pclIndSel6wayValveParams, CSelectedValve* pclSelectedValve, CSelectedValve* pclSelected6WayValve, int iResultCount, SideDefinition eSideDefinition )
{
	m_vecResultData.clear();
	CDB_TAProduct* pclTAProduct = dynamic_cast<CDB_TAProduct*>( pclSelectedValve->GetpData() );

	if( NULL == pclTAProduct )
	{
		return;
	}
	CDB_TAProduct* pcl6Wayvalve = dynamic_cast<CDB_TAProduct*>( pclSelected6WayValve->GetpData() );

	CString strValue;
	strValue.Format( _T( "Result number = %i" ), iResultCount );
	m_vecResultData.push_back( strValue );

	// Valve ID.
	m_vecResultData.push_back( pclTAProduct->GetIDPtr().ID );
	m_vecResultData.push_back( pcl6Wayvalve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclTAProduct->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclTAProduct->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclTAProduct->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclTAProduct->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclTAProduct->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclTAProduct->GetSize() );

	// Valve computed setting.
	strValue.Format( _T( "%g" ), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_ValveCharacteristic *pclValveCharacteristic = pclTAProduct->GetValveCharacteristic();
	
	if( NULL != pclValveCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclValveCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclValveCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Pressure drop through the valve.
	strValue.Format( _T( "%g" ), pclSelectedValve->GetDp() );
	m_vecResultData.push_back( strValue );

	// Pressure drop when valve fully opened.
	strValue.Format( _T( "%g" ), pclSelectedValve->GetDpFullOpen() );
	m_vecResultData.push_back( strValue );

	// Pressure drop when valve is half opened.
	strValue = _T( "" );

	if( false == pclIndSel6wayValveParams->m_bDpEnabled || -1.0 == pclSelectedValve->GetDp() )
	{
		strValue.Format( _T( "%g" ), pclSelectedValve->GetDpHalfOpen() );
	}

	m_vecResultData.push_back( strValue );

	// Temperature range of the valve.
	m_vecResultData.push_back( pclTAProduct->GetTempRange() );

	// Pipe size.
	// Set pipe size.
	CSelectPipe selPipe( pclIndSel6wayValveParams );

	// Pay attention: we need to take here the appropriate pipe list (Not the one that is in the 'm_pclIndSel6WayValveParams->m_pclSelect6WayValveList'.
	pclIndSel6wayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )->GetSelectPipeList()->GetMatchingPipe( pclTAProduct->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T( "%g" ), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T( "%g" ), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

UINT CTUIndSel6WayValveHelper::DropOutSelectionBV( CProductSelelectionParameters* pclProdSelParams, CTUProdSelFileHelper& clTUFileHelper )
{
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#ProductCategory = BV\n\n" ) );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#StartInputs\n" ) );
	CIndividualSelectionParameters* pclIndSelParams = dynamic_cast<CIndividualSelectionParameters*>( pclProdSelParams );
	CString strString = pclIndSelParams->m_strComboTypeID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboTypeID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboFamilyID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboMaterialID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboMaterialID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboConnectID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboConnectID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboVersionID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboVersionID = %s\n" ), strString );

	strString = pclIndSelParams->m_strComboPNID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T( "SecondComboPNID = %s\n" ), strString );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T( "#EndInputs BV\n\n" ) );

	return TU_PRODSELECT_ERROR_OK;
}

bool CTUIndSel6WayValveHelper::_VerifyFlows( CIndSel6WayValveParams* pclIndSel6wayValveParams, CSelectedValve* pSelected6WayValve )
{
	if( NULL == pclIndSel6wayValveParams || NULL == pclIndSel6wayValveParams->m_pclSelect6WayValveList
		|| NULL == pclIndSel6wayValveParams->m_pTADB || NULL == pclIndSel6wayValveParams->m_pPipeDB )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pSelected6WayValve )
	{
		return false;
	}

	bool bOK = true;
	CSelect6WayValveList* pcl6WayValveList = pclIndSel6wayValveParams->m_pclSelect6WayValveList;
	CSelectPICVList* pclSelectedPICvList1 = NULL;
	CSelectPICVList* pclSelectedPICvList2 = NULL;
	bool bAtLeastOneSizeShiftProblem = false;

	if( e6Way_EQMControl == pclIndSel6wayValveParams->m_e6WayValveSelectionMode )
	{
		if( pcl6WayValveList->SelectPIBCValve( pclIndSel6wayValveParams, pSelected6WayValve, BothSide, &bAtLeastOneSizeShiftProblem ) > 0 )
		{
			pclSelectedPICvList1 = pcl6WayValveList->GetPICVList( BothSide );
		}
	}
	else if ( e6Way_OnOffControlWithPIBCV == pclIndSel6wayValveParams->m_e6WayValveSelectionMode )
	{
		if( pcl6WayValveList->SelectPIBCValve( pclIndSel6wayValveParams, pSelected6WayValve, HeatingSide, &bAtLeastOneSizeShiftProblem ) > 0 )
		{
			pclSelectedPICvList1 = pcl6WayValveList->GetPICVList( HeatingSide );
		}

		if( pcl6WayValveList->SelectPIBCValve( pclIndSel6wayValveParams, pSelected6WayValve, CoolingSide, &bAtLeastOneSizeShiftProblem ) > 0 )
		{
			pclSelectedPICvList2 = pcl6WayValveList->GetPICVList( CoolingSide );
		}
	}
	else
	{
		return true;
	}

	bool bStop = false;
	bool bAllSettingsForCoolingFlowToLow = true;
	bool bAllSettingsForHeatingFlowToLow = true;
	SideDefinition eWhichSideNoFound = SideDefinition::Undefined;

	// HYS-1355: For now, there is only TA-Slider 160 for TA-Compact-P DN10/25 and TA-Modulator DN10/32.
	CDB_ElectroActuator* pclTASlider160 = dynamic_cast<CDB_ElectroActuator*>( TASApp.GetpTADB()->Get( _T( "SLIDER160-STD1M" ) ).MP );
	ASSERT( NULL != pclTASlider160 );

	// Remark: we start with the heating that has generally a lower flow defined than the cooling.
	for( int iLoop = 0; iLoop < 2 && false == bStop; iLoop++ )
	{
		CSelectPICVList* pclSelectedPICvList = ( 0 == iLoop ) ? pclSelectedPICvList1 : pclSelectedPICvList2;

		if( NULL == pclSelectedPICvList )
		{
			continue;
		}

		for( CSelectedValve* pclSelectedPICv = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedPICv && false == bStop;
			pclSelectedPICv = pclSelectedPICvList->GetNext<CSelectedValve>() )
		{
			CDB_PIControlValve* pclPICv = dynamic_cast<CDB_PIControlValve*>( pclSelectedPICv->GetpData() );

			if( NULL == pclPICv || NULL == pclPICv->GetPICVCharacteristic() )
			{
				continue;
			}

			if( e6Way_EQMControl == pclIndSel6wayValveParams->m_e6WayValveSelectionMode )
			{
				// In EQM mode we use the same valve for cooling and heating.
				double dHCooling = pclPICv->GetPresetting( pclIndSel6wayValveParams->GetCoolingFlow(), pclIndSel6wayValveParams->GetCoolingWaterChar().GetDens(),
					pclIndSel6wayValveParams->GetCoolingWaterChar().GetKinVisc() );

				double dHHeating = pclPICv->GetPresetting( pclIndSel6wayValveParams->GetHeatingFlow(), pclIndSel6wayValveParams->GetHeatingWaterChar().GetDens(),
					pclIndSel6wayValveParams->GetHeatingWaterChar().GetKinVisc() );

				if( dHCooling >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
				{
					bAllSettingsForCoolingFlowToLow = false;
				}

				if( dHHeating >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
				{
					bAllSettingsForHeatingFlowToLow = false;
				}

				if( dHCooling >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 )
					&& dHHeating >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
				{
					// If this current valve satisfy the two flow, we can stop the loop.
					bStop = true;
				}
			}
			else
			{
				// In On/off with PIBCv we have two different valves.
				double dH = 0.0;

				if( 0 == iLoop )
				{
					dH = pclPICv->GetPresetting( pclIndSel6wayValveParams->GetHeatingFlow(), pclIndSel6wayValveParams->GetHeatingWaterChar().GetDens(),
						pclIndSel6wayValveParams->GetHeatingWaterChar().GetKinVisc() );
				}
				else
				{
					dH = pclPICv->GetPresetting( pclIndSel6wayValveParams->GetCoolingFlow(), pclIndSel6wayValveParams->GetCoolingWaterChar().GetDens(),
						pclIndSel6wayValveParams->GetCoolingWaterChar().GetKinVisc() );
				}

				if( dH >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
				{
					if( 0 == iLoop )
					{
						bAllSettingsForHeatingFlowToLow = false;
					}
					else
					{
						bAllSettingsForCoolingFlowToLow = false;
					}
				}
			}
		}

		if( e6Way_OnOffControlWithPIBCV == pclIndSel6wayValveParams->m_e6WayValveSelectionMode && 0 == iLoop && true == bAllSettingsForHeatingFlowToLow )
		{
			// If no valve has been found in heating, we don't need to do the second loop with the cooling mode.
			bStop = true;
		}
	}

	if( true == bAllSettingsForCoolingFlowToLow || true == bAllSettingsForHeatingFlowToLow )
	{
		bOK = false;
	}

	return bOK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for smart control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUIndSelSmartControlValveHelper::CTUIndSelSmartControlValveHelper() 
	: CTUIndSelHelper( m_clIndSelSmartControlValveParams ) 
{
	m_clIndSelSmartControlValveParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_SmartControlValve;
	m_clIndSelSmartControlValveParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
}

CTUIndSelSmartControlValveHelper::~CTUIndSelSmartControlValveHelper()
{
	if( NULL != m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList )
	{
		delete m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList;
		m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList = NULL;
	}
}

UINT CTUIndSelSmartControlValveHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUIndSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelSmartControlValveHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_NOINPUT;
	}

	if( NULL != m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList )
	{
		delete m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList;
	}

	m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList = new CSelectSmartControlValveList();

	if( NULL == m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList || NULL == m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->GetSelectPipeList() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_CANTCREATESELECTLIST;
	}

	m_clIndSelSmartControlValveParams.m_SmartControlValveList.PurgeAll();

	m_clIndSelSmartControlValveParams.m_pTADB->GetSmartControlValveList( 
			&m_clIndSelSmartControlValveParams.m_SmartControlValveList,
			_T(""),
			_T(""), 
			(LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboMaterialID, 
			(LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboConnectID, 
			_T(""),
			(LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboPNID, 
			m_clIndSelSmartControlValveParams.m_eFilterSelection );

	m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->GetSelectPipeList()->SelectPipes( &m_clIndSelSmartControlValveParams, m_clIndSelSmartControlValveParams.m_dFlow );

	bool bSizeShiftProblem = false;
	bool bBestFound = false;
	int iDevFound = m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->SelectSmartControlValve( &m_clIndSelSmartControlValveParams, &bSizeShiftProblem, bBestFound );

	if( true == bNoVerify )
	{
		return TU_PRODSELECT_ERROR_OK;
	}

	if( iDevFound != (int)m_vecOutputValues.size() )
	{
		// Number of results not the same.
		return TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_RESULTNBRNOTSAME;
	}

	int iLoopResultInInputFile = 0;

	for( CSelectedValve *pclSelectedValve = m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->GetNext<CSelectedValve>() )
	{
		// Prepare vector with all value to verify.
		_PrepareResultData( &m_clIndSelSmartControlValveParams, pclSelectedValve, iLoopResultInInputFile );

		if( m_vecResultData.size() != m_vecOutputValues[iLoopResultInInputFile].size() )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_BADEXTRACT;
		}

		// Now just compare between results and input file.
		for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
		{
			if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopResultInInputFile][iLoop] ) )
			{
				strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopResultInInputFile,
						m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopResultInInputFile][iLoop], m_vecResultData[iLoop] );

				return TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_RESULTDIFF;
			}
		}

		iLoopResultInInputFile++;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUIndSelSmartControlValveHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = Smart control valve\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUIndSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CIndSelSmartControlValveParams *pclIndSelSmartControlValveParams = dynamic_cast<CIndSelSmartControlValveParams *>( pclProdSelParams );

	if( NULL != pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList )
	{
		int iResultCount = 0;

		for( CSelectedValve *pclSelectedValve = pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
				pclSelectedValve = pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetNext<CSelectedValve>() )
		{
			_PrepareResultData( pclIndSelSmartControlValveParams, pclSelectedValve, iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUIndSelSmartControlValveHelper::_PrepareResultData( CIndSelSmartControlValveParams *pclIndSelSmartControlValveParams, CSelectedValve *pclSelectedSmartControlValve, int iResultCount )
{
	m_vecResultData.clear();
	CDB_TAProduct *pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclSelectedSmartControlValve->GetpData() );

	if( NULL == pclTAProduct ) 
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );
	
	// Valve ID.
	m_vecResultData.push_back( pclTAProduct->GetIDPtr().ID );
	
	// Valve name.
	m_vecResultData.push_back( pclTAProduct->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclTAProduct->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclTAProduct->GetConnect() );

	// Valve PN.
	m_vecResultData.push_back( pclTAProduct->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclTAProduct->GetSize() );
		
	// Temperature range of the valve.
	m_vecResultData.push_back( pclTAProduct->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclIndSelSmartControlValveParams );
	pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetSelectPipeList()->GetMatchingPipe( pclTAProduct->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on batch selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUBatchSelHelper::CTUBatchSelHelper( CBatchSelectionParameters &clBatchSelParams )
	: CTUProdSelHelper( clBatchSelParams )
{
	m_bVerifyAlsoCombos = true;
	m_ivecOutputFlowIndex = 0;
	m_ivecOutputPowerIndex = 0;
	m_ivecOutputDTIndex = 0;
	m_ivecOutputDpIndex = 0;
	m_pclBatchSelParams = &clBatchSelParams;
}

UINT CTUBatchSelHelper::InterpreteInputs()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	UINT uiErrorCode = CTUProdSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_eFlowOrPowerDT' variable.
	// Remark: not mandatory. By default = 'flowmode'.
	if( 0 != m_mapInputs.count( _T("floworpowerdt" ) ) )
	{
		if( 0 == m_mapInputs[_T("floworpowerdt")].Compare( _T("flowmode") ) )
		{
			m_pclBatchSelParams->m_eFlowOrPowerDTMode = CDS_SelProd::efdFlow;
		}
		else if( 0 == m_mapInputs[_T("floworpowerdt")].Compare( _T("powerdtmode") ) )
		{
			m_pclBatchSelParams->m_eFlowOrPowerDTMode = CDS_SelProd::efdPower;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELBASE_FLOWORPOWERDTBAD;
		}
	}

	// The 'm_bIsDpGroupChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("dpgroupchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpgroupchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("dpgroupchecked")].Compare( _T("1") ) )
		{
			m_pclBatchSelParams->m_bIsDpGroupChecked = true;
		}
		else if( 0 == m_mapInputs[_T("dpgroupchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("dpgroupchecked")].Compare( _T("0") ) )
		{
			m_pclBatchSelParams->m_bIsDpGroupChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELBASE_DPGROUPCHECKEDBAD;
		}
	}

	// The 'm_strComboTypeBelow65ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("combotypebelow65id") ) )
	{
		m_pclBatchSelParams->m_strComboTypeBelow65ID = m_mapInputsWithCase[_T("combotypebelow65id")];
	}

	// The 'm_strComboFamilyBelow65ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("combofamilybelow65id") ) )
	{
		m_pclBatchSelParams->m_strComboFamilyBelow65ID = m_mapInputsWithCase[_T("combofamilybelow65id")];
	}

	// The 'm_strComboMaterialBelow65ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("combomaterialbelow65id") ) )
	{
		m_pclBatchSelParams->m_strComboMaterialBelow65ID = m_mapInputsWithCase[_T("combomaterialbelow65id")];
	}

	// The 'm_strComboConnectBelow65ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("comboconnectbelow65id") ) )
	{
		m_pclBatchSelParams->m_strComboConnectBelow65ID = m_mapInputsWithCase[_T("comboconnectbelow65id")];
	}

	// The 'm_strComboVersionBelow65ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("comboversionbelow65id") ) )
	{
		m_pclBatchSelParams->m_strComboVersionBelow65ID = m_mapInputsWithCase[_T("comboversionbelow65id")];
	}

	// The 'm_strComboPNBelow65ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("combopnbelow65id") ) )
	{
		m_pclBatchSelParams->m_strComboPNBelow65ID = m_mapInputsWithCase[_T("combopnbelow65id")];
	}

	// The 'm_strComboTypeAbove50ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("combotypeabove50id") ) )
	{
		m_pclBatchSelParams->m_strComboTypeAbove50ID = m_mapInputsWithCase[_T("combotypeabove50id")];
	}

	// The 'm_strComboFamilyAbove50ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("combofamilyabove50id") ) )
	{
		m_pclBatchSelParams->m_strComboFamilyAbove50ID = m_mapInputsWithCase[_T("combofamilyabove50id")];
	}

	// The 'm_strComboMaterialAbove50ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("combomaterialabove50id") ) )
	{
		m_pclBatchSelParams->m_strComboMaterialAbove50ID = m_mapInputsWithCase[_T("combomaterialabove50id")];
	}

	// The 'm_strComboConnectAbove50ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("comboconnectabove50id") ) )
	{
		m_pclBatchSelParams->m_strComboConnectAbove50ID = m_mapInputsWithCase[_T("comboconnectabove50id")];
	}

	// The 'm_strComboVersionAbove50ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("comboversionabove50id") ) )
	{
		m_pclBatchSelParams->m_strComboVersionAbove50ID = m_mapInputsWithCase[_T("comboversionabove50id")];
	}

	// The 'm_strComboPNAbove50ID' variable.
	// Remark: mandatory for BV, DPC, BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("combopnabove50id") ) )
	{
		m_pclBatchSelParams->m_strComboPNAbove50ID = m_mapInputsWithCase[_T("combopnabove50id")];
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	UINT uiErrorCode = CTUProdSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper);

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CBatchSelectionParameters *pclBatchSelParams = dynamic_cast<CBatchSelectionParameters *>( pclProdSelParams );

	CString strString = ( CDS_SelProd::efdFlow == pclBatchSelParams->m_eFlowOrPowerDTMode ) ? _T("FlowMode") : _T("PowerDTMode");
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("FlowOrPowerDT = %s\n"), strString );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpGroupChecked = %s\n"), ( false == pclBatchSelParams->m_bIsDpGroupChecked ) ? _T("false") : _T("true") );

	strString = pclBatchSelParams->m_strComboTypeBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboTypeBelow65ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboFamilyBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboFamilyBelow65ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboMaterialBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboMaterialBelow65ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboConnectBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboConnectBelow65ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboVersionBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboVersionBelow65ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboPNBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboPNBelow65ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboTypeAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboTypeAbove50ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboFamilyAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboFamilyAbove50ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboMaterialAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboMaterialAbove50ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboConnectAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboConnectAbove50ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboVersionAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboVersionAbove50ID = %s\n"), strString );

	strString = pclBatchSelParams->m_strComboPNAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboPNAbove50ID = %s\n"), strString );

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelHelper::VerifyInputs()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	UINT uiErrorCode = CTUProdSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_bVerifyAlsoCombos )
	{
		if( true == m_pclBatchSelParams->m_strComboTypeBelow65ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_TYPEB65IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboTypeBelow65ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_TYPEB65IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboFamilyBelow65ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_FAMILYB65IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboFamilyBelow65ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_FAMILYB65IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboMaterialBelow65ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_MATERIALB65IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboMaterialBelow65ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_MATERIALB65IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboConnectBelow65ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_CONNECTB65IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboConnectBelow65ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_CONNECTB65IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboVersionBelow65ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONB65IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboVersionBelow65ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONB65IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboPNBelow65ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNB65IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboPNBelow65ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNB65IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboTypeAbove50ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_TYPEA50IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboTypeAbove50ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_TYPEA50IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboFamilyAbove50ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_FAMILYA50IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboFamilyAbove50ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_FAMILYA50IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboMaterialAbove50ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_MATERIALA50IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboMaterialAbove50ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_MATERIALA50IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboConnectAbove50ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_CONNECTA50IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboConnectAbove50ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_CONNECTA50IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboVersionAbove50ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONA50IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboVersionAbove50ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONA50IDINVALID;
		}

		if( true == m_pclBatchSelParams->m_strComboPNAbove50ID.IsEmpty() )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNA50IDEMPTY;
		}

		if( _NULL_IDPTR == m_pclBatchSelParams->m_pTADB->Get( m_pclBatchSelParams->m_strComboPNAbove50ID ) )
		{
			return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNA50IDINVALID;
		}
	}

	return TU_PRODSELECT_ERROR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for regulating valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUBatchSelBVHelper::CTUBatchSelBVHelper() 
	: CTUBatchSelHelper( m_clBatchSelBVParams ) 
{
	m_clBatchSelBVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	m_clBatchSelBVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	// Can verify content of all combos.
	m_bVerifyAlsoCombos = true;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Flow") );
	m_vecOutputTitles.push_back( _T("Power") );
	m_vecOutputTitles.push_back( _T("DT") );
	m_vecOutputTitles.push_back( _T("Dp") );
	m_vecOutputTitles.push_back( _T("Status") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve type") );
	m_vecOutputTitles.push_back( _T("Valve material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Valve computed presetting") );
	m_vecOutputTitles.push_back( _T("Valve displayed presetting") );
	m_vecOutputTitles.push_back( _T("Valve signal") );
	m_vecOutputTitles.push_back( _T("Valve Dp") );
	m_vecOutputTitles.push_back( _T("Valve Dp at full opening") );
	m_vecOutputTitles.push_back( _T("Valve Dp at half opening") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear pressure drop") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );

	m_ivecOutputFlowIndex = 1;
	m_ivecOutputPowerIndex = 2;
	m_ivecOutputDTIndex = 3;
	m_ivecOutputDpIndex = 4;
}

CTUBatchSelBVHelper::~CTUBatchSelBVHelper() 
{
	for( int i = 0; i < (int)m_clBatchSelBVParams.m_vecRowParameters.size(); i++ )
	{
		if( NULL != m_clBatchSelBVParams.m_vecRowParameters.at( i ) )
		{
			if( NULL != m_clBatchSelBVParams.m_vecRowParameters.at( i )->m_pclBatchResults )
			{
				delete m_clBatchSelBVParams.m_vecRowParameters.at( i )->m_pclBatchResults;
			}
		}

		delete m_clBatchSelBVParams.m_vecRowParameters.at( i );
	}

	m_clBatchSelBVParams.m_vecRowParameters.clear();
}

UINT CTUBatchSelBVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_NOINPUT;
	}

	if( 0 == (int)m_vecOutputValues.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_NOOUTPUT;
	}

	for( int iLoopBatch = 0; iLoopBatch < (int)m_vecOutputValues.size(); iLoopBatch++ )
	{
		// If we verify results now, we can delete results. Otherwise we keep them in 'm_clBatchSelBVParams.m_vecRowParameters'.
		if( false == bNoVerify && NULL != m_clBatchSelBVParams.m_pclBatchBVList )
		{
			delete m_clBatchSelBVParams.m_pclBatchBVList;
			m_clBatchSelBVParams.m_pclBatchBVList = NULL;
		}

		m_clBatchSelBVParams.m_pclBatchBVList = new CBatchSelectBvList();

		if( NULL == m_clBatchSelBVParams.m_pclBatchBVList )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_CANTCREATESELECTLIST;
		}

		CDlgBatchSelBase::BSRowParameters *pclRowParameters = new CDlgBatchSelBase::BSRowParameters();

		if( NULL == pclRowParameters )
		{
			delete m_clBatchSelBVParams.m_pclBatchBVList;
			m_clBatchSelBVParams.m_pclBatchBVList = NULL;
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_CANTCREATEROWPARAMS;
		}

		pclRowParameters->m_pclBatchResults = m_clBatchSelBVParams.m_pclBatchBVList;
		pclRowParameters->m_dFlow = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputFlowIndex] );

		if( CDS_SelProd::efdPower == m_clBatchSelBVParams.m_eFlowOrPowerDTMode )
		{
			pclRowParameters->m_dPower = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputPowerIndex] );
			pclRowParameters->m_dDT = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDTIndex] );

			pclRowParameters->m_dFlow = CalcqFromPDT( pclRowParameters->m_dPower, pclRowParameters->m_dDT, m_clBatchSelBVParams.m_WC.GetDens(), 
					m_clBatchSelBVParams.m_WC.GetSpecifHeat() );
		}

		pclRowParameters->m_dDp = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDpIndex] );

		pclRowParameters->m_pclSelectedProduct = NULL;
		int iReturnCode = m_clBatchSelBVParams.m_pclBatchBVList->SelectBv( &m_clBatchSelBVParams, pclRowParameters->m_dFlow, pclRowParameters->m_dDp );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturnCode & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturnCode & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturnCode & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturnCode & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelBVParams.m_pclBatchBVList->GetBestProduct();

			if( false == bAlternative )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
			}
			else
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelBVParams.m_pclBatchBVList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelBVParams.m_pclBatchBVList->GetFirst<CSelectedBase>();

				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
				}
			}
			else
			{
				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutions;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutionsAlter;
				}
			}
		}

		if( false == bNoVerify )
		{
			_PrepareResultData( &m_clBatchSelBVParams, pclRowParameters, iLoopBatch );

			if( m_vecResultData.size() != m_vecOutputValues[iLoopBatch].size() )
			{
				delete pclRowParameters;
				delete m_clBatchSelBVParams.m_pclBatchBVList;
				m_clBatchSelBVParams.m_pclBatchBVList = NULL;
				return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BADEXTRACT;
			}

			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopBatch][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopBatch,
							m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopBatch][iLoop], m_vecResultData[iLoop] );

					delete pclRowParameters;
					delete m_clBatchSelBVParams.m_pclBatchBVList;
					m_clBatchSelBVParams.m_pclBatchBVList = NULL;
					return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_RESULTDIFF;
				}
			}

			delete m_clBatchSelBVParams.m_pclBatchBVList;
			m_clBatchSelBVParams.m_pclBatchBVList = NULL;
		
			delete pclRowParameters;
			pclRowParameters = NULL;
		}
		else
		{
			m_clBatchSelBVParams.m_vecRowParameters.push_back( pclRowParameters );
		}
	}

	if( false == bNoVerify && NULL != m_clBatchSelBVParams.m_pclBatchBVList )
	{
		delete m_clBatchSelBVParams.m_pclBatchBVList;
		m_clBatchSelBVParams.m_pclBatchBVList = NULL;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelBVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = BV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUBatchSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CBatchSelBVParams *pclBatchSelBVParams = dynamic_cast<CBatchSelBVParams *>( pclProdSelParams );

	if( 0 != (int)pclBatchSelBVParams->m_vecRowParameters.size() )
	{
		int iResultCount = 0;

		for( int i = 0; i < (int)pclBatchSelBVParams->m_vecRowParameters.size(); i++ )
		{
			_PrepareResultData( pclBatchSelBVParams, pclBatchSelBVParams->m_vecRowParameters[i], iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUBatchSelBVHelper::_PrepareResultData( CBatchSelBVParams *pclBatchSelBVParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclBatchSelBVParams || NULL == pclRowParameters )
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Flow or Power/DT.
	if( CDS_SelProd::efdFlow == pclBatchSelBVParams->m_eFlowOrPowerDTMode )
	{
		// Flow.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dFlow );
		m_vecResultData.push_back( strValue );

		// Power & DT.
		m_vecResultData.push_back( _T("") );
		m_vecResultData.push_back( _T("") );
	}
	else
	{
		// Flow.
		m_vecResultData.push_back( _T("") );

		// Power.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dPower );
		m_vecResultData.push_back( strValue );
		
		// DT.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDT );
		m_vecResultData.push_back( strValue );
	}

	// Dp.
	if( true == pclBatchSelBVParams->m_bIsDpGroupChecked )
	{
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDp );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("") );
	}

	// Status.
	if( CDlgBatchSelBase::BS_RS_FindOneSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutions == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutions") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindNoSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindNoSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindOneSolutionAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolutionWithAlternative") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutionsAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutionsWithAlternative") );
	}

	if( NULL == pclRowParameters->m_pclSelectedProduct 
			|| NULL == dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct )
			|| NULL == dynamic_cast<CDB_TAProduct *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ) )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	// ID.
	CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );
	CDB_TAProduct *pclRegulatingValve = dynamic_cast<CDB_TAProduct *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );
	m_vecResultData.push_back( pclRegulatingValve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclRegulatingValve->GetName() );

	// Valve type.
	m_vecResultData.push_back( pclRegulatingValve->GetType() );

	// Valve body material.
	m_vecResultData.push_back( pclRegulatingValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclRegulatingValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclRegulatingValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclRegulatingValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclRegulatingValve->GetSize() );

	// Valve computed presetting.
	strValue.Format( _T("%g"), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_ValveCharacteristic *pclValveCharacteristic = pclRegulatingValve->GetValveCharacteristic();
	
	if( NULL != pclValveCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclValveCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclValveCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Valve signal.
	strValue.Format( _T("%g"), pclSelectedValve->GetDpSignal() );
	m_vecResultData.push_back( strValue );

	// Valve Dp.
	strValue.Format( _T("%g"), pclSelectedValve->GetDp() );
	m_vecResultData.push_back( strValue );

	// Valve Dp at full opening.
	strValue.Format( _T("%g"), pclSelectedValve->GetDpFullOpen() );
	m_vecResultData.push_back( strValue );

	// Valve Dp at half opening.
	strValue.Format( _T("%g"), pclSelectedValve->GetDpHalfOpen() );
	m_vecResultData.push_back( strValue );

	// Valve temperature range.
	m_vecResultData.push_back( pclRegulatingValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclBatchSelBVParams );
	pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclRegulatingValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on batch selection for balancing and control valves and pressure independent 
// balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUBatchSelCtrlBaseHelper::CTUBatchSelCtrlBaseHelper( CBatchSelCtrlParamsBase &clBatchSelCtrlBaseParams )
	: CTUBatchSelHelper( clBatchSelCtrlBaseParams )
{
	m_pclBatchSelCtrlBaseParams = &clBatchSelCtrlBaseParams;
}

UINT CTUBatchSelCtrlBaseHelper::InterpreteInputs()
{
	if( NULL == m_pclBatchSelCtrlBaseParams )
	{
		ASSERTA_RETURN( -1 );
	}

	UINT uiErrorCode = CTUBatchSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_bIsCtrlTypeStrictChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("ctrltypestrictchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("ctrltypestrictchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("ctrltypestrictchecked")].Compare( _T("1") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_bIsCtrlTypeStrictChecked = true;
		}
		else if( 0 == m_mapInputs[_T("ctrltypestrictchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("ctrltypestrictchecked")].Compare( _T("0") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_bIsCtrlTypeStrictChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CTRLTYPESTRICTCHECKBAD;
		}
	}

	// The 'm_eCV2W3W' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("cv2w3w") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CV2W3WMISSING;
	}

	if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("cv2w") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::CV2W;
	}
	else if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("cv3w") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::CV3W;
	}
	else if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("cv4w") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::CV4W;
	}
	else if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("cv6w") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::CV6W;
	}
	else if( 0 == m_mapInputs[_T("cv2w3w")].Compare( _T("unknown") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCV2W3W = CDB_ControlProperties::LastCV2W3W;
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CV2W3WBAD;
	}

	// The 'm_eCvCtrlType' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("cvctrltype") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CVCTRLTYPEMISSING;
	}

	if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("no") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCvCtrlType = CDB_ControlProperties::eCvNU;
	}
	else if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("proportional") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCvCtrlType = CDB_ControlProperties::eCvProportional;
	}
	else if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("onoff") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCvCtrlType = CDB_ControlProperties::eCvOnOff;
	}
	else if( 0 == m_mapInputs[_T("cvctrltype")].Compare( _T("3points") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_eCvCtrlType = CDB_ControlProperties::eCv3point;
	}
	else
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CVCTRLTYPEBAD;
	}

	// The 'm_bActuatorSelectedAsSet' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("actuatorselectedasset" ) ) )
	{
		if( 0 == m_mapInputs[_T("actuatorselectedasset")].Compare( _T("true") ) || 0 == m_mapInputs[_T("actuatorselectedasset")].Compare( _T("1") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_bActuatorSelectedAsSet = true;
		}
		else if( 0 == m_mapInputs[_T("actuatorselectedasset")].Compare( _T("false") ) || 0 == m_mapInputs[_T("actuatorselectedasset")].Compare( _T("0") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_bActuatorSelectedAsSet = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_ACTRSELASSETCHECKBAD;
		}
	}

	// The 'm_strActuatorPowerSupplyID' variable.
	// Remark: mandatory for BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("comboactuatorpowersupplyid") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_strActuatorPowerSupplyID = m_mapInputsWithCase[_T("comboactuatorpowersupplyid")];
	}

	// The 'm_strActuatorInputSignalID' variable.
	// Remark: mandatory for BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	if( 0 != m_mapInputs.count( _T("comboactuatorinputsignalid") ) )
	{
		m_pclBatchSelCtrlBaseParams->m_strActuatorInputSignalID = m_mapInputsWithCase[_T("comboactuatorinputsignalid")];
	}

	// The 'm_iActuatorFailSafeFunction' variable.
	// Remark: mandatory for BCV and PIBCV. This variable will be checked in the 'VerifyInputs' inherited method.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("actuatorfailsafefunction") ) )
	{
		if( 0 == m_mapInputs[_T("actuatorfailsafefunction" )].Compare( _T( "no") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_iActuatorFailSafeFunction = 0;
		}
		else if( 0 == m_mapInputs[_T("actuatorfailsafefunction" )].Compare( _T( "yes") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_iActuatorFailSafeFunction = 1;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_ACTUATORFAILSAFEFCTBAD;
		}
	}

	// The 'm_eActuatorDRPFunction' variable for default return position.
	// Remark: not mandatory. By default = "".
	if( 0 != m_mapInputs.count( _T("comboactuatordefaultreturnpos") ) )
	{
		if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos" )].Compare( _T( "no") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfNone;
		}
		else if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos" )].Compare( _T( "closing") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfClosing;
		}
		else if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos" )].Compare( _T( "opening") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfOpening;
		}
		else if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos" )].Compare( _T( "configurable") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfOpeningOrClosing;
		}
		else if( 0 == m_mapInputs[_T("comboactuatordefaultreturnpos" )].Compare( _T( "unknown") ) )
		{
			m_pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfAll;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_ACTUATORFAILSAFEFCTBAD;
		}
	}


	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelCtrlBaseHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUBatchSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_pclBatchSelCtrlBaseParams->m_strActuatorPowerSupplyID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_ACTPOWERSUPPLYIDEMPTY;
	}

	if( _NULL_IDPTR == m_pclBatchSelCtrlBaseParams->m_pTADB->Get( m_pclBatchSelCtrlBaseParams->m_strActuatorPowerSupplyID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_ACTPOWERSUPPLYIDINVALID;
	}

	if( true == m_pclBatchSelCtrlBaseParams->m_strActuatorInputSignalID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_ACTINPUTSIGNALIDEMPTY;
	}

	if( _NULL_IDPTR == m_pclBatchSelCtrlBaseParams->m_pTADB->Get( m_pclBatchSelCtrlBaseParams->m_strActuatorInputSignalID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_ACTINPUTSIGNALIDINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelCtrlBaseHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	UINT uiErrorCode = CTUBatchSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CBatchSelCtrlParamsBase *pclBatchSelCtrlBaseParams = dynamic_cast<CBatchSelCtrlParamsBase *>( pclProdSelParams );

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("CtrlTypeStrictChecked = %s\n"), ( false == pclBatchSelCtrlBaseParams->m_bIsCtrlTypeStrictChecked ) 
			? _T("false") : _T("true") );

	if( CDB_ControlProperties::CV2W == pclBatchSelCtrlBaseParams->m_eCV2W3W )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = CV2w\n") );
	}
	else if( CDB_ControlProperties::CV3W == pclBatchSelCtrlBaseParams->m_eCV2W3W )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = CV3w\n") );
	}
	else if( CDB_ControlProperties::CV4W == pclBatchSelCtrlBaseParams->m_eCV2W3W )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = CV4w\n") );
	}
	else if( CDB_ControlProperties::CV6W == pclBatchSelCtrlBaseParams->m_eCV2W3W )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = CV6w\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CV2w3w = Unknown\n") );
	}

	if( CDB_ControlProperties::eCvNU == pclBatchSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = No\n") );
	}
	else if( CDB_ControlProperties::eCvProportional == pclBatchSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = Proportional\n") );
	}
	else if( CDB_ControlProperties::eCvOnOff == pclBatchSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = OnOff\n") );
	}
	else if( CDB_ControlProperties::eCv3point == pclBatchSelCtrlBaseParams->m_eCvCtrlType )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = 3points\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("CVCtrlType = Unknown\n") );
	}

	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ActuatorSelectedAsSet = %s\n"), ( false == pclBatchSelCtrlBaseParams->m_bActuatorSelectedAsSet ) 
			? _T("false") : _T("true") );

	CString strString = pclBatchSelCtrlBaseParams->m_strActuatorPowerSupplyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboActuatorPowerSupplyID = %s\n"), strString );

	strString = pclBatchSelCtrlBaseParams->m_strActuatorInputSignalID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("ComboActuatorInputSignalID = %s\n"), strString );

	if( 0 == pclBatchSelCtrlBaseParams->m_iActuatorFailSafeFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ActuatorFailSafeFunction = No\n") );
	}
	else if( 1 == pclBatchSelCtrlBaseParams->m_iActuatorFailSafeFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ActuatorFailSafeFunction = Yes\n") );
	}

	if( CDB_ControlValve::DRPFunction::drpfNone == pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = No\n") );
	}
	else if( CDB_ControlValve::DRPFunction::drpfClosing == pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = Closing\n") );
	}
	else if( CDB_ControlValve::DRPFunction::drpfOpening == pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = Opening\n") );
	}
	else if( CDB_ControlValve::DRPFunction::drpfOpeningOrClosing == pclBatchSelCtrlBaseParams->m_eActuatorDRPFunction )
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = Configurable\n") );
	}
	else
	{
		TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ComboActuatorDefaultReturnPos = Unknown\n") );
	}
	
	return TU_PRODSELECT_ERROR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUBatchSelBCVHelper::CTUBatchSelBCVHelper() 
	: CTUBatchSelCtrlBaseHelper( m_clBatchSelBCVParams ) 
{
	m_clBatchSelBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_BalAndCtrlValve;
	m_clBatchSelBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	// Can verify content of all combos.
	m_bVerifyAlsoCombos = true;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Flow") );
	m_vecOutputTitles.push_back( _T("Power") );
	m_vecOutputTitles.push_back( _T("DT") );
	m_vecOutputTitles.push_back( _T("Dp") );
	m_vecOutputTitles.push_back( _T("Status") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve type") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Valve rangeability") );
	m_vecOutputTitles.push_back( _T("Valve leakage rate") );
	m_vecOutputTitles.push_back( _T("Valve stroke length") );
	m_vecOutputTitles.push_back( _T("Valve control characteristic") );
	m_vecOutputTitles.push_back( _T("Valve push or pull to close") );
	m_vecOutputTitles.push_back( _T("Valve computed presetting") );
	m_vecOutputTitles.push_back( _T("Valve displayed presetting") );
	m_vecOutputTitles.push_back( _T("Valve Dp") );
	m_vecOutputTitles.push_back( _T("Valve Dp at full opening") );
	m_vecOutputTitles.push_back( _T("Valve Dp at half opening") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
	m_vecOutputTitles.push_back( _T("Actuator ID") );
	m_vecOutputTitles.push_back( _T("Actuator name") );
	m_vecOutputTitles.push_back( _T("Actuator close-off Dp") );
	m_vecOutputTitles.push_back( _T("Actuator max. inlet pressure") );
	m_vecOutputTitles.push_back( _T("Actuator actuating time") );
	m_vecOutputTitles.push_back( _T("Actuator IP") );
	m_vecOutputTitles.push_back( _T("Actuator power supply") );
	m_vecOutputTitles.push_back( _T("Actuator input signal") );
	m_vecOutputTitles.push_back( _T("Actuator output signal") );
	m_vecOutputTitles.push_back( _T("Actuator fail safe") );
	m_vecOutputTitles.push_back( _T("default return position") );

	m_ivecOutputFlowIndex = 1;
	m_ivecOutputPowerIndex = 2;
	m_ivecOutputDTIndex = 3;
	m_ivecOutputDpIndex = 4;
}

CTUBatchSelBCVHelper::~CTUBatchSelBCVHelper() 
{
	for( int i = 0; i < (int)m_clBatchSelBCVParams.m_vecRowParameters.size(); i++ )
	{
		if( NULL != m_clBatchSelBCVParams.m_vecRowParameters.at( i ) )
		{
			if( NULL != m_clBatchSelBCVParams.m_vecRowParameters.at( i )->m_pclBatchResults )
			{
				delete m_clBatchSelBCVParams.m_vecRowParameters.at( i )->m_pclBatchResults;
			}
		}

		delete m_clBatchSelBCVParams.m_vecRowParameters.at( i );
	}

	m_clBatchSelBCVParams.m_vecRowParameters.clear();
}

UINT CTUBatchSelBCVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_NOINPUT;
	}

	if( 0 == (int)m_vecOutputValues.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_NOOUTPUT;
	}

	for( int iLoopBatch = 0; iLoopBatch < (int)m_vecOutputValues.size(); iLoopBatch++ )
	{
		// If we verify results now, we can delete results. Otherwise we keep them in 'm_clBatchSelBCVParams.m_vecRowParameters'.
		if( false == bNoVerify && NULL != m_clBatchSelBCVParams.m_pclBatchBCVList )
		{
			delete m_clBatchSelBCVParams.m_pclBatchBCVList;
			m_clBatchSelBCVParams.m_pclBatchBCVList = NULL;
		}

		m_clBatchSelBCVParams.m_pclBatchBCVList = new CBatchSelectBCVList();

		if( NULL == m_clBatchSelBCVParams.m_pclBatchBCVList )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_CANTCREATESELECTLIST;
		}

		CDlgBatchSelBase::BSRowParameters *pclRowParameters = new CDlgBatchSelBase::BSRowParameters();

		if( NULL == pclRowParameters )
		{
			delete m_clBatchSelBCVParams.m_pclBatchBCVList;
			m_clBatchSelBCVParams.m_pclBatchBCVList = NULL;
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_CANTCREATEROWPARAMS;
		}

		pclRowParameters->m_pclBatchResults = m_clBatchSelBCVParams.m_pclBatchBCVList;
		pclRowParameters->m_dFlow = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputFlowIndex] );

		if( CDS_SelProd::efdPower == m_clBatchSelBCVParams.m_eFlowOrPowerDTMode )
		{
			pclRowParameters->m_dPower = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputPowerIndex] );
			pclRowParameters->m_dDT = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDTIndex] );

			pclRowParameters->m_dFlow = CalcqFromPDT( pclRowParameters->m_dPower, pclRowParameters->m_dDT, m_clBatchSelBCVParams.m_WC.GetDens(), 
					m_clBatchSelBCVParams.m_WC.GetSpecifHeat() );
		}

		pclRowParameters->m_dDp = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDpIndex] );

		pclRowParameters->m_pclSelectedProduct = NULL;
		int iReturnCode = m_clBatchSelBCVParams.m_pclBatchBCVList->SelectBCV( &m_clBatchSelBCVParams, pclRowParameters->m_dFlow, pclRowParameters->m_dDp );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturnCode & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturnCode & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturnCode & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturnCode & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			int iActMatch = -1;
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelBCVParams.m_pclBatchBCVList->GetBestProduct();

			pclRowParameters->m_pclSelectedActuator = m_clBatchSelBCVParams.m_pclBatchBCVList->SelectActuator( &m_clBatchSelBCVParams,

					(CDB_ControlValve *)(pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ), pclRowParameters->m_dDp,  &iActMatch );

			if( false == bAlternative && iActMatch == CBatchSelectBaseList::BatchReturn::BR_FoundOneBest )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
			}
			else
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			int iActMatch = -1;
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelBCVParams.m_pclBatchBCVList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelBCVParams.m_pclBatchBCVList->GetFirst<CSelectedBase>();

				pclRowParameters->m_pclSelectedActuator = m_clBatchSelBCVParams.m_pclBatchBCVList->SelectActuator( &m_clBatchSelBCVParams,

						(CDB_ControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ), pclRowParameters->m_dDp, &iActMatch );

				if( false == bAlternative && iActMatch == CBatchSelectBaseList::BatchReturn::BR_FoundOneBest )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
				}
			}
			else
			{
				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutions;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutionsAlter;
				}
			}
		}

		if( false == bNoVerify )
		{
			_PrepareResultData( &m_clBatchSelBCVParams, pclRowParameters, iLoopBatch );

			if( m_vecResultData.size() != m_vecOutputValues[iLoopBatch].size() )
			{
				delete pclRowParameters;
				delete m_clBatchSelBCVParams.m_pclBatchBCVList;
				m_clBatchSelBCVParams.m_pclBatchBCVList = NULL;
				return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BADEXTRACT;
			}

			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopBatch][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopBatch,
							m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopBatch][iLoop], m_vecResultData[iLoop] );

					delete pclRowParameters;
					delete m_clBatchSelBCVParams.m_pclBatchBCVList;
					m_clBatchSelBCVParams.m_pclBatchBCVList = NULL;
					return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_RESULTDIFF;
				}
			}

			delete m_clBatchSelBCVParams.m_pclBatchBCVList;
			m_clBatchSelBCVParams.m_pclBatchBCVList = NULL;
		
			delete pclRowParameters;
			pclRowParameters = NULL;
		}
		else
		{
			m_clBatchSelBCVParams.m_vecRowParameters.push_back( pclRowParameters );
		}
	}

	if( false == bNoVerify && NULL != m_clBatchSelBCVParams.m_pclBatchBCVList )
	{
		delete m_clBatchSelBCVParams.m_pclBatchBCVList;
		m_clBatchSelBCVParams.m_pclBatchBCVList = NULL;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelBCVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = BCV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUBatchSelCtrlBaseHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CBatchSelBCVParams *pclBatchSelBCVParams = dynamic_cast<CBatchSelBCVParams *>( pclProdSelParams );

	if( 0 != (int)pclBatchSelBCVParams->m_vecRowParameters.size() )
	{
		int iResultCount = 0;

		for( int i = 0; i < (int)pclBatchSelBCVParams->m_vecRowParameters.size(); i++ )
		{
			_PrepareResultData( pclBatchSelBCVParams, pclBatchSelBCVParams->m_vecRowParameters[i], iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUBatchSelBCVHelper::_PrepareResultData( CBatchSelBCVParams *pclBatchSelBCVParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclBatchSelBCVParams || NULL == pclRowParameters )
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Flow or Power/DT.
	if( CDS_SelProd::efdFlow == pclBatchSelBCVParams->m_eFlowOrPowerDTMode )
	{
		// Flow.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dFlow );
		m_vecResultData.push_back( strValue );

		// Power & DT.
		m_vecResultData.push_back( _T("") );
		m_vecResultData.push_back( _T("") );
	}
	else
	{
		// Flow.
		m_vecResultData.push_back( _T("") );

		// Power.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dPower );
		m_vecResultData.push_back( strValue );
		
		// DT.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDT );
		m_vecResultData.push_back( strValue );
	}

	// Dp.
	if( true == pclBatchSelBCVParams->m_bIsDpGroupChecked )
	{
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDp );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("") );
	}

	// Status.
	if( CDlgBatchSelBase::BS_RS_FindOneSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutions == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutions") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindNoSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindNoSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindOneSolutionAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolutionWithAlternative") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutionsAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutionsWithAlternative") );
	}

	// If no solution exist for this line...
	if( NULL == pclRowParameters->m_pclSelectedProduct 
			|| NULL == dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct )
			|| NULL == dynamic_cast<CDB_ControlValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ) )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	// ID.
	CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );
	CDB_ControlValve *pclBalancingAndControlValve = dynamic_cast<CDB_ControlValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );
	m_vecResultData.push_back( pclBalancingAndControlValve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetName() );

	// Valve type.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetType() );

	// Valve body material.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetSize() );

	// Valve rangeability.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetStrRangeability().c_str() );

	// Valve leakage rate.
	strValue.Format( _T("%g"), pclBalancingAndControlValve->GetLeakageRate() );
	m_vecResultData.push_back( strValue );

	// Valve stroke.
	strValue.Format( _T("%g"), pclBalancingAndControlValve->GetStroke() );
	m_vecResultData.push_back( strValue );

	// Control characteristic.
	if( CDB_ControlProperties::Linear == pclBalancingAndControlValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("Linear") );
	}
	else if( CDB_ControlProperties::EqualPc == pclBalancingAndControlValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("EqualPc") );
	}
	else
	{
		m_vecResultData.push_back( _T("NotCharacterized") );
	}

	// Push or pull to close.
	if( CDB_ControlProperties::PushToClose == pclBalancingAndControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PushToClose") );
	}
	else if( CDB_ControlProperties::PullToClose == pclBalancingAndControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PullToClose") );
	}
	else if( CDB_ControlProperties::Undef == pclBalancingAndControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("Undef") );
	}

	// Valve computed presetting.
	strValue.Format( _T("%g"), pclSelectedValve->GetH() );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_ValveCharacteristic *pclValveCharacteristic = pclBalancingAndControlValve->GetValveCharacteristic();
	
	if( NULL != pclValveCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclValveCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclValveCharacteristic->GetSettingString( pclSelectedValve->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Dp.
	strValue.Format( _T("%g"), pclSelectedValve->GetDp() );
	m_vecResultData.push_back( strValue );

	// Dp at full and half opening.
	CString strValueDpFO = _T("");
	CString strValueDpHO = _T("");

	if( NULL != pclValveCharacteristic )
	{
		double dHMax = pclValveCharacteristic->GetOpeningMax();

		if( dHMax > 0.0 )
		{
			double dDpFullyOpen;

			if( true == pclValveCharacteristic->GetValveDp( pclRowParameters->m_dFlow, &dDpFullyOpen, dHMax, pclBatchSelBCVParams->m_WC.GetDens(), 
					pclBatchSelBCVParams->m_WC.GetKinVisc() ) )
			{
				strValueDpFO.Format( _T("%g"), dDpFullyOpen );
			}

			double dDpHalpOpen;

			if( true == pclValveCharacteristic->GetValveDp( pclRowParameters->m_dFlow, &dDpHalpOpen, dHMax / 2, pclBatchSelBCVParams->m_WC.GetDens(), 
					pclBatchSelBCVParams->m_WC.GetKinVisc() ) )
			{
				strValueDpHO.Format( _T("%g"), dDpHalpOpen );
			}
		}
	}

	m_vecResultData.push_back( strValueDpFO );
	m_vecResultData.push_back( strValueDpHO );

	// Temperature range.
	m_vecResultData.push_back( pclBalancingAndControlValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclBatchSelBCVParams );
	pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBalancingAndControlValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );

	// If no actuator found with the current valve...
	if( NULL == dynamic_cast<CDB_ElectroActuator *>( pclRowParameters->m_pclSelectedActuator ) )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclRowParameters->m_pclSelectedActuator );

	// Actuator ID.
	m_vecResultData.push_back( pclElectroActuator->GetIDPtr().ID );

	// Actuator name.
	m_vecResultData.push_back( pclElectroActuator->GetName() );

	// Actuator close-off Dp.
	CDB_CloseOffChar *pclCloseOffChar = dynamic_cast<CDB_CloseOffChar *>( pclBalancingAndControlValve->GetCloseOffCharIDPtr().MP );

	double dCloseOffDp = -1;

	if( NULL != pclCloseOffChar && CDB_CloseOffChar::CloseOffDp == pclCloseOffChar->GetLimitType() )
	{
		dCloseOffDp = pclCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );
	}

	if( -1.0 == dCloseOffDp )
	{
		strValue = _T("-");
	}
	else
	{
		strValue.Format( _T("%g"), dCloseOffDp );
	}

	m_vecResultData.push_back( strValue );

	// Actuator max. inlet pressure.
	double dMaxInletPressure = -1.0;

	if( NULL != pclCloseOffChar && CDB_CloseOffChar::InletPressure == pclCloseOffChar->GetLimitType() )
	{
		dMaxInletPressure = pclCloseOffChar->GetMaxInletPressure( pclElectroActuator->GetMaxForceTorque() );
	}

	if( -1.0 == dMaxInletPressure )
	{
		strValue = _T("-");
	}
	else
	{
		strValue.Format( _T("%g"), dMaxInletPressure );
	}

	m_vecResultData.push_back( strValue );

	// Actuator actuating time.
	strValue = pclElectroActuator->GetActuatingTimesStr( pclBalancingAndControlValve->GetStroke(), false, _T("/") );
	m_vecResultData.push_back( strValue );

	// Actuator IP.
	m_vecResultData.push_back( pclElectroActuator->GetIPxxFull() );

	// Actuator power supply.
	m_vecResultData.push_back( pclElectroActuator->GetPowerSupplyStr( _T("/") ) );

	// Actuator input signal.
	m_vecResultData.push_back( pclElectroActuator->GetInOutSignalsStr( true, _T("/") ) );

	// Actuator output signal.
	m_vecResultData.push_back( pclElectroActuator->GetInOutSignalsStr( false, _T("/") ) );

	// Actuator fail safe.
	m_vecResultData.push_back( ( 0 < (int)pclElectroActuator->GetFailSafe() ) ? _T("Yes" ) : _T( "No") );

	// Actuator default return position.
	m_vecResultData.push_back( pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for Dp controllers.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUBatchSelDpCHelper::CTUBatchSelDpCHelper() 
	: CTUBatchSelHelper( m_clBatchSelDpCParams ) 
{
	// For DpC.
	m_clBatchSelDpCParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_DpController;
	m_clBatchSelDpCParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;
	
	// For BV.
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	// Can verify content of all combos.
	m_bVerifyAlsoCombos = true;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Flow") );
	m_vecOutputTitles.push_back( _T("Power") );
	m_vecOutputTitles.push_back( _T("DT") );
	m_vecOutputTitles.push_back( _T("Dp branch") );
	m_vecOutputTitles.push_back( _T("Kvs") );
	m_vecOutputTitles.push_back( _T("Status") );
	m_vecOutputTitles.push_back( _T("DpC ID") );
	m_vecOutputTitles.push_back( _T("DpC name") );
	m_vecOutputTitles.push_back( _T("DpC type") );
	m_vecOutputTitles.push_back( _T("DpC body material") );
	m_vecOutputTitles.push_back( _T("DpC connection") );
	m_vecOutputTitles.push_back( _T("DpC version") );
	m_vecOutputTitles.push_back( _T("DpC PN") );
	m_vecOutputTitles.push_back( _T("DpC size") );
	m_vecOutputTitles.push_back( _T("DpC Dp min.") );
	m_vecOutputTitles.push_back( _T("DpC Dpl range") );
	m_vecOutputTitles.push_back( _T("DpC Dp max.") );
	m_vecOutputTitles.push_back( _T("DpC temperature range") );
	m_vecOutputTitles.push_back( _T("DpC Pipe size") );
	m_vecOutputTitles.push_back( _T("DpC Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("DpC Pipe velocity") );
	m_vecOutputTitles.push_back( _T("BV ID") );
	m_vecOutputTitles.push_back( _T("BV name") );
	m_vecOutputTitles.push_back( _T("BV in") );
	m_vecOutputTitles.push_back( _T("BV type") );
	m_vecOutputTitles.push_back( _T("BV body material") );
	m_vecOutputTitles.push_back( _T("BV connection") );
	m_vecOutputTitles.push_back( _T("BV version") );
	m_vecOutputTitles.push_back( _T("BV PN") );
	m_vecOutputTitles.push_back( _T("BV size") );
	m_vecOutputTitles.push_back( _T("BV computed presetting") );
	m_vecOutputTitles.push_back( _T("BV displayed presetting") );
	m_vecOutputTitles.push_back( _T("BV Kv signal") );
	m_vecOutputTitles.push_back( _T("BV Dp") );
	m_vecOutputTitles.push_back( _T("BV Dp at full opening") );
	m_vecOutputTitles.push_back( _T("BV Dp at half opening") );
	m_vecOutputTitles.push_back( _T("BV temperature range") );
	m_vecOutputTitles.push_back( _T("BV Pipe size") );
	m_vecOutputTitles.push_back( _T("BV Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("BV Pipe velocity") );

	m_ivecOutputFlowIndex = 1;
	m_ivecOutputPowerIndex = 2;
	m_ivecOutputDTIndex = 3;
	m_ivecOutputDpbranchIndex = 4;
	m_ivecOutputKvsIndex = 5;
}

CTUBatchSelDpCHelper::~CTUBatchSelDpCHelper() 
{
	for( int i = 0; i < (int)m_clBatchSelDpCParams.m_vecRowParameters.size(); i++ )
	{
		if( NULL != m_clBatchSelDpCParams.m_vecRowParameters.at( i ) )
		{
			if( NULL != m_clBatchSelDpCParams.m_vecRowParameters.at( i )->m_pclBatchResults )
			{
				delete m_clBatchSelDpCParams.m_vecRowParameters.at( i )->m_pclBatchResults;
			}
		}

		delete m_clBatchSelDpCParams.m_vecRowParameters.at( i );
	}

	m_clBatchSelDpCParams.m_vecRowParameters.clear();
}

UINT CTUBatchSelDpCHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUBatchSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_bIsGroupDpBranchChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("groupdpbranchchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("groupdpbranchchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("groupdpbranchchecked")].Compare( _T("1") ) )
		{
			m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked = true;
		}
		else if( 0 == m_mapInputs[_T("groupdpbranchchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("groupdpbranchchecked")].Compare( _T("0") ) )
		{
			m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_GROUPDPBRANCHBAD;
		}
	}

	// The 'm_bIsGroupKvsChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("groupkvschecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("groupkvschecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("groupkvschecked")].Compare( _T("1") ) )
		{
			m_clBatchSelDpCParams.m_bIsCheckboxKvsChecked = true;
		}
		else if( 0 == m_mapInputs[_T("groupkvschecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("groupkvschecked")].Compare( _T("0") ) )
		{
			m_clBatchSelDpCParams.m_bIsCheckboxKvsChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_GROUPKVSBAD;
		}
	}

	// The 'm_bIsDpMaxChecked' variable.
	// Remark: not mandatory. By default = 'false'.
	if( 0 != m_mapInputs.count( _T("dpmaxchecked" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("true") ) || 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("1") ) )
		{
			m_clBatchSelDpCParams.m_bIsDpMaxChecked = true;
		}
		else if( 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("false") ) || 0 == m_mapInputs[_T("dpmaxchecked")].Compare( _T("0") ) )
		{
			m_clBatchSelDpCParams.m_bIsDpMaxChecked = false;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_DPMAXCHECKEDBAD;
		}
	}
	// The 'm_dDpMax' variable.
	// Remark: mandatory if 'dpmaxchecked' = 'true'.
	if( 0 != m_mapInputs.count( _T("dpmax" ) ) )
	{
		m_clBatchSelDpCParams.m_dDpMax = _wtof( m_mapInputs[_T("dpmax")] );
	}

	// The 'm_eDpStab' variable.
	// Remark: not mandatory. By default = 'branch'.
	if( 0 != m_mapInputs.count( _T("dpstabilizedon" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpstabilizedon")].Compare( _T("branch") ) )
		{
			m_clBatchSelDpCParams.m_eDpStab = eDpStab::DpStabOnBranch;
		}
		else if( 0 == m_mapInputs[_T("dpstabilizedon")].Compare( _T("controlvalve") ) )
		{
			m_clBatchSelDpCParams.m_eDpStab = eDpStab::DpStabOnCV;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_DPSTABBAD;
		}
	}

	// The 'm_eDpCLoc' variable.
	// Remark: not mandatory. By default = 'downstream'.
	if( 0 != m_mapInputs.count( _T("dpclocalization" ) ) )
	{
		if( 0 == m_mapInputs[_T("dpclocalization")].Compare( _T("downstream") ) )
		{
			m_clBatchSelDpCParams.m_eDpCLoc = eDpCLoc::DpCLocDownStream;
		}
		else if( 0 == m_mapInputs[_T("dpclocalization")].Compare( _T("upstream") ) )
		{
			m_clBatchSelDpCParams.m_eDpCLoc = eDpCLoc::DpCLocUpStream;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_DPCLOCBAD;
		}
	}

	// The 'm_eMvLoc' variable.
	// Remark: not mandatory. By default = 'primary'.
	if( 0 != m_mapInputs.count( _T("mvlocalization" ) ) )
	{
		if( 0 == m_mapInputs[_T("mvlocalization")].Compare( _T("primary") ) )
		{
			m_clBatchSelDpCParams.m_eMvLoc = eMvLoc::MvLocPrimary;
		}
		else if( 0 == m_mapInputs[_T("mvlocalization")].Compare( _T("secondary") ) )
		{
			m_clBatchSelDpCParams.m_eMvLoc = eMvLoc::MvLocSecondary;
		}
		else
		{
			return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_MVLOCBAD;
		}
	}

	// The 'm_clBatchSelBVParams.m_strComboTypeBelow65ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcombotypebelow65id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVTYPEB65IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboTypeBelow65ID = m_mapInputsWithCase[_T("bvcombotypebelow65id")];

	// The 'm_clBatchSelBVParams.m_strComboFamilyBelow65ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcombofamilybelow65id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVFAMILYB65IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboFamilyBelow65ID = m_mapInputsWithCase[_T("bvcombofamilybelow65id")];

	// The 'm_clBatchSelBVParams.m_strComboMaterialBelow65ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcombomaterialbelow65id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVMATERIALB65IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboMaterialBelow65ID = m_mapInputsWithCase[_T("bvcombomaterialbelow65id")];

	// The 'm_clBatchSelBVParams.m_strComboConnectBelow65ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcomboconnectbelow65id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVCONNECTB65IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboConnectBelow65ID = m_mapInputsWithCase[_T("bvcomboconnectbelow65id")];

	// The 'm_clBatchSelBVParams.m_strComboVersionBelow65ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcomboversionbelow65id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVVERSIONB65IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboVersionBelow65ID = m_mapInputsWithCase[_T("bvcomboversionbelow65id")];

	// The 'm_clBatchSelBVParams.m_strComboPNBelow65ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcombopnbelow65id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVPNB65IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboPNBelow65ID = m_mapInputsWithCase[_T("bvcombopnbelow65id")];

	// The 'm_clBatchSelBVParams.m_strComboTypeAbove50ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcombotypeabove50id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVTYPEA50IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboTypeAbove50ID = m_mapInputsWithCase[_T("bvcombotypeabove50id")];

	// The 'm_clBatchSelBVParams.m_strComboFamilyAbove50ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcombofamilyabove50id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVFAMILYA50IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboFamilyAbove50ID = m_mapInputsWithCase[_T("bvcombofamilyabove50id")];

	// The 'm_clBatchSelBVParams.m_strComboMaterialAbove50ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcombomaterialabove50id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVMATERIALA50IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboMaterialAbove50ID = m_mapInputsWithCase[_T("bvcombomaterialabove50id")];

	// The 'm_clBatchSelBVParams.m_strComboConnectAbove50ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcomboconnectabove50id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVCONNECTA50IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboConnectAbove50ID = m_mapInputsWithCase[_T("bvcomboconnectabove50id")];

	// The 'm_clBatchSelBVParams.m_strComboVersionAbove50ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcomboversionabove50id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVVERSIONA50IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboVersionAbove50ID = m_mapInputsWithCase[_T("bvcomboversionabove50id")];

	// The 'm_clBatchSelBVParams.m_strComboPNAbove50ID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("bvcombopnabove50id") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVPNA50IDEMPTY;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboPNAbove50ID = m_mapInputsWithCase[_T("bvcombopnabove50id")];

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelDpCHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUBatchSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_clBatchSelDpCParams.m_bIsDpMaxChecked && m_clBatchSelDpCParams.m_dDpMax <= 0.0 )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_DPMAXINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboTypeBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVTYPEB65IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboFamilyBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVFAMILYB65IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboMaterialBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVMATERIALB65IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboConnectBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVCONNECTB65IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboVersionBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVVERSIONB65IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboPNBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVPNB65IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboTypeAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVTYPEA50IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboFamilyAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVFAMILYA50IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboMaterialAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVMATERIALA50IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboConnectAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVCONNECTA50IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboVersionAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONA50IDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelDpCParams.m_pTADB->Get( m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strComboPNAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNA50IDINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelDpCHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_NOINPUT;
	}

	if( 0 == (int)m_vecOutputValues.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_NOOUTPUT;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pTADB = m_clBatchSelDpCParams.m_pTADB;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pTADS = m_clBatchSelDpCParams.m_pTADS;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_WC = m_clBatchSelDpCParams.m_WC;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pUserDB = m_clBatchSelDpCParams.m_pUserDB;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pPipeDB = m_clBatchSelDpCParams.m_pPipeDB;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_WC = m_clBatchSelDpCParams.m_WC;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strPipeSeriesID = m_clBatchSelDpCParams.m_strPipeSeriesID;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strPipeID = m_clBatchSelDpCParams.m_strPipeID;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_bEditModeRunning = m_clBatchSelDpCParams.m_bEditModeRunning;

	for( int iLoopBatch = 0; iLoopBatch < (int)m_vecOutputValues.size(); iLoopBatch++ )
	{
		// If we verify results now, we can delete results. Otherwise we keep them in 'm_clBatchSelDpCParams.m_vecRowParameters'.
		if( false == bNoVerify && NULL != m_clBatchSelDpCParams.m_pclBatchDpCList )
		{
			delete m_clBatchSelDpCParams.m_pclBatchDpCList;
			m_clBatchSelDpCParams.m_pclBatchDpCList = NULL;
		}

		m_clBatchSelDpCParams.m_pclBatchDpCList = new CBatchSelectDpCList();

		if( NULL == m_clBatchSelDpCParams.m_pclBatchDpCList )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_CANTCREATESELECTLIST;
		}

		CDlgBatchSelBase::BSRowParameters *pclRowParameters = new CDlgBatchSelBase::BSRowParameters();

		if( NULL == pclRowParameters )
		{
			delete m_clBatchSelDpCParams.m_pclBatchDpCList;
			m_clBatchSelDpCParams.m_pclBatchDpCList = NULL;
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_CANTCREATEROWPARAMS;
		}

		pclRowParameters->m_pclBatchResults = m_clBatchSelDpCParams.m_pclBatchDpCList;
		pclRowParameters->m_dFlow = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputFlowIndex] );

		if( CDS_SelProd::efdPower == m_clBatchSelDpCParams.m_eFlowOrPowerDTMode )
		{
			pclRowParameters->m_dPower = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputPowerIndex] );
			pclRowParameters->m_dDT = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDTIndex] );

			pclRowParameters->m_dFlow = CalcqFromPDT( pclRowParameters->m_dPower, pclRowParameters->m_dDT, m_clBatchSelDpCParams.m_WC.GetDens(), 
					m_clBatchSelDpCParams.m_WC.GetSpecifHeat() );
		}

		pclRowParameters->m_dDpBranch = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDpbranchIndex] );
		pclRowParameters->m_dKvs = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputKvsIndex] );

		pclRowParameters->m_pclSelectedProduct = NULL;
		int iReturnCode = m_clBatchSelDpCParams.m_pclBatchDpCList->SelectDpC( &m_clBatchSelDpCParams, pclRowParameters->m_dFlow, pclRowParameters->m_dDpBranch,
				pclRowParameters->m_dKvs );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturnCode & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturnCode & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturnCode & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturnCode & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetBestProduct();

			if( NULL != m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected() )
			{
				pclRowParameters->m_pclSelectedSecondaryProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected()->GetBestProduct();
			}

			if( false == bAlternative )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
			}
			else
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelDpCParams.m_pclBatchDpCList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetFirst<CSelectedBase>();

				if( NULL != m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected() )
				{
					pclRowParameters->m_pclSelectedSecondaryProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected()->GetBestProduct();
				}

				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
				}
			}
			else
			{
				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutions;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutionsAlter;
				}
			}
		}

		if( false == bNoVerify )
		{
			_PrepareResultData( &m_clBatchSelDpCParams, pclRowParameters, iLoopBatch );

			if( m_vecResultData.size() != m_vecOutputValues[iLoopBatch].size() )
			{
				delete pclRowParameters;
				delete m_clBatchSelDpCParams.m_pclBatchDpCList;
				m_clBatchSelDpCParams.m_pclBatchDpCList = NULL;
				return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BADEXTRACT;
			}

			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopBatch][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopBatch,
							m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopBatch][iLoop], m_vecResultData[iLoop] );

					delete pclRowParameters;
					delete m_clBatchSelDpCParams.m_pclBatchDpCList;
					m_clBatchSelDpCParams.m_pclBatchDpCList = NULL;
					return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_RESULTDIFF;
				}
			}

			delete m_clBatchSelDpCParams.m_pclBatchDpCList;
			m_clBatchSelDpCParams.m_pclBatchDpCList = NULL;
		
			delete pclRowParameters;
			pclRowParameters = NULL;
		}
		else
		{
			m_clBatchSelDpCParams.m_vecRowParameters.push_back( pclRowParameters );
		}
	}

	if( false == bNoVerify && NULL != m_clBatchSelDpCParams.m_pclBatchDpCList )
	{
		delete m_clBatchSelDpCParams.m_pclBatchDpCList;
		m_clBatchSelDpCParams.m_pclBatchDpCList = NULL;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelDpCHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = DpC\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUBatchSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CBatchSelDpCParams *pclBatchSelDpCParams = dynamic_cast<CBatchSelDpCParams *>( pclProdSelParams );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("GroupDpBranchChecked = %s\n"), ( false == pclBatchSelDpCParams->m_bIsCheckboxDpBranchChecked ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("GroupKvsChecked = %s\n"), ( false == pclBatchSelDpCParams->m_bIsCheckboxKvsChecked ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpMaxChecked = %s\n"), ( false == pclBatchSelDpCParams->m_bIsDpMaxChecked ) ? _T("false") : _T("true") );
	TU_FILEHELPER_WRITELINE_ARGFLOAT( clTUFileHelper, _T("DpMax = %.13g\n"), pclBatchSelDpCParams->m_dDpMax );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpStabilizedOn = %s\n"), ( eDpStab::DpStabOnBranch == pclBatchSelDpCParams->m_eDpStab ) ? _T("Branch") : _T("ControlValve") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("DpCLocalization = %s\n"), ( eDpCLoc::DpCLocDownStream == pclBatchSelDpCParams->m_eDpCLoc ) ? _T("DownStream") : _T("UpStream") );
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("MvLocalization = %s\n"), ( eMvLoc::MvLocPrimary == pclBatchSelDpCParams->m_eMvLoc ) ? _T("Primary") : _T("Secondary") );

	CString strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboTypeBelow65ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboFamilyBelow65ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboMaterialBelow65ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboConnectBelow65ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboVersionBelow65ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNBelow65ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboPNBelow65ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboTypeAbove50ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboFamilyAbove50ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboMaterialAbove50ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboConnectAbove50ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboVersionAbove50ID = %s\n"), strString );

	strString = pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNAbove50ID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("BVComboPNAbove50ID = %s\n"), strString );

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	if( 0 != (int)pclBatchSelDpCParams->m_vecRowParameters.size() )
	{
		int iResultCount = 0;

		for( int i = 0; i < (int)pclBatchSelDpCParams->m_vecRowParameters.size(); i++ )
		{
			_PrepareResultData( pclBatchSelDpCParams, pclBatchSelDpCParams->m_vecRowParameters[i], iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUBatchSelDpCHelper::_PrepareResultData( CBatchSelDpCParams *pclBatchSelDpCParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclBatchSelDpCParams || NULL == pclRowParameters )
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Flow or Power/DT.
	if( CDS_SelProd::efdFlow == pclBatchSelDpCParams->m_eFlowOrPowerDTMode )
	{
		// Flow.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dFlow );
		m_vecResultData.push_back( strValue );

		// Power & DT.
		m_vecResultData.push_back( _T("") );
		m_vecResultData.push_back( _T("") );
	}
	else
	{
		// Flow.
		m_vecResultData.push_back( _T("") );

		// Power.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dPower );
		m_vecResultData.push_back( strValue );
		
		// DT.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDT );
		m_vecResultData.push_back( strValue );
	}

	// HYS-1188: We split 'm_iDpCCheckDpKvs' in two variables.
	// Dp branch.
	if( DpStabOnBranch == pclBatchSelDpCParams->m_eDpStab && true == pclBatchSelDpCParams->m_bIsCheckboxDpBranchChecked )
	{
		// Dp branch.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDpBranch );
		m_vecResultData.push_back( strValue );

		// Kvs.
		m_vecResultData.push_back( _T("") );
	}
	else if( DpStabOnCV == pclBatchSelDpCParams->m_eDpStab && true == pclBatchSelDpCParams->m_bIsCheckboxKvsChecked )
	{
		// Dp branch.
		m_vecResultData.push_back( _T("") );

		// Kvs.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dKvs );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		// Dp branch and Kvs.
		m_vecResultData.push_back( _T("") );
		m_vecResultData.push_back( _T("") );
	}

	// Status.
	if( CDlgBatchSelBase::BS_RS_FindOneSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutions == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutions") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindNoSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindNoSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindOneSolutionAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolutionWithAlternative") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutionsAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutionsWithAlternative") );
	}

	// If no solution exist for this line...
	if( NULL == pclRowParameters->m_pclSelectedProduct 
			|| NULL == dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct )
			|| NULL == dynamic_cast<CDB_DpController *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ) )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	// DpC ID.
	CSelectedValve *pclSelectedDpC = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );
	CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );
	m_vecResultData.push_back( pclDpController->GetIDPtr().ID );

	// DpC name.
	m_vecResultData.push_back( pclDpController->GetName() );

	// DpC type.
	m_vecResultData.push_back( pclDpController->GetType() );

	// DpC body material.
	m_vecResultData.push_back( pclDpController->GetBodyMaterial() );

	// DpC connection.
	m_vecResultData.push_back( pclDpController->GetConnect() );

	// DpC version.
	m_vecResultData.push_back( pclDpController->GetVersion() );

	// DpC PN.
	m_vecResultData.push_back( pclDpController->GetPN().c_str() );

	// DpC size.
	m_vecResultData.push_back( pclDpController->GetSize() );

	// DpC Dpmin.
	strValue.Format( _T("%g"), pclSelectedDpC->GetDpMin() );
	m_vecResultData.push_back( strValue );

	// DpC Dpl range.
	strValue.Format( _T("[%g-%g]"), pclDpController->GetDplmin(), pclDpController->GetDplmax() );
	m_vecResultData.push_back( strValue );

	// DpC Dp max.
	strValue.Format( _T("%g"), pclDpController->GetDpmax() );
	m_vecResultData.push_back( strValue );

	// DpC temperature range.
	m_vecResultData.push_back( pclDpController->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipeDpC( pclBatchSelDpCParams );
	pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclDpController->GetSizeKey(), selPipeDpC );
	m_vecResultData.push_back( selPipeDpC.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipeDpC.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipeDpC.GetU() );
	m_vecResultData.push_back( strValue );

	CSelectedValve *pclSelectedBv = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedSecondaryProduct );
	CDB_TAProduct *pclBvTAProduct = NULL;

	if( NULL != pclSelectedBv )
	{
		pclBvTAProduct = dynamic_cast<CDB_TAProduct *>( pclSelectedBv->GetProductIDPtr().MP );
	}

	// If no regulating valve found with the current Dp controller...
	if( NULL == pclSelectedBv || NULL == pclBvTAProduct )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	CDB_ValveCharacteristic *pclBvValveCharacteristic = (CDB_ValveCharacteristic *)pclBvTAProduct->GetValveCharDataPointer();

	// BV ID.
	m_vecResultData.push_back( pclBvTAProduct->GetIDPtr().ID );

	// BV name.
	m_vecResultData.push_back( pclBvTAProduct->GetName() );

	// BV in.
	strValue = TASApp.LoadLocalizedString( ( eMvLoc::MvLocPrimary == pclBatchSelDpCParams->m_eMvLoc ) ? IDS_BATCHSELDPC_BVPRIMARY : IDS_BATCHSELDPC_BVSECONDARY );
	m_vecResultData.push_back( strValue );

	// BV type.
	m_vecResultData.push_back( pclBvTAProduct->GetType() );

	// BV body material.
	m_vecResultData.push_back( pclBvTAProduct->GetBodyMaterial() );

	// BV connection.
	m_vecResultData.push_back( pclBvTAProduct->GetConnect() );

	// BV version.
	m_vecResultData.push_back( pclBvTAProduct->GetVersion() );

	// BV PN.
	m_vecResultData.push_back( pclBvTAProduct->GetPN().c_str() );

	// BV size.
	m_vecResultData.push_back( pclBvTAProduct->GetSize() );

	// Bv computed preset.
	strValue.Format( _T("%g"), pclSelectedBv->GetH() );
	m_vecResultData.push_back( strValue );

	// Bv displayed presetting.
	CDB_ValveCharacteristic *pclValveCharacteristic = pclBvTAProduct->GetValveCharacteristic();
	
	if( NULL != pclValveCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclValveCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclValveCharacteristic->GetSettingString( pclSelectedBv->GetH() ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// BV Kv signal.
	strValue = _T("");

	if( true == pclBvTAProduct->IsKvSignalEquipped() )
	{
		strValue.Format( _T("%g"), pclSelectedBv->GetDpSignal() );
	}

	m_vecResultData.push_back( strValue );

	// BV Dp.
	strValue.Format( _T("%g"), pclSelectedBv->GetDp() );
	m_vecResultData.push_back( strValue );

	// BV Dp at full opening.
	strValue.Format( _T("%g"), pclSelectedBv->GetDpFullOpen() );
	m_vecResultData.push_back( strValue );

	// BV Dp at half opening.
	strValue.Format( _T("%g"), pclSelectedBv->GetDpHalfOpen() );
	m_vecResultData.push_back( strValue );

	// BV Temperature range.
	m_vecResultData.push_back( pclBvTAProduct->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipeBv( pclBatchSelDpCParams );
	pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBvTAProduct->GetSizeKey(), selPipeBv );
	m_vecResultData.push_back( selPipeBv.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipeBv.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipeBv.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for pressure independent balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUBatchSelPIBCVHelper::CTUBatchSelPIBCVHelper() 
	: CTUBatchSelCtrlBaseHelper( m_clBatchSelPIBCVParams ) 
{
	m_clBatchSelPIBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_PressureIndepCtrlValve;
	m_clBatchSelPIBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	// Can verify content of all combos.
	m_bVerifyAlsoCombos = true;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Flow") );
	m_vecOutputTitles.push_back( _T("Power") );
	m_vecOutputTitles.push_back( _T("DT") );
	m_vecOutputTitles.push_back( _T("Status") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve FC") );
	m_vecOutputTitles.push_back( _T("Valve type") );
	m_vecOutputTitles.push_back( _T("Valve body material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve version") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Valve rangeability") );
	m_vecOutputTitles.push_back( _T("Valve leakage rate") );
	m_vecOutputTitles.push_back( _T("Valve stroke length") );
	m_vecOutputTitles.push_back( _T("Valve control characteristic") );
	m_vecOutputTitles.push_back( _T("Valve push or pull to close") );
	m_vecOutputTitles.push_back( _T("Valve computed presetting") );
	m_vecOutputTitles.push_back( _T("Valve displayed presetting") );
	m_vecOutputTitles.push_back( _T("Valve Dp min") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear Dp") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );
	m_vecOutputTitles.push_back( _T("Actuator ID") );
	m_vecOutputTitles.push_back( _T("Actuator name") );
	m_vecOutputTitles.push_back( _T("Actuator close-off Dp") );
	m_vecOutputTitles.push_back( _T("Actuator max. inlet pressure") );
	m_vecOutputTitles.push_back( _T("Actuator actuating time") );
	m_vecOutputTitles.push_back( _T("Actuator IP") );
	m_vecOutputTitles.push_back( _T("Actuator power supply") );
	m_vecOutputTitles.push_back( _T("Actuator input signal") );
	m_vecOutputTitles.push_back( _T("Actuator output signal") );
	m_vecOutputTitles.push_back( _T("Actuator fail safe") );
	m_vecOutputTitles.push_back( _T("Actuator default return position") );
	
	m_ivecOutputFlowIndex = 1;
	m_ivecOutputPowerIndex = 2;
	m_ivecOutputDTIndex = 3;
}

CTUBatchSelPIBCVHelper::~CTUBatchSelPIBCVHelper() 
{
	for( int i = 0; i < (int)m_clBatchSelPIBCVParams.m_vecRowParameters.size(); i++ )
	{
		if( NULL != m_clBatchSelPIBCVParams.m_vecRowParameters.at( i ) )
		{
			if( NULL != m_clBatchSelPIBCVParams.m_vecRowParameters.at( i )->m_pclBatchResults )
			{
				delete m_clBatchSelPIBCVParams.m_vecRowParameters.at( i )->m_pclBatchResults;
			}
		}

		delete m_clBatchSelPIBCVParams.m_vecRowParameters.at( i );
	}

	m_clBatchSelPIBCVParams.m_vecRowParameters.clear();
}

UINT CTUBatchSelPIBCVHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_NOINPUT;
	}

	if( 0 == (int)m_vecOutputValues.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_NOOUTPUT;
	}

	for( int iLoopBatch = 0; iLoopBatch < (int)m_vecOutputValues.size(); iLoopBatch++ )
	{
		// If we verify results now, we can delete results. Otherwise we keep them in 'm_clBatchSelPIBCVParams.m_vecRowParameters'.
		if( false == bNoVerify && NULL != m_clBatchSelPIBCVParams.m_pclBatchPIBCVList )
		{
			delete m_clBatchSelPIBCVParams.m_pclBatchPIBCVList;
			m_clBatchSelPIBCVParams.m_pclBatchPIBCVList = NULL;
		}

		m_clBatchSelPIBCVParams.m_pclBatchPIBCVList = new CBatchSelectPICvList();

		if( NULL == m_clBatchSelPIBCVParams.m_pclBatchPIBCVList )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_CANTCREATESELECTLIST;
		}

		CDlgBatchSelBase::BSRowParameters *pclRowParameters = new CDlgBatchSelBase::BSRowParameters();

		if( NULL == pclRowParameters )
		{
			delete m_clBatchSelPIBCVParams.m_pclBatchPIBCVList;
			m_clBatchSelPIBCVParams.m_pclBatchPIBCVList = NULL;
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_CANTCREATEROWPARAMS;
		}

		pclRowParameters->m_pclBatchResults = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList;
		pclRowParameters->m_dFlow = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputFlowIndex] );

		if( CDS_SelProd::efdPower == m_clBatchSelPIBCVParams.m_eFlowOrPowerDTMode )
		{
			pclRowParameters->m_dPower = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputPowerIndex] );
			pclRowParameters->m_dDT = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDTIndex] );

			pclRowParameters->m_dFlow = CalcqFromPDT( pclRowParameters->m_dPower, pclRowParameters->m_dDT, m_clBatchSelPIBCVParams.m_WC.GetDens(), 
					m_clBatchSelPIBCVParams.m_WC.GetSpecifHeat() );
		}

		pclRowParameters->m_pclSelectedProduct = NULL;
		int iReturnCode = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->SelectPICv( &m_clBatchSelPIBCVParams, pclRowParameters->m_dFlow );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturnCode & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturnCode & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturnCode & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturnCode & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			int iActMatch = -1;
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->GetBestProduct();

			pclRowParameters->m_pclSelectedActuator = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->SelectActuator( &m_clBatchSelPIBCVParams,

					(CDB_ControlValve *)(pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ), pclRowParameters->m_dDp, &iActMatch );

			if( false == bAlternative && iActMatch == CBatchSelectBaseList::BatchReturn::BR_FoundOneBest )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
			}
			else
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			int iActMatch = -1;
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->GetFirst<CSelectedBase>();

				pclRowParameters->m_pclSelectedActuator = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->SelectActuator( &m_clBatchSelPIBCVParams,

						(CDB_ControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ), pclRowParameters->m_dDp,  &iActMatch );

				if( false == bAlternative && iActMatch == CBatchSelectBaseList::BatchReturn::BR_FoundOneBest )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
				}
			}
			else
			{
				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutions;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutionsAlter;
				}
			}
		}

		if( false == bNoVerify )
		{	
			_PrepareResultData( &m_clBatchSelPIBCVParams, pclRowParameters, iLoopBatch );

			if( m_vecResultData.size() != m_vecOutputValues[iLoopBatch].size() )
			{
				delete pclRowParameters;
				delete m_clBatchSelPIBCVParams.m_pclBatchPIBCVList;
				m_clBatchSelPIBCVParams.m_pclBatchPIBCVList = NULL;
				return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BADEXTRACT;
			}

			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopBatch][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopBatch,
							m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopBatch][iLoop], m_vecResultData[iLoop] );

					delete pclRowParameters;
					delete m_clBatchSelPIBCVParams.m_pclBatchPIBCVList;
					m_clBatchSelPIBCVParams.m_pclBatchPIBCVList = NULL;
					return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_RESULTDIFF;
				}
			}

			delete m_clBatchSelPIBCVParams.m_pclBatchPIBCVList;
			m_clBatchSelPIBCVParams.m_pclBatchPIBCVList = NULL;
		
			delete pclRowParameters;
			pclRowParameters = NULL;
		}
		else
		{
			m_clBatchSelPIBCVParams.m_vecRowParameters.push_back( pclRowParameters );
		}
	}

	if( false == bNoVerify && NULL != m_clBatchSelPIBCVParams.m_pclBatchPIBCVList )
	{
		delete m_clBatchSelPIBCVParams.m_pclBatchPIBCVList;
		m_clBatchSelPIBCVParams.m_pclBatchPIBCVList = NULL;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelPIBCVHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = PIBCV\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUBatchSelCtrlBaseHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CBatchSelPIBCVParams *pclBatchSelPIBCVParams = dynamic_cast<CBatchSelPIBCVParams *>( pclProdSelParams );

	if( 0 != (int)pclBatchSelPIBCVParams->m_vecRowParameters.size() )
	{
		int iResultCount = 0;

		for( int i = 0; i < (int)pclBatchSelPIBCVParams->m_vecRowParameters.size(); i++ )
		{
			_PrepareResultData( pclBatchSelPIBCVParams, pclBatchSelPIBCVParams->m_vecRowParameters[i], iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUBatchSelPIBCVHelper::_PrepareResultData( CBatchSelPIBCVParams *pclBatchSelPIBCVParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclBatchSelPIBCVParams || NULL == pclRowParameters )
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Flow or Power/DT.
	if( CDS_SelProd::efdFlow == pclBatchSelPIBCVParams->m_eFlowOrPowerDTMode )
	{
		// Flow.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dFlow );
		m_vecResultData.push_back( strValue );

		// Power & DT.
		m_vecResultData.push_back( _T("") );
		m_vecResultData.push_back( _T("") );
	}
	else
	{
		// Flow.
		m_vecResultData.push_back( _T("") );

		// Power.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dPower );
		m_vecResultData.push_back( strValue );
		
		// DT.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDT );
		m_vecResultData.push_back( strValue );
	}

	// Status.
	if( CDlgBatchSelBase::BS_RS_FindOneSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutions == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutions") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindNoSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindNoSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindOneSolutionAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolutionWithAlternative") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutionsAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutionsWithAlternative") );
	}

	// If no solution exist for this line...
	if( NULL == pclRowParameters->m_pclSelectedProduct 
			|| NULL == dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct )
			|| NULL == dynamic_cast<CDB_PIControlValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ) )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	// ID.
	CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );
	CDB_PIControlValve *pclPIControlValve = dynamic_cast<CDB_PIControlValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );
	m_vecResultData.push_back( pclPIControlValve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclPIControlValve->GetName() );

	// Valve FC.
	strValue.Format( _T("%g"), pclPIControlValve->GetFc() );
	m_vecResultData.push_back( strValue );

	// Valve type.
	m_vecResultData.push_back( pclPIControlValve->GetType() );

	// Valve body material.
	m_vecResultData.push_back( pclPIControlValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclPIControlValve->GetConnect() );

	// Valve version.
	m_vecResultData.push_back( pclPIControlValve->GetVersion() );

	// Valve PN.
	m_vecResultData.push_back( pclPIControlValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclPIControlValve->GetSize() );

	// Valve rangeability.
	m_vecResultData.push_back( pclPIControlValve->GetStrRangeability().c_str() );

	// Valve leakage rate.
	strValue.Format( _T("%g"), pclPIControlValve->GetLeakageRate() );
	m_vecResultData.push_back( strValue );

	// Valve stroke.
	strValue.Format( _T("%g"), pclPIControlValve->GetStroke() );
	m_vecResultData.push_back( strValue );

	// Control characteristic.
	if( CDB_ControlProperties::Linear == pclPIControlValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("Linear") );
	}
	else if( CDB_ControlProperties::EqualPc == pclPIControlValve->GetCtrlProp()->GetCvCtrlChar() )
	{
		m_vecResultData.push_back( _T("EqualPc") );
	}
	else
	{
		m_vecResultData.push_back( _T("NotCharacterized") );
	}

	// Push or pull to close.
	if( CDB_ControlProperties::PushToClose == pclPIControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PushToClose") );
	}
	else if( CDB_ControlProperties::PullToClose == pclPIControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("PullToClose") );
	}
	else if( CDB_ControlProperties::Undef == pclPIControlValve->GetCtrlProp()->GetCvPushClose() )
	{
		m_vecResultData.push_back( _T("Undef") );
	}

	// Valve computed presetting.
	double dComputedPresseting = pclPIControlValve->GetPresetting( pclRowParameters->m_dFlow, pclBatchSelPIBCVParams->m_WC.GetDens(), pclBatchSelPIBCVParams->m_WC.GetKinVisc() );
	strValue.Format( _T("%g"), dComputedPresseting );
	m_vecResultData.push_back( strValue );

	// Valve displayed presetting.
	CDB_PICVCharacteristic *pclPIBCVCharacteristic = pclPIControlValve->GetPICVCharacteristic();
	
	if( NULL != pclPIBCVCharacteristic )
	{
		CString strRounding;
		strRounding.Format( _T("%g"), pclPIBCVCharacteristic->GetSettingRounding() );

		CString strDisplayedSettings = pclPIBCVCharacteristic->GetSettingString( dComputedPresseting ) + _T(" (") + strRounding + _T(")");
		strValue.Format( _T("%s"), strDisplayedSettings );
		m_vecResultData.push_back( strValue );
	}
	else
	{
		m_vecResultData.push_back( _T("-") );
	}

	// Valve Dp min.
	strValue.Format( _T("%g"), pclPIControlValve->GetDpmin( pclRowParameters->m_dFlow, pclBatchSelPIBCVParams->m_WC.GetDens() ) );
	m_vecResultData.push_back( strValue );

	// Temperature range.
	m_vecResultData.push_back( pclPIControlValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclBatchSelPIBCVParams );
	pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclPIControlValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );

	// If no actuator found with the current valve...
	if( NULL == dynamic_cast<CDB_ElectroActuator *>( pclRowParameters->m_pclSelectedActuator ) )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclRowParameters->m_pclSelectedActuator );

	// Actuator ID.
	m_vecResultData.push_back( pclElectroActuator->GetIDPtr().ID );

	// Actuator name.
	m_vecResultData.push_back( pclElectroActuator->GetName() );

	// Actuator close-off Dp.
	CDB_CloseOffChar *pclCloseOffChar = dynamic_cast<CDB_CloseOffChar *>( pclPIControlValve->GetCloseOffCharIDPtr().MP );

	double dCloseOffDp = -1;

	if( NULL != pclCloseOffChar && CDB_CloseOffChar::CloseOffDp == pclCloseOffChar->GetLimitType() )
	{
		dCloseOffDp = pclCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );
	}

	if( -1.0 == dCloseOffDp )
	{
		strValue = _T("-");
	}
	else
	{
		strValue.Format( _T("%g"), dCloseOffDp );
	}

	m_vecResultData.push_back( strValue );

	// Actuator max. inlet pressure.
	double dMaxInletPressure = -1.0;

	if( NULL != pclCloseOffChar && CDB_CloseOffChar::InletPressure == pclCloseOffChar->GetLimitType() )
	{
		dMaxInletPressure = pclCloseOffChar->GetMaxInletPressure( pclElectroActuator->GetMaxForceTorque() );
	}

	if( -1.0 == dMaxInletPressure )
	{
		strValue = _T("-");
	}
	else
	{
		strValue.Format( _T("%g"), dMaxInletPressure );
	}

	m_vecResultData.push_back( strValue );

	// Actuator actuating time.
	strValue = pclElectroActuator->GetActuatingTimesStr( pclPIControlValve->GetStroke(), false, _T("/") );
	m_vecResultData.push_back( strValue );

	// Actuator IP.
	m_vecResultData.push_back( pclElectroActuator->GetIPxxFull() );

	// Actuator power supply.
	m_vecResultData.push_back( pclElectroActuator->GetPowerSupplyStr( _T("/") ) );

	// Actuator input signal.
	m_vecResultData.push_back( pclElectroActuator->GetInOutSignalsStr( true, _T("/") ) );

	// Actuator output signal.
	m_vecResultData.push_back( pclElectroActuator->GetInOutSignalsStr( false, _T("/") ) );

	// Actuator fail safe.
	m_vecResultData.push_back( ( 0 < (int)pclElectroActuator->GetFailSafe() ) ? _T("Yes" ) : _T( "No") );
	
	// Actuator default return position.
	m_vecResultData.push_back( pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for separators and air vents.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUBatchSelSeparatorHelper::CTUBatchSelSeparatorHelper() 
	: CTUBatchSelHelper( m_clBatchSelSeparatorParams ) 
{
	m_clBatchSelSeparatorParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_Separator;
	m_clBatchSelSeparatorParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	// Can't verify content of all combos.
	m_bVerifyAlsoCombos = false;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Flow") );
	m_vecOutputTitles.push_back( _T("Power") );
	m_vecOutputTitles.push_back( _T("DT") );
	m_vecOutputTitles.push_back( _T("Status") );
	m_vecOutputTitles.push_back( _T("Separator ID") );
	m_vecOutputTitles.push_back( _T("Separator name") );
	m_vecOutputTitles.push_back( _T("Separator type") );
	m_vecOutputTitles.push_back( _T("Separator material") );
	m_vecOutputTitles.push_back( _T("Separator connection") );
	m_vecOutputTitles.push_back( _T("Separator version") );
	m_vecOutputTitles.push_back( _T("Separator PN") );
	m_vecOutputTitles.push_back( _T("Separator size") );
	m_vecOutputTitles.push_back( _T("Separator Dp") );
	m_vecOutputTitles.push_back( _T("Separator temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear pressure drop") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );

	m_ivecOutputFlowIndex = 1;
	m_ivecOutputPowerIndex = 2;
	m_ivecOutputDTIndex = 3;
}

CTUBatchSelSeparatorHelper::~CTUBatchSelSeparatorHelper() 
{
	for( int i = 0; i < (int)m_clBatchSelSeparatorParams.m_vecRowParameters.size(); i++ )
	{
		if( NULL != m_clBatchSelSeparatorParams.m_vecRowParameters.at( i ) )
		{
			if( NULL != m_clBatchSelSeparatorParams.m_vecRowParameters.at( i )->m_pclBatchResults )
			{
				delete m_clBatchSelSeparatorParams.m_vecRowParameters.at( i )->m_pclBatchResults;
			}
		}

		delete m_clBatchSelSeparatorParams.m_vecRowParameters.at( i );
	}

	m_clBatchSelSeparatorParams.m_vecRowParameters.clear();
}

UINT CTUBatchSelSeparatorHelper::InterpreteInputs()
{
	UINT uiErrorCode = CTUBatchSelHelper::InterpreteInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	// The 'm_strComboTypeID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("separatorcombotypeid") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_TYPEIDEMPTY;
	}

	m_clBatchSelSeparatorParams.m_strComboTypeID = m_mapInputsWithCase[_T("separatorcombotypeid")];

	// The 'm_strComboFamilyID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("separatorcombofamilyid") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_FAMILYIDEMPTY;
	}

	m_clBatchSelSeparatorParams.m_strComboFamilyID = m_mapInputsWithCase[_T("separatorcombofamilyid")];

	// The 'm_strComboConnectID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("separatorcomboconnectid") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_CONNECTIDEMPTY;
	}

	m_clBatchSelSeparatorParams.m_strComboConnectID = m_mapInputsWithCase[_T("separatorcomboconnectid")];

	// The 'm_strComboVersionID' variable.
	// Remark: mandatory.
	if( 0 == m_mapInputs.count( _T("separatorcomboversionid") ) )
	{
		return TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_VERSIONIDEMPTY;
	}

	m_clBatchSelSeparatorParams.m_strComboVersionID = m_mapInputsWithCase[_T("separatorcomboversionid")];

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelSeparatorHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUBatchSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( _NULL_IDPTR == m_clBatchSelSeparatorParams.m_pTADB->Get( m_clBatchSelSeparatorParams.m_strComboTypeID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_TYPEIDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelSeparatorParams.m_pTADB->Get( m_clBatchSelSeparatorParams.m_strComboFamilyID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_FAMILYIDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelSeparatorParams.m_pTADB->Get( m_clBatchSelSeparatorParams.m_strComboConnectID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_CONNECTIDINVALID;
	}

	if( _NULL_IDPTR == m_clBatchSelSeparatorParams.m_pTADB->Get( m_clBatchSelSeparatorParams.m_strComboVersionID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_VERSIONIDINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelSeparatorHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_NOINPUT;
	}

	if( 0 == (int)m_vecOutputValues.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_NOOUTPUT;
	}

	for( int iLoopBatch = 0; iLoopBatch < (int)m_vecOutputValues.size(); iLoopBatch++ )
	{
		// If we verify results now, we can delete results. Otherwise we keep them in 'm_clBatchSelSeparatorParams.m_vecRowParameters'.
		if( false == bNoVerify && NULL != m_clBatchSelSeparatorParams.m_pclBatchSeparatorList )
		{
			delete m_clBatchSelSeparatorParams.m_pclBatchSeparatorList;
			m_clBatchSelSeparatorParams.m_pclBatchSeparatorList = NULL;
		}

		m_clBatchSelSeparatorParams.m_pclBatchSeparatorList = new CBatchSelectSeparatorList();

		if( NULL == m_clBatchSelSeparatorParams.m_pclBatchSeparatorList )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_CANTCREATESELECTLIST;
		}

		CDlgBatchSelBase::BSRowParameters *pclRowParameters = new CDlgBatchSelBase::BSRowParameters();

		if( NULL == pclRowParameters )
		{
			delete m_clBatchSelSeparatorParams.m_pclBatchSeparatorList;
			m_clBatchSelSeparatorParams.m_pclBatchSeparatorList = NULL;
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_CANTCREATEROWPARAMS;
		}

		pclRowParameters->m_pclBatchResults = m_clBatchSelSeparatorParams.m_pclBatchSeparatorList;
		pclRowParameters->m_dFlow = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputFlowIndex] );

		if( CDS_SelProd::efdPower == m_clBatchSelSeparatorParams.m_eFlowOrPowerDTMode )
		{
			pclRowParameters->m_dPower = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputPowerIndex] );
			pclRowParameters->m_dDT = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDTIndex] );

			pclRowParameters->m_dFlow = CalcqFromPDT( pclRowParameters->m_dPower, pclRowParameters->m_dDT, m_clBatchSelSeparatorParams.m_WC.GetDens(), 
					m_clBatchSelSeparatorParams.m_WC.GetSpecifHeat() );
		}

		pclRowParameters->m_pclSelectedProduct = NULL;
		int iReturnCode = m_clBatchSelSeparatorParams.m_pclBatchSeparatorList->SelectSeparator( &m_clBatchSelSeparatorParams, pclRowParameters->m_dFlow );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturnCode & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturnCode & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturnCode & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturnCode & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelSeparatorParams.m_pclBatchSeparatorList->GetBestProduct();

			if( false == bAlternative )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
			}
			else
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelSeparatorParams.m_pclBatchSeparatorList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelSeparatorParams.m_pclBatchSeparatorList->GetFirst<CSelectedBase>();

				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
				}
			}
			else
			{
				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutions;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutionsAlter;
				}
			}
		}

		if( false == bNoVerify )
		{
			_PrepareResultData( &m_clBatchSelSeparatorParams, pclRowParameters, iLoopBatch );

			if( m_vecResultData.size() != m_vecOutputValues[iLoopBatch].size() )
			{
				delete pclRowParameters;
				delete m_clBatchSelSeparatorParams.m_pclBatchSeparatorList;
				m_clBatchSelSeparatorParams.m_pclBatchSeparatorList = NULL;
				return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BADEXTRACT;
			}

			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopBatch][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopBatch,
							m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopBatch][iLoop], m_vecResultData[iLoop] );

					delete pclRowParameters;
					delete m_clBatchSelSeparatorParams.m_pclBatchSeparatorList;
					m_clBatchSelSeparatorParams.m_pclBatchSeparatorList = NULL;
					return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_RESULTDIFF;
				}
			}

			delete m_clBatchSelSeparatorParams.m_pclBatchSeparatorList;
			m_clBatchSelSeparatorParams.m_pclBatchSeparatorList = NULL;
		
			delete pclRowParameters;
			pclRowParameters = NULL;
		}
		else
		{
			m_clBatchSelSeparatorParams.m_vecRowParameters.push_back( pclRowParameters );
		}
	}

	if( false == bNoVerify && NULL != m_clBatchSelSeparatorParams.m_pclBatchSeparatorList )
	{
		delete m_clBatchSelSeparatorParams.m_pclBatchSeparatorList;
		m_clBatchSelSeparatorParams.m_pclBatchSeparatorList = NULL;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelSeparatorHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = Separator\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUBatchSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	CBatchSelSeparatorParams *pclBatchSelSeparatorParams = dynamic_cast<CBatchSelSeparatorParams *>( pclProdSelParams );

	CString strString = pclBatchSelSeparatorParams->m_strComboTypeID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("SeparatorComboTypeID = %s\n"), strString.MakeUpper() );

	strString = pclBatchSelSeparatorParams->m_strComboFamilyID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("SeparatorComboFamilyID = %s\n"), strString.MakeUpper() );

	strString = pclBatchSelSeparatorParams->m_strComboConnectID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("SeparatorComboConnectID = %s\n"), strString.MakeUpper() );

	strString = pclBatchSelSeparatorParams->m_strComboVersionID;
	TU_FILEHELPER_WRITELINE_ARGTEXT( clTUFileHelper, _T("SeparatorComboVersionID = %s\n"), strString.MakeUpper() );


	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	if( 0 != (int)pclBatchSelSeparatorParams->m_vecRowParameters.size() )
	{
		int iResultCount = 0;

		for( int i = 0; i < (int)pclBatchSelSeparatorParams->m_vecRowParameters.size(); i++ )
		{
			_PrepareResultData( pclBatchSelSeparatorParams, pclBatchSelSeparatorParams->m_vecRowParameters[i], iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUBatchSelSeparatorHelper::_PrepareResultData( CBatchSelSeparatorParams *pclBatchSelSeparatorParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclBatchSelSeparatorParams || NULL == pclRowParameters )
	{
		return;
	}
		
	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Flow or Power/DT.
	if( CDS_SelProd::efdFlow == pclBatchSelSeparatorParams->m_eFlowOrPowerDTMode )
	{
		// Flow.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dFlow );
		m_vecResultData.push_back( strValue );

		// Power & DT.
		m_vecResultData.push_back( _T("") );
		m_vecResultData.push_back( _T("") );
	}
	else
	{
		// Flow.
		m_vecResultData.push_back( _T("") );

		// Power.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dPower );
		m_vecResultData.push_back( strValue );
		
		// DT.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDT );
		m_vecResultData.push_back( strValue );
	}

	// Status.
	if( CDlgBatchSelBase::BS_RS_FindOneSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutions == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutions") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindNoSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindNoSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindOneSolutionAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolutionWithAlternative") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutionsAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutionsWithAlternative") );
	}

	if( NULL == pclRowParameters->m_pclSelectedProduct 
			|| NULL == dynamic_cast<CDB_Separator *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ) )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	// ID.
	CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );
	m_vecResultData.push_back( pclSeparator->GetIDPtr().ID );

	// Separator name.
	m_vecResultData.push_back( pclSeparator->GetName() );

	// Separator type.
	m_vecResultData.push_back( pclSeparator->GetType() );

	// Separator body material.
	m_vecResultData.push_back( pclSeparator->GetBodyMaterial() );

	// Separator connection.
	m_vecResultData.push_back( pclSeparator->GetConnect() );

	// Separator version.
	m_vecResultData.push_back( pclSeparator->GetVersion() );

	// Separator PN.
	m_vecResultData.push_back( pclSeparator->GetPN().c_str() );

	// Separator size.
	m_vecResultData.push_back( pclSeparator->GetSize() );

	// Separator Dp.
	strValue.Format( _T("%g"), pclRowParameters->m_pclSelectedProduct->GetDp() );
	m_vecResultData.push_back( strValue );

	// Separator temperature range.
	m_vecResultData.push_back( pclSeparator->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclBatchSelSeparatorParams );
	pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSeparator->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for smart control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTUBatchSelSmartControlValveHelper::CTUBatchSelSmartControlValveHelper() 
	: CTUBatchSelHelper( m_clBatchSelSmartControlValveParams ) 
{
	m_clBatchSelSmartControlValveParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_SmartControlValve;
	m_clBatchSelSmartControlValveParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	// Can't verify content of all combos.
	m_bVerifyAlsoCombos = false;

	m_vecOutputTitles.push_back( _T("Result number") );
	m_vecOutputTitles.push_back( _T("Flow") );
	m_vecOutputTitles.push_back( _T("Power") );
	m_vecOutputTitles.push_back( _T("DT") );
	m_vecOutputTitles.push_back( _T("Status") );
	m_vecOutputTitles.push_back( _T("Valve ID") );
	m_vecOutputTitles.push_back( _T("Valve name") );
	m_vecOutputTitles.push_back( _T("Valve material") );
	m_vecOutputTitles.push_back( _T("Valve connection") );
	m_vecOutputTitles.push_back( _T("Valve PN") );
	m_vecOutputTitles.push_back( _T("Valve size") );
	m_vecOutputTitles.push_back( _T("Valve temperature range") );
	m_vecOutputTitles.push_back( _T("Pipe size") );
	m_vecOutputTitles.push_back( _T("Pipe linear pressure drop") );
	m_vecOutputTitles.push_back( _T("Pipe velocity") );

	m_ivecOutputFlowIndex = 1;
	m_ivecOutputPowerIndex = 2;
	m_ivecOutputDTIndex = 3;
	m_ivecOutputDpIndex = 4;
}

CTUBatchSelSmartControlValveHelper::~CTUBatchSelSmartControlValveHelper() 
{
	for( int i = 0; i < (int)m_clBatchSelSmartControlValveParams.m_vecRowParameters.size(); i++ )
	{
		if( NULL != m_clBatchSelSmartControlValveParams.m_vecRowParameters.at( i ) )
		{
			if( NULL != m_clBatchSelSmartControlValveParams.m_vecRowParameters.at( i )->m_pclBatchResults )
			{
				delete m_clBatchSelSmartControlValveParams.m_vecRowParameters.at( i )->m_pclBatchResults;
			}
		}

		delete m_clBatchSelSmartControlValveParams.m_vecRowParameters.at( i );
	}

	m_clBatchSelSmartControlValveParams.m_vecRowParameters.clear();
}

UINT CTUBatchSelSmartControlValveHelper::VerifyInputs()
{
	UINT uiErrorCode = CTUBatchSelHelper::VerifyInputs();

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	if( true == m_clBatchSelSmartControlValveParams.m_strComboMaterialBelow65ID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_MATERIALB65IDEMPTY;
	}

	if( _NULL_IDPTR == m_clBatchSelSmartControlValveParams.m_pTADB->Get( m_clBatchSelSmartControlValveParams.m_strComboMaterialBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_MATERIALB65IDINVALID;
	}

	if( true == m_clBatchSelSmartControlValveParams.m_strComboConnectBelow65ID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_CONNECTB65IDEMPTY;
	}

	if( _NULL_IDPTR == m_clBatchSelSmartControlValveParams.m_pTADB->Get( m_clBatchSelSmartControlValveParams.m_strComboConnectBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_CONNECTB65IDINVALID;
	}

	if( true == m_clBatchSelSmartControlValveParams.m_strComboPNBelow65ID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_PNB65IDEMPTY;
	}

	if( _NULL_IDPTR == m_clBatchSelSmartControlValveParams.m_pTADB->Get( m_clBatchSelSmartControlValveParams.m_strComboPNBelow65ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_PNB65IDINVALID;
	}

	if( true == m_clBatchSelSmartControlValveParams.m_strComboMaterialAbove50ID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_MATERIALA50IDEMPTY;
	}

	if( _NULL_IDPTR == m_clBatchSelSmartControlValveParams.m_pTADB->Get( m_clBatchSelSmartControlValveParams.m_strComboMaterialAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_MATERIALA50IDINVALID;
	}

	if( true == m_clBatchSelSmartControlValveParams.m_strComboConnectAbove50ID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_CONNECTA50IDEMPTY;
	}

	if( _NULL_IDPTR == m_clBatchSelSmartControlValveParams.m_pTADB->Get( m_clBatchSelSmartControlValveParams.m_strComboConnectAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_CONNECTA50IDINVALID;
	}

	if( true == m_clBatchSelSmartControlValveParams.m_strComboPNAbove50ID.IsEmpty() )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_PNA50IDEMPTY;
	}

	if( _NULL_IDPTR == m_clBatchSelSmartControlValveParams.m_pTADB->Get( m_clBatchSelSmartControlValveParams.m_strComboPNAbove50ID ) )
	{
		return TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_PNA50IDINVALID;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelSmartControlValveHelper::LaunchTest( CString &strErrorDetails, bool bNoVerify )
{
	strErrorDetails = _T("");

	if( 0 == m_mapInputs.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_NOINPUT;
	}

	if( 0 == (int)m_vecOutputValues.size() )
	{
		return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_NOOUTPUT;
	}

	for( int iLoopBatch = 0; iLoopBatch < (int)m_vecOutputValues.size(); iLoopBatch++ )
	{
		// If we verify results now, we can delete results. Otherwise we keep them in 'm_clBatchSelSmartControlValveParams.m_vecRowParameters'.
		if( false == bNoVerify && NULL != m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList )
		{
			delete m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList;
			m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList = NULL;
		}

		m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList = new CBatchSelectSmartControlValveList();

		if( NULL == m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList )
		{
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_CANTCREATESELECTLIST;
		}

		CDlgBatchSelBase::BSRowParameters *pclRowParameters = new CDlgBatchSelBase::BSRowParameters();

		if( NULL == pclRowParameters )
		{
			delete m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList;
			m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList = NULL;
			return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_CANTCREATEROWPARAMS;
		}

		pclRowParameters->m_pclBatchResults = m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList;
		pclRowParameters->m_dFlow = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputFlowIndex] );

		if( CDS_SelProd::efdPower == m_clBatchSelSmartControlValveParams.m_eFlowOrPowerDTMode )
		{
			pclRowParameters->m_dPower = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputPowerIndex] );
			pclRowParameters->m_dDT = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDTIndex] );

			pclRowParameters->m_dFlow = CalcqFromPDT( pclRowParameters->m_dPower, pclRowParameters->m_dDT, m_clBatchSelSmartControlValveParams.m_WC.GetDens(), 
					m_clBatchSelSmartControlValveParams.m_WC.GetSpecifHeat() );
		}

		pclRowParameters->m_dDp = _wtof( m_vecOutputValues[iLoopBatch][m_ivecOutputDpIndex] );

		pclRowParameters->m_pclSelectedProduct = NULL;
		int iReturnCode = m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList->SelectSmartControlValve( &m_clBatchSelSmartControlValveParams, pclRowParameters->m_dFlow );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturnCode & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturnCode & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturnCode & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturnCode & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList->GetBestProduct();

			if( false == bAlternative )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
			}
			else
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList->GetFirst<CSelectedBase>();

				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
				}
			}
			else
			{
				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutions;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutionsAlter;
				}
			}
		}

		if( false == bNoVerify )
		{
			_PrepareResultData( &m_clBatchSelSmartControlValveParams, pclRowParameters, iLoopBatch );

			if( m_vecResultData.size() != m_vecOutputValues[iLoopBatch].size() )
			{
				delete pclRowParameters;
				delete m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList;
				m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList = NULL;
				return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BADEXTRACT;
			}

			// Now just compare between results and input file.
			for( int iLoop = 0; iLoop < (int)m_vecResultData.size(); iLoop++ )
			{
				if( 0 != m_vecResultData[iLoop].CompareNoCase( m_vecOutputValues[iLoopBatch][iLoop] ) )
				{
					strErrorDetails.Format( _T("Result %i: '%s' are not the same -> Input file: %s - Current result: %s"), iLoopBatch,
							m_vecOutputTitles[iLoop], m_vecOutputValues[iLoopBatch][iLoop], m_vecResultData[iLoop] );

					delete pclRowParameters;
					delete m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList;
					m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList = NULL;
					return TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_RESULTDIFF;
				}
			}

			delete m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList;
			m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList = NULL;
		
			delete pclRowParameters;
			pclRowParameters = NULL;
		}
		else
		{
			m_clBatchSelSmartControlValveParams.m_vecRowParameters.push_back( pclRowParameters );
		}
	}

	if( false == bNoVerify && NULL != m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList )
	{
		delete m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList;
		m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList = NULL;
	}

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUBatchSelSmartControlValveHelper::DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper )
{
	// Pointers have been already verified by 'CTUProdSelLauncher'.

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("ProductCategory = Smart control valve\n\n") );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all variables used to make the selection.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartInputs\n") );

	UINT uiErrorCode = CTUBatchSelHelper::DropOutSelection( pclProdSelParams, clTUFileHelper );

	if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
	{
		return uiErrorCode;
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndInputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Extract all results.
	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("StartOutputs\n") );
	
	CBatchSelSmartControlValveParams *pclBatchSelSmartControlValveParams = dynamic_cast<CBatchSelSmartControlValveParams *>( pclProdSelParams );

	if( 0 != (int)pclBatchSelSmartControlValveParams->m_vecRowParameters.size() )
	{
		int iResultCount = 0;

		for( int i = 0; i < (int)pclBatchSelSmartControlValveParams->m_vecRowParameters.size(); i++ )
		{
			_PrepareResultData( pclBatchSelSmartControlValveParams, pclBatchSelSmartControlValveParams->m_vecRowParameters[i], iResultCount++ );

			uiErrorCode = WriteVecResultInFile( clTUFileHelper );

			if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
			{
				return uiErrorCode;
			}
		}
	}

	TU_FILEHELPER_WRITELINE( clTUFileHelper, _T("EndOutputs\n\n") );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TU_PRODSELECT_ERROR_OK;
}

void CTUBatchSelSmartControlValveHelper::_PrepareResultData( CBatchSelSmartControlValveParams *pclBatchSelSmartControlValveParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters,
		int iResultCount )
{
	m_vecResultData.clear();

	if( NULL == pclBatchSelSmartControlValveParams || NULL == pclRowParameters )
	{
		return;
	}

	CString strValue;
	strValue.Format( _T("Result number = %i"), iResultCount );
	m_vecResultData.push_back( strValue );

	// Flow or Power/DT.
	if( CDS_SelProd::efdFlow == pclBatchSelSmartControlValveParams->m_eFlowOrPowerDTMode )
	{
		// Flow.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dFlow );
		m_vecResultData.push_back( strValue );

		// Power & DT.
		m_vecResultData.push_back( _T("") );
		m_vecResultData.push_back( _T("") );
	}
	else
	{
		// Flow.
		m_vecResultData.push_back( _T("") );

		// Power.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dPower );
		m_vecResultData.push_back( strValue );
		
		// DT.
		strValue.Format( _T("%.13g"), pclRowParameters->m_dDT );
		m_vecResultData.push_back( strValue );
	}

	// Status.
	if( CDlgBatchSelBase::BS_RS_FindOneSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutions == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutions") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindNoSolution == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindNoSolution") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindOneSolutionAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindOneSolutionWithAlternative") );
	}
	else if( CDlgBatchSelBase::BS_RS_FindSolutionsAlter == pclRowParameters->m_eStatus )
	{
		m_vecResultData.push_back( _T("FindSolutionsWithAlternative") );
	}

	if( NULL == pclRowParameters->m_pclSelectedProduct 
			|| NULL == dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct )
			|| NULL == dynamic_cast<CDB_TAProduct *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ) )
	{
		int iEmptyFields = (int)m_vecOutputTitles.size() - (int)m_vecResultData.size();

		for( int i = 0; i < iEmptyFields; i++ )
		{
			m_vecResultData.push_back( _T("") );
		}

		return;
	}

	// ID.
	CSelectedValve *pclSelectedSmartControlValve = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );
	CDB_TAProduct *pclSmartControlValve = dynamic_cast<CDB_TAProduct *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );
	m_vecResultData.push_back( pclSmartControlValve->GetIDPtr().ID );

	// Valve name.
	m_vecResultData.push_back( pclSmartControlValve->GetName() );

	// Valve body material.
	m_vecResultData.push_back( pclSmartControlValve->GetBodyMaterial() );

	// Valve connection.
	m_vecResultData.push_back( pclSmartControlValve->GetConnect() );

	// Valve PN.
	m_vecResultData.push_back( pclSmartControlValve->GetPN().c_str() );

	// Valve size.
	m_vecResultData.push_back( pclSmartControlValve->GetSize() );

	// Valve temperature range.
	m_vecResultData.push_back( pclSmartControlValve->GetTempRange() );

	// Pipe size.
	CSelectPipe selPipe( pclBatchSelSmartControlValveParams );
	pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSmartControlValve->GetSizeKey(), selPipe );
	m_vecResultData.push_back( selPipe.GetpPipe()->GetName() );

	// Pipe linear pressure drop.
	strValue.Format( _T("%g"), selPipe.GetLinDp() );
	m_vecResultData.push_back( strValue );

	// Pipe velocity.
	strValue.Format( _T("%g"), selPipe.GetU() );
	m_vecResultData.push_back( strValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main class to launch all unit tests in regards in what's in the input file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTUProdSelLauncher::CTUProdSelLauncher()
{
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUserDB = NULL;
	m_pPipeDB = NULL;
	m_pclProdSelHelper = NULL;
	m_iDropNumbers = 0;
	m_pclTechParamBackup = NULL;
	m_bWaterCharSaved = false;
}

CTUProdSelLauncher::~CTUProdSelLauncher()
{
	_Clean();
}

UINT CTUProdSelLauncher::ExecuteTest( CString strInputFileName, CString &strOutputString, CListBox *pclOutputBox )
{
	if( NULL == m_pTADB )
	{
		TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_TADBNOTDEFINED, strOutputString, pclOutputBox );
	}

	if( NULL == m_pTADS )
	{
		TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_TADSNOTDEFINED, strOutputString, pclOutputBox );
	}

	_Clean();

	_OutputMessage( _T("Test is running..."), pclOutputBox );

	UINT uiErrorCode = m_clTUExecuteFileHelper.OpenFile( strInputFileName, _T("r") );
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

	// Save a copy of the current technical parameters.
	IDPTR TechParamIDPtr;

	try
	{
		m_pTADB->CreateObject( TechParamIDPtr, CLASS( CDS_TechnicalParameter ), _T("PARAM_TECHSAVED") );
	}
	catch( CHySelectException &clHySelectException )
	{
		// To avoid compilation warning.
		clHySelectException.ClearMessages();

		TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_CANTCREATETECHPARAMBACKUP, strOutputString, pclOutputBox );
	}

	m_pclTechParamBackup = dynamic_cast<CDS_TechnicalParameter *>( TechParamIDPtr.MP );

	if( NULL == m_pclTechParamBackup )
	{
		TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_ERRORWITHTECHPARAMBACKUP, strOutputString, pclOutputBox );
	}

	m_pTADS->GetpTechParams()->Copy( m_pclTechParamBackup );

	// Save also a copy of the current water characteristic.
	CDS_WaterCharacteristic *pclWaterCharacteristic = m_pTADS->GetpWCForProductSelection();

	if( NULL == pclWaterCharacteristic || NULL == pclWaterCharacteristic->GetpWCData() )
	{
		TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_ERRORWITHWATERCHARBACKUP, strOutputString, pclOutputBox );
	}

	m_clWaterCharBackup = *( pclWaterCharacteristic->GetpWCData() );
	m_bWaterCharSaved = true;

	uiErrorCode = _ReadTechnicalParameters();
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );
	
	bool bStop = false;
	CString strLine;
	CString strKey;

	// 1st line is the test number ("Test number = xxx").
	CString strTestNumber;
	uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strTestNumber, _T("=" ) ); 
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

	do 
	{
		if( 0 != strKey.Compare( _T("test number") ) )
		{
			m_clTUExecuteFileHelper.CloseFile();
			TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_CANTREADTESTNUMBER, strOutputString, pclOutputBox );
		}

		int iTestNumber = _ttoi( strTestNumber );

		CString strTestNumberText;
		strTestNumberText.Format( _T("Test number = %i"), iTestNumber );
		_OutputMessage( strTestNumberText, pclOutputBox );

		// 2nd line is the selection mode ("SelectionMode = Individual", "SelectionMode = Batch" or "SelectionMode = Wizard").
		CString strMode;
		uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strMode, _T("=" ) ); 
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		if( 0 == strMode.Compare( _T("individual") ) )
		{
			// 3rd line is the product category ("ProductCategory = BCV", "ProductCategory = BV", "ProductCategory = CV", 
			// "ProductCategory = DpC", "ProductCategory = DpCBCV", "ProductCategory = PIBCV", "ProductCategory = PM", 
			// "ProductCategory = Separator", "ProductCategory = SV" or "ProductCategory = Trv" or "ProductCategory = Safety valve" or "ProductCategory = TA-6-way valve").
			CString strProduct;
			uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strProduct, _T("=" ) ); 
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			if( 0 == strProduct.Compare( _T("bcv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("bv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelBVHelper();
			}
			else if( 0 == strProduct.Compare( _T("cv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("dpc") ) )
			{
				m_pclProdSelHelper = new CTUIndSelDpCHelper();
			}
			else if( 0 == strProduct.Compare( _T("dpcbcv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelDpCBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("pibcv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelPIBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("pm") ) )
			{
				m_pclProdSelHelper = new CTUIndSelPMHelper();
			}
			else if( 0 == strProduct.Compare( _T("separator") ) )
			{
				m_pclProdSelHelper = new CTUIndSelSeparatorHelper();
			}
			else if( 0 == strProduct.Compare( _T("sv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelSVHelper();
			}
			else if( 0 == strProduct.Compare( _T("trv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelTRVHelper();
			}
			else if( 0 == strProduct.Compare( _T("safety valve") ) )
			{
				m_pclProdSelHelper = new CTUIndSelSafetyValveHelper();
			}
			else if( 0 == strProduct.Compare( _T( "ta-6-way valve" ) ) )
			{
				m_pclProdSelHelper = new CTUIndSel6WayValveHelper();
			}
			else if( 0 == strProduct.Compare( _T( "smart control valve" ) ) )
			{
				m_pclProdSelHelper = new CTUIndSelSmartControlValveHelper();
			}
			else
			{
				m_clTUExecuteFileHelper.CloseFile();
				TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_BADPRODUCT, strOutputString, pclOutputBox );
			}
		}
		else if( 0 == strMode.Compare( _T("batch") ) )
		{
			// 3rd line is the product category ("BCV", "BV", "DpC", "PIBCV" or "Separator").
			CString strProduct;
			uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strProduct, _T("=" ) ); 
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			if( 0 == strProduct.Compare( _T("bcv") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("bv") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelBVHelper();
			}
			else if( 0 == strProduct.Compare( _T("dpc") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelDpCHelper();
			}
			else if( 0 == strProduct.Compare( _T("pibcv") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelPIBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("separator") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelSeparatorHelper();
			}
			else if( 0 == strProduct.Compare( _T("smart control valve") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelSmartControlValveHelper();
			}
			else
			{
				m_clTUExecuteFileHelper.CloseFile();
				TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_BADPRODUCT, strOutputString, pclOutputBox );
			}
		}
		else if( 0 == strMode.Compare( _T("wizard") ) )
		{
			// 3rd line is the product category ("PM").
			CString strProduct;
			uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strProduct, _T("=" ) );
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			if( 0 == strProduct.Compare( _T("pm") ) )
			{
				//TODO
			}
			else
			{
				m_clTUExecuteFileHelper.CloseFile();
				TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_BADPRODUCT, strOutputString, pclOutputBox );
			}
		}

		if( NULL == m_pclProdSelHelper )
		{
			m_clTUExecuteFileHelper.CloseFile();
			TU_RETURNERROR( TU_ERROR_LAUNCHER_EXECUTE_CTUPRODSELERROR, strOutputString, pclOutputBox );
		}

		m_pclProdSelHelper->SetpTADB( m_pTADB );
		m_pclProdSelHelper->SetpTADS( m_pTADS );
		m_pclProdSelHelper->SetpUserDB( m_pUserDB );
		m_pclProdSelHelper->SetpPipeDB( m_pPipeDB );

		uiErrorCode = m_pclProdSelHelper->ReadInputs( m_clTUExecuteFileHelper );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		uiErrorCode = m_pclProdSelHelper->InterpreteInputs();
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		uiErrorCode = m_pclProdSelHelper->VerifyInputs();
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		uiErrorCode = m_pclProdSelHelper->ReadOutputs( m_clTUExecuteFileHelper );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		CString strErrorDetails;
		uiErrorCode = m_pclProdSelHelper->LaunchTest( strErrorDetails );
		
		delete m_pclProdSelHelper;
		m_pclProdSelHelper = NULL;

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			_Clean();
			CString strTemp = strTestNumberText + _T("\r\n");
			strErrorDetails.Insert( 0, strTemp );
			TU_RETURNERROR( uiErrorCode, strOutputString, pclOutputBox, strErrorDetails );
		}

		// Check if there is one another test to launch.
		uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strTestNumber, _T("=") );
		
		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{ 
			if( TU_ERROR_FILEHELPER_READLINE_UNEXPECTEDEOF == uiErrorCode )
			{
				// End of file. End of test. OK!
				bStop = true;
			}
			else
			{
				TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );
			}
		}

		CString strMsg;
		strMsg.Format( _T("  -> End of test %i with success."), iTestNumber++ );
		_OutputMessage( strMsg, pclOutputBox );

	}while( false == bStop );

	_Clean();
	
	CString strMsg = _T("All tests in input file passed with success!");
	_OutputMessage( strMsg, pclOutputBox );

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelLauncher::DropOutCurrentSelection( CProductSelelectionParameters *pclProdSelParams, CString strOutputFileName, CString &strOutputString, 
		CListBox *pclOutputBox )
{
	if( NULL == pclProdSelParams )
	{
		TU_RETURNERROR( TU_ERROR_LAUNCHER_DROPOUTESEL_PROSELPARAMSNULL, strOutputString, pclOutputBox );
	}

	_Clean();

	_OutputMessage( _T("Export is running..."), pclOutputBox );

	UINT uiErrorCode = 0;

	if( false == m_clTUDropOutFileHelper.IsFileExist( strOutputFileName ) )
	{
		// Try to create the file.
		uiErrorCode = m_clTUDropOutFileHelper.OpenFile( strOutputFileName, _T("w") );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( _T("#Start of test definition\n\n") );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		uiErrorCode = _DropOutTechnicalParameters( pclProdSelParams->m_pTADS->GetpTechParams() );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );
	}
	else
	{
		// Try to open the file in append mode.
		uiErrorCode = m_clTUDropOutFileHelper.OpenFile( strOutputFileName, _T("a") );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );
	}

	// 1st line is the test number ("Test number = xxx").
	CString strTestNumber;
	strTestNumber.Format( _T("Test number = %i\n\n"), m_iDropNumbers );
	uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( strTestNumber );
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );
	
	if( NULL != dynamic_cast<CIndividualSelectionParameters *>( pclProdSelParams ) 
			|| NULL != dynamic_cast<CIndSelPMParams *>( pclProdSelParams ) )
	{
		uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( _T("SelectionMode = Individual\n") );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		if( NULL != dynamic_cast<CIndSelBCVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelBCVHelper();
		}
		else if( NULL != dynamic_cast<CIndSelBVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelBVHelper();
		}
		else if( NULL != dynamic_cast<CIndSelCVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelCVHelper();
		}
		else if( NULL != dynamic_cast<CIndSelDpCParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelDpCHelper();
		}
		else if( NULL != dynamic_cast<CIndSelDpCBCVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelDpCBCVHelper();
		}
		else if( NULL != dynamic_cast<CIndSelPIBCVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelPIBCVHelper();
		}
		else if( NULL != dynamic_cast<CIndSelPMParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelPMHelper();
		}
		else if( NULL != dynamic_cast<CIndSelSeparatorParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelSeparatorHelper();
		}
		else if( NULL != dynamic_cast<CIndSelSVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelSVHelper();
		}
		else if( NULL != dynamic_cast<CIndSelTRVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelTRVHelper();
		}
		else if( NULL != dynamic_cast<CIndSelSafetyValveParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelSafetyValveHelper();
		}
		else if( NULL != dynamic_cast<CIndSel6WayValveParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSel6WayValveHelper();
		}
		else if( NULL != dynamic_cast<CIndSelSmartControlValveParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUIndSelSmartControlValveHelper();
		}
		else
		{
			TU_CHECKERROR( TU_ERROR_LAUNCHER_DROPOUTESEL_PROSELPARAMSINVALID, strOutputString, pclOutputBox );
		}
	}
	else if( NULL != dynamic_cast<CBatchSelectionParameters *>( pclProdSelParams ) )
	{
		uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( _T("SelectionMode = Batch\n") );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		if( NULL != dynamic_cast<CBatchSelBCVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUBatchSelBCVHelper();
		}
		else if( NULL != dynamic_cast<CBatchSelBVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUBatchSelBVHelper();
		}
		else if( NULL != dynamic_cast<CBatchSelDpCParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUBatchSelDpCHelper();
		}
		else if( NULL != dynamic_cast<CBatchSelDpCBCVParams *>( pclProdSelParams ) )
		{
		}
		else if( NULL != dynamic_cast<CBatchSelPIBCVParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUBatchSelPIBCVHelper();
		}
		else if( NULL != dynamic_cast<CBatchSelSeparatorParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUBatchSelSeparatorHelper();
		}
		else if( NULL != dynamic_cast<CBatchSelSmartControlValveParams *>( pclProdSelParams ) )
		{
			m_pclProdSelHelper = new CTUBatchSelSmartControlValveHelper();
		}
		else
		{
			TU_CHECKERROR( TU_ERROR_LAUNCHER_DROPOUTESEL_PROSELPARAMSINVALID, strOutputString, pclOutputBox );
		}
	}
	else if( NULL != dynamic_cast<CWizardSelectionParameters *>( pclProdSelParams ) )
	{
		uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( _T("SelectionMode = Wizard\n") );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		if( NULL != dynamic_cast<CWizardSelPMParams *>( pclProdSelParams ) )
		{
		}
		else 
		{
			TU_CHECKERROR( TU_ERROR_LAUNCHER_DROPOUTESEL_PROSELPARAMSINVALID, strOutputString, pclOutputBox );
		}
	}
	else
	{
		TU_CHECKERROR( TU_ERROR_LAUNCHER_DROPOUTESEL_PROSELPARAMSINVALID, strOutputString, pclOutputBox );
	}

	if( NULL == m_pclProdSelHelper )
	{
		TU_CHECKERROR( TU_ERROR_LAUNCHER_DROPOUTESEL_CANTCREATEPRODSELHELPER, strOutputString, pclOutputBox );
	}

	uiErrorCode = m_pclProdSelHelper->DropOutSelection( pclProdSelParams, m_clTUDropOutFileHelper );
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

	m_clTUDropOutFileHelper.WriteOneLine( _T("#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n") );

	_Clean();

	m_iDropNumbers++;
	_OutputMessage( _T("Exportation done with success."), pclOutputBox );

	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelLauncher::LoadTestAndDropResults( CString strInputFileName, CString strOutputFileName, CString &strOutputString, CListBox *pclOutputBox, bool bUpdate )
{
	if( NULL == m_pTADB )
	{
		TU_RETURNERROR( TU_ERROR_LAUNCHER_LOADTESTDROPRESULTS_TADBNOTDEFINED, strOutputString, pclOutputBox );
	}

	if( NULL == m_pTADS )
	{
		TU_RETURNERROR( TU_ERROR_LAUNCHER_LOADTESTDROPRESULTS_TADSNOTDEFINED, strOutputString, pclOutputBox );
	}

	_Clean();

	_OutputMessage( _T("Update is running..."), pclOutputBox );

	// Try to open the input file.
	UINT uiErrorCode = m_clTUExecuteFileHelper.OpenFile( strInputFileName, _T("r") );
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

	// Try to create the output file.
	uiErrorCode = m_clTUDropOutFileHelper.OpenFile( strOutputFileName, _T("w") );
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

	uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( _T("#Start of test definition\n\n") );
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

	// Read technical parameters.
	uiErrorCode = _ReadTechnicalParameters();
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );
	
	// Write technical parameters.
	uiErrorCode = _DropOutTechnicalParameters( m_pTADS->GetpTechParams() );
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

	bool bStop = false;
	CString strLine;
	CString strKey;

	// 1st line is the test number ("Test number = xxx").
	CString strTestNumber;
	uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strTestNumber, _T("=" ) ); 
	TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

	int iTestNumber = 0;

	do 
	{
		if( 0 != strKey.Compare( _T("test number") ) )
		{
			TU_CHECKERROR( TU_ERROR_LAUNCHER_EXECUTE_CANTREADTESTNUMBER, strOutputString, pclOutputBox );
		}

		CString strTestNumberText;
		strTestNumberText.Format( _T("Test number = %i\n\n"), iTestNumber++ );
		uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( strTestNumberText );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		// 2nd line is the selection mode ("SelectionMode = Individual", "SelectionMode = Batch" or "SelectionMode = Wizard").
		CString strMode;
		uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strMode, _T("=" ) ); 
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		if( 0 == strMode.Compare( _T("individual") ) )
		{
			// 3rd line is the product category ("ProductCategory = BCV", "ProductCategory = BV", "ProductCategory = CV", 
			// "ProductCategory = DpC", "ProductCategory = DpCBCV", "ProductCategory = PIBCV", "ProductCategory = PM", 
			// "ProductCategory = Separator", "ProductCategory = SV" or "ProductCategory = Trv" or "ProductCategory = Safety valve").

			uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( _T("SelectionMode = Individual\n") );
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			CString strProduct;
			uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strProduct, _T("=" ) );
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			if( 0 == strProduct.Compare( _T("bcv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("bv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelBVHelper();
			}
			else if( 0 == strProduct.Compare( _T("cv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("dpc") ) )
			{
				m_pclProdSelHelper = new CTUIndSelDpCHelper();
			}
			else if( 0 == strProduct.Compare( _T("dpcbcv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelDpCBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("pibcv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelPIBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("pm") ) )
			{
				m_pclProdSelHelper = new CTUIndSelPMHelper();
			}
			else if( 0 == strProduct.Compare( _T("separator") ) )
			{
				m_pclProdSelHelper = new CTUIndSelSeparatorHelper();
			}
			else if( 0 == strProduct.Compare( _T("sv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelSVHelper();
			}
			else if( 0 == strProduct.Compare( _T("trv") ) )
			{
				m_pclProdSelHelper = new CTUIndSelTRVHelper();
			}
			else if( 0 == strProduct.Compare( _T("safety valve") ) )
			{
				m_pclProdSelHelper = new CTUIndSelSafetyValveHelper();
			}
			else if( 0 == strProduct.Compare( _T( "ta-6-way valve" ) ) )
			{
				m_pclProdSelHelper = new CTUIndSel6WayValveHelper();
			}
			else if( 0 == strProduct.Compare( _T( "smart control valve" ) ) )
			{
				m_pclProdSelHelper = new CTUIndSelSmartControlValveHelper();
			}
			else
			{
				TU_CHECKERROR( TU_ERROR_LAUNCHER_EXECUTE_BADPRODUCT, strOutputString, pclOutputBox );
			}
		}
		else if( 0 == strMode.Compare( _T("batch") ) )
		{
			// 3rd line is the product category ("BCV", "BV", "DpC", "PIBCV" or "Separator").
			uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( _T("SelectionMode = Batch\n") );
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			CString strProduct;
			uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strProduct, _T("=" ) );
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			if( 0 == strProduct.Compare( _T("bcv") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("bv") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelBVHelper();
			}
			else if( 0 == strProduct.Compare( _T("dpc") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelDpCHelper();
			}
			else if( 0 == strProduct.Compare( _T("pibcv") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelPIBCVHelper();
			}
			else if( 0 == strProduct.Compare( _T("separator") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelSeparatorHelper();
			}
			else if( 0 == strProduct.Compare( _T("smart control valve") ) )
			{
				m_pclProdSelHelper = new CTUBatchSelSmartControlValveHelper();
			}
			else
			{
				TU_CHECKERROR( TU_ERROR_LAUNCHER_EXECUTE_BADPRODUCT, strOutputString, pclOutputBox );
			}
		}
		else if( 0 == strMode.Compare( _T("wizard") ) )
		{
			// 3rd line is the product category ("PM").
			uiErrorCode = m_clTUDropOutFileHelper.WriteOneLine( _T("SelectionMode = Wizard\n") );
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			CString strProduct;
			uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strProduct, _T("=" ) );
			TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

			if( 0 == strProduct.Compare( _T("pm") ) )
			{
				//TODO
			}
			else
			{
				TU_CHECKERROR( TU_ERROR_LAUNCHER_EXECUTE_BADPRODUCT, strOutputString, pclOutputBox );
			}
		}

		if( NULL == m_pclProdSelHelper )
		{
			TU_CHECKERROR( TU_ERROR_LAUNCHER_EXECUTE_CTUPRODSELERROR, strOutputString, pclOutputBox );
		}

		m_pclProdSelHelper->SetpTADB( m_pTADB );
		m_pclProdSelHelper->SetpTADS( m_pTADS );
		m_pclProdSelHelper->SetpUserDB( m_pUserDB );
		m_pclProdSelHelper->SetpPipeDB( m_pPipeDB );

		uiErrorCode = m_pclProdSelHelper->ReadInputs( m_clTUExecuteFileHelper );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		uiErrorCode = m_pclProdSelHelper->InterpreteInputs();
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );
		
		uiErrorCode = m_pclProdSelHelper->VerifyInputs();
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		uiErrorCode = m_pclProdSelHelper->ReadOutputs( m_clTUExecuteFileHelper, bUpdate );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		CString strErrorDetails;
		uiErrorCode = m_pclProdSelHelper->LaunchTest( strErrorDetails, true );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			_Clean();
			CString strTemp = strTestNumberText + _T("\r\n");
			strErrorDetails.Insert( 0, strTemp );
			TU_RETURNERROR( uiErrorCode, strOutputString, pclOutputBox, strErrorDetails );
		}

		uiErrorCode = m_pclProdSelHelper->DropOutSelection( m_pclProdSelHelper->GetProductSelectionParameters(), m_clTUDropOutFileHelper );
		TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );

		// Check if there is one another test to launch.
		uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strTestNumber, _T("=") );
		
		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{ 
			if( TU_ERROR_FILEHELPER_READLINE_UNEXPECTEDEOF == uiErrorCode )
			{
				// End of file. End of test. OK!
				bStop = true;
			}
			else
			{
				TU_CHECKERROR( uiErrorCode, strOutputString, pclOutputBox );
			}
		}

		m_clTUDropOutFileHelper.WriteOneLine( _T("#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n") );

		delete m_pclProdSelHelper;
		m_pclProdSelHelper = NULL;

	}while( false == bStop );

	_Clean();
	_OutputMessage( _T("Update done with success."), pclOutputBox );

	return TU_PRODSELECT_ERROR_OK;
}

void CTUProdSelLauncher::_Clean()
{
	if( NULL != m_pclProdSelHelper )
	{
		delete m_pclProdSelHelper;
		m_pclProdSelHelper = NULL;
	}
	
	m_clTUExecuteFileHelper.CloseFile();
	m_clTUDropOutFileHelper.CloseFile();

	if( NULL != m_pclTechParamBackup )
	{
		// Restore the copy of the technical parameters as it was before the test.
		m_pclTechParamBackup->Copy( m_pTADS->GetpTechParams() );
		IDPTR TechParamIDPtr = m_pclTechParamBackup->GetIDPtr();
		m_pTADB->DeleteObject( TechParamIDPtr );
		m_pclTechParamBackup = NULL;
	}

	if( true == m_bWaterCharSaved )
	{
		// Restore the copy of the water characteristic as it was before the test.
		CDS_WaterCharacteristic *pclWaterCharacteristic = m_pTADS->GetpWCForProductSelection();
		*( pclWaterCharacteristic->GetpWCData() ) = m_clWaterCharBackup;
	}
}

UINT CTUProdSelLauncher::_DropOutTechnicalParameters( CDS_TechnicalParameter *pclTechParameters )
{
	if( NULL == pclTechParameters )
	{
		return TU_ERROR_LAUNCHER_DROPOUTESEL_TECHPARAMSNOTDEFINED;
	}
	
	TU_FILEHELPER_WRITELINE( m_clTUDropOutFileHelper, _T("StartTechnicalParameters\n") );

	std::map<CString, CString> mapTechParams;
	pclTechParameters->ExportTechParams( &mapTechParams );

	for( std::map<CString, CString>::iterator iter = mapTechParams.begin(); iter != mapTechParams.end(); iter++ )
	{
		CString strLine = iter->first + _T(" = ") + iter->second + _T("\n");
		TU_FILEHELPER_WRITELINE( m_clTUDropOutFileHelper, strLine );
	}

	TU_FILEHELPER_WRITELINE( m_clTUDropOutFileHelper, _T("EndTechnicalParameters\n\n") );
	return TU_PRODSELECT_ERROR_OK;
}

UINT CTUProdSelLauncher::_ReadTechnicalParameters()
{
	if( NULL == m_pTADS->GetpTechParams() )
	{
		return TU_ERROR_LAUNCHER_EXECUTE_TECHPARAMSNOTDEFINED;
	}

	CString strLine;
	TU_FILEHELPER_READLINE( m_clTUExecuteFileHelper, strLine );

	if( 0 != strLine.CompareNoCase( _T("StartTechnicalParameters") ) )
	{
		return TU_ERROR_LAUNCHER_EXECUTE_TECHPARAMSBADSTART;
	}

	std::map<CString, CString> mapTechParams;
	bool bStop = false;

	do 
	{
		CString strKey;
		CString strValue;
		UINT uiErrorCode = m_clTUExecuteFileHelper.ReadOneLineKeyValue( strLine, strKey, strValue, _T("=") );

		if( TU_PRODSELECT_ERROR_OK != uiErrorCode )
		{
			return uiErrorCode;
		}
		else if( 0 == strLine.CompareNoCase( _T("EndTechnicalParameters" ) ) )
		{
			bStop = true;
		}
		else
		{
			mapTechParams[strKey] = strValue;
		}

	}while( false == bStop );

	// Import the technical parameters for the test.
	m_pTADS->GetpTechParams()->ImportTechParams( &mapTechParams );

	return TU_PRODSELECT_ERROR_OK;
}

void CTUProdSelLauncher::_PrepareError( UINT uiErrorCode, CString &strErrorMessage, CString strErrorAddtionnalInfo )
{
	std::vector<CString> vecErrors;
	m_clErrors.GetErrorText( vecErrors, uiErrorCode );
	strErrorMessage = _T("");

	for( int i = 0; i < (int)vecErrors.size(); i++ )
	{
		strErrorMessage += vecErrors[i] + _T("\r\n");
	}

	if( false == strErrorAddtionnalInfo.IsEmpty() )
	{
		strErrorMessage += _T("  Additional info:\r\n");

		int iCurPos = 0;
		CString strValue = strErrorAddtionnalInfo.Tokenize( _T("\r\n"), iCurPos );
			
		if( false == strValue.IsEmpty() )
		{
			do 
			{
				strErrorMessage += _T("    ") + strValue + _T("\r\n");
				strValue = strErrorAddtionnalInfo.Tokenize( _T("\r\n"), iCurPos );
			}while( false == strValue.IsEmpty() );
		}
	}
}

void CTUProdSelLauncher::_OutputMessage( CString strMessage, CListBox *pclOutput )
{
	if( NULL == pclOutput )
	{
		return;
	}

	int iCurPos = 0;
	CString strValue = strMessage.Tokenize( _T("\r\n"), iCurPos );

	if( true == strValue.IsEmpty() )
	{
		return;
	}

	do
	{
		pclOutput->SetCurSel( pclOutput->InsertString( pclOutput->GetCount(), strValue ) );
		strValue = strMessage.Tokenize( _T("\r\n"), iCurPos );
	}while( false == strValue.IsEmpty() );
}

#endif
