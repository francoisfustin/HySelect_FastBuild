#pragma once


#ifdef DEBUG

#include "vector"
#include "map"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Errors defines.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TU_ERRORMASK_WHERE									0xF0000000
#define TU_ERROR_WHERE_FILEHELPER_OPENFILE					0x10000000
#define TU_ERROR_WHERE_FILEHELPER_READLINE					0x20000000
#define TU_ERROR_WHERE_FILEHELPER_WRITELINE					0x30000000
#define TU_ERROR_WHERE_LAUNCHER_EXECUTE						0x40000000
#define TU_ERROR_WHERE_LAUNCHER_DROPOUTSELECTION			0x50000000
#define TU_ERROR_WHERE_LAUNCHER_LOADTESTDROPRESULTS			0x60000000
#define TU_ERROR_WHERE_HELPER_READINPUTS					0x70000000
#define TU_ERROR_WHERE_HELPER_INTERPRETINPUTS				0x80000000
#define TU_ERROR_WHERE_HELPER_VERIFYINPUTS					0x90000000
#define TU_ERROR_WHERE_HELPER_READOUTPUTS					0xA0000000
#define TU_ERROR_WHERE_HELPER_LAUNCHTEST					0xB0000000
#define TU_ERROR_WHERE_HELPER_DROPOUTSELECTION				0xC0000000

#define TU_ERRORMASK_SELECTIONTYPE							0x0F000000
#define TU_ERROR_SELECTIONTYPE_PRODUCT						0x01000000
#define TU_ERROR_SELECTIONTYPE_INDIVIDUAL					0x02000000
#define TU_ERROR_SELECTIONTYPE_BATCH						0x03000000
#define TU_ERROR_SELECTIONTYPE_WIZARD						0x04000000
#define TU_ERROR_SELECTIONTYPE_DIRECT						0x05000000

#define TU_ERRORMASK_PRODUCTYPE								0x00F00000
#define TU_ERROR_PRODUCTTYPE_BV								0x00100000
#define TU_ERROR_PRODUCTTYPE_CTRLBASE						0x00200000
#define TU_ERROR_PRODUCTTYPE_BCV							0x00300000
#define TU_ERROR_PRODUCTTYPE_CV								0x00400000
#define TU_ERROR_PRODUCTTYPE_DPC							0x00500000
#define TU_ERROR_PRODUCTTYPE_DPCBCV							0x00600000
#define TU_ERROR_PRODUCTTYPE_PIBCV							0x00700000
#define TU_ERROR_PRODUCTTYPE_PM								0x00800000
#define TU_ERROR_PRODUCTTYPE_SEPARATOR						0x00900000
#define TU_ERROR_PRODUCTTYPE_SV								0x00A00000
#define TU_ERROR_PRODUCTTYPE_TRV							0x00B00000
#define TU_ERROR_PRODUCTTYPE_SAFETYVALVE					0x00C00000
#define TU_ERROR_PRODUCTTYPE_SIXWAYVALVE					0x00D00000
#define TU_ERROR_PRODUCTTYPE_SMARTCONTROLVALVE						0x00E00000

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for product selection errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TU_ERRORMASK_DEFINITION								0x000000FF
#define TU_PRODSELECT_ERROR_OK								0

// "File name is empty."
#define TU_ERROR_FILENAMEEMPTY								( TU_PRODSELECT_ERROR_OK + 0x01 )

// "Open mode must be specified.";
#define TU_ERROR_OPENMODEMUSTBESPECIFIED					( TU_ERROR_FILENAMEEMPTY + 0x01 )

// "Can't open file.";
#define TU_ERROR_CANTOPENFILE								( TU_ERROR_OPENMODEMUSTBESPECIFIED + 0x01 )

// "File not opened."
#define	TU_ERROR_FILENOTOPENED								( TU_ERROR_CANTOPENFILE + 0x01 )

// "Unexpected end of file."
#define TU_ERROR_UNEXPECTEDEOF								( TU_ERROR_FILENOTOPENED + 0x01 )

// "Error in file."
#define	TU_ERROR_ERRORINFILE								( TU_ERROR_UNEXPECTEDEOF + 0x01 )

// "Can't tokenize string."
#define	TU_ERROR_BADTOKENINFILE								( TU_ERROR_ERRORINFILE + 0x01 )

// "Error when writing in the file."
#define	TU_ERROR_WRITEERROR									( TU_ERROR_BADTOKENINFILE + 0x01 )

// "'m_pTADB' must be set."
#define	TU_ERROR_TADBNOTDEFINED								( TU_ERROR_WRITEERROR + 0x01 )

// "'m_pTADS' must be set."
#define	TU_ERROR_TADSNOTDEFINED								( TU_ERROR_TADBNOTDEFINED + 0x01 )

// "Can't create the technical parameter object for the backup."
#define	TU_ERROR_CANTCREATETECHPARAMBACKUP					( TU_ERROR_TADSNOTDEFINED + 0x01 )

// "Dynamic cast problem with IDPtr on the technical parameter object for the backup."
#define TU_ERROR_ERRORWITHTECHPARAMBACKUP					( TU_ERROR_CANTCREATETECHPARAMBACKUP + 0x01 )

// "Can't retrieve the current water characteristic for the product selection."
#define TU_ERROR_ERRORWITHWATERCHARBACKUP					( TU_ERROR_ERRORWITHTECHPARAMBACKUP + 0x01 )

// "'m_pTADS->GetpTechParams' not defined."
#define	TU_ERROR_TECHPARAMSNOTDEFINED						( TU_ERROR_ERRORWITHTECHPARAMBACKUP + 0x01 )

// "'StartTechnicalParameters' must be set."
#define	TU_ERROR_TECHPARAMSBADSTART							( TU_ERROR_TECHPARAMSNOTDEFINED + 0x01 )

// "Product not recognized."
#define	TU_ERROR_BADPRODUCT									( TU_ERROR_TECHPARAMSBADSTART + 0x01 )

// "Can't instantiate 'CTUProdSelLauncher'."
#define	TU_ERROR_CTUPRODSELERROR							( TU_ERROR_BADPRODUCT + 0x01 )

// "'pclProdSelParams' can't be NULL."
#define	TU_ERROR_PROSELPARAMSNULL							( TU_ERROR_CTUPRODSELERROR + 0x01 )

// "'pclProdSelParams' is invalid."
#define	TU_ERROR_PROSELPARAMSINVALID						( TU_ERROR_PROSELPARAMSNULL + 0x01 )

// "'pclProdSelParams->m_pTADS' can't be NULL."
#define	TU_ERROR_PRODSELTADSNOTDEFINED						( TU_ERROR_PROSELPARAMSINVALID + 0x01 )

// "'pclProdSelParams->m_pTADS->GetpTechParams' can't be NULL."
#define	TU_ERROR_PRODSELTECHPARAMSNOTDEFINED				( TU_ERROR_PRODSELTADSNOTDEFINED + 0x01 )

// "Can't create 'pclProdSelHelper'."
#define	TU_ERROR_CANTCREATEPRODSELHELPER					( TU_ERROR_PRODSELTECHPARAMSNOTDEFINED + 0x01 )

// "Bad format of the input file. Must begin by 'StartInputs'."
#define	TU_ERROR_INPUTBADSTART								( TU_ERROR_CANTCREATEPRODSELHELPER + 0x01 )

// "Bad format of the input file. Input must be encoded 'Key = Value'."
#define	TU_ERROR_INPUTBADPAIR								( TU_ERROR_INPUTBADSTART + 0x01 )

// "Unexpected end of file. Don't you forget 'EndInputs'?"
#define	TU_ERROR_INPUTBADEND								( TU_ERROR_INPUTBADPAIR + 0x01 )

// "Water additive family ID is defined but doesn't exist in the database."
#define	TU_ERROR_WATERADDITFAMIDINVALID						( TU_ERROR_INPUTBADEND + 0x01 )

// "Water additive ID is defined but doesn't exist in the database."
#define	TU_ERROR_WATERADDITIDINVALID						( TU_ERROR_WATERADDITFAMIDINVALID + 0x01 )

// "Water temperature too low."
#define	TU_ERROR_WATERTEMPTOOLOW							( TU_ERROR_WATERADDITIDINVALID + 0x01 )

// "Water temperature too high."
#define	TU_ERROR_WATERTEMPTOOHIGH							( TU_ERROR_WATERTEMPTOOLOW + 0x01 )

// "Water temperature additive too high."
#define	TU_ERROR_WATERADDTOOHIGH							( TU_ERROR_WATERTEMPTOOHIGH + 0x01 )

// "Pipe series ID can't be empty."
#define	TU_ERROR_PIPESERIESIDEMPTY							( TU_ERROR_WATERADDTOOHIGH + 0x01 )

// "Pipe series ID is defined but doesn't exist in the database."
#define	TU_ERROR_PIPESERIESIDINVALID						( TU_ERROR_PIPESERIESIDEMPTY + 0x01 )

// "Pipe ID is defined but doesn't exist in the database."
#define	TU_ERROR_PIPEIDINVALID								( TU_ERROR_PIPESERIESIDINVALID + 0x01 )

// "Bad format of the input file. Must begin by 'StartOutputs'."
#define	TU_ERROR_OUTPUTBADSTART								( TU_ERROR_PIPEIDINVALID + 0x01 )

// "The number of fields in one output is not correct."
#define	TU_ERROR_OUTPUTBADNBRINOUTPUT						( TU_ERROR_OUTPUTBADSTART + 0x01 )

// "There is no input to do test."
#define TU_ERROR_NOINPUTTOTEST								( TU_ERROR_OUTPUTBADNBRINOUTPUT + 0x01 )

