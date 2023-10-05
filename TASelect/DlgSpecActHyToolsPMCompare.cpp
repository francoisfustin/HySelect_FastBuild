#include "stdafx.h"


#include "mainfrm.h"
#include "taselectdoc.h"
#include "Utilities.h"
#include "DlgLeftTabSelManager.h"
#include "DlgSpecActHyToolsPMCompare.h"

CDlgSpecActHyToolsPMCompare::CDlgSpecActHyToolsPMCompare( CWnd *pParent )
	: CDialogExt( CDlgSpecActHyToolsPMCompare::IDD, pParent )
{
	m_mapHyToolsNorm2HySelect[_T("EN 12828" )] = _T( "PM_NORM_EN12828");
	m_mapHyToolsNorm2HySelect[_T("SWKI/SICC/SITC HE301-01" )] = _T( "PM_NORM_SWKI_HE301_01");
	m_mapHyToolsNorm2HySelect[_T("DM 1.12.75" )] = _T( "PM_NORM_DM11275");
	m_iLargestText = 0;
	m_bPreferTecboxOnTop = false;
	m_bPreferVesselBag = true;
	SetBackgroundColor( _WHITE_DLGBOX );
}

CDlgSpecActHyToolsPMCompare::~CDlgSpecActHyToolsPMCompare()
{
	if( NULL != m_clIndSelPMParams.m_pclSelectPMList )
	{
		delete m_clIndSelPMParams.m_pclSelectPMList;
	}
}

void CDlgSpecActHyToolsPMCompare::SetInputSelParam()
{
	m_clIndSelPMParams.m_pTADB = TASApp.GetpTADB();
	m_clIndSelPMParams.m_pTADS = TASApp.GetpTADS();
	m_clIndSelPMParams.m_pUserDB = TASApp.GetpUserDB();
	m_clIndSelPMParams.m_pPipeDB = TASApp.GetpPipeDB();
}

void CDlgSpecActHyToolsPMCompare::ExportResults( CString strCompleteFileName )
{
	CPath clPath( strCompleteFileName );
	CString strApplicationType = clPath.SplitPath( CPath::epfFName );
	strApplicationType.MakeLower();
	ProjectType eProjectType;

	if( -1 != strApplicationType.Find( _T("heating") ) )
	{
		eProjectType = ProjectType::Heating;
	}
	else if( -1 != strApplicationType.Find( _T("cooling") ) )
	{
		eProjectType = ProjectType::Cooling;
	}
	else if( -1 != strApplicationType.Find( _T("solar") ) )
	{
		eProjectType = ProjectType::Solar;
	}
	else
	{
		_WriteToListCtrl( _T("Name of the file must be 'Heating', 'Cooling' or 'Solar'.") );
		return;
	}

	CString strOutputFileName;
	strOutputFileName = strCompleteFileName;
	strOutputFileName.Insert( strOutputFileName.ReverseFind( _T( '.' ) ), _T("-OUT") );

	if( false == m_clInputFile.IsFileExist( strCompleteFileName ) )
	{
		CString strMsg;
		strMsg.Format( _T("File '%' doesn't exist."), strCompleteFileName );
		_WriteToListCtrl( strMsg );
		return;
	}

	if( TU_PRODSELECT_ERROR_OK != m_clInputFile.OpenFile( strCompleteFileName, _T("r") ) )
	{
		CString strMsg;
		strMsg.Format( _T("Can't open the '%' file."), strCompleteFileName );
		_WriteToListCtrl( strMsg );
		return;
	}

	if( TU_PRODSELECT_ERROR_OK != m_clOutputFile.OpenFile( strOutputFileName, _T("w") ) )
	{
		CString strMsg;
		strMsg.Format( _T("Can't create the '%' file."), strOutputFileName );
		_WriteToListCtrl( strMsg );
		return;
	}

	CString strLine;
	std::vector<CString> vecStrings;
	bool bCanContinue = true;
	_SetDefaultValues( eProjectType );

	_LaunchExport();

	m_clInputFile.CloseFile();
	m_clOutputFile.CloseFile();
}

BEGIN_MESSAGE_MAP( CDlgSpecActHyToolsPMCompare, CDialogExt )
	ON_BN_CLICKED( IDC_BUTTONOPENINPUTFILE, &OnBnClickedOpenInputFileName )
	ON_BN_CLICKED( IDC_BUTTONSTARTCOMPARE, &OnBnClickedStartCompare )
	ON_BN_CLICKED( IDC_BUTTONSTARTEXPORT, &OnBnClickedStartExport )
	ON_BN_CLICKED( IDC_BUTTONCLEAR, &OnBnClickedClear )
END_MESSAGE_MAP()

void CDlgSpecActHyToolsPMCompare::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITINPUTFILENAME, m_InputFileName );
	DDX_Control( pDX, IDC_LIST, m_List );
}

BOOL CDlgSpecActHyToolsPMCompare::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CString str = TASApp.GetProfileString( _T("Testing"), _T("HyToolsPMCompareInputFilename"), _T("") );
	m_InputFileName.SetWindowText( str );

	m_List.ModifyStyle( LBS_SORT, 0 );

	m_clIndSelPMParams.m_pTADB = TASApp.GetpTADB();
	m_clIndSelPMParams.m_pTADS = TASApp.GetpTADS();
	m_clIndSelPMParams.m_pUserDB = TASApp.GetpUserDB();
	m_clIndSelPMParams.m_pPipeDB = TASApp.GetpPipeDB();

	return TRUE;
}

void CDlgSpecActHyToolsPMCompare::OnBnClickedOpenInputFileName()
{
	CString str;
	m_InputFileName.GetWindowText( str );
	CString strFilter = _T("Input product selection files (*.csv)|*.csv||");
	CFileDialog dlg( TRUE, _T("txt"), str, OFN_EXTENSIONDIFFERENT, (LPCTSTR)strFilter, NULL, 0, FALSE );

	if( IDOK == dlg.DoModal() )
	{
		m_InputFileName.SetWindowText( dlg.GetPathName() );
		TASApp.WriteProfileString( _T("Testing"), _T("HyToolsPMCompareInputFilename"), (LPCTSTR)dlg.GetPathName() );
	}
}

void CDlgSpecActHyToolsPMCompare::OnBnClickedStartCompare()
{
	CString strInputFileName;
	m_InputFileName.GetWindowText( strInputFileName );

	CPath clPath( strInputFileName );
	CString strApplicationType = clPath.SplitPath( CPath::epfFName );
	strApplicationType.MakeLower();
	ProjectType eProjectType;

	if( -1 != strApplicationType.Find( _T("heating") ) )
	{
		eProjectType = ProjectType::Heating;
	}
	else if( -1 != strApplicationType.Find( _T("cooling") ) )
	{
		eProjectType = ProjectType::Cooling;
	}
	else if( -1 != strApplicationType.Find( _T("solar") ) )
	{
		eProjectType = ProjectType::Solar;
	}
	else
	{
		_WriteToListCtrl( _T("Name of the file must be 'Heating', 'Cooling' or 'Solar'.") );
		return;
	}

	CString strOutputFileName;
	strOutputFileName = strInputFileName;
	strOutputFileName.Insert( strOutputFileName.ReverseFind( _T( '.' ) ), _T("-OUT") );

	CString strReportFileName;
	strReportFileName = strInputFileName;
	strReportFileName.Insert( strReportFileName.ReverseFind( _T( '.' ) ), _T("-REPORT") );

	if( false == m_clInputFile.IsFileExist( strInputFileName ) )
	{
		CString strMsg;
		strMsg.Format( _T("File '%' doesn't exist."), strInputFileName );
		_WriteToListCtrl( strMsg );
		return;
	}

	if( TU_PRODSELECT_ERROR_OK != m_clInputFile.OpenFile( strInputFileName, _T("r") ) )
	{
		CString strMsg;
		strMsg.Format( _T("Can't open the '%' file."), strInputFileName );
		_WriteToListCtrl( strMsg );
		return;
	}

	CString strLine;
	std::vector<CString> vecStrings;
	bool bCanContinue = true;
	_SetDefaultValues( eProjectType );

	_LaunchTestMain();

	m_clInputFile.CloseFile();
}

void CDlgSpecActHyToolsPMCompare::OnBnClickedStartExport()
{
	CString strInputFileName;
	m_InputFileName.GetWindowText( strInputFileName );
	ExportResults( strInputFileName );
}

void CDlgSpecActHyToolsPMCompare::OnBnClickedClear()
{
	m_List.ResetContent();
}

void CDlgSpecActHyToolsPMCompare::_SetDefaultValues( ProjectType eProjectType )
{
	if( NULL != m_clIndSelPMParams.m_pclSelectPMList )
	{
		delete m_clIndSelPMParams.m_pclSelectPMList;
	}

	CDimValue::AccessUDB()->ResetToSI();

	m_clIndSelPMParams.m_pclSelectPMList = new CSelectPMList();

	m_clIndSelPMParams.m_pclSelectPMList->SetpTADB( m_clIndSelPMParams.m_pTADB );
	m_clIndSelPMParams.m_pclSelectPMList->SetpTADS( m_clIndSelPMParams.m_pTADS );

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechParam );

	if( NULL == pTechParam )
	{
		return;
	}

	CWaterChar *pWC = TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData();
	ASSERT( NULL != pWC );

	if( NULL == pWC )
	{
		return;
	}

	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();

	// In the HyTools unit test, the number of additional vessel in parallel is set to 2.
	pTechParam->SetMaxNumberOfAdditionalVesselsInParallel( 2 );
	pTechParam->SetInitialPressureMargin( 3.0e+4 );
	pTechParam->SetMinPressureMargin( 2.0e+4 );

	if( ProjectType::Heating == eProjectType )
	{
		pclPMInputUser->SetApplicationType( ProjectType::Heating );
		pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_ALL_TAB") );
		pclPMInputUser->SetNormID( _T("PM_NORM_EN12828") );
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_NONE") );
		pclPMInputUser->SetWaterMakeUpWaterTemp( pTechParam->GetDefaultWaterMakeUpTemp() );
		pclPMInputUser->SetFillTemperature( pTechParam->GetDefaultFillTemperature() );
		pclPMInputUser->SetWaterMakeUpNetworkPN( pTechParam->GetDefaultStaticPressureOfWaterNetwork() );
		pclPMInputUser->SetSafetyValveResponsePressure( 300000.0 );
		pclPMInputUser->SetSafetyTempLimiter( 100.0 );
		pclPMInputUser->SetSupplyTemperature( 75.0 );
		pclPMInputUser->SetReturnTemperature( 65.0 );
		pclPMInputUser->SetMinTemperature( 5.0 );
		pclPMInputUser->SetDegassingChecked( false );
		pclPMInputUser->SetDegassingPressureConnectPoint( 100000 );
		pclPMInputUser->SetStaticHeight( 10 );
		pclPMInputUser->SetPzChecked( BST_UNCHECKED );
		pclPMInputUser->SetPz( 0.0 );
		pclPMInputUser->SetSystemVolume( 1.882 );
		pclPMInputUser->SetInstalledPower( 100000.0 );
		pclPMInputUser->SetPressOn( PressurON::poPumpSuction );
		pclPMInputUser->SetPumpHead( 0.0 );
		pclPMInputUser->SetMaxWidth( 0.0 );
		pclPMInputUser->SetMaxHeight( 0.0 );

		// In case of heating/solar the max temperature for inlet of the degassing is the same as the return temperature.
		// Remark: Vento is always installed on the return pipe.
		pclPMInputUser->SetDegassingMaxTempConnectPoint( pTechParam->GetDefaultISHeatingTpr() );

		pclPMInputUser->SetWaterMakeUpWaterHardness( pTechParam->GetDefaultWaterHardness() );

		pclPMInputUser->GetHeatConsumersList()->Reset();
		pclPMInputUser->GetHeatGeneratorList()->Reset();
		pclPMInputUser->GetHeatingPipeList()->Reset();
	}
	else if( ProjectType::Cooling == eProjectType )
	{
		pclPMInputUser->SetApplicationType( ProjectType::Cooling );
		pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_ALL_TAB") );
		pclPMInputUser->SetNormID( _T( "" ) );
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_NONE") );
		pclPMInputUser->SetMaxTemperature( 45.0 );
		pclPMInputUser->SetSupplyTemperature( 6.0 );
		pclPMInputUser->SetReturnTemperature( 12.0 );
		pclPMInputUser->SetMinTemperature( 0.0 );
		pclPMInputUser->SetWaterMakeUpWaterTemp( pTechParam->GetDefaultFillTemperature() );
		pclPMInputUser->SetFillTemperature( 15.0 );
		pclPMInputUser->SetWaterMakeUpNetworkPN( pTechParam->GetDefaultStaticPressureOfWaterNetwork() );
		pclPMInputUser->SetWaterMakeUpWaterHardness( pTechParam->GetDefaultWaterHardness() );
		pclPMInputUser->SetSafetyValveResponsePressure( 300000.0 );
		pclPMInputUser->SetDegassingChecked( false );
		pclPMInputUser->SetDegassingPressureConnectPoint( 100000 );
		pclPMInputUser->SetStaticHeight( 10.0 );
		pclPMInputUser->SetPzChecked( BST_UNCHECKED );
		pclPMInputUser->SetPz( 0.0 );
		pclPMInputUser->SetSystemVolume( 1.3969 );
		pclPMInputUser->SetInstalledPower( 100000.0 );
		pclPMInputUser->SetPressOn( PressurON::poPumpSuction );
		pclPMInputUser->SetPumpHead( 0.0 );
		pclPMInputUser->SetMaxWidth( 0.0 );
		pclPMInputUser->SetMaxHeight( 0.0 );

		// In case of cooling the max temperature for inlet of the degassing is the same as the return temperature.
		// Remark: Vento is always installed on the return pipe.
		pclPMInputUser->SetDegassingMaxTempConnectPoint( pclPMInputUser->GetReturnTemperature() );

		pclPMInputUser->GetColdConsumersList()->Reset();
		pclPMInputUser->GetColdGeneratorList()->Reset();
		pclPMInputUser->GetCoolingPipeList()->Reset();
	}
	else if( ProjectType::Solar == eProjectType )
	{
		pclPMInputUser->SetApplicationType( ProjectType::Solar );
		pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_ALL_TAB") );
		pclPMInputUser->SetNormID( _T( "" ) );
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_NONE") );
		pclPMInputUser->SetWaterMakeUpWaterTemp( pTechParam->GetDefaultWaterMakeUpTemp() );
		pclPMInputUser->SetWaterMakeUpNetworkPN( pTechParam->GetDefaultStaticPressureOfWaterNetwork() );
		pclPMInputUser->SetFillTemperature( pTechParam->GetDefaultFillTemperature() );
		pclPMInputUser->SetSafetyValveResponsePressure( 300000.0 );
		pclPMInputUser->SetSafetyTempLimiter( 100.0 );
		pclPMInputUser->SetSupplyTemperature( 60.0 );
		pclPMInputUser->SetReturnTemperature( 30.0 );
		pclPMInputUser->SetMinTemperature( 5.0 );
		pclPMInputUser->SetDegassingChecked( false );
		pclPMInputUser->SetDegassingPressureConnectPoint( 100000 );
		pclPMInputUser->SetStaticHeight( 10.0 );
		pclPMInputUser->SetPzChecked( BST_UNCHECKED );
		pclPMInputUser->SetPz( 0.0 );
		pclPMInputUser->SetSystemVolume( 0.275 );
		pclPMInputUser->SetSolarCollectorVolume( 0.025 );
		pclPMInputUser->SetInstalledPower( 100000.0 );
		pclPMInputUser->SetPressOn( PressurON::poPumpSuction );
		pclPMInputUser->SetPumpHead( 0.0 );
		pclPMInputUser->SetMaxWidth( 0.0 );
		pclPMInputUser->SetMaxHeight( 0.0 );

		// In case of heating/solar the max temperature for inlet of the degassing is the same as the return temperature.
		// Remark: Vento is always installed on the return pipe.
		pclPMInputUser->SetDegassingMaxTempConnectPoint( pTechParam->GetDefaultISSolarTpr() );

		pclPMInputUser->SetWaterMakeUpWaterHardness( pTechParam->GetDefaultWaterHardness() );

		pclPMInputUser->GetHeatConsumersList()->Reset();
		pclPMInputUser->GetHeatGeneratorList()->Reset();
		pclPMInputUser->GetHeatingPipeList()->Reset();
	}
}

