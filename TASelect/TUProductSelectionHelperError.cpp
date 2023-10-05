#include "stdafx.h"

#include "TUProductSelectionHelperError.h"

#ifdef DEBUG

CTUProductSelectionHelperError::CTUProductSelectionHelperError()
{
	m_mapErrorList[TU_ERROR_WHERE_FILEHELPER_OPENFILE] = _T("CTUProdSelFileHelper::OpenFile");
	m_mapErrorList[TU_ERROR_WHERE_FILEHELPER_READLINE] = _T("CTUProdSelFileHelper::ReadLine");
	m_mapErrorList[TU_ERROR_WHERE_FILEHELPER_WRITELINE] = _T("CTUProdSelFileHelper::WriteLine");
	m_mapErrorList[TU_ERROR_WHERE_LAUNCHER_EXECUTE] = _T("CTUProdSelLauncher::Execute");
	m_mapErrorList[TU_ERROR_WHERE_LAUNCHER_DROPOUTSELECTION] = _T("CTUProdSelLauncher::DropOutSelection");
	m_mapErrorList[TU_ERROR_WHERE_LAUNCHER_LOADTESTDROPRESULTS] = _T("CTUProdSelLauncher::LoadTestAndDropResults");
	m_mapErrorList[TU_ERROR_WHERE_HELPER_READINPUTS] = _T("CTUProdSelHelper::ReadInputs");
	m_mapErrorList[TU_ERROR_WHERE_HELPER_INTERPRETINPUTS] = _T("CTUProdSelHelper::InterpreteInputs");
	m_mapErrorList[TU_ERROR_WHERE_HELPER_VERIFYINPUTS] = _T("CTUProdSelHelper::VerifyInputs");
	m_mapErrorList[TU_ERROR_WHERE_HELPER_READOUTPUTS] = _T("CTUProdSelHelper::ReadOutputs");
	m_mapErrorList[TU_ERROR_WHERE_HELPER_LAUNCHTEST] = _T("CTUProdSelHelper::LaunchTest");
	m_mapErrorList[TU_ERROR_WHERE_HELPER_DROPOUTSELECTION] = _T("CTUProdSelHelper::DropOutSelection");

	m_mapErrorList[TU_ERROR_SELECTIONTYPE_PRODUCT] = _T("Product");
	m_mapErrorList[TU_ERROR_SELECTIONTYPE_INDIVIDUAL] = _T("Individual");
	m_mapErrorList[TU_ERROR_SELECTIONTYPE_BATCH] = _T("Batch");
	m_mapErrorList[TU_ERROR_SELECTIONTYPE_WIZARD] = _T("Wizard");
	m_mapErrorList[TU_ERROR_SELECTIONTYPE_DIRECT] = _T("Direct");

	m_mapErrorList[TU_ERROR_PRODUCTTYPE_BV] = _T("Balancing valves");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_CTRLBASE] = _T("Base for control valves");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_BCV] = _T("Balancing and control valves");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_CV] = _T("Control valves");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_DPC] = _T("Dp controller");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_DPCBCV] = _T("Combined Dp controller, balancing and control valves");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_PIBCV] = _T("Pressure independent balancing and control valves");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_PM] = _T("Pressurisation maintenance");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_SEPARATOR] = _T("Separators and air vents");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_SV] = _T("Shutoff valves");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_TRV] = _T("Thermostatic valves");
	m_mapErrorList[TU_ERROR_PRODUCTTYPE_SAFETYVALVE] = _T("Safety valves");

	m_mapErrorList[TU_ERROR_FILENAMEEMPTY] = _T("File name is empty.");
	m_mapErrorList[TU_ERROR_OPENMODEMUSTBESPECIFIED] = _T("Open mode must be specified.");
	m_mapErrorList[TU_ERROR_CANTOPENFILE] = _T("Can't open file.");
	m_mapErrorList[TU_ERROR_FILENOTOPENED] = _T("File not opened.");
	m_mapErrorList[TU_ERROR_UNEXPECTEDEOF] = _T("Unexpected end of file.");
	m_mapErrorList[TU_ERROR_ERRORINFILE] = _T("Error in file.");
	m_mapErrorList[TU_ERROR_BADTOKENINFILE] = _T("Can't tokenize string.");
	m_mapErrorList[TU_ERROR_WRITEERROR] = _T("Error when writing in the file.");
	m_mapErrorList[TU_ERROR_TADBNOTDEFINED] = _T("'m_pTADB' must be set.");
	m_mapErrorList[TU_ERROR_TADSNOTDEFINED] = _T("'m_pTADS' must be set.");
	m_mapErrorList[TU_ERROR_CANTCREATETECHPARAMBACKUP] = _T("Can't create the technical parameter object for the backup.");
	m_mapErrorList[TU_ERROR_ERRORWITHTECHPARAMBACKUP] = _T("Dynamic cast problem with IDPtr on the technical parameter object for the backup.");
	m_mapErrorList[TU_ERROR_ERRORWITHWATERCHARBACKUP] = _T("Can't retrieve the current water characteristic for the product selection.");
	m_mapErrorList[TU_ERROR_TECHPARAMSNOTDEFINED] = _T("'m_pTADS->GetpTechParams' not defined.");
	m_mapErrorList[TU_ERROR_TECHPARAMSBADSTART] = _T("'StartTechnicalParameters' must be set.");
	m_mapErrorList[TU_ERROR_BADPRODUCT] = _T("Product not recognized.");
	m_mapErrorList[TU_ERROR_CTUPRODSELERROR] = _T("Can't instantiate 'CTUProdSelLauncher'.");
	m_mapErrorList[TU_ERROR_PROSELPARAMSNULL] = _T("'pclProdSelParams' can't be NULL.");
	m_mapErrorList[TU_ERROR_PROSELPARAMSINVALID] = _T("'pclProdSelParams' is invalid.");
	m_mapErrorList[TU_ERROR_PRODSELTADSNOTDEFINED] = _T("'pclProdSelParams->m_pTADS' can't be NULL.");
	m_mapErrorList[TU_ERROR_PRODSELTECHPARAMSNOTDEFINED] = _T("'pclProdSelParams->m_pTADS->GetpTechParams' can't be NULL.");
	m_mapErrorList[TU_ERROR_CANTCREATEPRODSELHELPER] = _T("Can't create 'pclProdSelHelper'.");
	m_mapErrorList[TU_ERROR_INPUTBADSTART] = _T("Bad format of the input file. Must begin by 'StartInputs'.");
	m_mapErrorList[TU_ERROR_INPUTBADPAIR] = _T("Bad format of the input file. Input must be encoded 'Key = Value'.");
	m_mapErrorList[TU_ERROR_INPUTBADEND] = _T("Unexpected end of file. Don't you forget 'EndInputs'?");
	m_mapErrorList[TU_ERROR_WATERADDITFAMIDINVALID] = _T("Water additive family ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_WATERADDITIDINVALID] = _T("Water additive ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_WATERTEMPTOOLOW] = _T("Water temperature too low.");
	m_mapErrorList[TU_ERROR_WATERTEMPTOOHIGH] = _T("Water temperature too high.");
	m_mapErrorList[TU_ERROR_WATERADDTOOHIGH] = _T("Water temperature additive too high.");
	m_mapErrorList[TU_ERROR_PIPESERIESIDEMPTY] = _T("Pipe series ID can't be empty.");
	m_mapErrorList[TU_ERROR_PIPESERIESIDINVALID] = _T("Pipe series ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_PIPEIDINVALID] = _T("Pipe ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_OUTPUTBADSTART] = _T("Bad format of the input file. Must begin by 'StartOutputs'.");
	m_mapErrorList[TU_ERROR_OUTPUTBADNBRINOUTPUT] = _T("The number of fields in one output is not correct.");
	m_mapErrorList[TU_ERROR_NOINPUTTOTEST] = _T("There is no input to do test.");
	m_mapErrorList[TU_ERROR_CANTCREATESELECTLIST] = _T("Internal error. Can't create a instance of 'CSelectBCVList'.");
	m_mapErrorList[TU_ERROR_RESULTNBRNOTSAME] = _T("The number of result is not the same as the input file (Between the 'StartOutputs/EndOutputs').");
	m_mapErrorList[TU_ERROR_BADEXTRACT] = _T("Number of extracted values for one result is not the same as the input file.");
	m_mapErrorList[TU_ERROR_RESULTDIFFERENCE] = _T("Difference in one result.");
	m_mapErrorList[TU_ERROR_NOOUTPUT] = _T("There is no output to do test. For batch selection, each output contains value needed for the selection (Flow or Power/DT).");
	m_mapErrorList[TU_ERROR_CANTCREATEROWPARAMS] = _T("Internal error. Can't create a instance of 'CDlgBatchSelBase::BSRowParameters'.");
	m_mapErrorList[TU_ERROR_FILEPOINTERNULL] = _T("File pointer can't be NULL.");
	m_mapErrorList[TU_ERROR_FLOWORPOWERDTBAD] = _T("'FlowOrPowerDT' must be 'FlowMode' or 'PowerDTMode'.");
	m_mapErrorList[TU_ERROR_DPENABLEBAD] = _T("'DpEnable' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_ONLYFORSETBAD] = _T("'OnlyForSet' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_FLOWNOTVALID] = _T("Flow can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_POWERNOTVALID] = _T("Power can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_DTNOTVALID] = _T("DT can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_CANTCOMPUTEFLOW] = _T("Can't compute the flow with the power and DT defined.");
	m_mapErrorList[TU_ERROR_DPNOTVALID] = _T("Dp can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_TYPEIDINVALID] = _T("Type ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_FAMILYIDINVALID] = _T("Family ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_MATERIALIDINVALID] = _T("Material ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_CONNECTIDINVALID] = _T("Connect ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_VERSIONIDINVALID] = _T("Version ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_PNIDINVALID] = _T("PN ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_TYPEIDMISSING] = _T("Type ID must be defined.");
	m_mapErrorList[TU_ERROR_CV2W3WMISSING] = _T("'CV2w3w' must be defined.");
	m_mapErrorList[TU_ERROR_CV2W3WBAD] = _T("'CV2w3w' must be 'CV2W', 'CV3W', 'CV4W', 'CV6W' or 'None'.");
	m_mapErrorList[TU_ERROR_CVCTRLTYPEMISSING] = _T("'CVCtrlType' must be defined.");
	m_mapErrorList[TU_ERROR_CVCTRLTYPEBAD] = _T("'CVCtrlType' must be 'No', 'Proportional', 'OnOff' or '3point'.");
	m_mapErrorList[TU_ERROR_ACTUATORFAILSAFEFCTBAD] = _T("'ActuatorFailSafeFunction'  must be 'No' or 'Yes'.");
	m_mapErrorList[TU_ERROR_ACTPOWERSUPPLYIDINVALID] = _T("Actuator power supply ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_ACTINPUTSIGNALIDINVALID] = _T("Actuator input signal ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_GROUPKVSORDPBAD] = _T("'GroupKvsOrDpChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_KVSORDPBAD] = _T("'RadioKvsOrDp' must be 'Kvs' or 'Dp'.");
	m_mapErrorList[TU_ERROR_KVSNOTVALID] = _T("Kvs can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_GROUPDPBRANCHORKVBAD] = _T("'GroupDpBranchOrKvChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_GROUPDPBRANCHBAD] = _T("'GroupDpBranchChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_GROUPKVSBAD] = _T("'GroupKvsChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_DPMAXCHECKEDBAD] = _T("'DpMaxChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_DPSTABBAD] = _T("'DpStabilizedOn' must be 'Branch' or 'ControlValve'.");
	m_mapErrorList[TU_ERROR_DPCLOCBAD] = _T("'DpCLocalization' must be 'DownStream' or 'UpStream'.");
	m_mapErrorList[TU_ERROR_MVLOCBAD] = _T("'MvLocalization' must be 'Primary' or 'Secondary'.");
	m_mapErrorList[TU_ERROR_DPBRANCHINVALID] = _T("Dp branch can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_KVINVALID] = _T("Kv can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_DPMAXINVALID] = _T("Dp max can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_GROUPDPLBAD] = _T("'GroupDpToStabilizeChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_WITHSTSBAD] = _T("'WithSTSChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_DPLINVALID] = _T("Dp to stabilize can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_APPTYPEBAD] = _T("'ApplicationType' must be 'Heating, 'Solar' or 'Cooling.'");
	m_mapErrorList[TU_ERROR_DEGASSINGCHECKEDBAD] = _T("'DegassingChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_SYSVOLMISSING] = _T("'SystemVolume' is missing.");
	m_mapErrorList[TU_ERROR_PZMISSING] = _T("'Pz' is missing.");
	m_mapErrorList[TU_ERROR_PSVSMISSING] = _T("'PSVS' is missing.");
	m_mapErrorList[TU_ERROR_SFTYTEMPLIMITERMISSING] = _T("'SafetyTempLimiter' is missing.");
	m_mapErrorList[TU_ERROR_SUPPLYTEMPMISSING] = _T("'SupplyTemperature' is missing.");
	m_mapErrorList[TU_ERROR_RETURNTEMPMISSING] = _T("'ReturnTemperature' is missing.");
	m_mapErrorList[TU_ERROR_MINTEMPCHECKEDBAD] = _T("'MinTemperatureChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_PRESSONBAD] = _T("'PressurisationOn' must be 'PumpSuction' or 'PumpDischarge'.");
	m_mapErrorList[TU_ERROR_GENCOMBINONEDEVICEBAD] = _T("'GeneralCombinedInOneDevice' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_GENREDPUMPCOMPBAD] = _T("'GeneralRedundancyPumpComp' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_GENREDTECBOXBAD] = _T("'GeneralRedundancyTecBox' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_COMPINTCOATBAD] = _T("'CompressoInternalCoating' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_COMPEXTAIRBAD] = _T("'CompressoExternalAir' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_PUMPDEGCOOLVERSBAD] = _T("'PumpDegassingCoolingVersion' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_WMUPBREAKTANKBAD] = _T("'WaterMakeupBreakTank' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_WMUPSTDBYBAD] = _T("'WaterMakeupDutyStandBy' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_SYSVOLINVALID] = _T("System volume can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_SOLCONTINVALID] = _T("Solar content can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_PZINVALID] = _T("'Pz' can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_MAXTEMPINVALID] = _T("Max temperature can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_MINTEMPINVALID] = _T("Min temperature can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_FILLTEMPINVALID] = _T("Fill temperature can't below or equal to 0.");
	m_mapErrorList[TU_ERROR_PUMPHEADINVALID] = _T("Pump head can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_NPSHINVALID] = _T("NPSH can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_DEGMAXTEMPATCONPTINVALID] = _T("Degassing max temperature at the connection point can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_DEGPRESSCONPTINVALID] = _T("Degassing pressure at the connection point can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_WMUPNETPNINVALID] = _T("Water make-up network PN can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_WMUPTEMPINVALID] = _T("Water make-up water temperature can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_WMUPHARDINVALID] = _T("Water make-up water hardness can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_ISTRVTYPEPRESETBAD] = _T("'IsTrvTypePreset' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_VALVETYPEMISSING] = _T("'ValveType' is missing.");
	m_mapErrorList[TU_ERROR_VALVETYPEBAD] = _T("'ValveType must be 'Standard', 'Presettable', 'WithFlowLimitation' or 'Inserts'.");
	m_mapErrorList[TU_ERROR_INSERTTYPEBAD] = _T("'InsertType must be Heimeier' or 'ByKv'.");
	m_mapErrorList[TU_ERROR_RETURNVALVEMODEMISSING] = _T("'ReturnValveMode' is mussing.");
	m_mapErrorList[TU_ERROR_RETURNVALVEMODEBAD] = _T("'ReturnValveMode' must be 'Nothing', 'Other' or 'IMI'.");
	m_mapErrorList[TU_ERROR_HEADTYPEBAD] = _T("'HeadType' must be 'Thermostatic' or 'ElectroActuator'.");
	m_mapErrorList[TU_ERROR_KVINSERTINVALID] = _T("Kv value for insert can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_COMBOSVTYPEIDEMPTY] = _T("Combo SV type ID must be defined.");
	m_mapErrorList[TU_ERROR_COMBOSVTYPEIDINVALID] = _T("Combo SV typed ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_COMBOSVFAMIDEMPTY] = _T("Combo SV family ID must be defined.");
	m_mapErrorList[TU_ERROR_COMBOSVFAMIDINVALID] = _T("Combo SV family ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_COMBOSVCONIDINVALID] = _T("Combo SV connect ID defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_COMBOSVVERSIDINVALID] = _T("Combo SV version ID defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_COMBORVFAMIDEMPTY] = _T("Combo RV family ID must be defined.");
	m_mapErrorList[TU_ERROR_COMBORVFAMIDINVALID] = _T("Combo RV family ID defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_COMBORVCONIDINVALID] = _T("Combo RV connect ID defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_COMBORVVERSIDINVALID] = _T("Combo RV version ID defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_COMBOSVINSNAMEEMPTY] = _T("Combo SV insert name must be defined.");
	m_mapErrorList[TU_ERROR_COMBOSVINSNAMEINVALID] = _T("Combo SV insert name defined but invalid.");
	m_mapErrorList[TU_ERROR_COMBOSVINSTYPEIDEMPTY] = _T("Combo SV insert connect ID must be defined.");
	m_mapErrorList[TU_ERROR_COMBOSVINSTYPEIDINVALID] = _T("Combo SV insert connect ID defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_COMBORVINSFAMIDINVALID] = _T("Combo SV insert version ID defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_DPGROUPCHECKEDBAD] = _T("'DpGroupChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_TYPEB65IDEMPTY] = _T("Combo type below 65 ID must be defined.");
	m_mapErrorList[TU_ERROR_TYPEB65IDINVALID] = _T("Combo type below 65 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_FAMILYB65IDEMPTY] = _T("Combo family below 65 ID must be defined.");
	m_mapErrorList[TU_ERROR_FAMILYB65IDINVALID] = _T("Combo family below 65 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_MATERIALB65IDEMPTY] = _T("Combo material below 65 ID must be defined.");
	m_mapErrorList[TU_ERROR_MATERIALB65IDINVALID] = _T("Combo material below 65 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_CONNECTB65IDEMPTY] = _T("Combo connection below 65 ID must be defined.");
	m_mapErrorList[TU_ERROR_CONNECTB65IDINVALID] = _T("Combo connection below 65 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_VERSIONB65IDEMPTY] = _T("Combo version below 65 ID must be defined.");
	m_mapErrorList[TU_ERROR_VERSIONB65IDINVALID] = _T("Combo version below 65 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_PNB65IDEMPTY] = _T("Combo PN below 65 ID must be defined.");
	m_mapErrorList[TU_ERROR_PNB65IDINVALID] = _T("Combo PN below 65 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_TYPEA50IDEMPTY] = _T("Combo type above 50 ID must be defined.");
	m_mapErrorList[TU_ERROR_TYPEA50IDINVALID] = _T("Combo type above 50 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_FAMILYA50IDEMPTY] = _T("Combo family above 50 ID must be defined.");
	m_mapErrorList[TU_ERROR_FAMILYA50IDINVALID] = _T("Combo family above 50 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_MATERIALA50IDEMPTY] = _T("Combo material above 50 ID must be defined.");
	m_mapErrorList[TU_ERROR_MATERIALA50IDINVALID] = _T("Combo material above 50 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_CONNECTA50IDEMPTY] = _T("Combo connection above 50 ID must be defined.");
	m_mapErrorList[TU_ERROR_CONNECTA50IDINVALID] = _T("Combo connection above 50 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_VERSIONA50IDEMPTY] = _T("Combo version above 50 ID must be defined.");
	m_mapErrorList[TU_ERROR_VERSIONA50IDINVALID] = _T("Combo version above 50 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_PNA50IDEMPTY] = _T("Combo PN above 50 ID must be defined.");
	m_mapErrorList[TU_ERROR_PNA50IDINVALID] = _T("Combo PN above 50 ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_CTRLTYPESTRICTCHECKBAD] = _T("'CtrlTypeStrictChecked' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_ACTRSELASSETCHECKBAD] = _T("'ActuatorSelectedAsSet' must be 'True', '1', 'False' or '0'.");
	m_mapErrorList[TU_ERROR_ACTPOWERSUPPLYIDEMPTY] = _T("Actuator power supply ID must be defined.");
	m_mapErrorList[TU_ERROR_ACTINPUTSIGNALIDEMPTY] = _T("Actuator input signal ID must be defined.");
	m_mapErrorList[TU_ERROR_BVTYPEB65IDEMPTY] = _T("Combo type below 65 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVFAMILYB65IDEMPTY] = _T("Combo family below 65 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVMATERIALB65IDEMPTY] = _T("Combo material below 65 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVCONNECTB65IDEMPTY] = _T("Combo connection below 65 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVVERSIONB65IDEMPTY] = _T("Combo version below 65 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVPNB65IDEMPTY] = _T("Combo PN below 65 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVTYPEA50IDEMPTY] = _T("Combo type above 50 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVFAMILYA50IDEMPTY] = _T("Combo family above 50 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVMATERIALA50IDEMPTY] = _T("Combo material above 50 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVCONNECTA50IDEMPTY] = _T("Combo connection above 50 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVVERSIONA50IDEMPTY] = _T("Combo version above 50 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVPNA50IDEMPTY] = _T("Combo PN above 50 ID for regulating valve must be defined.");
	m_mapErrorList[TU_ERROR_BVTYPEB65IDINVALID] = _T("Combo type below 65 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVFAMILYB65IDINVALID] = _T("Combo family below 65 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVMATERIALB65IDINVALID] = _T("Combo material below 65 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVCONNECTB65IDINVALID] = _T("Combo connection below 65 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVVERSIONB65IDINVALID] = _T("Combo version below 65 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVPNB65IDINVALID] = _T("Combo PN below 65 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVTYPEA50IDINVALID] = _T("Combo type above 50 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVFAMILYA50IDINVALID] = _T("Combo family above 50 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVMATERIALA50IDINVALID] = _T("Combo material above 50 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVCONNECTA50IDINVALID] = _T("Combo connection above 50 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVVERSIONA50IDINVALID] = _T("Combo version above 50 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_BVPNA50IDINVALID] = _T("Combo PN above 50 ID is defined for regulating valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_TYPEIDEMPTY] = _T("Combo type ID must be defined.");
	m_mapErrorList[TU_ERROR_FAMILYIDEMPTY] = _T("Combo family ID must be defined.");
	m_mapErrorList[TU_ERROR_CONNECTIDEMPTY] = _T("Combo connect ID must be defined.");
	m_mapErrorList[TU_ERROR_VERSIONIDEMPTY] = _T("Combo version ID must be defined.");
	m_mapErrorList[TU_ERROR_TYPEIDINVALID] = _T("Combo type ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_FAMILYIDINVALID] = _T("Combo family ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_CONNECTIDINVALID] = _T("Combo connect ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_CANTREADTESTNUMBER] = _T("'Test number' must be set.");
	m_mapErrorList[TU_ERROR_CANTFINDTESTNUMBER] = _T("Can't read 'Test number' in the current drop out file.");
	m_mapErrorList[TU_ERROR_SYSTEMAPPLICATIONTYPEBAD] = _T("System application type must be 'Heating, 'Solar' or 'Cooling.'");
	m_mapErrorList[TU_ERROR_SYSTEMHEATGENERATORTYPEIDEMPTY] = _T("System heat generator type ID must be defined.");
	m_mapErrorList[TU_ERROR_NORMIDEMPTY] = _T("Norm ID must be defined.");
	m_mapErrorList[TU_ERROR_INSTALLEDPOWEREMPTY] = _T("Installed power must be defined.");
	m_mapErrorList[TU_ERROR_INSTALLEDCOLLECTOREMPTY] = _T("Installed collector must be defined.");
	m_mapErrorList[TU_ERROR_USERSETPRESSURECHOICEEMPTY] = _T("User set pressure choice must be defined.");
	m_mapErrorList[TU_ERROR_HEATGENERATORTYPEIDINVALID] = _T("System heat generator type ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_NORMIDINVALID] = _T("Norm ID is defined but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_INSTALLEDPOWERNOVALID] = _T("Installed power value can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_INSTALLEDCOLLECTORNOVALID] = _T("Installed collector value can't be below or equal to 0.");
	m_mapErrorList[TU_ERROR_SAFETYVALVEFAMILYIDINVALID] = _T("Combo family ID is defined for safety valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_SAFETYVALVECONNECTIDINVALID] = _T("Combo connection ID is defined for safety valve but doesn't exist in the database.");
	m_mapErrorList[TU_ERROR_USERSETPRESSURENOVALID] = _T("User set pressure choice value can't be below or equal to 0.");
}

void CTUProductSelectionHelperError::GetErrorText( std::vector<CString> &vecMessage, UINT uiErrorCode )
{
	vecMessage.clear();

	CString strError;
	strError.Format( _T("  Error code: 0x%08X"), uiErrorCode );
	vecMessage.push_back( strError );

	UINT uiWhere = uiErrorCode & TU_ERRORMASK_WHERE;

	if( 0 != m_mapErrorList.count( uiWhere ) )
	{
		strError = _T("  Where: ") + m_mapErrorList[uiWhere];
		vecMessage.push_back( strError );
	}

	UINT uiSelectionType = uiErrorCode & TU_ERRORMASK_SELECTIONTYPE;

	if( 0 != m_mapErrorList.count( uiSelectionType ) )
	{
		strError = _T("  Selection type: ") + m_mapErrorList[uiSelectionType];
		vecMessage.push_back( strError );
	}

	UINT uiProductType = uiErrorCode & TU_ERRORMASK_PRODUCTYPE;

	if( 0 != m_mapErrorList.count( uiProductType ) )
	{
		strError = _T("  Product type: ") + m_mapErrorList[uiProductType];
		vecMessage.push_back( strError );
	}

	UINT uiDefinition = uiErrorCode & TU_ERRORMASK_DEFINITION;

	if( 0 != m_mapErrorList.count( uiDefinition ) )
	{
		strError = _T("  Details: ") + m_mapErrorList[uiDefinition];
		vecMessage.push_back( strError );
	}
}

#endif