// "Internal error. Can't create a instance of 'CSelectBCVList'."
#define TU_ERROR_CANTCREATESELECTLIST						( TU_ERROR_NOINPUTTOTEST + 0x01 )

// "The number of result is not the same as the input file (Between the 'StartOutputs/EndOutputs')."
#define TU_ERROR_RESULTNBRNOTSAME							( TU_ERROR_CANTCREATESELECTLIST + 0x01 )

// "Number of extracted values for one result is not the same as the input file."
#define TU_ERROR_BADEXTRACT									( TU_ERROR_RESULTNBRNOTSAME + 0x01 )

// "Difference in one result."
#define TU_ERROR_RESULTDIFFERENCE							( TU_ERROR_BADEXTRACT + 0x01 )

// "There is no output to do test. For batch selection, each output contains value needed for the selection (Flow or Power/DT)."
#define TU_ERROR_NOOUTPUT									( TU_ERROR_RESULTDIFFERENCE + 0x01 )

// "Internal error. Can't create a instance of 'CDlgBatchSelBase::BSRowParameters'."
#define TU_ERROR_CANTCREATEROWPARAMS						( TU_ERROR_NOOUTPUT + 0x01 )

// "File pointer can't be NULL."
#define TU_ERROR_FILEPOINTERNULL							( TU_ERROR_CANTCREATEROWPARAMS + 0x01 )

// "'FlowOrPowerDT' must be 'FlowMode' or 'PowerDTMode'."
#define	TU_ERROR_FLOWORPOWERDTBAD							( TU_ERROR_FILEPOINTERNULL + 0x01 )

// "'DpEnable' must be 'True', '1', 'False' or '0'."
#define TU_ERROR_DPENABLEBAD								( TU_ERROR_FLOWORPOWERDTBAD + 0x01 )

// "'OnlyForSet' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_ONLYFORSETBAD								( TU_ERROR_DPENABLEBAD + 0x01 )

// "Flow can't be below or equal to 0."
#define TU_ERROR_FLOWNOTVALID								( TU_ERROR_ONLYFORSETBAD + 0x01 )

// "Power can't be below or equal to 0."
#define TU_ERROR_POWERNOTVALID								( TU_ERROR_FLOWNOTVALID + 0x01 )

// "DT can't be below or equal to 0."
#define TU_ERROR_DTNOTVALID									( TU_ERROR_POWERNOTVALID + 0x01 )

// "Can't compute the flow with the power and DT defined."
#define TU_ERROR_CANTCOMPUTEFLOW							( TU_ERROR_DTNOTVALID + 0x01 )

// "Dp can't be below or equal to 0."
#define	TU_ERROR_DPNOTVALID									( TU_ERROR_CANTCOMPUTEFLOW + 0x01 )

// "Type ID is defined but doesn't exist in the database."
#define	TU_ERROR_TYPEIDINVALID								( TU_ERROR_DPNOTVALID + 0x01 )

// "Family ID is defined but doesn't exist in the database."
#define	TU_ERROR_FAMILYIDINVALID							( TU_ERROR_TYPEIDINVALID + 0x01 )

// "Material ID is defined but doesn't exist in the database."
#define	TU_ERROR_MATERIALIDINVALID							( TU_ERROR_FAMILYIDINVALID + 0x01 )

// "Connect ID is defined but doesn't exist in the database."
#define	TU_ERROR_CONNECTIDINVALID							( TU_ERROR_MATERIALIDINVALID + 0x01 )

// "Version ID is defined but doesn't exist in the database."
#define	TU_ERROR_VERSIONIDINVALID							( TU_ERROR_CONNECTIDINVALID + 0x01 )

// "PN ID is defined but doesn't exist in the database."
#define	TU_ERROR_PNIDINVALID								( TU_ERROR_VERSIONIDINVALID + 0x01 )

// "Type ID must be defined."
#define	TU_ERROR_TYPEIDMISSING								( TU_ERROR_PNIDINVALID + 0x01 )

// "'CV2w3w' must be defined."
#define	TU_ERROR_CV2W3WMISSING								( TU_ERROR_TYPEIDMISSING + 0x01 )

// "'CV2w3w' must be 'CV2W', 'CV3W', 'CV4W', 'CV6W' or 'None'."
#define	TU_ERROR_CV2W3WBAD									( TU_ERROR_CV2W3WMISSING + 0x01 )

// "'CVCtrlType' must be defined."
#define	TU_ERROR_CVCTRLTYPEMISSING							( TU_ERROR_CV2W3WBAD + 0x01 )

// "'CVCtrlType' must be 'No', 'Proportional', 'OnOff' or '3point'."
#define	TU_ERROR_CVCTRLTYPEBAD								( TU_ERROR_CVCTRLTYPEMISSING + 0x01 )

// "'ActuatorFailSafeFunction'  must be 'No', 'Closing' or 'Opening'."
#define	TU_ERROR_ACTUATORFAILSAFEFCTBAD						( TU_ERROR_CVCTRLTYPEBAD + 0x01 )

// "Actuator power supply ID is defined but doesn't exist in the database."
#define	TU_ERROR_ACTPOWERSUPPLYIDINVALID					( TU_ERROR_ACTUATORFAILSAFEFCTBAD + 0x01 )

// "Actuator input signal ID is defined but doesn't exist in the database."
#define	TU_ERROR_ACTINPUTSIGNALIDINVALID					( TU_ERROR_ACTPOWERSUPPLYIDINVALID + 0x01 )

// "'GroupKvsOrDpChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_GROUPKVSORDPBAD							( TU_ERROR_ACTINPUTSIGNALIDINVALID + 0x01 )

// "'RadioKvsOrDp' must be 'Kvs' or 'Dp'."
#define	TU_ERROR_KVSORDPBAD									( TU_ERROR_GROUPKVSORDPBAD + 0x01 )

// "Kvs can't be below or equal to 0."
#define	TU_ERROR_KVSNOTVALID								( TU_ERROR_KVSORDPBAD + 0x01 )

// "'GroupDpBranchOrKvChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_GROUPDPBRANCHORKVBAD						( TU_ERROR_KVSNOTVALID + 0x01 )

// "'GroupDpBranchChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_GROUPDPBRANCHBAD							( TU_ERROR_GROUPDPBRANCHORKVBAD + 0x01 )

// "'GroupKvsChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_GROUPKVSBAD								( TU_ERROR_GROUPDPBRANCHBAD + 0x01 )

// "'DpMaxChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_DPMAXCHECKEDBAD							( TU_ERROR_GROUPKVSBAD + 0x01 )

// "'DpStabilizedOn' must be 'Branch' or 'ControlValve'."
#define	TU_ERROR_DPSTABBAD									( TU_ERROR_DPMAXCHECKEDBAD + 0x01 )

// "'DpCLocalization' must be 'DownStream' or 'UpStream'."
#define	TU_ERROR_DPCLOCBAD									( TU_ERROR_DPSTABBAD + 0x01 )

// "'MvLocalization' must be 'Primary' or 'Secondary'."
#define	TU_ERROR_MVLOCBAD									( TU_ERROR_DPCLOCBAD + 0x01 )

// "Dp branch can't be below or equal to 0."
#define	TU_ERROR_DPBRANCHINVALID							( TU_ERROR_MVLOCBAD + 0x01 )

// "Kv can't be below or equal to 0."
#define	TU_ERROR_KVINVALID									( TU_ERROR_DPBRANCHINVALID + 0x01 )

// "Dp max can't be below or equal to 0."
#define	TU_ERROR_DPMAXINVALID								( TU_ERROR_KVINVALID + 0x01 )

// "'GroupDpToStabilizeChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_GROUPDPLBAD								( TU_ERROR_DPMAXINVALID + 0x01 )

// "'WithSTSChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_WITHSTSBAD									( TU_ERROR_GROUPDPLBAD + 0x01 )

// "Dp to stabilize can't be below or equal to 0."
#define	TU_ERROR_DPLINVALID									( TU_ERROR_WITHSTSBAD + 0x01 )

// "'ApplicationType' must be 'Heating, 'Solar' or 'Cooling.'"
#define	TU_ERROR_APPTYPEBAD									( TU_ERROR_DPLINVALID + 0x01 )

// "'DegassingChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_DEGASSINGCHECKEDBAD						( TU_ERROR_APPTYPEBAD + 0x01 )

// "'SystemVolume' is missing."
#define	TU_ERROR_SYSVOLMISSING								( TU_ERROR_DEGASSINGCHECKEDBAD + 0x01 )

// "'Pz' is missing."
#define TU_ERROR_PZMISSING									( TU_ERROR_SYSVOLMISSING + 0x01 )

// "'PSVS' is missing."
#define	TU_ERROR_PSVSMISSING								( TU_ERROR_PZMISSING + 0x01 )

// "'SafetyTempLimiter' is missing."
#define	TU_ERROR_SFTYTEMPLIMITERMISSING						( TU_ERROR_PSVSMISSING + 0x01 )

// "'SupplyTemperature' is missing."
#define	TU_ERROR_SUPPLYTEMPMISSING							( TU_ERROR_SFTYTEMPLIMITERMISSING + 0x01 )

// "'ReturnTemperature' is missing."
#define	TU_ERROR_RETURNTEMPMISSING							( TU_ERROR_SUPPLYTEMPMISSING + 0x01 )

// "'MinTemperatureChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_MINTEMPCHECKEDBAD							( TU_ERROR_RETURNTEMPMISSING + 0x01 )