void CDlgSpecActHyToolsPMCompare::_LaunchTestMain()
{
	CString strLine;
	std::vector<CString> vecStrings;
	bool bCanContinue = true;
	bool bAllTestOK = false;
	int iCountTest = 0;

	do
	{
		CString strOutput;
		strOutput.Format( _T("Test %i"), iCountTest );
		UINT uiErrorCode = m_clInputFile.ReadOneLine( strLine, false );

		if( TU_PRODSELECT_ERROR_OK == uiErrorCode )
		{
			if( false == strLine.IsEmpty() )
			{
				if( _T( ';' ) == strLine.GetAt( 0 ) )
				{
					if( 0 == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPressureMaintenanceTypeID().Compare( _T("PMT_EXPVSSL_TAB") ) )
					{
						_WriteToListCtrl( strOutput );
						bCanContinue = _LaunchTestStatico( strLine );
						iCountTest++;
					}
					else if( 0 == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPressureMaintenanceTypeID().Compare( _T("PMT_WITHCOMPRESS_TAB") ) )
					{
						_WriteToListCtrl( strOutput );
						bCanContinue = _LaunchTestCompresso( strLine );
						iCountTest++;
					}
					else if( 0 == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPressureMaintenanceTypeID().Compare( _T("PMT_WITHPUMP_TAB") ) )
					{
						_WriteToListCtrl( strOutput );
						bCanContinue = _LaunchTestTransfero( strLine );
						iCountTest++;
					}
					else
					{
						_WriteToListCtrl( _T("Pressurisation type ('Statico', 'Compresso' or 'Transfero') must be defined before launchin test.") );
					}
				}
				else
				{
					if( TU_PRODSELECT_ERROR_OK == m_clInputFile.SplitOneLineMultiValues( strLine, vecStrings, _T(";") ) )
					{
						if( 0 == vecStrings[0].CompareNoCase( _T("statico") ) )
						{
							m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( _T("PMT_EXPVSSL_TAB") );
						}
						else if( 0 == vecStrings[0].CompareNoCase( _T("compresso") ) )
						{
							m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( _T("PMT_WITHCOMPRESS_TAB") );
						}
						else if( 0 == vecStrings[0].CompareNoCase( _T("transfero") ) )
						{
							m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( _T("PMT_WITHPUMP_TAB") );
						}
						else
						{
							_WriteToListCtrl( _T("Pressurisation type must be 'Statico', 'Compresso' or 'Transfero'.") );
							bCanContinue = false;
						}
					}
					else
					{
						_WriteToListCtrl( _T("Error in the file.") );
						bCanContinue = false;
					}
				}
			}
		}
		else
		{
			bCanContinue = false;
			bAllTestOK = true;
		}
	}
	while( true == bCanContinue );

	if( true == bAllTestOK )
	{
		_WriteToListCtrl( _T( "" ) );
		_WriteToListCtrl( _T("Yeah BABY... yeah!!!") );
	}
}