// "'PressurisationOn' must be 'PumpSuction' or 'PumpDischarge'."
#define	TU_ERROR_PRESSONBAD									( TU_ERROR_MINTEMPCHECKEDBAD + 0x01 )

// "'GeneralCombinedInOneDevice' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_GENCOMBINONEDEVICEBAD						( TU_ERROR_PRESSONBAD + 0x01 )

// "'GeneralRedundancyPumpComp' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_GENREDPUMPCOMPBAD							( TU_ERROR_GENCOMBINONEDEVICEBAD + 0x01 )

// "'GeneralRedundancyTecBox' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_GENREDTECBOXBAD							( TU_ERROR_GENREDPUMPCOMPBAD + 0x01 )

// "'CompressoInternalCoating' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_COMPINTCOATBAD								( TU_ERROR_GENREDTECBOXBAD + 0x01 )

// "'CompressoExternalAir' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_COMPEXTAIRBAD								( TU_ERROR_COMPINTCOATBAD + 0x01 )

// "'PumpDegassingCoolingVersion' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_PUMPDEGCOOLVERSBAD							( TU_ERROR_COMPEXTAIRBAD + 0x01 )

// "'WaterMakeupBreakTank' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_WMUPBREAKTANKBAD							( TU_ERROR_PUMPDEGCOOLVERSBAD + 0x01 )

// "'WaterMakeupDutyStandBy' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_WMUPSTDBYBAD								( TU_ERROR_WMUPBREAKTANKBAD + 0x01 )

// "System volume can't be below or equal to 0."
#define	TU_ERROR_SYSVOLINVALID								( TU_ERROR_WMUPSTDBYBAD + 0x01 )

// "Solar content can't be below or equal to 0."
#define	TU_ERROR_SOLCONTINVALID								( TU_ERROR_SYSVOLINVALID + 0x01 )

// "'Pz' can't be below or equal to 0."
#define TU_ERROR_PZINVALID									( TU_ERROR_SOLCONTINVALID + 0x01 )

// "Max temperature can't be below or equal to 0."
#define	TU_ERROR_MAXTEMPINVALID								( TU_ERROR_PZINVALID + 0x01 )

// "Min temperature can't be below or equal to 0."
#define	TU_ERROR_MINTEMPINVALID								( TU_ERROR_MAXTEMPINVALID + 0x01 )

// "Fill temperature can't below or equal to 0."
#define	TU_ERROR_FILLTEMPINVALID							( TU_ERROR_MINTEMPINVALID + 0x01 )

// "Pump head can't be below or equal to 0."
#define	TU_ERROR_PUMPHEADINVALID							( TU_ERROR_FILLTEMPINVALID + 0x01 )

// "NPSH can't be below or equal to 0."
#define	TU_ERROR_NPSHINVALID								( TU_ERROR_PUMPHEADINVALID + 0x01 )

// "Degassing max temperature at the connection point can't be below or equal to 0."
#define	TU_ERROR_DEGMAXTEMPATCONPTINVALID					( TU_ERROR_NPSHINVALID + 0x01 )

// "Degassing pressure at the connection point can't be below or equal to 0."
#define	TU_ERROR_DEGPRESSCONPTINVALID						( TU_ERROR_DEGMAXTEMPATCONPTINVALID + 0x01 )

// "Water make-up network PN can't be below or equal to 0."
#define	TU_ERROR_WMUPNETPNINVALID							( TU_ERROR_DEGPRESSCONPTINVALID + 0x01 )

// "Water make-up water temperature can't be below or equal to 0."
#define	TU_ERROR_WMUPTEMPINVALID							( TU_ERROR_WMUPNETPNINVALID + 0x01 )

// "Water make-up water hardness can't be below or equal to 0."
#define	TU_ERROR_WMUPHARDINVALID							( TU_ERROR_WMUPTEMPINVALID + 0x01 )

// "'IsTrvTypePreset' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_ISTRVTYPEPRESETBAD							( TU_ERROR_WMUPHARDINVALID + 0x01 )

// "'ValveType' is missing."
#define	TU_ERROR_VALVETYPEMISSING							( TU_ERROR_ISTRVTYPEPRESETBAD + 0x01 )

// "'ValveType must be 'Standard', 'Presettable', 'WithFlowLimitation' or 'Inserts'."
#define	TU_ERROR_VALVETYPEBAD								( TU_ERROR_VALVETYPEMISSING + 0x01 )

// "'InsertType must be Heimeier' or 'ByKv'."
#define	TU_ERROR_INSERTTYPEBAD								( TU_ERROR_VALVETYPEBAD + 0x01 )

// "'ReturnValveMode' is mussing."
#define	TU_ERROR_RETURNVALVEMODEMISSING						( TU_ERROR_INSERTTYPEBAD + 0x01 )

// "'ReturnValveMode' must be 'Nothing', 'Other' or 'IMI'."
#define	TU_ERROR_RETURNVALVEMODEBAD							( TU_ERROR_RETURNVALVEMODEMISSING + 0x01 )

// "'HeadType' must be 'Thermostatic' or 'ElectroActuator'."
#define	TU_ERROR_HEADTYPEBAD								( TU_ERROR_RETURNVALVEMODEBAD + 0x01 )

// "Kv value for insert can't be below or equal to 0."
#define	TU_ERROR_KVINSERTINVALID							( TU_ERROR_HEADTYPEBAD + 0x01 )

// "Combo SV type ID must be defined."
#define	TU_ERROR_COMBOSVTYPEIDEMPTY							( TU_ERROR_KVINSERTINVALID + 0x01 )

// "Combo SV typed ID is defined but doesn't exist in the database."
#define	TU_ERROR_COMBOSVTYPEIDINVALID						( TU_ERROR_COMBOSVTYPEIDEMPTY + 0x01 )

// "Combo SV family ID must be defined."
#define	TU_ERROR_COMBOSVFAMIDEMPTY							( TU_ERROR_COMBOSVTYPEIDINVALID + 0x01 )

// "Combo SV family ID is defined but doesn't exist in the database."
#define	TU_ERROR_COMBOSVFAMIDINVALID						( TU_ERROR_COMBOSVFAMIDEMPTY + 0x01 )

// "Combo SV connect ID defined but doesn't exist in the database."
#define	TU_ERROR_COMBOSVCONIDINVALID						( TU_ERROR_COMBOSVFAMIDINVALID + 0x01 )

// "Combo SV version ID defined but doesn't exist in the database."
#define	TU_ERROR_COMBOSVVERSIDINVALID						( TU_ERROR_COMBOSVCONIDINVALID + 0x01 )

// "Combo RV family ID must be defined."
#define	TU_ERROR_COMBORVFAMIDEMPTY							( TU_ERROR_COMBOSVVERSIDINVALID + 0x01 )

// "Combo RV family ID defined but doesn't exist in the database."
#define	TU_ERROR_COMBORVFAMIDINVALID						( TU_ERROR_COMBORVFAMIDEMPTY + 0x01 )

// "Combo RV connect ID defined but doesn't exist in the database."
#define	TU_ERROR_COMBORVCONIDINVALID						( TU_ERROR_COMBORVFAMIDINVALID + 0x01 )

// "Combo RV version ID defined but doesn't exist in the database."
#define	TU_ERROR_COMBORVVERSIDINVALID						( TU_ERROR_COMBORVCONIDINVALID + 0x01 )

// "Combo SV insert name must be defined."
#define	TU_ERROR_COMBOSVINSNAMEEMPTY						( TU_ERROR_COMBORVVERSIDINVALID + 0x01 )

// "Combo SV insert name defined but invalid."
#define	TU_ERROR_COMBOSVINSNAMEINVALID						( TU_ERROR_COMBOSVINSNAMEEMPTY + 0x01 )

// "Combo SV insert connect ID must be defined."
#define	TU_ERROR_COMBOSVINSTYPEIDEMPTY						( TU_ERROR_COMBOSVINSNAMEINVALID + 0x01 )

// "Combo SV insert connect ID defined but doesn't exist in the database."
#define	TU_ERROR_COMBOSVINSTYPEIDINVALID					( TU_ERROR_COMBOSVINSTYPEIDEMPTY + 0x01 )

// "Combo SV insert version ID defined but doesn't exist in the database."
#define	TU_ERROR_COMBORVINSFAMIDINVALID						( TU_ERROR_COMBOSVINSTYPEIDINVALID + 0x01 )

// "'DpGroupChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_DPGROUPCHECKEDBAD							( TU_ERROR_COMBORVINSFAMIDINVALID + 0x01 )

// "Combo type below 65 ID must be defined."
#define TU_ERROR_TYPEB65IDEMPTY								( TU_ERROR_DPGROUPCHECKEDBAD + 0x01 )

// "Combo type below 65 ID is defined but doesn't exist in the database."
#define TU_ERROR_TYPEB65IDINVALID							( TU_ERROR_TYPEB65IDEMPTY + 0x01 )

// "Combo family below 65 ID must be defined."
#define TU_ERROR_FAMILYB65IDEMPTY							( TU_ERROR_TYPEB65IDINVALID + 0x01 )

// "Combo family below 65 ID is defined but doesn't exist in the database."
#define TU_ERROR_FAMILYB65IDINVALID							( TU_ERROR_FAMILYB65IDEMPTY + 0x01 )

// "Combo material below 65 ID must be defined."
#define TU_ERROR_MATERIALB65IDEMPTY							( TU_ERROR_FAMILYB65IDINVALID + 0x01 )

// "Combo material below 65 ID is defined but doesn't exist in the database."
#define TU_ERROR_MATERIALB65IDINVALID						( TU_ERROR_MATERIALB65IDEMPTY + 0x01 )

// "Combo connection below 65 ID must be defined."
#define TU_ERROR_CONNECTB65IDEMPTY							( TU_ERROR_MATERIALB65IDINVALID + 0x01 )

// "Combo connection below 65 ID is defined but doesn't exist in the database."
#define TU_ERROR_CONNECTB65IDINVALID						( TU_ERROR_CONNECTB65IDEMPTY + 0x01 )

// "Combo version below 65 ID must be defined."
#define TU_ERROR_VERSIONB65IDEMPTY							( TU_ERROR_CONNECTB65IDINVALID + 0x01 )

// "Combo version below 65 ID is defined but doesn't exist in the database."
#define TU_ERROR_VERSIONB65IDINVALID						( TU_ERROR_VERSIONB65IDEMPTY + 0x01 )

// "Combo PN below 65 ID must be defined."
#define TU_ERROR_PNB65IDEMPTY								( TU_ERROR_VERSIONB65IDINVALID + 0x01 )

// "Combo PN below 65 ID is defined but doesn't exist in the database."
#define TU_ERROR_PNB65IDINVALID								( TU_ERROR_PNB65IDEMPTY + 0x01 )

// "Combo type above 50 ID must be defined."
#define TU_ERROR_TYPEA50IDEMPTY								( TU_ERROR_PNB65IDINVALID + 0x01 )

// "Combo type above 50 ID is defined but doesn't exist in the database."
#define TU_ERROR_TYPEA50IDINVALID							( TU_ERROR_TYPEA50IDEMPTY + 0x01 )

// "Combo family above 50 ID must be defined."
#define TU_ERROR_FAMILYA50IDEMPTY							( TU_ERROR_TYPEA50IDINVALID + 0x01 )

// "Combo family above 50 ID is defined but doesn't exist in the database."
#define TU_ERROR_FAMILYA50IDINVALID							( TU_ERROR_FAMILYA50IDEMPTY + 0x01 )

// "Combo material above 50 ID must be defined."
#define TU_ERROR_MATERIALA50IDEMPTY							( TU_ERROR_FAMILYA50IDINVALID + 0x01 )

// "Combo material above 50 ID is defined but doesn't exist in the database."
#define TU_ERROR_MATERIALA50IDINVALID						( TU_ERROR_MATERIALA50IDEMPTY + 0x01 )

// "Combo connection above 50 ID must be defined."
#define TU_ERROR_CONNECTA50IDEMPTY							( TU_ERROR_MATERIALA50IDINVALID + 0x01 )

// "Combo connection above 50 ID is defined but doesn't exist in the database."
#define TU_ERROR_CONNECTA50IDINVALID						( TU_ERROR_CONNECTA50IDEMPTY + 0x01 )

// "Combo version above 50 ID must be defined."
#define TU_ERROR_VERSIONA50IDEMPTY							( TU_ERROR_CONNECTA50IDINVALID + 0x01 )

// "Combo version above 50 ID is defined but doesn't exist in the database."
#define TU_ERROR_VERSIONA50IDINVALID						( TU_ERROR_VERSIONA50IDEMPTY + 0x01 )

// "Combo PN above 50 ID must be defined."
#define TU_ERROR_PNA50IDEMPTY								( TU_ERROR_VERSIONA50IDINVALID + 0x01 )

// "Combo PN above 50 ID is defined but doesn't exist in the database."
#define TU_ERROR_PNA50IDINVALID								( TU_ERROR_PNA50IDEMPTY + 0x01 )

// "'CtrlTypeStrictChecked' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_CTRLTYPESTRICTCHECKBAD						( TU_ERROR_PNA50IDINVALID + 0x01 )

// "'ActuatorSelectedAsSet' must be 'True', '1', 'False' or '0'."
#define	TU_ERROR_ACTRSELASSETCHECKBAD						( TU_ERROR_CTRLTYPESTRICTCHECKBAD + 0x01 )

// "Actuator power supply ID must be defined."
#define TU_ERROR_ACTPOWERSUPPLYIDEMPTY						( TU_ERROR_ACTRSELASSETCHECKBAD + 0x01 )

// "Actuator input signal ID must be defined."
#define TU_ERROR_ACTINPUTSIGNALIDEMPTY						( TU_ERROR_ACTPOWERSUPPLYIDEMPTY + 0x01 )

// "Combo type below 65 ID for regulating valve must be defined."
#define TU_ERROR_BVTYPEB65IDEMPTY							( TU_ERROR_ACTINPUTSIGNALIDEMPTY + 0x01 )

// "Combo family below 65 ID for regulating valve must be defined."
#define TU_ERROR_BVFAMILYB65IDEMPTY							( TU_ERROR_BVTYPEB65IDEMPTY + 0x01 )

// "Combo material below 65 ID for regulating valve must be defined."
#define TU_ERROR_BVMATERIALB65IDEMPTY						( TU_ERROR_BVFAMILYB65IDEMPTY + 0x01 )

// "Combo connection below 65 ID for regulating valve must be defined."
#define TU_ERROR_BVCONNECTB65IDEMPTY						( TU_ERROR_BVMATERIALB65IDEMPTY + 0x01 )

// "Combo version below 65 ID for regulating valve must be defined."
#define TU_ERROR_BVVERSIONB65IDEMPTY						( TU_ERROR_BVCONNECTB65IDEMPTY + 0x01 )

// "Combo PN below 65 ID for regulating valve must be defined."
#define TU_ERROR_BVPNB65IDEMPTY								( TU_ERROR_BVVERSIONB65IDEMPTY + 0x01 )

// "Combo type above 50 ID for regulating valve must be defined."
#define TU_ERROR_BVTYPEA50IDEMPTY							( TU_ERROR_BVPNB65IDEMPTY + 0x01 )

// "Combo family above 50 ID for regulating valve must be defined."
#define TU_ERROR_BVFAMILYA50IDEMPTY							( TU_ERROR_BVTYPEA50IDEMPTY + 0x01 )

// "Combo material above 50 ID for regulating valve must be defined."
#define TU_ERROR_BVMATERIALA50IDEMPTY						( TU_ERROR_BVFAMILYA50IDEMPTY + 0x01 )

// "Combo connection above 50 ID for regulating valve must be defined."
#define TU_ERROR_BVCONNECTA50IDEMPTY						( TU_ERROR_BVMATERIALA50IDEMPTY + 0x01 )

// "Combo version above 50 ID for regulating valve must be defined."
#define TU_ERROR_BVVERSIONA50IDEMPTY						( TU_ERROR_BVCONNECTA50IDEMPTY + 0x01 )

// "Combo PN above 50 ID for regulating valve must be defined."
#define TU_ERROR_BVPNA50IDEMPTY								( TU_ERROR_BVVERSIONA50IDEMPTY + 0x01 )

// "Combo type below 65 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVTYPEB65IDINVALID							( TU_ERROR_BVPNA50IDEMPTY + 0x01 )

// "Combo family below 65 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVFAMILYB65IDINVALID						( TU_ERROR_BVTYPEB65IDINVALID + 0x01 )

// "Combo material below 65 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVMATERIALB65IDINVALID						( TU_ERROR_BVFAMILYB65IDINVALID + 0x01 )

// "Combo connection below 65 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVCONNECTB65IDINVALID						( TU_ERROR_BVMATERIALB65IDINVALID + 0x01 )

// "Combo version below 65 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVVERSIONB65IDINVALID						( TU_ERROR_BVCONNECTB65IDINVALID + 0x01 )

// "Combo PN below 65 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVPNB65IDINVALID							( TU_ERROR_BVVERSIONB65IDINVALID + 0x01 )

// "Combo type above 50 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVTYPEA50IDINVALID							( TU_ERROR_BVPNB65IDINVALID + 0x01 )

// "Combo family above 50 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVFAMILYA50IDINVALID						( TU_ERROR_BVTYPEA50IDINVALID + 0x01 )

// "Combo material above 50 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVMATERIALA50IDINVALID						( TU_ERROR_BVFAMILYA50IDINVALID + 0x01 )

// "Combo connection above 50 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVCONNECTA50IDINVALID						( TU_ERROR_BVMATERIALA50IDINVALID + 0x01 )

// "Combo version above 50 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVVERSIONA50IDINVALID						( TU_ERROR_BVCONNECTA50IDINVALID + 0x01 )

// "Combo PN above 50 ID is defined for regulating valve but doesn't exist in the database."
#define TU_ERROR_BVPNA50IDINVALID							( TU_ERROR_BVVERSIONA50IDINVALID + 0x01 )

// "Combo type ID must be defined."
#define TU_ERROR_TYPEIDEMPTY								( TU_ERROR_BVPNA50IDINVALID + 0x01 )

// "Combo family ID must be defined."
#define TU_ERROR_FAMILYIDEMPTY								( TU_ERROR_TYPEIDEMPTY + 0x01 )

// "Combo connect ID must be defined."
#define TU_ERROR_CONNECTIDEMPTY								( TU_ERROR_FAMILYIDEMPTY + 0x01 )

// "Combo version ID must be defined."
#define TU_ERROR_VERSIONIDEMPTY								( TU_ERROR_CONNECTIDEMPTY + 0x01 )

// "'Test number' must be set."
#define	TU_ERROR_CANTREADTESTNUMBER							( TU_ERROR_VERSIONIDEMPTY + 0x01 )

// "Can't read 'Test number' in the current drop out file."
#define TU_ERROR_CANTFINDTESTNUMBER							( TU_ERROR_CANTREADTESTNUMBER + 0x01 )