bool CDlgSpecActHyToolsPMCompare::_LaunchTestStatico( CString strLine )
{
	_WriteToListCtrl( _T("    Statico test...") );

	CString strOutput = _T("    ") + strLine;
	_WriteToListCtrl( strOutput );

	int iCurrentLineNumber = m_clInputFile.GetCurrentLineNumber();
	CString strMsg;
	
	if( TU_PRODSELECT_ERROR_OK != m_clInputFile.SplitOneLineMultiValues( strLine, m_vecStrings, _T(";"), false ) )
	{
		strMsg.Format( _T("    Error -> Line %i: can't interpret this line."), iCurrentLineNumber );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( SF_Last != (int)m_vecStrings.size() )
	{
		strMsg.Format( _T("    Error -> Line %i: one test line must contain %i tokens."), iCurrentLineNumber, SF_Last );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( false == _ReadInputs() )
	{
		return false;
	}

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechParam );

	if( NULL == pTechParam )
	{
		return false;
	}

	// In HyTools, the variable is yet 'Number of vessels in parallel'.
	// For Statico, if we put in HyTools 2, we have to a max for 2 vessels in parallel.
	// For Compresso/Transfero, we can have 2 secondary vessels in parallel with the primary vessel.
	pTechParam->SetMaxNumberOfAdditionalVesselsInParallel( 1 );

	// Launch the computation.
	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetPressureMaintenanceTypeID( ( true == m_bPreferVesselBag ) ? _T("PMT_EXPVSSL_TAB") : _T("PMT_EXPVSSLMBR_TAB") );
	m_clIndSelPMParams.m_pclSelectPMList->Select();

	// Verify the expansion vessel.
	CSelectedPMBase *pclHySelectSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_ExpansionVessel );
	CSelectedVssl *pclHySelectSelectedStaticoVessel = NULL;
	CDB_Vessel *pclHySelectExpansionVessel = NULL;

	if( NULL != pclHySelectSelectedProduct )
	{
		pclHySelectSelectedStaticoVessel = dynamic_cast<CSelectedVssl *>( pclHySelectSelectedProduct );

		if( NULL == pclHySelectSelectedStaticoVessel )
		{
			strMsg.Format( _T("    Error -> Line %i: object found is not a vessel."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}
	}

	if( NULL != pclHySelectSelectedStaticoVessel && false == pclHySelectSelectedStaticoVessel->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) )
	{
		pclHySelectExpansionVessel = dynamic_cast<CDB_Vessel *>( pclHySelectSelectedStaticoVessel->GetpData() );

		if( NULL == pclHySelectExpansionVessel )
		{
			strMsg.Format( _T("    Error -> Line %i: internal error -> Can't retrieve the vessel selected."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify vessel name.
		CString strHyToolsVesselName = m_vecStrings[SF_VesselName];
		CString strHySelectVesselName = pclHySelectExpansionVessel->GetName();

		if( 0 != strHyToolsVesselName.Compare( strHySelectVesselName ) )
		{
			strMsg.Format( _T("    Error -> Line %i: vessel names are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsVesselName, strHySelectVesselName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify vessel ID.
		CString strHyToolsVesselID = m_vecStrings[SF_VesselID];
		strHyToolsVesselID.Trim();

		if( 0 != strHyToolsVesselID.CompareNoCase( pclHySelectExpansionVessel->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: vessels are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsVesselID, pclHySelectExpansionVessel->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify vessel number.
		int iHyToolsVesselNumber = _ttoi( m_vecStrings[SF_VesselNumber] );
		int iHySelectVesselNumber = pclHySelectSelectedStaticoVessel->GetNbreOfVsslNeeded();

		if( iHyToolsVesselNumber != iHySelectVesselNumber )
		{
			strMsg.Format( _T("    Error -> Line %i: vessel number are not the same -> HyTools: %i; HySelect: %i"), iCurrentLineNumber, iHyToolsVesselNumber, iHySelectVesselNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify initial pressure.
		double dHyToolsInitialPressure = _ttof( m_vecStrings[SF_InitialPressure] );
		double dHySelectInitialPressure = pclHySelectSelectedStaticoVessel->GetInitialPressure();

		if( true == _IsDoubleValueDifferent( dHyToolsInitialPressure, dHySelectInitialPressure ) )
		{
			strMsg.Format( _T("    Error -> Line %i: initial pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsInitialPressure, dHySelectInitialPressure );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify filling pressure.
		double dHyToolsFillingPressure = _ttof( m_vecStrings[SF_FillingPressure] );
		double dTotalVesselVolume = pclHySelectSelectedStaticoVessel->GetNbreOfVsslNeeded() * pclHySelectExpansionVessel->GetNominalVolume();
		double dHySelectFillingPressure = pclPMInputUser->GetIntermediatePressure( pclPMInputUser->GetFillTemperature(), pclHySelectSelectedStaticoVessel->GetWaterReserve(), dTotalVesselVolume );

		if( true == _IsDoubleValueDifferent( dHyToolsFillingPressure, dHySelectFillingPressure ) )
		{
			strMsg.Format( _T("    Error -> Line %i: filling pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsFillingPressure, dHySelectFillingPressure );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify water reserve.
		double dHyToolsWaterReserve = _ttof( m_vecStrings[SF_WaterReserve] );
		double dHySelectWaterReserve = pclHySelectSelectedStaticoVessel->GetWaterReserve();

		if( true == _IsDoubleValueDifferent( dHyToolsWaterReserve, dHySelectWaterReserve ) )
		{
			strMsg.Format( _T("    Error -> Line %i: water reserve are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsWaterReserve, dHySelectWaterReserve );
			_WriteToListCtrl( strMsg );
			return false;
		}
	
		// Verify the intermediate vessel.
		CSelectedVssl *pclHySelectSelectedIntermVssl = NULL;
		CDB_Vessel *pclHySelectIntermediateVessel = NULL;

		if( true == m_clIndSelPMParams.m_pclSelectPMList->IsIntermVesselNeeded( pclHySelectExpansionVessel ) )
		{
			m_clIndSelPMParams.m_pclSelectPMList->GetIntermVsslList( pclHySelectSelectedStaticoVessel );
			pclHySelectSelectedIntermVssl = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_IntermediateVessel ) );
		}

		CString strHyToolsIntermediateVesselName = m_vecStrings[SF_IntermediateVesselName];
		strHyToolsIntermediateVesselName.Trim();

		if( NULL != pclHySelectSelectedIntermVssl && true == strHyToolsIntermediateVesselName.IsEmpty() )
		{
			pclHySelectIntermediateVessel = dynamic_cast<CDB_Vessel *>( pclHySelectSelectedIntermVssl->GetpData() );

			strMsg.Format( _T("    Error -> Line %i: HySelect has an intermediate vessel (%s) but not HyTools."), iCurrentLineNumber, pclHySelectIntermediateVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pclHySelectSelectedIntermVssl && false == strHyToolsIntermediateVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an intermediate vessel (%s) but not HySelect."), iCurrentLineNumber, strHyToolsIntermediateVesselName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		if( NULL != pclHySelectIntermediateVessel 
				&& 0 != strHyToolsIntermediateVesselName.CompareNoCase( pclHySelectIntermediateVessel->GetName() ) )
		{
			strMsg.Format( _T("    Error -> Line %i: intermediate vessels are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsIntermediateVesselName, pclHySelectIntermediateVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Intermediate vessel ID.
		CString strHyToolsIntermediateVesselID = m_vecStrings[SF_IntermediateVesselID];
		strHyToolsIntermediateVesselID.Trim();
	
		if( NULL != pclHySelectIntermediateVessel
				&& 0 != strHyToolsIntermediateVesselID.CompareNoCase( pclHySelectIntermediateVessel->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: intermediate vessel IDs are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsIntermediateVesselID, pclHySelectIntermediateVessel->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the degassing.
		m_clIndSelPMParams.m_pclSelectPMList->SelectVentoPleno( pclHySelectSelectedStaticoVessel );
		CSelectedVento *pclHySelectSelectedTBVento = dynamic_cast<CSelectedVento *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_Vento ) );
		CDB_TBPlenoVento *pclHySelectTecBoxVento = NULL;

		CString strDegassingName = m_vecStrings[SF_DegassingName];
		strDegassingName.Trim();

		if( NULL != pclHySelectSelectedTBVento && true == strDegassingName.IsEmpty() )
		{
			pclHySelectTecBoxVento = dynamic_cast<CDB_TBPlenoVento *>( pclHySelectSelectedTBVento->GetpData() );

			strMsg.Format( _T("    Error -> Line %i: HySelect has a degassing device (%s) but not HyTools."), iCurrentLineNumber, pclHySelectTecBoxVento->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pclHySelectSelectedTBVento && false == strDegassingName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an degassing device (%s) but not HySelect."), iCurrentLineNumber, strDegassingName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Degassing ID.
		CString strHyToolsDegassingID = m_vecStrings[SF_DegassingID];
		strHyToolsDegassingID.Trim();

		if( NULL != pclHySelectTecBoxVento
				&& 0 != strHyToolsDegassingID.CompareNoCase( pclHySelectTecBoxVento->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: degassing devices are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsDegassingID, pclHySelectTecBoxVento->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}
	}

	// Verify contraction volume.
	double dHyToolsContractionVolume = _wtof( m_vecStrings[SF_ContractionVolume] );
	double dHySelectContractionVolume = pclPMInputUser->ComputeContractionVolume();

	if( true == _IsDoubleValueDifferent( dHyToolsContractionVolume, dHySelectContractionVolume ) )
	{
		strMsg.Format( _T("    Error -> Line %i: contraction volumes are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsContractionVolume, dHySelectContractionVolume );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify expansion coefficient.
	double dHyToolsExpansionCoefficient = _wtof( m_vecStrings[SF_ExpansionCoefficent] );
	double dHySelectExpansionCoefficient = pclPMInputUser->GetSystemExpansionCoefficient();

	if( true == _IsDoubleValueDifferent( dHyToolsExpansionCoefficient, dHySelectExpansionCoefficient ) )
	{
		strMsg.Format( _T("    Error -> Line %i: expansion coeffecient are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsExpansionCoefficient, dHySelectExpansionCoefficient );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify expansion volume.
	double dHyToolsExpansionVolume = _wtof( m_vecStrings[SF_ExpansionVolume] );
	double dHySelectExpansionVolume = pclPMInputUser->GetSystemExpansionVolume();

	if( true == _IsDoubleValueDifferent( dHyToolsExpansionVolume, dHySelectExpansionVolume ) )
	{
		strMsg.Format( _T("    Error -> Line %i: expansion volume are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsExpansionVolume, dHySelectExpansionVolume );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum water reserve.
	double dHyToolsMinimumWaterReserve = _wtof( m_vecStrings[SF_MinimumWaterReserve] );
	double dHySelectMinimumWaterReserve = pclPMInputUser->GetMinimumWaterReserve();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumWaterReserve, dHySelectMinimumWaterReserve ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum water reserve are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumWaterReserve, dHySelectMinimumWaterReserve );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify vapor over pressure.
	double dHyToolsVaporOverPressure = _wtof( m_vecStrings[SF_VaporOverPressure] );
	double dHySelectVaporOverPressure = pclPMInputUser->GetVaporPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsVaporOverPressure, dHySelectVaporOverPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: vapor over pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsVaporOverPressure, dHySelectVaporOverPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum PSV.
	double dHyToolsMinimumPSV = _wtof( m_vecStrings[SF_MinimumPSV] );
	double dHySelectMinimumPSV = pclPMInputUser->GetMinimumRequiredPSVRaw( CPMInputUser::MT_ExpansionVessel );

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumPSV, dHySelectMinimumPSV ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum PSV are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumPSV, dHySelectMinimumPSV );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum initial pressure.
	double dHyToolsMinimumInitialPressure = _wtof( m_vecStrings[SF_MinimumInitialPressure] );
	double dHySelectMinimumInitialPressure = pclPMInputUser->GetMinimumInitialPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumInitialPressure, dHySelectMinimumInitialPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum initial pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumInitialPressure, dHySelectMinimumInitialPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum pressure.
	double dHyToolsMinimumPressure = _wtof( m_vecStrings[SF_MinimumPressure] );
	double dHySelectMinimumPressure = pclPMInputUser->GetMinimumPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumPressure, dHySelectMinimumPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumPressure, dHySelectMinimumPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify final pressure.
	double dHyToolsFinalPressure = _wtof( m_vecStrings[SF_FinalPressure] );
	double dHySelectFinalPressure = pclPMInputUser->GetFinalPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsFinalPressure, dHySelectFinalPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: final pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsFinalPressure, dHySelectFinalPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify pressure factor.
	double dHyToolsPressureFactor = _wtof( m_vecStrings[SF_PressureFactor] );
	double dHySelectPressureFactor = pclPMInputUser->GetPressureFactor();

	if( true == _IsDoubleValueDifferent( dHyToolsPressureFactor, dHySelectPressureFactor ) )
	{
		strMsg.Format( _T("    Error -> Line %i: pressure factor are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsPressureFactor, dHySelectPressureFactor );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify nominal volume.
	double dHyToolsNominalVolume = _wtof( m_vecStrings[SF_NominalVolume] );
	double dHySelectNominalVolume = pclPMInputUser->GetVesselNominalVolume();

	if( true == _IsDoubleValueDifferent( dHyToolsNominalVolume, dHySelectNominalVolume ) )
	{
		strMsg.Format( _T("    Error -> Line %i: nominal volume are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsNominalVolume, dHySelectNominalVolume );
		_WriteToListCtrl( strMsg );
		return false;
	}

	_WriteToListCtrl( _T("    OK!") );
	return true;
}

bool CDlgSpecActHyToolsPMCompare::_LaunchTestCompresso( CString strLine )
{
	_WriteToListCtrl( _T("    Compresso test...") );

	CString strOutput = _T("    ") + strLine;
	_WriteToListCtrl( strOutput );

	int iCurrentLineNumber = m_clInputFile.GetCurrentLineNumber();
	CString strMsg;

	if( TU_PRODSELECT_ERROR_OK != m_clInputFile.SplitOneLineMultiValues( strLine, m_vecStrings, _T(";"), false ) )
	{
		strMsg.Format( _T("    Error -> Line %i: can't interpret this line."), iCurrentLineNumber );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( CF_Last != (int)m_vecStrings.size() )
	{
		strMsg.Format( _T("    Error -> Line %i: one test line must contain %i tokens."), iCurrentLineNumber, CF_Last );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( false == _ReadInputs() )
	{
		return false;
	}

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechParam );

	if( NULL == pTechParam )
	{
		return false;
	}

	// In HyTools, the variable is yet 'Number of vessels in parallel'.
	// For Statico, if we put in HyTools 2, we have to a max for 2 vessels in parallel.
	// For Compresso/Transfero, we can have 2 secondary vessels in parallel with the primary vessel.
	pTechParam->SetMaxNumberOfAdditionalVesselsInParallel( 2 );

	// Launch the computation.
	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_WITHCOMPRESS_TAB") );
	m_clIndSelPMParams.m_pclSelectPMList->Select();

	// Verify the Compresso.
	CSelectedPMBase *pclHySelectSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_Compresso );
	CSelectedCompresso *pclHySelectSelectedTecBoxCompresso = NULL;
	CDB_TecBox *pclHySelectTecBoxCompresso = NULL;

	if( NULL != pclHySelectSelectedProduct )
	{
		pclHySelectSelectedTecBoxCompresso = dynamic_cast<CSelectedCompresso *>( pclHySelectSelectedProduct );

		if( NULL == pclHySelectSelectedTecBoxCompresso )
		{
			strMsg.Format( _T("    Error -> Line %i: object found is not a Compresso."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}
	}

	if( NULL != pclHySelectSelectedTecBoxCompresso )
	{
		pclHySelectTecBoxCompresso = dynamic_cast<CDB_TecBox *>( pclHySelectSelectedTecBoxCompresso->GetpData() );

		if( NULL == pclHySelectTecBoxCompresso )
		{
			strMsg.Format( _T("    Error -> Line %i: internal error -> Can't retrieve the Compresso selected."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the Compresso name.
		CString strHyToolsCompressoName = m_vecStrings[CF_CompressoName];
		CString strHySelectCompressoName = pclHySelectTecBoxCompresso->GetName();

		// In HyTools the name for the Simply Compresso is truncated (Simply Compresso C 2.1-80 S -> C 2.1-80 S).
		if( 0 == strHyToolsCompressoName.Compare( _T("C 2.1-80 S") ) )
		{
			strHyToolsCompressoName = _T("Simply Compresso C 2.1-80 S");
		}

		if( 0 != strHyToolsCompressoName.Compare( strHySelectCompressoName ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Compresso names are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsCompressoName, strHySelectCompressoName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the Compresso ID.
		CString strHyToolsCompressoID = m_vecStrings[CF_CompressoID];
		strHyToolsCompressoID.Trim();

		if( 0 != strHyToolsCompressoID.CompareNoCase( pclHySelectTecBoxCompresso->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Compresso ID are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsCompressoID, pclHySelectTecBoxCompresso->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the Compresso vessel.
		m_clIndSelPMParams.m_pclSelectPMList->SelectCompressoVessel( pclHySelectSelectedTecBoxCompresso );
		CSelectedVssl *pclHySelectSelectedCompressoVessel = NULL;
		CDB_Vessel *pclHySelectCompressoVessel = NULL;

		if( false == pclHySelectTecBoxCompresso->IsVariantIntegratedPrimaryVessel() )
		{
			pclHySelectSelectedCompressoVessel = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_CompressoVessel ) );
		}
		else
		{
			pclHySelectSelectedCompressoVessel = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_CompressoVessel ) );
		}

		if( NULL != pclHySelectSelectedCompressoVessel && NULL != dynamic_cast<CDB_Vessel *>( pclHySelectSelectedCompressoVessel->GetpData() ) )
		{
			pclHySelectCompressoVessel = dynamic_cast<CDB_Vessel *>( pclHySelectSelectedCompressoVessel->GetpData() );
		}

		CString strHyToolsCompressoVesselName = m_vecStrings[CF_CompressoVesselName];
		strHyToolsCompressoVesselName.Trim();

		if( NULL != pclHySelectSelectedCompressoVessel && true == strHyToolsCompressoVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HySelect has an Compresso vessel (%s) but not HyTools."), iCurrentLineNumber, pclHySelectCompressoVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pclHySelectSelectedCompressoVessel && false == strHyToolsCompressoVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an Compresso vessel (%s) but not HySelect."), iCurrentLineNumber, strHyToolsCompressoVesselName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		if( NULL != pclHySelectCompressoVessel 
				&& 0 != strHyToolsCompressoVesselName.CompareNoCase( pclHySelectCompressoVessel->GetName() ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Compresso vessels are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsCompressoVesselName, pclHySelectCompressoVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Compresso vessel ID.
		CString strHyToolsCompressoVesselID = m_vecStrings[CF_CompressoVesselID];
		strHyToolsCompressoVesselID.Trim();

		if( NULL != pclHySelectCompressoVessel
				&& 0 != strHyToolsCompressoVesselID.CompareNoCase( pclHySelectCompressoVessel->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Compresso vessel IDs are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsCompressoVesselID, pclHySelectCompressoVessel->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Compresso secondary vessel name.
		CString strHyToolsCompressoSecondaryVesselName = m_vecStrings[CF_CompressoSecondaryVesselName];
		strHyToolsCompressoSecondaryVesselName.Trim();
		CDB_Vessel *pcldbSecVessel = NULL;

		if( NULL != pclHySelectSelectedCompressoVessel && pclHySelectSelectedCompressoVessel->GetNbreOfVsslNeeded() > 1 
				&& NULL != pclHySelectCompressoVessel->GetAssociatedSecondaryVessel() )
		{
			pcldbSecVessel = pclHySelectCompressoVessel->GetAssociatedSecondaryVessel();
		}

		if( NULL != pcldbSecVessel && true == strHyToolsCompressoSecondaryVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HySelect has an Compresso secondary vessel (%s) but not HyTools."), iCurrentLineNumber, pcldbSecVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pcldbSecVessel && false == strHyToolsCompressoSecondaryVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an Compresso vessel (%s) but not HySelect."), iCurrentLineNumber, strHyToolsCompressoSecondaryVesselName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		if( NULL != pcldbSecVessel 
				&& 0 != strHyToolsCompressoSecondaryVesselName.CompareNoCase( pcldbSecVessel->GetName() ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Compresso vessels are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsCompressoSecondaryVesselName, pcldbSecVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Compresso secondary vessel ID.
		CString strHyToolsCompressoSecondaryVesselID = m_vecStrings[CF_CompressoSecondaryVesselID];
		strHyToolsCompressoSecondaryVesselID.Trim();

		if( NULL != pcldbSecVessel
				&& 0 != strHyToolsCompressoSecondaryVesselID.CompareNoCase( pcldbSecVessel->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Compresso secondary vessel IDs are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsCompressoSecondaryVesselID, pcldbSecVessel->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Compresso secondary vessel number.
		int iHyToolsCompressoSecondaryVesselNumber = _ttoi( m_vecStrings[CF_CompressoSecondaryVesselNumber] );

		if( NULL != pclHySelectSelectedCompressoVessel
				&& pclHySelectSelectedCompressoVessel->GetNbreOfVsslNeeded() - 1 != iHyToolsCompressoSecondaryVesselNumber )
		{
			strMsg.Format( _T("    Error -> Line %i: Compresso secondary vessel number are not the same -> HyTools: %i; HySelect: %i"), iCurrentLineNumber, iHyToolsCompressoSecondaryVesselNumber, pclHySelectSelectedCompressoVessel->GetNbreOfVsslNeeded() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the intermediate vessel.
		CSelectedVssl *pclHySelectSelectedIntermVssl = NULL;
		CDB_Vessel *pclHySelectIntermediateVessel = NULL;

		if( true == m_clIndSelPMParams.m_pclSelectPMList->IsIntermVesselNeeded( pclHySelectTecBoxCompresso ) )
		{
			m_clIndSelPMParams.m_pclSelectPMList->GetIntermVsslList( pclHySelectSelectedCompressoVessel );
			pclHySelectSelectedIntermVssl = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_IntermediateVessel ) );
		}

		CString strHyToolsIntermediateVesselName = m_vecStrings[CF_IntermediateVesselName];
		strHyToolsIntermediateVesselName.Trim();

		if( NULL != pclHySelectSelectedIntermVssl && true == strHyToolsIntermediateVesselName.IsEmpty() )
		{
			pclHySelectIntermediateVessel = dynamic_cast<CDB_Vessel *>( pclHySelectSelectedIntermVssl->GetpData() );

			strMsg.Format( _T("    Error -> Line %i: HySelect has an intermediate vessel (%s) but not HyTools."), iCurrentLineNumber, pclHySelectIntermediateVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pclHySelectSelectedIntermVssl && false == strHyToolsIntermediateVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an intermediate vessel (%s) but not HySelect."), iCurrentLineNumber, strHyToolsIntermediateVesselName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		if( NULL != pclHySelectIntermediateVessel 
				&& 0 != strHyToolsIntermediateVesselName.CompareNoCase( pclHySelectIntermediateVessel->GetName() ) )
		{
			strMsg.Format( _T("    Error -> Line %i: intermediate vessels are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsIntermediateVesselName, pclHySelectIntermediateVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Intermediate vessel ID.
		CString strHyToolsIntermediateVesselID = m_vecStrings[CF_IntermediateVesselID];
		strHyToolsIntermediateVesselID.Trim();

		if( NULL != pclHySelectIntermediateVessel
				&& 0 != strHyToolsIntermediateVesselID.CompareNoCase( pclHySelectIntermediateVessel->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: intermediate vessel IDs are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsIntermediateVesselID, pclHySelectIntermediateVessel->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the degassing.
		m_clIndSelPMParams.m_pclSelectPMList->SelectVentoPleno( pclHySelectSelectedTecBoxCompresso );
		CSelectedVento *pclHySelectSelectedTBVento = dynamic_cast<CSelectedVento *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_Vento ) );
		CDB_TBPlenoVento *pclHySelectTecBoxVento = NULL;

		CString strDegassingName = m_vecStrings[CF_DegassingName];
		strDegassingName.Trim();

		if( NULL != pclHySelectSelectedTBVento && true == strDegassingName.IsEmpty() )
		{
			pclHySelectTecBoxVento = dynamic_cast<CDB_TBPlenoVento *>( pclHySelectSelectedTBVento->GetpData() );

			strMsg.Format( _T("    Error -> Line %i: HySelect has a degassing device (%s) but not HyTools."), iCurrentLineNumber, pclHySelectTecBoxVento->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pclHySelectSelectedTBVento && false == strDegassingName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an degassing device (%s) but not HySelect."), iCurrentLineNumber, strDegassingName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Degassing ID.
		CString strHyToolsDegassingID = m_vecStrings[CF_DegassingID];
		strHyToolsDegassingID.Trim();

		if( NULL != pclHySelectTecBoxVento
				&& 0 != strHyToolsDegassingID.CompareNoCase( pclHySelectTecBoxVento->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: degassing devices are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsDegassingID, pclHySelectTecBoxVento->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}
	}

	// Verify expansion coefficient.
	double dHyToolsExpansionCoefficient = _wtof( m_vecStrings[CF_ExpansionCoefficient] );
	double dHySelectExpansionCoefficient = pclPMInputUser->GetSystemExpansionCoefficient();

	if( true == _IsDoubleValueDifferent( dHyToolsExpansionCoefficient, dHySelectExpansionCoefficient ) )
	{
		strMsg.Format( _T("    Error -> Line %i: expansion coeffecient are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsExpansionCoefficient, dHySelectExpansionCoefficient );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify expansion volume.
	double dHyToolsExpansionVolume = _wtof( m_vecStrings[CF_ExpansionVolume] );
	double dHySelectExpansionVolume = pclPMInputUser->GetSystemExpansionVolume();

	if( true == _IsDoubleValueDifferent( dHyToolsExpansionVolume, dHySelectExpansionVolume ) )
	{
		strMsg.Format( _T("    Error -> Line %i: expansion volume are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsExpansionVolume, dHySelectExpansionVolume );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum water reserve.
	double dHyToolsMinimumWaterReserve = _wtof( m_vecStrings[CF_MinimumWaterReserve] );
	double dHySelectMinimumWaterReserve = pclPMInputUser->GetMinimumWaterReserve();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumWaterReserve, dHySelectMinimumWaterReserve ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum water reserve are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumWaterReserve, dHySelectMinimumWaterReserve );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify vapor over pressure.
	double dHyToolsVaporOverPressure = _wtof( m_vecStrings[CF_VaporOverPressure] );
	double dHySelectVaporOverPressure = pclPMInputUser->GetVaporPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsVaporOverPressure, dHySelectVaporOverPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: vapor over pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsVaporOverPressure, dHySelectVaporOverPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum pressure.
	double dHyToolsMinimumPressure = _wtof( m_vecStrings[CF_MinimumPressure] );
	double dHySelectMinimumPressure = pclPMInputUser->GetMinimumPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumPressure, dHySelectMinimumPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumPressure, dHySelectMinimumPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum PSV.
	double dHyToolsMinimumPSV = _wtof( m_vecStrings[CF_MinimumPSV] );
	double dHySelectMinimumPSV = pclPMInputUser->GetMinimumRequiredPSVRaw( CPMInputUser::MT_WithCompressor );

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumPSV, dHySelectMinimumPSV ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum PSV are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumPSV, dHySelectMinimumPSV );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum initial pressure.
	double dHyToolsMinimumInitialPressure = _wtof( m_vecStrings[CF_MinimumInitialPressure] );
	double dHySelectMinimumInitialPressure = pclPMInputUser->GetMinimumInitialPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumInitialPressure, dHySelectMinimumInitialPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum initial pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumInitialPressure, dHySelectMinimumInitialPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify target pressure.
	double dHyToolsTargetPressure = _wtof( m_vecStrings[CF_TargetPressure] );
	double dHySelectTargetPressure = pclPMInputUser->GetTargetPressureForTecBox( CDB_TecBox::TecBoxType::etbtCompresso );

	if( true == _IsDoubleValueDifferent( dHyToolsTargetPressure, dHySelectTargetPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: target pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsTargetPressure, dHySelectTargetPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify final pressure.
	double dHyToolsFinalPressure = _wtof( m_vecStrings[CF_FinalPressure] );
	double dHySelectFinalPressure = pclPMInputUser->GetFinalPressure( CDB_TecBox::etbtCompresso );

	if( true == _IsDoubleValueDifferent( dHyToolsFinalPressure, dHySelectFinalPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: final pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsFinalPressure, dHySelectFinalPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify pressure factor.
	double dHyToolsPressureFactor = _wtof( m_vecStrings[CF_PressureFactor] );
	double dHySelectPressureFactor = pclPMInputUser->GetPressureFactor( true );

	if( true == _IsDoubleValueDifferent( dHyToolsPressureFactor, dHySelectPressureFactor ) )
	{
		strMsg.Format( _T("    Error -> Line %i: pressure factor are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsPressureFactor, dHySelectPressureFactor );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify nominal volume.
	double dHyToolsNominalVolume = _wtof( m_vecStrings[CF_NominalVolume] );
	double dHySelectNominalVolume = pclPMInputUser->GetVesselNominalVolume( true );

	if( true == _IsDoubleValueDifferent( dHyToolsNominalVolume, dHySelectNominalVolume ) )
	{
		strMsg.Format( _T("    Error -> Line %i: nominal volume are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsNominalVolume, dHySelectNominalVolume );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify equalization flow.
	double dHyToolsEqualizationFlow = _wtof( m_vecStrings[CF_EqualisationFlow] );
	double dHySelectEqualizationFlow = pclPMInputUser->GetVD();

	if( true == _IsDoubleValueDifferent( dHyToolsEqualizationFlow, dHySelectEqualizationFlow ) )
	{
		strMsg.Format( _T("    Error -> Line %i: equalization flow are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsEqualizationFlow, dHySelectEqualizationFlow );
		_WriteToListCtrl( strMsg );
		return false;
	}

	_WriteToListCtrl( _T("    OK!") );
	return true;
}

bool CDlgSpecActHyToolsPMCompare::_LaunchTestTransfero( CString strLine )
{
	_WriteToListCtrl( _T("    Transfero test...") );

	CString strOutput = _T("    ") + strLine;
	_WriteToListCtrl( strOutput );

	int iCurrentLineNumber = m_clInputFile.GetCurrentLineNumber();
	CString strMsg;

	if( TU_PRODSELECT_ERROR_OK != m_clInputFile.SplitOneLineMultiValues( strLine, m_vecStrings, _T(";"), false ) )
	{
		strMsg.Format( _T("    Error -> Line %i: can't interpret this line."), iCurrentLineNumber );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( TF_Last != (int)m_vecStrings.size() )
	{
		strMsg.Format( _T("    Error -> Line %i: one test line must contain %i tokens."), iCurrentLineNumber, TF_Last );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( false == _ReadInputs() )
	{
		return false;
	}

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechParam );

	if( NULL == pTechParam )
	{
		return false;
	}

	// In HyTools, the variable is yet 'Number of vessels in parallel'.
	// For Statico, if we put in HyTools 2, we have to a max for 2 vessels in parallel.
	// For Compresso/Transfero, we can have 2 secondary vessels in parallel with the primary vessel.
	pTechParam->SetMaxNumberOfAdditionalVesselsInParallel( 2 );

	// Launch the computation.
	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_WITHPUMP_TAB") );
	m_clIndSelPMParams.m_pclSelectPMList->Select();

	// Verify the Transfero.
	CSelectedPMBase *pclHySelectSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_Transfero );
	CSelectedTransfero *pclHySelectSelectedTecBoxTransfero = NULL;
	CDB_TecBox *pclHySelectTecBoxTransfero = NULL;

	if( NULL != pclHySelectSelectedProduct )
	{
		pclHySelectSelectedTecBoxTransfero = dynamic_cast<CSelectedTransfero *>( pclHySelectSelectedProduct );

		if( NULL == pclHySelectSelectedTecBoxTransfero )
		{
			strMsg.Format( _T("    Error -> Line %i: object found is not a Transfero."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}
	}

	if( NULL != pclHySelectSelectedTecBoxTransfero )
	{
		pclHySelectTecBoxTransfero = dynamic_cast<CDB_TecBox *>( pclHySelectSelectedTecBoxTransfero->GetpData() );

		if( NULL == pclHySelectTecBoxTransfero )
		{
			strMsg.Format( _T("    Error -> Line %i: internal error -> Can't retrieve the Transfero selected."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the Transfero name.
		CString strHyToolsTransferoName = m_vecStrings[TF_TransferoName];
		CString strHySelectTransferoName = pclHySelectTecBoxTransfero->GetName();

		if( 0 != strHyToolsTransferoName.Compare( strHySelectTransferoName ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Transfero names are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsTransferoName, strHySelectTransferoName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the Transfero ID.
		CString strHyToolsTransferoID = m_vecStrings[TF_TransferoID];
		strHyToolsTransferoID.Trim();

		if( 0 != strHyToolsTransferoID.CompareNoCase( pclHySelectTecBoxTransfero->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Transfero ID are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsTransferoID, pclHySelectTecBoxTransfero->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the Transfero vessel.
		m_clIndSelPMParams.m_pclSelectPMList->SelectTransferoVessel( pclHySelectSelectedTecBoxTransfero );
		CSelectedVssl *pclHySelectSelectedTransferoVessel = NULL;
		CDB_Vessel *pclHySelectTransferoVessel = NULL;

		if( false == pclHySelectTecBoxTransfero->IsVariantIntegratedPrimaryVessel() )
		{
			pclHySelectSelectedTransferoVessel = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_TransferoVessel ) );
		}
		else
		{
			pclHySelectSelectedTransferoVessel = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_TransferoVessel ) );
		}

		if( NULL != pclHySelectSelectedTransferoVessel && NULL != dynamic_cast<CDB_Vessel *>( pclHySelectSelectedTransferoVessel->GetpData() ) )
		{
			pclHySelectTransferoVessel = dynamic_cast<CDB_Vessel *>( pclHySelectSelectedTransferoVessel->GetpData() );
		}

		CString strHyToolsTransferoVesselName = m_vecStrings[TF_TransferoVesselName];
		strHyToolsTransferoVesselName.Trim();

		if( NULL != pclHySelectSelectedTransferoVessel && true == strHyToolsTransferoVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HySelect has an Transfero vessel (%s) but not HyTools."), iCurrentLineNumber, pclHySelectTransferoVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pclHySelectSelectedTransferoVessel && false == strHyToolsTransferoVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an Transfero vessel (%s) but not HySelect."), iCurrentLineNumber, strHyToolsTransferoVesselName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		if( NULL != pclHySelectTransferoVessel 
				&& 0 != strHyToolsTransferoVesselName.CompareNoCase( pclHySelectTransferoVessel->GetName() ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Transfero vessels are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsTransferoVesselName, pclHySelectTransferoVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Transfero vessel ID.
		CString strHyToolsTransferoVesselID = m_vecStrings[TF_TransferoVesselID];
		strHyToolsTransferoVesselID.Trim();

		if( NULL != pclHySelectTransferoVessel
				&& 0 != strHyToolsTransferoVesselID.CompareNoCase( pclHySelectTransferoVessel->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Transfero vessel IDs are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsTransferoVesselID, pclHySelectTransferoVessel->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Transfero secondary vessel name.
		CString strHyToolsTransferoSecondaryVesselName = m_vecStrings[TF_TransferoSecondaryVesselName];
		strHyToolsTransferoSecondaryVesselName.Trim();
		CDB_Vessel *pcldbSecVessel = NULL;

		if( NULL != pclHySelectTransferoVessel  && pclHySelectSelectedTransferoVessel->GetNbreOfVsslNeeded() > 1 
				&& NULL != pclHySelectTransferoVessel->GetAssociatedSecondaryVessel() )
		{
			pcldbSecVessel = pclHySelectTransferoVessel->GetAssociatedSecondaryVessel();
		}

		if( NULL != pcldbSecVessel && true == strHyToolsTransferoSecondaryVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HySelect has an Transfero secondary vessel (%s) but not HyTools."), iCurrentLineNumber, pcldbSecVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pcldbSecVessel && false == strHyToolsTransferoSecondaryVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an Transfero vessel (%s) but not HySelect."), iCurrentLineNumber, strHyToolsTransferoSecondaryVesselName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		if( NULL != pcldbSecVessel 
				&& 0 != strHyToolsTransferoSecondaryVesselName.CompareNoCase( pcldbSecVessel->GetName() ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Transfero vessels are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsTransferoSecondaryVesselName, pcldbSecVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Transfero secondary vessel ID.
		CString strHyToolsTransferoSecondaryVesselID = m_vecStrings[TF_TransferoSecondaryVesselID];
		strHyToolsTransferoSecondaryVesselID.Trim();

		if( NULL != pcldbSecVessel
				&& 0 != strHyToolsTransferoSecondaryVesselID.CompareNoCase( pcldbSecVessel->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: Transfero secondary vessel IDs are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsTransferoSecondaryVesselID, pcldbSecVessel->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Transfero secondary vessel number.
		int iHyToolsTransferoSecondaryVesselNumber = _ttoi( m_vecStrings[TF_TransferoSecondaryVesselNumber] );

		if( NULL != pclHySelectSelectedTransferoVessel
				&& pclHySelectSelectedTransferoVessel->GetNbreOfVsslNeeded() - 1 != iHyToolsTransferoSecondaryVesselNumber )
		{
			strMsg.Format( _T("    Error -> Line %i: Transfero secondary vessel number are not the same -> HyTools: %i; HySelect: %i"), iCurrentLineNumber, iHyToolsTransferoSecondaryVesselNumber, pclHySelectSelectedTransferoVessel->GetNbreOfVsslNeeded() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Verify the intermediate vessel.
		CSelectedVssl *pclHySelectSelectedIntermVssl = NULL;
		CDB_Vessel *pclHySelectIntermediateVessel = NULL;

		if( true == m_clIndSelPMParams.m_pclSelectPMList->IsIntermVesselNeeded( pclHySelectTransferoVessel ) )
		{
			m_clIndSelPMParams.m_pclSelectPMList->GetIntermVsslList( pclHySelectSelectedTransferoVessel );
			pclHySelectSelectedIntermVssl = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_IntermediateVessel ) );
		}

		CString strHyToolsIntermediateVesselName = m_vecStrings[TF_IntermediateVesselName];
		strHyToolsIntermediateVesselName.Trim();

		if( NULL != pclHySelectSelectedIntermVssl && true == strHyToolsIntermediateVesselName.IsEmpty() )
		{
			pclHySelectIntermediateVessel = dynamic_cast<CDB_Vessel *>( pclHySelectSelectedIntermVssl->GetpData() );

			strMsg.Format( _T("    Error -> Line %i: HySelect has an intermediate vessel (%s) but not HyTools."), iCurrentLineNumber, pclHySelectIntermediateVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}
		else if( NULL == pclHySelectSelectedIntermVssl && false == strHyToolsIntermediateVesselName.IsEmpty() )
		{
			strMsg.Format( _T("    Error -> Line %i: HyTools has an intermediate vessel (%s) but not HySelect."), iCurrentLineNumber, strHyToolsIntermediateVesselName );
			_WriteToListCtrl( strMsg );
			return false;
		}

		if( NULL != pclHySelectIntermediateVessel 
				&& 0 != strHyToolsIntermediateVesselName.CompareNoCase( pclHySelectIntermediateVessel->GetName() ) )
		{
			strMsg.Format( _T("    Error -> Line %i: intermediate vessels are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsIntermediateVesselName, pclHySelectIntermediateVessel->GetName() );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// Intermediate vessel ID.
		CString strHyToolsIntermediateVesselID = m_vecStrings[TF_IntermediateVesselID];
		strHyToolsIntermediateVesselID.Trim();

		if( NULL != pclHySelectIntermediateVessel
				&& 0 != strHyToolsIntermediateVesselID.CompareNoCase( pclHySelectIntermediateVessel->GetIDPtr().ID ) )
		{
			strMsg.Format( _T("    Error -> Line %i: intermediate vessel IDs are not the same -> HyTools: %s; HySelect: %s"), iCurrentLineNumber, strHyToolsIntermediateVesselID, pclHySelectIntermediateVessel->GetIDPtr().ID );
			_WriteToListCtrl( strMsg );
			return false;
		}
	}

	// Verify expansion coefficient.
	double dHyToolsExpansionCoefficient = _wtof( m_vecStrings[TF_ExpansionCoefficient] );
	double dHySelectExpansionCoefficient = pclPMInputUser->GetSystemExpansionCoefficient();

	if( true == _IsDoubleValueDifferent( dHyToolsExpansionCoefficient, dHySelectExpansionCoefficient ) )
	{
		strMsg.Format( _T("    Error -> Line %i: expansion coeffecient are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsExpansionCoefficient, dHySelectExpansionCoefficient );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify expansion volume.
	double dHyToolsExpansionVolume = _wtof( m_vecStrings[TF_ExpansionVolume] );
	double dHySelectExpansionVolume = pclPMInputUser->GetSystemExpansionVolume();

	if( true == _IsDoubleValueDifferent( dHyToolsExpansionVolume, dHySelectExpansionVolume ) )
	{
		strMsg.Format( _T("    Error -> Line %i: expansion volume are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsExpansionVolume, dHySelectExpansionVolume );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum water reserve.
	double dHyToolsMinimumWaterReserve = _wtof( m_vecStrings[TF_MinimumWaterReserve] );
	double dHySelectMinimumWaterReserve = pclPMInputUser->GetMinimumWaterReserve();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumWaterReserve, dHySelectMinimumWaterReserve ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum water reserve are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumWaterReserve, dHySelectMinimumWaterReserve );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify vapor over pressure.
	double dHyToolsVaporOverPressure = _wtof( m_vecStrings[TF_VaporOverPressure] );
	double dHySelectVaporOverPressure = pclPMInputUser->GetVaporPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsVaporOverPressure, dHySelectVaporOverPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: vapor over pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsVaporOverPressure, dHySelectVaporOverPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum pressure.
	double dHyToolsMinimumPressure = _wtof( m_vecStrings[TF_MinimumPressure] );
	double dHySelectMinimumPressure = pclPMInputUser->GetMinimumPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumPressure, dHySelectMinimumPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumPressure, dHySelectMinimumPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum PSV.
	double dHyToolsMinimumPSV = _wtof( m_vecStrings[TF_MinimumPSV] );
	double dHySelectMinimumPSV = pclPMInputUser->GetMinimumRequiredPSVRaw( CPMInputUser::MT_WithPump );

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumPSV, dHySelectMinimumPSV ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum PSV are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumPSV, dHySelectMinimumPSV );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify minimum initial pressure.
	double dHyToolsMinimumInitialPressure = _wtof( m_vecStrings[TF_MinimumInitialPressure] );
	double dHySelectMinimumInitialPressure = pclPMInputUser->GetMinimumInitialPressure();

	if( true == _IsDoubleValueDifferent( dHyToolsMinimumInitialPressure, dHySelectMinimumInitialPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: minimum initial pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsMinimumInitialPressure, dHySelectMinimumInitialPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify target pressure.
	double dHyToolsTargetPressure = _wtof( m_vecStrings[TF_TargetPressure] );
	double dHySelectTargetPressure = pclPMInputUser->GetTargetPressureForTecBox( CDB_TecBox::TecBoxType::etbtTransfero );

	if( true == _IsDoubleValueDifferent( dHyToolsTargetPressure, dHySelectTargetPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: target pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsTargetPressure, dHySelectTargetPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify final pressure.
	double dHyToolsFinalPressure = _wtof( m_vecStrings[TF_FinalPressure] );
	double dHySelectFinalPressure = pclPMInputUser->GetFinalPressure( CDB_TecBox::etbtTransfero );

	if( true == _IsDoubleValueDifferent( dHyToolsFinalPressure, dHySelectFinalPressure ) )
	{
		strMsg.Format( _T("    Error -> Line %i: final pressure are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsFinalPressure, dHySelectFinalPressure );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify pressure factor.
	double dHyToolsPressureFactor = _wtof( m_vecStrings[TF_PressureFactor] );
	double dHySelectPressureFactor = pclPMInputUser->GetPressureFactor( true );

	if( true == _IsDoubleValueDifferent( dHyToolsPressureFactor, dHySelectPressureFactor ) )
	{
		strMsg.Format( _T("    Error -> Line %i: pressure factor are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsPressureFactor, dHySelectPressureFactor );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify nominal volume.
	double dHyToolsNominalVolume = _wtof( m_vecStrings[TF_NominalVolume] );
	double dHySelectNominalVolume = pclPMInputUser->GetVesselNominalVolume( true );

	if( true == _IsDoubleValueDifferent( dHyToolsNominalVolume, dHySelectNominalVolume ) )
	{
		strMsg.Format( _T("    Error -> Line %i: nominal volume are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsNominalVolume, dHySelectNominalVolume );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Verify equalization flow.
	double dHyToolsEqualizationFlow = _wtof( m_vecStrings[TF_EqualisationFlow] );
	double dHySelectEqualizationFlow = pclPMInputUser->GetVD();

	if( true == _IsDoubleValueDifferent( dHyToolsEqualizationFlow, dHySelectEqualizationFlow ) )
	{
		strMsg.Format( _T("    Error -> Line %i: equalization flow are not the same -> HyTools: %g; HySelect: %g"), iCurrentLineNumber, dHyToolsEqualizationFlow, dHySelectEqualizationFlow );
		_WriteToListCtrl( strMsg );
		return false;
	}

	_WriteToListCtrl( _T("    OK!") );
	return true;
}

void CDlgSpecActHyToolsPMCompare::_LaunchExport()
{
	CString strLine;
	std::vector<CString> vecStrings;
	bool bCanContinue = true;
	bool bAllTestOK = false;
	int iCountTest = 0;

	do
	{
		CString strOutput;
		strOutput.Format( _T("Test %i"), iCountTest );
		UINT uiErrorCode = m_clInputFile.ReadOneLine( strLine, false );

		if( TU_PRODSELECT_ERROR_OK == uiErrorCode )
		{
			if( false == strLine.IsEmpty() )
			{
				if( _T( ';' ) == strLine.GetAt( 0 ) )
				{
					if( 0 == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPressureMaintenanceTypeID().Compare( _T("PMT_EXPVSSL_TAB") ) )
					{
						_WriteToListCtrl( strOutput );
						bCanContinue = _LaunchExportStatico( strLine );
						iCountTest++;
					}
					else if( 0 == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPressureMaintenanceTypeID().Compare( _T("PMT_WITHCOMPRESS_TAB") ) )
					{
						_WriteToListCtrl( strOutput );
						bCanContinue = _LaunchExportCompresso( strLine );
						iCountTest++;
					}
					else if( 0 == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPressureMaintenanceTypeID().Compare( _T("PMT_WITHPUMP_TAB") ) )
					{
						if( NULL != m_List.GetSafeHwnd() )
						{
							m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strOutput ) );
							_WriteToListCtrl( strOutput );
						}
						bCanContinue = _LaunchExportTransfero( strLine );
						iCountTest++;
					}
					else
					{
						_WriteToListCtrl( _T("Pressurisation type ('Statico', 'Compresso' or 'Transfero') must be defined before launching export.") );
					}
				}
				else
				{
					if( TU_PRODSELECT_ERROR_OK == m_clInputFile.SplitOneLineMultiValues( strLine, vecStrings, _T(";") ) )
					{
						m_clOutputFile.WriteOneLine( strLine, false );

						if( 0 == vecStrings[0].CompareNoCase( _T("statico") ) )
						{
							m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( _T("PMT_EXPVSSL_TAB") );
						}
						else if( 0 == vecStrings[0].CompareNoCase( _T("compresso") ) )
						{
							m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( _T("PMT_WITHCOMPRESS_TAB") );
						}
						else if( 0 == vecStrings[0].CompareNoCase( _T("transfero") ) )
						{
							m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( _T("PMT_WITHPUMP_TAB") );
						}
						else
						{
							_WriteToListCtrl( _T("Pressurisation type must be 'Statico', 'Compresso' or 'Transfero'.") );
							bCanContinue = false;
						}
					}
					else
					{
						_WriteToListCtrl( _T("Error in the file.") );
						bCanContinue = false;
					}
				}
			}
			else
			{
				m_clOutputFile.WriteOneLine( _T( "" ), false );
			}
		}
		else
		{
			bCanContinue = false;
			bAllTestOK = true;
		}
	}
	while( true == bCanContinue );

	if( true == bAllTestOK )
	{
		_WriteToListCtrl( _T( "" ) );
		_WriteToListCtrl( _T("Yeah BABY... yeah!!!") );
	}
}

bool CDlgSpecActHyToolsPMCompare::_LaunchExportStatico( CString strLine )
{
	int iCurrentLineNumber = m_clInputFile.GetCurrentLineNumber();
	CString strMsg;

	if( TU_PRODSELECT_ERROR_OK != m_clInputFile.SplitOneLineMultiValues( strLine, m_vecStrings, _T(";"), false ) )
	{
		strMsg.Format( _T("    Error -> Line %i: can't interpret this line."), iCurrentLineNumber );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( SF_Last != (int)m_vecStrings.size() )
	{
		strMsg.Format( _T("    Error -> Line %i: one test line must contain %i tokens."), iCurrentLineNumber, SF_Last );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( false == _ReadInputs() )
	{
		return false;
	}

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechParam );

	if( NULL == pTechParam )
	{
		return false;
	}

	// In HyTools, the variable is yet 'Number of vessels in parallel'.
	// For Statico, if we put in HyTools 2, we have to a max for 2 vessels in parallel.
	// For Compresso/Transfero, we can have 2 secondary vessels in parallel with the primary vessel.
	pTechParam->SetMaxNumberOfAdditionalVesselsInParallel( 1 );

	// Launch the computation.
	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetPressureMaintenanceTypeID( ( true == m_bPreferVesselBag ) ? _T("PMT_EXPVSSL_TAB") : _T("PMT_EXPVSSLMBR_TAB") );
	m_clIndSelPMParams.m_pclSelectPMList->Select();

	// Write all outputs.
	CString strOutput;
	_WriteInputs( strOutput );

	CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_ExpansionVessel );
	CSelectedVssl *pclSelectedExpansionVessel = NULL;
	CDB_Vessel *pclExpansionVessel = NULL;

	if( NULL != pclSelectedProduct )
	{
		pclSelectedExpansionVessel = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

		if( NULL == pclSelectedExpansionVessel )
		{
			strMsg.Format( _T("    Error -> Line %i: object found is not a vessel."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}
	}

	if( NULL != pclSelectedExpansionVessel && false == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) )
	{
		pclExpansionVessel = dynamic_cast<CDB_Vessel *>( pclSelectedExpansionVessel->GetpData() );

		if( NULL == pclExpansionVessel )
		{
			strMsg.Format( _T("    Error -> Line %i: internal error -> Can't retrieve the vessel selected."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// SF_VesselName.
		strOutput.Format( _T("%s %s;"), strOutput, pclExpansionVessel->GetName() );

		// SF_VesselID.
		strOutput.Format( _T("%s %s;"), strOutput, pclExpansionVessel->GetIDPtr().ID );

		// SF_VesselNumber.
		strOutput.Format( _T("%s %i;"), strOutput, pclSelectedExpansionVessel->GetNbreOfVsslNeeded() );

		// SF_InitialPressure.
		strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclSelectedExpansionVessel->GetInitialPressure(), m_vecStrings[SF_InitialPressure] ) );

		// SF_FillingPressure.
		double dTotalVesselVolume = pclSelectedExpansionVessel->GetNbreOfVsslNeeded() * pclExpansionVessel->GetNominalVolume();
		double dFillingPressure = pclPMInputUser->GetIntermediatePressure( pclPMInputUser->GetFillTemperature(), pclSelectedExpansionVessel->GetWaterReserve(), dTotalVesselVolume );
		strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( dFillingPressure, m_vecStrings[SF_FillingPressure] ) );

		// SF_WaterReserve.
		strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclSelectedExpansionVessel->GetWaterReserve(), m_vecStrings[SF_WaterReserve] ) );

		// SF_IntermediateVesselName.
		CString strIntermediateVesselName( _T("") );
		CString strIntermediateVesselID( _T("") );

		if( true == m_clIndSelPMParams.m_pclSelectPMList->IsIntermVesselNeeded( pclExpansionVessel ) )
		{
			m_clIndSelPMParams.m_pclSelectPMList->GetIntermVsslList( pclSelectedExpansionVessel );
			CSelectedVssl *pclSelectedIntermVssl = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_IntermediateVessel ) );

			if( NULL != pclSelectedIntermVssl && NULL != dynamic_cast<CDB_Vessel *>( pclSelectedIntermVssl->GetpData() ) )
			{
				CDB_Vessel *pclInterVssl = dynamic_cast<CDB_Vessel *>( pclSelectedIntermVssl->GetpData() );
				strIntermediateVesselName = pclInterVssl->GetName();
				strIntermediateVesselID = pclInterVssl->GetIDPtr().ID;
			}
		}

		strOutput.Format( _T("%s %s;"), strOutput, strIntermediateVesselName );

		// SF_IntermediateVesselID.
		strOutput.Format( _T("%s %s;"), strOutput, strIntermediateVesselID );

		// SF_DegassingName.
		CString strVentoName( _T("") );
		CString strVentoID( _T("") );

		m_clIndSelPMParams.m_pclSelectPMList->SelectVentoPleno( pclSelectedExpansionVessel );
		CSelectedVento *pclSelectedTBVento = dynamic_cast<CSelectedVento *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_Vento ) );
	
		if( NULL != pclSelectedTBVento && NULL != dynamic_cast<CDB_TBPlenoVento *>( pclSelectedTBVento->GetpData() ) )
		{
			CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedTBVento->GetpData() );
			strVentoName = pclVento->GetName();
			strVentoID = pclVento->GetIDPtr().ID;
		}

		strOutput.Format( _T("%s %s;"), strOutput, strVentoName );

		// SF_DegassingID.
		strOutput.Format( _T("%s %s;"), strOutput, strVentoID );
	}
	else
	{
		for( int i = 0; i < ( SF_DegassingID - SF_VesselName + 1 ); i++ )
		{
			strOutput.Format( _T("%s ;"), strOutput );
		}
	}

	// SF_ContractionVolume.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->ComputeContractionVolume(), m_vecStrings[SF_ContractionVolume] ) );

	// SF_ExpansionCoefficent.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetSystemExpansionCoefficient(), m_vecStrings[SF_ExpansionCoefficent] ) );

	// SF_ExpansionVolume.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetSystemExpansionVolume(), m_vecStrings[SF_ExpansionVolume] ) );

	// SF_MinimumWaterReserve.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumWaterReserve(), m_vecStrings[SF_MinimumWaterReserve] ) );

	// SF_VaporOverPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetVaporPressure(), m_vecStrings[SF_VaporOverPressure] ) );

	// SF_MinimumPSV.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumRequiredPSVRaw( CPMInputUser::MT_ExpansionVessel ), m_vecStrings[SF_MinimumPSV] ) );

	// SF_MinimumInitialPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumInitialPressure(), m_vecStrings[SF_MinimumInitialPressure] ) );

	// SF_MinimumPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumPressure(), m_vecStrings[SF_MinimumPressure] ) );

	// SF_FinalPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetFinalPressure(), m_vecStrings[SF_FinalPressure] ) );

	// SF_PressureFactor.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetPressureFactor(), m_vecStrings[SF_PressureFactor] ) );

	// SF_NominalVolume.
	strOutput.Format( _T("%s %s"), strOutput, _ConvertDouble2String( pclPMInputUser->GetVesselNominalVolume(), m_vecStrings[SF_NominalVolume] ) );

	m_clOutputFile.WriteOneLine( strOutput, false );
	
	return true;
}

bool CDlgSpecActHyToolsPMCompare::_LaunchExportCompresso( CString strLine )
{
	int iCurrentLineNumber = m_clInputFile.GetCurrentLineNumber();
	CString strMsg;

	if( TU_PRODSELECT_ERROR_OK != m_clInputFile.SplitOneLineMultiValues( strLine, m_vecStrings, _T(";"), false ) )
	{
		strMsg.Format( _T("    Error -> Line %i: can't interpret this line."), iCurrentLineNumber );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( CF_Last != (int)m_vecStrings.size() )
	{
		strMsg.Format( _T("    Error -> Line %i: one test line must contain %i tokens."), iCurrentLineNumber, CF_Last );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( false == _ReadInputs() )
	{
		return false;
	}

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechParam );

	if( NULL == pTechParam )
	{
		return false;
	}

	// In HyTools, the variable is yet 'Number of vessels in parallel'.
	// For Statico, if we put in HyTools 2, we have to a max for 2 vessels in parallel.
	// For Compresso/Transfero, we can have 2 secondary vessels in parallel with the primary vessel.
	pTechParam->SetMaxNumberOfAdditionalVesselsInParallel( 2 );

	// Launch the computation.
	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_WITHCOMPRESS_TAB") );
	m_clIndSelPMParams.m_pclSelectPMList->Select();

	// Write all outputs.
	CString strOutput;
	_WriteInputs( strOutput );

	CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_Compresso );

	if( NULL != pclSelectedProduct )
	{
		CSelectedCompresso *pclSelectedTecBoxCompresso = dynamic_cast<CSelectedCompresso *>( pclSelectedProduct );

		if( NULL == pclSelectedTecBoxCompresso )
		{
			strMsg.Format( _T("    Error -> Line %i: object found is not a Compresso."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		CDB_TecBox *pclTecBoxCompresso = dynamic_cast<CDB_TecBox *>( pclSelectedTecBoxCompresso->GetpData() );

		if( NULL == pclTecBoxCompresso )
		{
			strMsg.Format( _T("    Error -> Line %i: internal error -> Can't retrieve the Compresso selected."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// CF_CompressoName.
		if( 0 == CString( pclTecBoxCompresso->GetIDPtr().ID ).Compare( _T("SIMP_COMP_2_1_80") ) )
		{
			// In HyTools the name is truncated (Simply Compresso C 2.1-80 S -> C 2.1-80 S).
			strOutput.Format( _T("%s C 2.1-80 S;"), strOutput );
		}
		else
		{
			strOutput.Format( _T("%s %s;"), strOutput, pclTecBoxCompresso->GetName() );
		}

		// CF_CompressoID.
		strOutput.Format( _T("%s %s;"), strOutput, pclTecBoxCompresso->GetIDPtr().ID );

		// CF_CompressoVesselName.
		CString strCompressoVesselName( _T("") );
		CString strCompressoVesselID( _T("") );

		m_clIndSelPMParams.m_pclSelectPMList->SelectCompressoVessel( pclSelectedTecBoxCompresso );
		CSelectedVssl *pclSelectedCompressoVessel = NULL;
		CDB_Vessel *pclCompressoVessel = NULL;

		if( false == pclTecBoxCompresso->IsVariantIntegratedPrimaryVessel() )
		{
			pclSelectedCompressoVessel = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_CompressoVessel ) );
		}
		else
		{
			pclSelectedCompressoVessel = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_CompressoVessel ) );
		}

		if( NULL != pclSelectedCompressoVessel && NULL != dynamic_cast<CDB_Vessel *>( pclSelectedCompressoVessel->GetpData() ) )
		{
			pclCompressoVessel = dynamic_cast<CDB_Vessel *>( pclSelectedCompressoVessel->GetpData() );
			strCompressoVesselName = pclCompressoVessel->GetName();
			strCompressoVesselID = pclCompressoVessel->GetIDPtr().ID;
		}

		strOutput.Format( _T("%s %s; %s;"), strOutput, strCompressoVesselName, strCompressoVesselID );

		// CF_CompressoSecondaryVesselName, CF_CompressoSecondaryVesselID and CF_CompressoSecondaryVesselNumber.
		if( NULL != pclSelectedCompressoVessel && pclSelectedCompressoVessel->GetNbreOfVsslNeeded() > 1 
				&& NULL != pclCompressoVessel->GetAssociatedSecondaryVessel() )
		{
			CDB_Vessel *pcldbSecVessel = pclCompressoVessel->GetAssociatedSecondaryVessel();

			strOutput.Format( _T("%s %s;"), strOutput, pcldbSecVessel->GetName() );
			strOutput.Format( _T("%s %s;"), strOutput, pcldbSecVessel->GetIDPtr().ID );
			strOutput.Format( _T("%s %i;"), strOutput, pclSelectedCompressoVessel->GetNbreOfVsslNeeded() - 1 );
		}
		else
		{
			strOutput.Format( _T("%s ; ; ;"), strOutput );
		}

		// CF_IntermediateVesselName.
		CString strIntermediateVesselName( _T("") );
		CString strIntermediateVesselID( _T("") );

		if( true == m_clIndSelPMParams.m_pclSelectPMList->IsIntermVesselNeeded( pclCompressoVessel ) )
		{
			m_clIndSelPMParams.m_pclSelectPMList->GetIntermVsslList( pclSelectedCompressoVessel );
			CSelectedVssl *pclSelectedIntermVssl = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_IntermediateVessel ) );

			if( NULL != pclSelectedIntermVssl && NULL != dynamic_cast<CDB_Vessel *>( pclSelectedIntermVssl->GetpData() ) )
			{
				CDB_Vessel *pclInterVssl = dynamic_cast<CDB_Vessel *>( pclSelectedIntermVssl->GetpData() );
				strIntermediateVesselName = pclInterVssl->GetName();
				strIntermediateVesselID = pclInterVssl->GetIDPtr().ID;
			}
		}

		strOutput.Format( _T("%s %s;"), strOutput, strIntermediateVesselName );

		// CF_IntermediateVesselID.
		strOutput.Format( _T("%s %s;"), strOutput, strIntermediateVesselID );

		// CF_DegassingName.
		CString strVentoName( _T("") );
		CString strVentoID( _T("") );

		m_clIndSelPMParams.m_pclSelectPMList->SelectVentoPleno( pclSelectedTecBoxCompresso );
		CSelectedVento *pclSelectedTBVento = dynamic_cast<CSelectedVento *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_Vento ) );

		if( NULL != pclSelectedTBVento && NULL != dynamic_cast<CDB_TBPlenoVento *>( pclSelectedTBVento->GetpData() ) )
		{
			CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedTBVento->GetpData() );
			strVentoName = pclVento->GetName();
			strVentoID = pclVento->GetIDPtr().ID;
		}

		strOutput.Format( _T("%s %s;"), strOutput, strVentoName );

		// CF_DegassingID.
		strOutput.Format( _T("%s %s;"), strOutput, strVentoID );
	}
	else
	{
		for( int i = 0; i < ( CF_DegassingID - CF_CompressoName + 1 ); i++ )
		{
			strOutput.Format( _T("%s ;"), strOutput );
		}
	}

	// CF_ExpansionCoefficent.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetSystemExpansionCoefficient(), m_vecStrings[CF_ExpansionCoefficient] ) );

	// CF_ExpansionVolume.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetSystemExpansionVolume(), m_vecStrings[CF_ExpansionVolume] ) );

	// CF_MinimumWaterReserve.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumWaterReserve(), m_vecStrings[CF_MinimumWaterReserve] ) );

	// CF_VaporOverPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetVaporPressure(), m_vecStrings[CF_VaporOverPressure] ) );

	// CF_MinimumPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumPressure(), m_vecStrings[CF_MinimumPressure] ) );

	// CF_MinimumPSV.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumRequiredPSVRaw( CPMInputUser::MT_WithCompressor ), m_vecStrings[CF_MinimumPSV] ) );

	// CF_MinimumInitialPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumInitialPressure(), m_vecStrings[CF_MinimumInitialPressure] ) );

	// CF_TargetPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetTargetPressureForTecBox( CDB_TecBox::TecBoxType::etbtCompresso ), m_vecStrings[CF_TargetPressure] ) );

	// CF_FinalPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetFinalPressure( CDB_TecBox::etbtCompresso ), m_vecStrings[CF_FinalPressure] ) );

	// CF_PressureFactor.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetPressureFactor( true ), m_vecStrings[CF_PressureFactor] ) );

	// CF_NominalVolume.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetVesselNominalVolume( true ), m_vecStrings[CF_NominalVolume] ) );

	// CF_EqualisationFlow.
	strOutput.Format( _T("%s %s"), strOutput, _ConvertDouble2String( pclPMInputUser->GetVD(), m_vecStrings[CF_EqualisationFlow] ) );

	m_clOutputFile.WriteOneLine( strOutput, false );

	return true;
}

bool CDlgSpecActHyToolsPMCompare::_LaunchExportTransfero( CString strLine )
{
	int iCurrentLineNumber = m_clInputFile.GetCurrentLineNumber();
	CString strMsg;

	if( TU_PRODSELECT_ERROR_OK != m_clInputFile.SplitOneLineMultiValues( strLine, m_vecStrings, _T(";"), false ) )
	{
		strMsg.Format( _T("    Error -> Line %i: can't interpret this line."), iCurrentLineNumber );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( TF_Last != (int)m_vecStrings.size() )
	{
		strMsg.Format( _T("    Error -> Line %i: one test line must contain %i tokens."), iCurrentLineNumber, TF_Last );
		_WriteToListCtrl( strMsg );
		return false;
	}

	if( false == _ReadInputs() )
	{
		return false;
	}

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechParam );

	if( NULL == pTechParam )
	{
		return false;
	}

	// In HyTools, the variable is yet 'Number of vessels in parallel'.
	// For Statico, if we put in HyTools 2, we have to a max for 2 vessels in parallel.
	// For Compresso/Transfero, we can have 2 secondary vessels in parallel with the primary vessel.
	pTechParam->SetMaxNumberOfAdditionalVesselsInParallel( 2 );

	// Launch the computation.
	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_WITHPUMP_TAB") );
	m_clIndSelPMParams.m_pclSelectPMList->Select();

	// Write all outputs.
	CString strOutput;
	_WriteInputs( strOutput );

	CSelectedPMBase *pclSelectedProduct = m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_Transfero );

	if( NULL != pclSelectedProduct )
	{
		CSelectedTransfero *pclSelectedTecBoxTransfero = dynamic_cast<CSelectedTransfero *>( pclSelectedProduct );

		if( NULL == pclSelectedTecBoxTransfero )
		{
			strMsg.Format( _T("    Error -> Line %i: object found is not a Transfero."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		CDB_TecBox *pclTecBoxTransfero = dynamic_cast<CDB_TecBox *>( pclSelectedTecBoxTransfero->GetpData() );

		if( NULL == pclTecBoxTransfero )
		{
			strMsg.Format( _T("    Error -> Line %i: internal error -> Can't retrieve the Transfero selected."), iCurrentLineNumber );
			_WriteToListCtrl( strMsg );
			return false;
		}

		// TF_TransferoName.
		strOutput.Format( _T("%s %s;"), strOutput, pclTecBoxTransfero->GetName() );

		// TF_TransferoID.
		strOutput.Format( _T("%s %s;"), strOutput, pclTecBoxTransfero->GetIDPtr().ID );

		// TF_TransferoVesselName.
		CString strTransferoVesselName( _T("") );
		CString strTransferoVesselID( _T("") );

		m_clIndSelPMParams.m_pclSelectPMList->SelectTransferoVessel( pclSelectedTecBoxTransfero );
		CSelectedVssl *pclSelectedTransferoVessel = NULL;
		CDB_Vessel *pclTransferoVessel = NULL;

		if( false == pclTecBoxTransfero->IsVariantIntegratedPrimaryVessel() )
		{
			pclSelectedTransferoVessel = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_TransferoVessel ) );
		}
		else
		{
			pclSelectedTransferoVessel = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_TransferoVessel ) );
		}

		if( NULL != pclSelectedTransferoVessel && NULL != dynamic_cast<CDB_Vessel *>( pclSelectedTransferoVessel->GetpData() ) )
		{
			pclTransferoVessel = dynamic_cast<CDB_Vessel *>( pclSelectedTransferoVessel->GetpData() );
			strTransferoVesselName = pclTransferoVessel->GetName();
			strTransferoVesselID = pclTransferoVessel->GetIDPtr().ID;
		}

		strOutput.Format( _T("%s %s; %s;"), strOutput, strTransferoVesselName, strTransferoVesselID );

		// TF_TransferoSecondaryVesselName, TF_TransferoSecondaryVesselID and TF_TransferoSecondaryVesselNumber.
		if( NULL != pclSelectedTransferoVessel && pclSelectedTransferoVessel->GetNbreOfVsslNeeded() > 1 
				&& NULL != pclTransferoVessel->GetAssociatedSecondaryVessel() )
		{
			CDB_Vessel *pcldbSecVessel = pclTransferoVessel->GetAssociatedSecondaryVessel();

			strOutput.Format( _T("%s %s;"), strOutput, pcldbSecVessel->GetName() );
			strOutput.Format( _T("%s %s;"), strOutput, pcldbSecVessel->GetIDPtr().ID );
			strOutput.Format( _T("%s %i;"), strOutput, pclSelectedTransferoVessel->GetNbreOfVsslNeeded() - 1 );
		}
		else
		{
			strOutput.Format( _T("%s ; ; ;"), strOutput );
		}

		// TF_IntermediateVesselName.
		CString strIntermediateVesselName( _T("") );
		CString strIntermediateVesselID( _T("") );

		if( true == m_clIndSelPMParams.m_pclSelectPMList->IsIntermVesselNeeded( pclTransferoVessel ) )
		{
			m_clIndSelPMParams.m_pclSelectPMList->GetIntermVsslList( pclSelectedTransferoVessel );
			CSelectedVssl *pclSelectedIntermVssl = dynamic_cast<CSelectedVssl *>( m_clIndSelPMParams.m_pclSelectPMList->GetBestProduct( CSelectPMList::PT_IntermediateVessel ) );

			if( NULL != pclSelectedIntermVssl && NULL != dynamic_cast<CDB_Vessel *>( pclSelectedIntermVssl->GetpData() ) )
			{
				CDB_Vessel *pclInterVssl = dynamic_cast<CDB_Vessel *>( pclSelectedIntermVssl->GetpData() );
				strIntermediateVesselName = pclInterVssl->GetName();
				strIntermediateVesselID = pclInterVssl->GetIDPtr().ID;
			}
		}

		strOutput.Format( _T("%s %s;"), strOutput, strIntermediateVesselName );

		// TF_IntermediateVesselID.
		strOutput.Format( _T("%s %s;"), strOutput, strIntermediateVesselID );
	}
	else
	{
		for( int i = 0; i < ( TF_IntermediateVesselID - TF_TransferoName + 1 ); i++ )
		{
			strOutput.Format( _T("%s ;"), strOutput );
		}
	}

	// TF_ExpansionCoefficent.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetSystemExpansionCoefficient(), m_vecStrings[TF_ExpansionCoefficient] ) );

	// TF_ExpansionVolume.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetSystemExpansionVolume(), m_vecStrings[TF_ExpansionVolume] ) );

	// TF_MinimumWaterReserve.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumWaterReserve(), m_vecStrings[TF_MinimumWaterReserve] ) );

	// TF_VaporOverPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetVaporPressure(), m_vecStrings[TF_VaporOverPressure] ) );

	// TF_MinimumPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumPressure(), m_vecStrings[TF_MinimumPressure] ) );

	// TF_MinimumPSV.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumRequiredPSVRaw( CPMInputUser::MT_WithPump ), m_vecStrings[TF_MinimumPSV] ) );

	// TF_MinimumInitialPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetMinimumInitialPressure(), m_vecStrings[TF_MinimumInitialPressure] ) );

	// TF_TargetPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetTargetPressureForTecBox( CDB_TecBox::TecBoxType::etbtTransfero ), m_vecStrings[TF_TargetPressure] ) );

	// TF_FinalPressure.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetFinalPressure( CDB_TecBox::etbtTransfero ), m_vecStrings[TF_FinalPressure] ) );

	// TF_PressureFactor.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetPressureFactor( true ), m_vecStrings[TF_PressureFactor] ) );

	// TF_NominalVolume.
	strOutput.Format( _T("%s %s;"), strOutput, _ConvertDouble2String( pclPMInputUser->GetVesselNominalVolume( true ), m_vecStrings[TF_NominalVolume] ) );

	// TF_EqualisationFlow.
	strOutput.Format( _T("%s %s"), strOutput, _ConvertDouble2String( pclPMInputUser->GetVD(), m_vecStrings[TF_EqualisationFlow] ) );

	m_clOutputFile.WriteOneLine( strOutput, false );

	return true;
}

bool CDlgSpecActHyToolsPMCompare::_ReadInputs( )
{
	int iCurrentLineNumber = m_clInputFile.GetCurrentLineNumber();
	CString strMsg;

	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();

	pclPMInputUser->SetSafetyTempLimiter( _ttof( m_vecStrings[IF_SafetyTempLimiter] ) );
	pclPMInputUser->SetSupplyTemperature( _ttof( m_vecStrings[IF_SupplyTemperature] ) );
	pclPMInputUser->SetReturnTemperature( _ttof( m_vecStrings[IF_ReturnTemperature] ) );
	pclPMInputUser->SetMaxTemperature( _ttof( m_vecStrings[IF_MaxTemperature] ) );
	pclPMInputUser->SetMinTemperature( _ttof( m_vecStrings[IF_MinTemperature] ) );
	pclPMInputUser->SetFillTemperature( _ttof( m_vecStrings[IF_FillTemperature] ) );

	// Norm.
	CString strHyToolsNorm = m_vecStrings[IF_Norm];
	strHyToolsNorm.Trim();

	if( 0 == m_mapHyToolsNorm2HySelect.count( strHyToolsNorm ) )
	{
		strMsg.Format( _T("    Error -> Line %i: norm %s is not recognized."), iCurrentLineNumber, strHyToolsNorm );
		_WriteToListCtrl( strMsg );
		return false;
	}

	pclPMInputUser->SetNormID( m_mapHyToolsNorm2HySelect[strHyToolsNorm] );

	CString strPressureVolumeLimitEnabled = m_vecStrings[IF_PressureVolumeIndexEnabled];
	if( 0 == strPressureVolumeLimitEnabled.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->GetpTADS()->GetpTechParams()->SetUseVesselPressureVolumeLimit( true );
	}
	else if( 0 == strPressureVolumeLimitEnabled.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->GetpTADS()->GetpTechParams()->SetUseVesselPressureVolumeLimit( false );
	}

	pclPMInputUser->SetStaticHeight( _ttof( m_vecStrings[IF_StaticHeight] ) );

	pclPMInputUser->SetPz( _ttof( m_vecStrings[IF_PZ] ) );
	pclPMInputUser->SetPzChecked( ( pclPMInputUser->GetPz() > 0.0 ) ? BST_CHECKED : BST_UNCHECKED );
	pclPMInputUser->SetSafetyValveResponsePressure( _ttof( m_vecStrings[IF_PSV] ) );

	// Pressurisation side (Pump suction or Pump discharge).
	CString strHyToolsPressurisationSide = m_vecStrings[IF_PressurisationSide];
	strHyToolsPressurisationSide.Trim();

	if( 0 == strHyToolsPressurisationSide.CompareNoCase( _T("Pump suction") ) )
	{
		pclPMInputUser->SetPressOn( PressurON::poPumpSuction );
	}
	else if( 0 == strHyToolsPressurisationSide.CompareNoCase( _T("Pump discharge") ) )
	{
		pclPMInputUser->SetPressOn( PressurON::poPumpDischarge );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: pressurisation side '%s' is not recognized."), iCurrentLineNumber, strHyToolsPressurisationSide );
		_WriteToListCtrl( strMsg );
		return false;
	}

	pclPMInputUser->SetPumpHead( _ttof( m_vecStrings[IF_PumpHead] ) );
	pclPMInputUser->SetSystemVolume( _ttof( m_vecStrings[IF_VolumeSystem] ) );
	pclPMInputUser->SetSolarCollectorVolume( _ttof( m_vecStrings[IF_SolarCollectorContent] ) );
	pclPMInputUser->SetInstalledPower( _ttof( m_vecStrings[IF_InstalledPower] ) );

	// Pressure maintenance enabled.
	CString strHyToolsPressureMaintenanceEnabled = m_vecStrings[IF_PressureMaintenanceEnabled];
	strHyToolsPressureMaintenanceEnabled.Trim();

	if( 0 == strHyToolsPressureMaintenanceEnabled.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_ALL_TAB") );
	}
	else if( 0 == strHyToolsPressureMaintenanceEnabled.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_NONE") );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: pressure maintenance enabled '%s' is not recognized."), iCurrentLineNumber, strHyToolsPressureMaintenanceEnabled );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Degassing.
	CString strHyToolsDegassingEnabled = m_vecStrings[IF_Degassing];
	strHyToolsDegassingEnabled.Trim();

	if( 0 == strHyToolsDegassingEnabled.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->SetDegassingChecked( BST_CHECKED );
	}
	else if( 0 == strHyToolsDegassingEnabled.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->SetDegassingChecked( BST_UNCHECKED );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: degassing '%s' is not recognized."), iCurrentLineNumber, strHyToolsDegassingEnabled );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Water Make-up type.
	CString strHyToolsWaterMakeUpType = m_vecStrings[IF_WaterMakeUp];
	strHyToolsWaterMakeUpType.Trim();

	if( 0 == strHyToolsWaterMakeUpType.CompareNoCase( _T("None") ) )
	{
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_NONE") );
	}
	else if( 0 == strHyToolsWaterMakeUpType.CompareNoCase( _T("Standard") ) )
	{
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_STD") );
	}
	else if( 0 == strHyToolsWaterMakeUpType.CompareNoCase( _T("With softening") ) )
	{
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_WITHSOFT") );
	}
	else if( 0 == strHyToolsWaterMakeUpType.CompareNoCase( _T("With desalination") ) )
	{
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_WITHDESAL") );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: water make-up type '%s' is not recognized."), iCurrentLineNumber, strHyToolsWaterMakeUpType );
		_WriteToListCtrl( strMsg );
		return false;
	}

	pclPMInputUser->SetWaterMakeUpNetworkPN( _ttof( m_vecStrings[IF_StaticPressureOfWaterNetwork] ) );
	pclPMInputUser->SetWaterMakeUpWaterTemp( _ttof( m_vecStrings[IF_MakeUpWaterTemperature] ) );
	pclPMInputUser->SetWaterMakeUpWaterHardness( _ttof( m_vecStrings[IF_WaterHardness] ) );

	// Use expansion vessel with bag or membrane.
	CString strHyToolsExpansionVesselBag = m_vecStrings[IF_ExpansionVesselBag];
	strHyToolsExpansionVesselBag.Trim();

	if( 0 == strHyToolsExpansionVesselBag.CompareNoCase( _T("true") ) )
	{
		m_bPreferVesselBag = true;
	}
	else if( 0 == strHyToolsExpansionVesselBag.CompareNoCase( _T("false") ) )
	{
		m_bPreferVesselBag = false;
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: expansion vessel bag '%s' is not recognized."), iCurrentLineNumber, strHyToolsExpansionVesselBag );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Prefer tecbox on top.
	CString strHyToolsPreferTecboxOnTop = m_vecStrings[IF_PreferTecboxOnTop];
	strHyToolsPreferTecboxOnTop.Trim();

	if( 0 == strHyToolsPreferTecboxOnTop.CompareNoCase( _T("true") ) )
	{
		m_bPreferTecboxOnTop = true;
	}
	else if( 0 == strHyToolsPreferTecboxOnTop.CompareNoCase( _T("false") ) )
	{
		m_bPreferTecboxOnTop = false;
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: prefer tecbox on top '%s' is not recognized."), iCurrentLineNumber, strHyToolsPreferTecboxOnTop );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Compresso vessel internal coating.
	CString strCompressoVesselInternalCoating = m_vecStrings[IF_CompressoVesselInternalCoating];
	strCompressoVesselInternalCoating.Trim();

	if( 0 == strCompressoVesselInternalCoating.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::CompressoInternalCoating, true );
	}
	else if( 0 == strCompressoVesselInternalCoating.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::CompressoInternalCoating, false );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: Compresso vessel internal coating '%s' is not recognized."), iCurrentLineNumber, strCompressoVesselInternalCoating );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Compresso external compressed air.
	CString strCompressoExternalCompressedAir = m_vecStrings[IF_ExternalCompressedAir];
	strCompressoExternalCompressedAir.Trim();

	if( 0 == strCompressoExternalCompressedAir.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::CompressoExternalAir, true );
	}
	else if( 0 == strCompressoExternalCompressedAir.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::CompressoExternalAir, false );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: Compresso external compressed air '%s' is not recognized."), iCurrentLineNumber, strCompressoExternalCompressedAir );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Pump-based and vacuum degassing cooling insulation.
	CString strPumpDegassingCoolingInsulation = m_vecStrings[IF_PumpDegassingCoolingInsulation];
	strPumpDegassingCoolingInsulation.Trim();

	if( 0 == strPumpDegassingCoolingInsulation.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, true );
	}
	else if( 0 == strPumpDegassingCoolingInsulation.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, false );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: Pump degassing cooling insulation '%s' is not recognized."), iCurrentLineNumber, strPumpDegassingCoolingInsulation );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Redundancy pump/compressor.
	CString strRedundancyPumpCompressor = m_vecStrings[IF_RedundancyPumpCompressor];
	strRedundancyPumpCompressor.Trim();

	if( 0 == strRedundancyPumpCompressor.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralRedundancyPumpComp, true );
	}
	else if( 0 == strRedundancyPumpCompressor.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralRedundancyPumpComp, false );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: redundancy pump/compressor '%s' is not recognized."), iCurrentLineNumber, strRedundancyPumpCompressor );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Redundancy complete tecbox.
	CString strRedundancyCompleteTecbox = m_vecStrings[IF_RedundancyCompleteTecbox];
	strRedundancyCompleteTecbox.Trim();

	if( 0 == strRedundancyCompleteTecbox.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralRedundancyTecBox, true );
	}
	else if( 0 == strRedundancyCompleteTecbox.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralRedundancyTecBox, false );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: redundancy complete tecbox '%s' is not recognized."), iCurrentLineNumber, strRedundancyCompleteTecbox );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Water make-up break tank required.
	CString strWaterMakeUpBreakTankRequired = m_vecStrings[IF_WaterMakeUpBreakTankRequired];
	strWaterMakeUpBreakTankRequired.Trim();

	if( 0 == strWaterMakeUpBreakTankRequired.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::WaterMakeupBreakTank, true );
	}
	else if( 0 == strWaterMakeUpBreakTankRequired.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::WaterMakeupBreakTank, false );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: water make-up break tank required '%s' is not recognized."), iCurrentLineNumber, strWaterMakeUpBreakTankRequired );
		_WriteToListCtrl( strMsg );
		return false;
	}

	// Water make-up break duty and stand-by.
	CString strWaterMakeUpDutyAndStandby = m_vecStrings[IF_WaterMakeUpDutyAndStandby];
	strWaterMakeUpDutyAndStandby.Trim();

	if( 0 == strWaterMakeUpDutyAndStandby.CompareNoCase( _T("true") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::WaterMakeupDutyStandBy, true );
	}
	else if( 0 == strWaterMakeUpDutyAndStandby.CompareNoCase( _T("false") ) )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::WaterMakeupDutyStandBy, false );
	}
	else
	{
		strMsg.Format( _T("    Error -> Line %i: water make-up break duty and stand-by '%s' is not recognized."), iCurrentLineNumber, strWaterMakeUpDutyAndStandby );
		_WriteToListCtrl( strMsg );
		return false;
	}

	pclPMInputUser->SetMaxWidth( _ttof( m_vecStrings[IF_MaxWidth] ) );
	pclPMInputUser->SetMaxHeight( _ttof( m_vecStrings[IF_MaxHeight] ) );

	return true;
}

void CDlgSpecActHyToolsPMCompare::_WriteInputs( CString &strOutput )
{
	strOutput = _T(";");

	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_RegionId] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_RegionCode] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_SafetyTempLimiter] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_SupplyTemperature] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_ReturnTemperature] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_MaxTemperature] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_MinTemperature] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_FillTemperature] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_Norm] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_PressureVolumeIndexEnabled] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_StaticHeight] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_PZ] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_PSV] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_PressurisationSide] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_PumpHead] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_VolumeSystem] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_SolarCollectorContent] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_InstalledPower] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_PressureMaintenanceEnabled] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_Degassing] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_WaterMakeUp] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_StaticPressureOfWaterNetwork] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_MakeUpWaterTemperature] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_WaterHardness] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_ExpansionVesselBag] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_PreferTecboxOnTop] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_CompressoVesselInternalCoating] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_ExternalCompressedAir] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_PumpDegassingCoolingInsulation] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_RedundancyPumpCompressor] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_RedundancyCompleteTecbox] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_WaterMakeUpBreakTankRequired] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_WaterMakeUpDutyAndStandby] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_MaxWidth] );
	strOutput.Format( _T("%s %s;"), strOutput, m_vecStrings[IF_MaxHeight ] );
}

void CDlgSpecActHyToolsPMCompare::_WriteToListCtrl( CString strMsg )
{
	if( NULL != m_List.GetSafeHwnd() )
	{
		m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strMsg ) );
		CDC *pDC = m_List.GetDC();
		CSize sz = pDC->GetTextExtent( strMsg );

		if( sz.cx > m_iLargestText )
		{
			m_List.SetHorizontalExtent( sz.cx );
			m_iLargestText = sz.cx;
		}

		m_List.ReleaseDC( pDC );
	}
}

CString CDlgSpecActHyToolsPMCompare::_ConvertDouble2String( double dValue, CString strReference )
{
	CString str;

	double dReference = _wtof( strReference );
	
	if( abs( dValue - dReference ) < 1e-14 )
	{
		// It can happen that we have difference from the 14th decimal between
		// HyTools and HySelect. In this case, we export the value from HyTools to avoid
		// to have differences when comparing files.
		str = strReference;
	}
	else
	{
		int iBeforeDec = strReference.Find( _T('.') );
		int iAfterDec = strReference.GetLength() - iBeforeDec - 1;
		int iExponant = strReference.Find( _T('e') );

		if( -1 != iExponant )
		{
			str.Format( _T("%.*E"), iExponant - iBeforeDec - 1, dValue );
			str.Delete( str.GetLength() - 2 );
		}
		else
		{
			str.Format( _T("%.*f"), iAfterDec, dValue );
		}
	}

	return str;
}

bool CDlgSpecActHyToolsPMCompare::_IsDoubleValueDifferent( double dValue1, double dValue2, double dPrecision )
{
	bool bDifferent = false;

	if( ( dValue1 != dValue2 ) && ( abs( dValue1 - dValue2 ) > dPrecision ) )
	{
		bDifferent = true;
	}

	return bDifferent;
}