// "System application type must be 'Heating, 'Solar' or 'Cooling.'"
#define TU_ERROR_SYSTEMAPPLICATIONTYPEBAD					( TU_ERROR_CANTFINDTESTNUMBER + 0x01 )

// "System heat generator type must be defined."
#define TU_ERROR_SYSTEMHEATGENERATORTYPEIDEMPTY				( TU_ERROR_SYSTEMAPPLICATIONTYPEBAD + 0x01 )

// "Norm ID must be defined."
#define TU_ERROR_NORMIDEMPTY								( TU_ERROR_SYSTEMHEATGENERATORTYPEIDEMPTY + 0x01 )

// "Installed power must be defined."
#define TU_ERROR_INSTALLEDPOWEREMPTY						( TU_ERROR_NORMIDEMPTY + 0x01 )

// "Installed collector must be defined."
#define TU_ERROR_INSTALLEDCOLLECTOREMPTY					( TU_ERROR_INSTALLEDPOWEREMPTY + 0x01 )

// "User set pressure choice must be defined."
#define TU_ERROR_USERSETPRESSURECHOICEEMPTY					( TU_ERROR_INSTALLEDCOLLECTOREMPTY + 0x01 )

// "System heat generator type ID is defined but doesn't exist in the database."
#define TU_ERROR_HEATGENERATORTYPEIDINVALID					( TU_ERROR_USERSETPRESSURECHOICEEMPTY + 0x01 )

// "Norm ID is defined but doesn't exist in the database."
#define TU_ERROR_NORMIDINVALID								( TU_ERROR_HEATGENERATORTYPEIDINVALID + 0x01 )

// "Installed power value can't be below or equal to 0."
#define TU_ERROR_INSTALLEDPOWERNOVALID						( TU_ERROR_NORMIDINVALID + 0x01 )

// "Installed collector value can't be below or equal to 0."
#define TU_ERROR_INSTALLEDCOLLECTORNOVALID					( TU_ERROR_INSTALLEDPOWERNOVALID + 0x01 )

// "Combo family ID is defined for safety valve but doesn't exist in the database."
#define TU_ERROR_SAFETYVALVEFAMILYIDINVALID					( TU_ERROR_INSTALLEDCOLLECTORNOVALID + 0x01 )

// "Combo connection ID is defined for safety valve but doesn't exist in the database."
#define TU_ERROR_SAFETYVALVECONNECTIDINVALID				( TU_ERROR_SAFETYVALVEFAMILYIDINVALID + 0x01 )

// "User set pressure choice value can't be below or equal to 0."
#define TU_ERROR_USERSETPRESSURENOVALID						( TU_ERROR_SAFETYVALVECONNECTIDINVALID + 0x01 )
#define TU_ERROR_FASTELECTCONNECTIONNOTVALID				( TU_ERROR_USERSETPRESSURENOVALID + 0x01 )
#define TU_ERROR_INVALIDSIXWAYVALVESELECTIONMODE			( TU_ERROR_FASTELECTCONNECTIONNOTVALID + 0x01 )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for TU product selection file helper errors
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUProdSelFileHelper::OpenFile' method. //////////////////////////////////////////////////////////////
#define TU_ERROR_FILEHELPER_OPENFILE_FILENAMEEMPTY										( TU_ERROR_WHERE_FILEHELPER_OPENFILE + TU_ERROR_FILENAMEEMPTY )
#define TU_ERROR_FILEHELPER_OPENFILE_MODEEMPTY											( TU_ERROR_WHERE_FILEHELPER_OPENFILE + TU_ERROR_OPENMODEMUSTBESPECIFIED )
#define TU_ERROR_FILEHELPER_OPENFILE_CANTOPEN											( TU_ERROR_WHERE_FILEHELPER_OPENFILE + TU_ERROR_CANTOPENFILE )

// For 'CTUProdSelFileHelper::ReadOneLine' and 'CTUProdSelFileHelper::ReadOneLineKeyValue'method. ////////////
#define	TU_ERROR_FILEHELPER_READLINE_FILENOTOPENED										( TU_ERROR_WHERE_FILEHELPER_READLINE + TU_ERROR_FILENOTOPENED )
#define	TU_ERROR_FILEHELPER_READLINE_UNEXPECTEDEOF										( TU_ERROR_WHERE_FILEHELPER_READLINE + TU_ERROR_UNEXPECTEDEOF )
#define	TU_ERROR_FILEHELPER_READLINE_ERRORINFILE										( TU_ERROR_WHERE_FILEHELPER_READLINE + TU_ERROR_ERRORINFILE )
#define	TU_ERROR_FILEHELPER_READLINE_BADTOKEN											( TU_ERROR_WHERE_FILEHELPER_READLINE + TU_ERROR_BADTOKENINFILE )

// For 'CTUProdSelFileHelper::WriteOneLine', 'CTUProdSelFileHelper::WriteOneLineArgText' and
// 'CTUProdSelFileHelper::WriteOneLineArgFloat' method. //////////////////////////////////////////////////////
#define	TU_ERROR_FILEHELPER_WRITELINE_FILENOTOPENED										( TU_ERROR_WHERE_FILEHELPER_WRITELINE + TU_ERROR_FILENOTOPENED )
#define	TU_ERROR_FILEHELPER_WRITELINE_WRITEERROR										( TU_ERROR_WHERE_FILEHELPER_WRITELINE + TU_ERROR_WRITEERROR )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for TU product selection launcher errors
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUProdSelLauncher::Execute' method. /////////////////////////////////////////////////////////////////
#define	TU_ERROR_LAUNCHER_EXECUTE_TADBNOTDEFINED										( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_TADBNOTDEFINED )
#define	TU_ERROR_LAUNCHER_EXECUTE_TADSNOTDEFINED										( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_TADSNOTDEFINED )
#define TU_ERROR_LAUNCHER_EXECUTE_CANTCREATETECHPARAMBACKUP								( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_CANTCREATETECHPARAMBACKUP )
#define TU_ERROR_LAUNCHER_EXECUTE_ERRORWITHTECHPARAMBACKUP								( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_ERRORWITHTECHPARAMBACKUP )
#define TU_ERROR_LAUNCHER_EXECUTE_ERRORWITHWATERCHARBACKUP								( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_ERRORWITHWATERCHARBACKUP )
#define	TU_ERROR_LAUNCHER_EXECUTE_TECHPARAMSNOTDEFINED									( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_TECHPARAMSNOTDEFINED )
#define	TU_ERROR_LAUNCHER_EXECUTE_TECHPARAMSBADSTART									( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_TECHPARAMSBADSTART )
#define	TU_ERROR_LAUNCHER_EXECUTE_CANTREADTESTNUMBER									( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_CANTREADTESTNUMBER )
#define	TU_ERROR_LAUNCHER_EXECUTE_BADPRODUCT											( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_BADPRODUCT )
#define	TU_ERROR_LAUNCHER_EXECUTE_CTUPRODSELERROR										( TU_ERROR_WHERE_LAUNCHER_EXECUTE + TU_ERROR_CTUPRODSELERROR )

// For 'CTUProdSelLauncher::DropOutSelection' method. ////////////////////////////////////////////////////////
#define	TU_ERROR_LAUNCHER_DROPOUTESEL_PROSELPARAMSNULL									( TU_ERROR_WHERE_LAUNCHER_DROPOUTSELECTION + TU_ERROR_PROSELPARAMSNULL )
#define	TU_ERROR_LAUNCHER_DROPOUTESEL_PROSELPARAMSINVALID								( TU_ERROR_WHERE_LAUNCHER_DROPOUTSELECTION + TU_ERROR_PROSELPARAMSINVALID )
#define	TU_ERROR_LAUNCHER_DROPOUTESEL_TADSNOTDEFINED									( TU_ERROR_WHERE_LAUNCHER_DROPOUTSELECTION + TU_ERROR_PRODSELTADSNOTDEFINED )
#define	TU_ERROR_LAUNCHER_DROPOUTESEL_TECHPARAMSNOTDEFINED								( TU_ERROR_WHERE_LAUNCHER_DROPOUTSELECTION + TU_ERROR_PRODSELTECHPARAMSNOTDEFINED )
#define TU_ERROR_LAUNCHER_DROPOUTESEL_CANTCREATEPRODSELHELPER							( TU_ERROR_WHERE_LAUNCHER_DROPOUTSELECTION + TU_ERROR_CANTCREATEPRODSELHELPER )
#define TU_ERROR_LAUNCHER_DROPOUTESEL_CANTFINDTESTNUMBER								( TU_ERROR_WHERE_LAUNCHER_DROPOUTSELECTION + TU_ERROR_CANTFINDTESTNUMBER )

// For 'CTUProdSelLauncher::LoadTestAndDropResults' method. //////////////////////////////////////////////////
#define	TU_ERROR_LAUNCHER_LOADTESTDROPRESULTS_TADBNOTDEFINED							( TU_ERROR_WHERE_LAUNCHER_LOADTESTDROPRESULTS + TU_ERROR_TADBNOTDEFINED )
#define	TU_ERROR_LAUNCHER_LOADTESTDROPRESULTS_TADSNOTDEFINED							( TU_ERROR_WHERE_LAUNCHER_LOADTESTDROPRESULTS + TU_ERROR_TADSNOTDEFINED )



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for product selection errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUProdSelHelper::ReadInputs' method. ////////////////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_READINPUTS_PRODSEL_BASE											( TU_ERROR_WHERE_HELPER_READINPUTS + TU_ERROR_SELECTIONTYPE_PRODUCT )

#define	TU_ERROR_HELPER_READINPUTS_PRODSEL_BADSTART										( TU_ERROR_HELPER_READINPUTS_PRODSEL_BASE + TU_ERROR_INPUTBADSTART )
#define	TU_ERROR_HELPER_READINPUTS_PRODSEL_BADINPUT										( TU_ERROR_HELPER_READINPUTS_PRODSEL_BASE + TU_ERROR_INPUTBADPAIR )
#define	TU_ERROR_HELPER_READINPUTS_PRODSEL_BADEND										( TU_ERROR_HELPER_READINPUTS_PRODSEL_BASE + TU_ERROR_INPUTBADEND )

// For 'CTUProdSelHelper::VerifyInputs' method. //////////////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE										( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_PRODUCT )

#define TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDITFAMIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE + TU_ERROR_WATERADDITFAMIDINVALID)
#define TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDITIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE + TU_ERROR_WATERADDITIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERTEMPTOOLOW							( TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE + TU_ERROR_WATERTEMPTOOLOW )
#define TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERTEMPTOOHIGH							( TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE + TU_ERROR_WATERTEMPTOOHIGH )
#define TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_WATERADDTOOHIGH							( TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE + TU_ERROR_WATERADDTOOHIGH )
#define TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_PIPESERIESIDEMPTY							( TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE + TU_ERROR_PIPESERIESIDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_PIPESERIESIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE + TU_ERROR_PIPESERIESIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_PIPEIDINVALID								( TU_ERROR_HELPER_VERIFYINPUTS_PRODSEL_BASE + TU_ERROR_PIPEIDINVALID )

// For 'CTUProdSelHelper::ReadOutputs' method. ///////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BASE										( TU_ERROR_WHERE_HELPER_READOUTPUTS + TU_ERROR_SELECTIONTYPE_PRODUCT )

#define TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BADSTART									( TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BASE + TU_ERROR_OUTPUTBADSTART )
#define TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BADNBRINOUTPUT								( TU_ERROR_HELPER_READOUTPUTS_PRODSEL_BASE + TU_ERROR_OUTPUTBADNBRINOUTPUT )

// For 'CTUProdSelHelper::DropOutSelection' method. ////////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_DROPOUTSEL_PRODSEL_BASE											( TU_ERROR_WHERE_HELPER_DROPOUTSELECTION + TU_ERROR_SELECTIONTYPE_PRODUCT )

#define TU_ERROR_HELPER_DROPOUTSEL_PRODSEL_FILEPOINTERNULL								( TU_ERROR_HELPER_DROPOUTSEL_PRODSEL_BASE + TU_ERROR_FILEPOINTERNULL )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelHelper::InterpretInputs' method. ////////////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_BASE									( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL )

#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_FLOWORPOWERDTBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_BASE + TU_ERROR_FLOWORPOWERDTBAD )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_DPENABLEBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_BASE + TU_ERROR_DPENABLEBAD )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_ONLYFORSETBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELBASE_BASE + TU_ERROR_ONLYFORSETBAD )

// For 'CTUIndSelHelper::VerifyInputs' method. ///////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE									( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_FLOWNOTVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_FLOWNOTVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_POWERNOTVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_POWERNOTVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_DTNOTVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_DTNOTVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_CANTCOMPUTEFLOW							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_CANTCOMPUTEFLOW )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_DPNOTVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_DPNOTVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_TYPEIDINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_TYPEIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_FAMILYIDINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_FAMILYIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_MATERIALIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_MATERIALIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_CONNECTIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_CONNECTIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_VERSIONIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_VERSIONIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_PNIDINVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBASE_BASE + TU_ERROR_PNIDINVALID )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for regulating valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelBVHelper::VerifyInputs' method. /////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBV_BASE										( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_BV )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELBV_TYPEIDMISSING								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELBV_BASE + TU_ERROR_TYPEIDMISSING )

// For 'CTUIndSelBVHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_BV )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_NOINPUT										( TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_RESULTNBRNOTSAME							( TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BADEXTRACT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_RESULTDIFF									( TU_ERROR_HELPER_LAUNCHTEST_INDSELBV_BASE + TU_ERROR_RESULTDIFFERENCE )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for base class for control valves, balancing and control valves, 
// pressure independent balancing and control valves and combined Dp controller, balancing and control valves
// errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelCtrlBaseHelper::InterpretInputs' method. ////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_BASE								( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_CTRLBASE )

#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CV2W3WMISSING					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_BASE + TU_ERROR_CV2W3WMISSING )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CV2W3WBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_BASE + TU_ERROR_CV2W3WBAD )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CVCTRLTYPEMISSING				( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_BASE + TU_ERROR_CVCTRLTYPEMISSING )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_CVCTRLTYPEBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_BASE + TU_ERROR_CVCTRLTYPEBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_ACTUATORFAILSAFEFCTBAD			( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCTRLBASE_BASE + TU_ERROR_ACTUATORFAILSAFEFCTBAD )

// For 'CTUIndSelCtrlBaseHelper::VerifyInputs' method. ///////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELCTRLBASE_BASE								( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_CTRLBASE )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELCTRLBASE_ACTPOWERSUPPLYIDINVALID				( TU_ERROR_HELPER_VERIFYINPUTS_INDSELCTRLBASE_BASE + TU_ERROR_ACTPOWERSUPPLYIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELCTRLBASE_ACTINPUTSIGNALIDINVALID				( TU_ERROR_HELPER_VERIFYINPUTS_INDSELCTRLBASE_BASE + TU_ERROR_ACTINPUTSIGNALIDINVALID )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for balancing and control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelBCVHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_BCV )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_RESULTNBRNOTSAME							( TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_BADEXTRACT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_RESULTDIFF									( TU_ERROR_HELPER_LAUNCHTEST_INDSELBCV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for pure control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelCVHelper::InterpretInputs' method. //////////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCV_BASE									( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_CV )

#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCV_GROUPKVSORDPBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCV_BASE + TU_ERROR_GROUPKVSORDPBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCV_KVSORDPBAD								( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELCV_BASE + TU_ERROR_KVSORDPBAD)

// For 'CTUIndSelCVHelper::VerifyInputs' method. /////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_BASE										( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_CV )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_KVSNOTVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_BASE + TU_ERROR_KVSNOTVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_DPNOTVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_BASE + TU_ERROR_DPNOTVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_TYPEIDMISSING								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELCV_BASE + TU_ERROR_TYPEIDMISSING )

// For 'CTUIndSelCVHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_CV )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_NOINPUT										( TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_RESULTNBRNOTSAME							( TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_BADEXTRACT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_RESULTDIFF									( TU_ERROR_HELPER_LAUNCHTEST_INDSELCV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for Dp controller valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelDpCHelper::InterpretInputs' method. /////////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_BASE									( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_DPC )

#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_GROUPDPBRANCHORKVBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_BASE + TU_ERROR_GROUPDPBRANCHORKVBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_DPMAXCHECKEDBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_BASE + TU_ERROR_DPMAXCHECKEDBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_DPSTABBAD								( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_BASE + TU_ERROR_DPSTABBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_DPCLOCBAD								( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_BASE + TU_ERROR_DPCLOCBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_MVLOCBAD								( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPC_BASE + TU_ERROR_MVLOCBAD )

// For 'CTUIndSelDpCHelper::VerifyInputs' method. ////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_BASE										( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_DPC )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_DPBRANCHINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_BASE + TU_ERROR_DPBRANCHINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_KVINVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_BASE + TU_ERROR_KVINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_DPMAXINVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPC_BASE + TU_ERROR_DPMAXINVALID )

// For 'CTUIndSelDpCHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_DPC )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_RESULTNBRNOTSAME							( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_BADEXTRACT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_RESULTDIFF									( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPC_BASE + TU_ERROR_RESULTDIFFERENCE )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for combined Dp controller, balancing and control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelDpCBCVHelper::InterpretInputs' method. //////////////////////////////////////////////////////
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPCBCV_BASE								( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_DPCBCV )

#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPCBCV_GROUPDPLBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPCBCV_BASE + TU_ERROR_GROUPDPLBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPCBCV_WITHSTSBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELDPCBCV_BASE + TU_ERROR_WITHSTSBAD )

// For 'CTUIndSelDpCBCVHelper::VerifyInputs' method. /////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPCBCV_BASE									( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_DPCBCV )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPCBCV_DPLINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPCBCV_BASE + TU_ERROR_DPLINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPCBCV_TYPEIDMISSING							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELDPCBCV_BASE + TU_ERROR_TYPEIDMISSING )

// For 'CTUIndSelDpCHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_BASE									( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_DPCBCV )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_CANTCREATESELECTLIST					( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_RESULTNBRNOTSAME						( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_BADEXTRACT								( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_RESULTDIFF								( TU_ERROR_HELPER_LAUNCHTEST_INDSELDPCBCV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for pressure independent balancing and control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelPIBCVHelper::InterpretInputs' method. ///////////////////////////////////////////////////////
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPIBCV_BASE								( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_PIBCV )

#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPIBCV_DPMAXCHECKEDBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPIBCV_BASE + TU_ERROR_DPMAXCHECKEDBAD )

// For 'CTUIndSelPIBCVHelper::VerifyInputs' method. //////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_BASE									( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_PIBCV )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_DPMAXINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_BASE + TU_ERROR_DPLINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_TYPEIDMISSING							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPIBCV_BASE + TU_ERROR_TYPEIDMISSING )

// For 'CTUIndSelPIBCVHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_PIBCV )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_RESULTNBRNOTSAME							( TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BADEXTRACT								( TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_RESULTDIFF								( TU_ERROR_HELPER_LAUNCHTEST_INDSELPIBCV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for pressure maintenance errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelPMHelper::InterpretInputs' method. //////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE									( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_PM )

#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_APPTYPEBAD								( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_APPTYPEBAD )		
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_DEGASSINGCHECKEDBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_DEGASSINGCHECKEDBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_SYSVOLMISSING							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_SYSVOLMISSING )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_PZMISSING								( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_PZMISSING )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_PSVSMISSING							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_PSVSMISSING )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_SFTYTEMPLIMITERMISSING					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_SFTYTEMPLIMITERMISSING )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_SUPPLYTEMPMISSING						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_SUPPLYTEMPMISSING )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_RETURNTEMPMISSING						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_RETURNTEMPMISSING )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_MINTEMPCHECKEDBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_MINTEMPCHECKEDBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_PRESSONBAD								( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_PRESSONBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_GENCOMBINONEDEVICEBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_GENCOMBINONEDEVICEBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_GENREDPUMPCOMPBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_GENREDPUMPCOMPBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_GENREDTECBOXBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_GENREDTECBOXBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_COMPINTCOATBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_COMPINTCOATBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_COMPEXTAIRBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_COMPEXTAIRBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_PUMPDEGCOOLVERSBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_PUMPDEGCOOLVERSBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_WMUPBREAKTANKBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_WMUPBREAKTANKBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_WMUPSTDBYBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELPM_BASE + TU_ERROR_WMUPSTDBYBAD )

// For 'CTUIndSelPMHelper::VerifyInputs' method. /////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE										( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_PM )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_SYSVOLINVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_SYSVOLINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_SOLCONTINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_SOLCONTINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_PZINVALID									( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_PZINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_MAXTEMPINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_MAXTEMPINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_MINTEMPINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_MINTEMPINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_FILLTEMPINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_FILLTEMPINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_PUMPHEADINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_PUMPHEADINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_NPSHINVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_NPSHINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_DEGMAXTEMPATCONPTINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_DEGMAXTEMPATCONPTINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_DEGPRESSCONPTINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_DEGPRESSCONPTINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_WMUPNETPNINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_WMUPNETPNINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_WMUPTEMPINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_WMUPTEMPINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_WMUPHARDINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_INDSELPM_BASE + TU_ERROR_WMUPHARDINVALID )

// For 'CTUIndSelPMHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_PM )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_NOINPUT										( TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_BADEXTRACT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_RESULTDIFF									( TU_ERROR_HELPER_LAUNCHTEST_INDSELPM_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for separators and air vents errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelSeparatorHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_BASE									( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_SEPARATOR )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_NOINPUT								( TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_CANTCREATESELECTLIST					( TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_RESULTNBRNOTSAME						( TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_BADEXTRACT							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_RESULTDIFF							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSEPARATOR_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for shutoff valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelSVHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_SV )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_NOINPUT										( TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_RESULTNBRNOTSAME							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_BADEXTRACT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_RESULTDIFF									( TU_ERROR_HELPER_LAUNCHTEST_INDSELSV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for thermostatic regulative valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelTRVHelper::InterpretInputs' method. /////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_BASE									( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_TRV )

#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_ISTRVTYPEPRESETBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_BASE + TU_ERROR_ISTRVTYPEPRESETBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_VALVETYPEMISSING						( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_BASE + TU_ERROR_VALVETYPEMISSING )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_VALVETYPEBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_BASE + TU_ERROR_VALVETYPEBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_INSERTTYPEBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_BASE + TU_ERROR_INSERTTYPEBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_RETURNVALVEMODEMISSING				( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_BASE + TU_ERROR_RETURNVALVEMODEMISSING )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_RETURNVALVEMODEBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_BASE + TU_ERROR_RETURNVALVEMODEBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_HEADTYPEBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELTRV_BASE + TU_ERROR_HEADTYPEBAD )

// For 'CTUIndSelTRVHelper::VerifyInputs' method. ////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE										( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_TRV )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_KVINVALID								( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_KVINSERTINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVTYPEIDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVTYPEIDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVTYPEIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVTYPEIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVFAMIDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVFAMIDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVFAMIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVFAMIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVCONIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVCONIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVVERSIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVVERSIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVFAMIDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBORVFAMIDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVFAMIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBORVFAMIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVCONIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBORVCONIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVVERSIDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBORVVERSIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVINSNAMEEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVINSNAMEEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVINSNAMEINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVINSNAMEINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVINSTYPEIDEMPTY					( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVINSTYPEIDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBOSVINSTYPEIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBOSVINSTYPEIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_COMBORVINSFAMIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_COMBORVINSFAMIDINVALID )

// For 'CTUIndSelTRVHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_TRV )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_RESULTNBRNOTSAME							( TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BADEXTRACT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_RESULTDIFF									( TU_ERROR_HELPER_LAUNCHTEST_INDSELTRV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for TA-6-way valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSel6WayValveHelper::InterpretInputs' method. /////////////////////////////////////////////////////////

// For 'CTUIndSel6WayValveHelper::VerifyInputs' method. ////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSIXWAYVALVE_BASE										( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_SIXWAYVALVE )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSIXWAYVALVE_FASTELECCONNNOTVALID					    ( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSIXWAYVALVE_BASE + TU_ERROR_FASTELECTCONNECTIONNOTVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSIXWAYVALVE_INVALIDSELECTIONMODE						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELTRV_BASE + TU_ERROR_INVALIDSIXWAYVALVESELECTIONMODE )

// For 'CTUIndSel6WayValveHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_SIXWAYVALVE )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_RESULTNBRNOTSAME							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_BADEXTRACT									( TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_RESULTDIFF									( TU_ERROR_HELPER_LAUNCHTEST_INDSELSIXWAYVALVE_BASE + TU_ERROR_RESULTDIFFERENCE )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for class for safety valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelSafetyValveHelper::InterpretInputs' method. ///////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_BASE							( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_SAFETYVALVE )

#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_SYSTEMAPPTYPEBAD				( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_SYSTEMAPPLICATIONTYPEBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_HEATGENTYPEIDEMPTY			( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_SYSTEMHEATGENERATORTYPEIDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_NORMIDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_NORMIDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_INSTALLEDPOWEREMPTY			( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_INSTALLEDPOWEREMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_INSTALLEDCOLLECTOREMPTY		( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_INSTALLEDCOLLECTOREMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_USERSETPRESSURECHOICEEMPTY    ( TU_ERROR_HELPER_INTERPRETINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_USERSETPRESSURECHOICEEMPTY )

// For 'CTUIndSelSafetyValveHelper::VerifyInputs' method. /////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_BASE								( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_HEATGENERATORTYPEIDINVALID		( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_HEATGENERATORTYPEIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_NORMIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_NORMIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_INSTALLEDPOWERNOVALID			( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_INSTALLEDPOWERNOVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_INSTALLEDCOLLECTORNOVALID		( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_INSTALLEDCOLLECTORNOVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_FAMILYIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_SAFETYVALVEFAMILYIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_CONNECTIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_SAFETYVALVECONNECTIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_USERSETPRESSURENOVALID			( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSAFETYVALVE_BASE + TU_ERROR_USERSETPRESSURENOVALID )

// For 'CTUIndSelSafetyValveHelper::LaunchTest' method. //////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_BASE								( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_SAFETYVALVE )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_NOINPUT							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_CANTCREATESELECTLIST				( TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_RESULTNBRNOTSAME					( TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_BADEXTRACT							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_RESULTDIFF							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSAFETYVALVE_BASE + TU_ERROR_RESULTDIFFERENCE )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for individual selection for smart control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUIndSelSmartControlValveHelper::VerifyInputs' method. /////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSMARTCONTROLVALVE_BASE								( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_SMARTCONTROLVALVE )

#define TU_ERROR_HELPER_VERIFYINPUTS_INDSELSMARTCONTROLVALVE_TYPEIDMISSING						( TU_ERROR_HELPER_VERIFYINPUTS_INDSELSMARTCONTROLVALVE_BASE + TU_ERROR_TYPEIDMISSING )

// For 'CTUIndSelSmartControlValveHelper::LaunchTest' method. ///////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_BASE								( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_INDIVIDUAL + TU_ERROR_PRODUCTTYPE_SMARTCONTROLVALVE )

#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_NOINPUT								( TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_CANTCREATESELECTLIST				( TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_RESULTNBRNOTSAME					( TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_BASE + TU_ERROR_RESULTNBRNOTSAME )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_BADEXTRACT							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_RESULTDIFF							( TU_ERROR_HELPER_LAUNCHTEST_INDSELSMARTCONTROLVALVE_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for batch selection errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUBatchSelHelper::InterpretInputs' method. //////////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELBASE_BASE								( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_BATCH )

#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELBASE_FLOWORPOWERDTBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELBASE_BASE + TU_ERROR_FLOWORPOWERDTBAD )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELBASE_DPGROUPCHECKEDBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELBASE_BASE + TU_ERROR_DPGROUPCHECKEDBAD )

// For 'CTUBatchSelHelper::VerifyInputs' method. /////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE									( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_BATCH )

#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_TYPEB65IDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_TYPEB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_TYPEB65IDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_TYPEB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_FAMILYB65IDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_FAMILYB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_FAMILYB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_FAMILYB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_MATERIALB65IDEMPTY					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_MATERIALB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_MATERIALB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_MATERIALB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_CONNECTB65IDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_CONNECTB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_CONNECTB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_CONNECTB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONB65IDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_VERSIONB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_VERSIONB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNB65IDEMPTY							( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_PNB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNB65IDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_PNB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_TYPEA50IDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_TYPEA50IDEMPTY)
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_TYPEA50IDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_TYPEA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_FAMILYA50IDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_FAMILYA50IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_FAMILYA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_FAMILYA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_MATERIALA50IDEMPTY					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_MATERIALA50IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_MATERIALA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_MATERIALA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_CONNECTA50IDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_CONNECTA50IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_CONNECTA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_CONNECTA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONA50IDEMPTY						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_VERSIONA50IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_VERSIONA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_VERSIONA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNA50IDEMPTY							( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_PNA50IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_PNA50IDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELBASE_BASE + TU_ERROR_PNA50IDINVALID )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for batch selection for class for balancing valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUBatchSelBVHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_BV )

#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_NOOUTPUT									( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BASE + TU_ERROR_NOOUTPUT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_CANTCREATEROWPARAMS						( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BASE + TU_ERROR_CANTCREATEROWPARAMS )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BADEXTRACT								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_RESULTDIFF								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for batch selection for base class for balancing and control valves and pressure independent 
// balancing and control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUBatchSelCtrlBaseHelper::InterpretInputs' method. //////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_BASE							( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_CTRLBASE )

#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CTRLTYPESTRICTCHECKBAD			( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELBASE_BASE + TU_ERROR_CTRLTYPESTRICTCHECKBAD )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CV2W3WMISSING					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_CV2W3WMISSING )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CV2W3WBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_CV2W3WBAD )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CVCTRLTYPEMISSING				( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_CVCTRLTYPEMISSING )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_CVCTRLTYPEBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_CVCTRLTYPEBAD )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_ACTRSELASSETCHECKBAD			( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_ACTRSELASSETCHECKBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_ACTUATORFAILSAFEFCTBAD			( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_ACTUATORFAILSAFEFCTBAD )

// For 'CTUBatchSelCtrlBaseHelper::VerifyInputs' method. /////////////////////////////////////////////////////

#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_BASE								( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_CTRLBASE )

#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_ACTPOWERSUPPLYIDEMPTY				( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_ACTPOWERSUPPLYIDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_ACTPOWERSUPPLYIDINVALID			( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_ACTPOWERSUPPLYIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_ACTINPUTSIGNALIDEMPTY				( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_ACTINPUTSIGNALIDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_ACTINPUTSIGNALIDINVALID			( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELCTRLBASE_BASE + TU_ERROR_ACTINPUTSIGNALIDINVALID )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for batch selection for class for balancing and control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUBatchSelBCVHelper::LaunchTest' method. ////////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_BCV )

#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_NOOUTPUT									( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BASE + TU_ERROR_NOOUTPUT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_CANTCREATEROWPARAMS						( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BASE + TU_ERROR_CANTCREATEROWPARAMS )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BADEXTRACT								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_RESULTDIFF								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELBCV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for batch selection for class for Dp controller valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUBatchSelDpCHelper::InterpretInputs' method. ///////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE								( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_DPC )

#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_GROUPDPBRANCHBAD					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_GROUPDPBRANCHBAD )
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_GROUPKVSBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_GROUPKVSBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_DPMAXCHECKEDBAD						( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_DPMAXCHECKEDBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_DPSTABBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_DPSTABBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_DPCLOCBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_DPCLOCBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_MVLOCBAD							( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_MVLOCBAD )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVTYPEB65IDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVTYPEB65IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVFAMILYB65IDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVFAMILYB65IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVMATERIALB65IDEMPTY				( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVMATERIALB65IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVCONNECTB65IDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVCONNECTB65IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVVERSIONB65IDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVVERSIONB65IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVPNB65IDEMPTY						( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVPNB65IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVTYPEA50IDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVTYPEA50IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVFAMILYA50IDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVFAMILYA50IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVMATERIALA50IDEMPTY				( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVMATERIALA50IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVCONNECTA50IDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVCONNECTA50IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVVERSIONA50IDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVVERSIONA50IDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BVPNA50IDEMPTY						( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVPNA50IDEMPTY )

// For 'CTUBatchSelDpCHelper::VerifyInputs' method. //////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE									( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_DPC )

#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_DPMAXINVALID							( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_DPMAXINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVTYPEB65IDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVTYPEB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVFAMILYB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVFAMILYB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVMATERIALB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVMATERIALB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVCONNECTB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVCONNECTB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVVERSIONB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVVERSIONB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVPNB65IDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVPNB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVTYPEA50IDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVTYPEA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVFAMILYA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVFAMILYA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVMATERIALA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVMATERIALA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVCONNECTA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVCONNECTA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVVERSIONA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVVERSIONA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BVPNA50IDINVALID						( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELDPC_BASE + TU_ERROR_BVPNA50IDINVALID )

// For 'CTUBatchSelDpCHelper::LaunchTest' method. ////////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BASE										( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_DPC )

#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_NOINPUT									( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_NOOUTPUT									( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BASE + TU_ERROR_NOOUTPUT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_CANTCREATESELECTLIST						( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_CANTCREATEROWPARAMS						( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BASE + TU_ERROR_CANTCREATEROWPARAMS )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BADEXTRACT								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_RESULTDIFF								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELDPC_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for batch selection for class for pressure independent balancing and control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUBatchSelPIBCVHelper::LaunchTest' method. //////////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BASE									( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_PIBCV )

#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_NOINPUT								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_NOOUTPUT								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BASE + TU_ERROR_NOOUTPUT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_CANTCREATESELECTLIST					( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_CANTCREATEROWPARAMS					( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BASE + TU_ERROR_CANTCREATEROWPARAMS )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BADEXTRACT								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_RESULTDIFF								( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELPIBCV_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for batch selection for class for separators and air vents errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUBatchSelSeparatorHelper::InterpretInputs' method. /////////////////////////////////////////////////
#define	TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_BASE							( TU_ERROR_WHERE_HELPER_INTERPRETINPUTS + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_SEPARATOR )

#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_TYPEIDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_BASE + TU_ERROR_TYPEIDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_FAMILYIDEMPTY					( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_BASE + TU_ERROR_FAMILYIDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_CONNECTIDEMPTY				( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_BASE + TU_ERROR_CONNECTIDEMPTY )
#define TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_VERSIONIDEMPTY				( TU_ERROR_HELPER_INTERPRETINPUTS_BATCHSELSEPARATOR_BASE + TU_ERROR_VERSIONIDEMPTY )

// For 'CTUBatchSelSeparatorHelper::VerifyInputs' method. ////////////////////////////////////////////////////

#define	TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_BASE								( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_SEPARATOR )

#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_TYPEIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_BASE + TU_ERROR_TYPEIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_FAMILYIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_BASE + TU_ERROR_FAMILYIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_CONNECTIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_BASE + TU_ERROR_CONNECTIDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_VERSIONIDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSEPARATOR_BASE + TU_ERROR_VERSIONIDINVALID )

// For 'CTUBatchSelSeparatorHelper::LaunchTest' method. //////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BASE								( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_SEPARATOR )

#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_NOINPUT							( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_NOOUTPUT							( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BASE + TU_ERROR_NOOUTPUT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_CANTCREATESELECTLIST				( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_CANTCREATEROWPARAMS				( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BASE + TU_ERROR_CANTCREATEROWPARAMS )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BADEXTRACT							( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_RESULTDIFF							( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSEPARATOR_BASE + TU_ERROR_RESULTDIFFERENCE )


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines for batch selection for class for smart control valves errors.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For 'CTUBatchSelSmartControlValveHelper::VerifyInputs' method. ////////////////////////////////////////////////////
#define	TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE							( TU_ERROR_WHERE_HELPER_VERIFYINPUTS + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_SMARTCONTROLVALVE )

#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_MATERIALB65IDEMPTY				( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_MATERIALB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_MATERIALB65IDINVALID			( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_MATERIALB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_CONNECTB65IDEMPTY				( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_CONNECTB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_CONNECTB65IDINVALID				( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_CONNECTB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_PNB65IDEMPTY					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_PNB65IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_PNB65IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_PNB65IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_MATERIALA50IDEMPTY				( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_MATERIALA50IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_MATERIALA50IDINVALID			( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_MATERIALA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_CONNECTA50IDEMPTY				( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_CONNECTA50IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_CONNECTA50IDINVALID				( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_CONNECTA50IDINVALID )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_PNA50IDEMPTY					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_PNA50IDEMPTY )
#define TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_PNA50IDINVALID					( TU_ERROR_HELPER_VERIFYINPUTS_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_PNA50IDINVALID )

// For 'CTUBatchSelSmartControlValveHelper::LaunchTest' method. /////////////////////////////////////////////////////
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BASE								( TU_ERROR_WHERE_HELPER_LAUNCHTEST + TU_ERROR_SELECTIONTYPE_BATCH + TU_ERROR_PRODUCTTYPE_SMARTCONTROLVALVE )

#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_NOINPUT							( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_NOINPUTTOTEST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_NOOUTPUT							( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_NOOUTPUT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_CANTCREATESELECTLIST				( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_CANTCREATESELECTLIST )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_CANTCREATEROWPARAMS				( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_CANTCREATEROWPARAMS )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BADEXTRACT						( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_BADEXTRACT )
#define TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_RESULTDIFF						( TU_ERROR_HELPER_LAUNCHTEST_BATCHSELSMARTCONTROLVALVE_BASE + TU_ERROR_RESULTDIFFERENCE )

class CTUProductSelectionHelperError
{
public:
	CTUProductSelectionHelperError();
	virtual ~CTUProductSelectionHelperError() {}
	void GetErrorText( std::vector<CString> &vecMessage, UINT uiErrorCode );

private:
	std::map<UINT, CString> m_mapErrorList;
};

#endif
