#pragma once

#include "global.h"
#include "database.h"
#include "databobj.h"
// Activate trace of all read lines of database.txt 
#define ACTIVATE_TRACE_DB_READING	false
#define TAFILEKEY { 0x26, 0xE9, 0x27,0x28, 0xA7, 0xE8, 0x21, 0xE7, 0xE0, 0x29, 0x2D, 0xB5, 0x00 }

class CTADatabase : public CDataBase
{
public:
	CTADatabase();

	// This enum allow us to filter results from any of next methods in regards to the context (for individual selection, direct selection or
	// hydraulic network calculation) -> From 2014/01 it has been added 3 bits in Integer2 in the 'CDB_Thing' class to manage that (see CDataObj.h).
	enum FilterSelection
	{
		NoFiltering,
		ForIndAndBatchSel,
		ForDirSel,
		ForHMCalc
	};
	
	enum CvTargetTab
	{
		eForSSelCv,
		eForHMCv,					// All control valves (Included also these ones with a integrated Dp controller as TA-COMPACT-DP).
		eForHMCvBalTypeMV,			// Exclude control valves that contain a integrated Dp controller.
		eForPiCv,
		eForBCv,
		eForTrv,
		eForTechHCCv,
		eForTechHCBCv,
		eForDpCBCV,
		eFor6WayCV,					// HYS-1150
		eForSmartControlValve,
		eForSmartDpC,
		eLast
	};

	enum FailSafeCheckStatus
	{
		eFirst = -1,
		eOnlyWithoutFailSafe = 0,
		eOnlyWithFailSafe = 1,
		eBothFailSafe = 2
	};

	// Verify if the product with a VersionID and a FamilyID is included in the MEASVALV_TAB, could be used for DpC
	bool IsIncludedInMVTAB( _string strFamilyID, _string strVersionID, CTable *pclTable = NULL );
	// Fill a list with all balancing valve from RVTYPE_TAB
	// To keep a BV DN must be between DNMin and DNMax
	// if flag bRemoveTvFo is set reject valve RVTYPE_FO and RVTYPE_BVC  

#ifdef TACBX
	int GetBVTypeList( 
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bRemoveFO = false,
			bool bRemoveSTAM = true,
			bool bForDpC = false );
#else
	int GetBVTypeList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bRemoveTVFO = false,
			bool bRemoveSTAM = true,
			bool bForDpC = false );
#endif

	// Same as above but extract all the types from the balancing valve list given in 'pSrcList'.
	int GetBVTypeList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList );

	int GetBVFamilyList(
			CRankEx *pList,
			_string strSelTypeID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bForDpC = false,
			bool bIsForTools = false );

#ifndef TACBX
	// Same as above but extract all the families from the balancing valve list given in 'pSrcList'.
	int GetBVFamilyList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID );
#endif
	
	int GetBVBdyMatList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bForDpC = false );

#ifndef TACBX
	// Same as above but extract all the materials from the balancing valve list given in 'pSrcList'.
	int GetBVBdyMatList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID );
#endif

	int GetBVConnList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bForDpC = false );

#ifndef TACBX
	// Same as above but extract all the connections from the balancing valve list given in 'pSrcList'.
	int GetBVConnList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID );
#endif
	
	int GetBVVersList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bForDpC = false );

#ifndef TACBX
	// Same as above but extract all the versions from the balancing valve list given in 'pSrcList'.
	int GetBVVersList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID,
			_string strConnectID );
#endif

	int GetBVPNList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bForDpC = false );

#ifndef TACBX
	// Same as above but extract all the PN from the balancing valve list given in 'pSrcList'.
	int GetBVPNList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID,
			_string strConnectID,
			_string strVersionID );
#endif
	
	int GetBVList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamilyID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL,
			bool bForDpC = false,
			bool bIsForTools = false );

// A special version for BV selection with 6-way valve. We give a vector of family IDs.
#ifndef TACBX
	int GetBVFor6WayValveList(
			CRankEx *pList,
			_string strSelFamilyIDList,		// ID of a table containing all the family ID.
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL );
#endif

	int GetAlienManuList( CRankEx *pList );
	int GetAlienFamilyList( CRankEx *pList, _string SelManuID );
	int GetAlienValveList( CRankEx *pList, _string SelManuID, _string SelFamID );
	bool OneAlienValvePresent();
	bool OneAlienValveExist( _string SelManuID, _string SelFamID );
	
#ifndef TACBX
	int GetDpCDpStabList( CRankEx *pList );
	int GetDpCMvLocList( CRankEx *pList );
	int GetDpCLocList( CRankEx *pList, FilterSelection eFilterSelection = NoFiltering );
#endif

	int GetDpCTypeList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			eDpCLoc DpCLoc = DpCLocDownStream,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetDpCFamilyList(
			CRankEx *pList,
			eDpCLoc DpCLoc,
			_string strSelTypeID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	int GetDpCBdyMatList(
			CRankEx *pList,
			eDpCLoc DpCLoc,
			_string strSelTypeID,
			_string strSelFamID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetDpCConnList(
			CRankEx *pList,
			eDpCLoc DpCLoc,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetDpCVersList(
			CRankEx *pList,
			eDpCLoc DpCLoc,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetDpCPNList(
			CRankEx *pList,
			eDpCLoc DpCLoc,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnID,
			_string strSelVersionID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	int GetDpCList(
			CRankEx *pList,
			eDpCLoc DpCLoc,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnID,
			_string strSelVersID,
			_string strSelPNID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL );
	
	int GetTrvList(
			CRankEx *pList,
			_string strTypeID,
			_string strSelFamID,
			_string strSelConnID,
			_string strSelVersID,
			_string strSelPNID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL );
	
	// If 'IncludeOnlyTypeID' is not defined, the method returns all families. If defined, the method returns only families that contains the
	// specified type.
	int GetTrvFamilyList(
			CRankEx *pList,
			_string strIncludeOnlyTypeID = L"",
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the families from the thermostatic valve list given in 'pSrcList'.
	int GetTrvFamilyList( CRankEx *pSrcList, CRankEx *pDestList );
#endif
	
	int GetTrvTypeList(
			CRankEx *pList,
			_string strSelFamilyID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	int GetTrvConnectList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the connections from the thermostatic valve list given in 'pSrcList'.
	int GetTrvConnectList( CRankEx *pSrcList, CRankEx *pDestList, _string strSelFamID );
#endif
	
	int GetTrvVersList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the versions from the thermostatic valve list given in 'pSrcList'.
	int GetTrvVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strSelFamID, _string strSelConnectID );
#endif

	int GetReturnValveFamilyList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	int GetReturnValveConnectList(
			CRankEx *pList,
			_string strSelFamID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetReturnValveVersList(
			CRankEx *pList,
			_string strSelFamID,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	int GetTrvInsertList(
			CRankEx *pList,
			_string strTypeID,
			_string strSelFamID,
			_string strSelConnID,
			_string strSelVersID,
			_string strSelPNID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL );
	
	int GetTrvInsertFamilyList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
			
	// HYS-1467 : Add CVFunction to differentiate presettable and control only
	int GetTrvInsertTypeList(
			CRankEx *pList,
			_string strSelFamilyID,
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	// HYS-1305 : Add insert with automatic flow limiter
	int GetFLCVInsertList( 
			CRankEx * pList, 
			_string strSelTypeID,
			_string strSelFamID, 
			_string strSelConnID, 
			_string strSelVersID, 
			_string strSelPNID, 
			FilterSelection eFilterSelection, 
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData * pProd = NULL );
			
	// HYS-1305 : Add insert with automatic flow limiter
	int GetFLCVInsertFamilyList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
			
	// HYS-1305 : Add insert with automatic flow limiter
	int GetFLCVInsertTypeList(
			CRankEx *pList,
			_string strSelFamilyID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetRVInsertFamilyList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	int GetFLCVList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelConnID,
			_string strSelVersID,
			_string strSelPNID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL );
	
	// If 'IncludeOnlyTypeID' is not defined, the method returns all families. If defined, the method returns only families that contains the
	// specified type.
	int GetFLCVFamilyList(
			CRankEx *pList,
			_string strIncludeOnlyTypeID = L"",
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetFLCVTypeList(
			CRankEx *pList,
			_string strSelFamilyID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	int GetFLCVConnectList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	int GetFLCVVersList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	int GetSeparatorTypeList( CRankEx *pList, FilterSelection eFilterSelection = NoFiltering );
	
	// Same as above but extract all the types from the separator list given in 'pSrcList'.
	int GetSeparatorTypeList( CRankEx *pSrcList, CRankEx *pDestList );

	int GetSeparatorFamilyList( CRankEx *pList, _string strSelTypeID, FilterSelection eFilterSelection = NoFiltering, int iDNMin = 0, int iDNMax = INT_MAX );

	// Same as above but extract all the families from the separator list given in 'pSrcList'.
	int GetSeparatorFamilyList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID );

	int GetSeparatorConnList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection = NoFiltering, int iDNMin = 0, int iDNMax = INT_MAX );

	// Same as above but extract all the connections from the separator list given in 'pSrcList'.
	int GetSeparatorConnList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID );

	int GetSeparatorVersList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelConnectID, FilterSelection eFilterSelection = NoFiltering, int iDNMin = 0, int iDNMax = INT_MAX );

	// Same as above but extract all the versions from the separator list given in 'pSrcList'.
	int GetSeparatorVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strConnectID );

	int GetSeparatorList( CRankEx *pList, _string strSelTypeID, _string strSelFamilyID, _string strSelConnectID, _string strSelVersionID, FilterSelection eFilterSelection = NoFiltering, int iDNMin = 0, int iDNMax = INT_MAX , CData *pProd = NULL );
#endif

#ifndef TACBX
	int GetSafetyValveTypeList( CRankEx *pList, FilterSelection eFilterSelection = NoFiltering );

	// Same as above but extract all the types from the safety valve list given in 'pSrcList'.
	int GetSafetyValveTypeList( CRankEx *pSrcList, CRankEx *pDestList );

	int GetSafetyValveFamilyList( CRankEx *pList, ProjectType eSystemType, FilterSelection eFilterSelection = NoFiltering );
	
	// Same as above but extract all the types from the safety valve list given in 'pSrcList'.
	int GetSafetyValveFamilyList( CRankEx *pSrcList, CRankEx *pDestList, ProjectType eSystemType );

	int GetSafetyValveConnList( CRankEx *pList, ProjectType eSystemType, _string strSelFamilyID, FilterSelection eFilterSelection = NoFiltering, int iDNMin = 0, int iDNMax = INT_MAX );

	// Same as above but extract all the connections from the safety valve list given in 'pSrcList'.
	int GetSafetyValveConnList( CRankEx *pSrcList, CRankEx *pDestList, ProjectType eSystemType, _string strSelFamilyID );

	int GetSafetyValveSetPressureList( CRankEx *pList, ProjectType eSystemType, _string strSelFamilyID, _string strSelConnectID, FilterSelection eFilterSelection = NoFiltering, int iDNMin = 0, int iDNMax = INT_MAX );

	// Same as above but extract all the versions from the safety valve list given in 'pSrcList'.
	int GetSafetyValveSetPressureList( CRankEx *pSrcList, CRankEx *pDestList, ProjectType eSystemType, _string strSelFamilyID, _string strConnectID );

	int GetSafetyValveList( CRankEx *pList, ProjectType eSystemType, _string strSelFamilyID, _string strSelConnectID, double dSetPressure, FilterSelection eFilterSelection = NoFiltering, int iDNMin = 0, int iDNMax = INT_MAX , CData *pProd = NULL );
#endif

#ifndef TACBX
	int GetActuatorTypeList( CRankEx *pList, FilterSelection eFilterSelection = NoFiltering );
	int GetActuatorFamilyList( CRankEx *pList, _string strSelActuatorTypeID, FilterSelection eFilterSelection = NoFiltering );
	int GetActuatorVersionList( CRankEx *pList, _string strSelActuatorTypeID, _string strSelActuatorFamilyID, FilterSelection eFilterSelection = NoFiltering );
#endif

#ifdef TACBX
	int GetPipeSerieList( CRankEx *pList, CDataBase *pDB, FilterSelection eFilterSelection = NoFiltering );
#endif

	bool CheckIfCharactAlreadyExist( CRankEx *pList, CData *pTAp, CData *pMatchTAp, bool bTestSize = true );

#ifdef TACBX
	// If returns true, we can add 'pTap'.
	bool CheckLocalizedVersion( CRankEx *pList, CData *pTApData );
#endif
	
	// Local function used to complete correctly the list of table according the enum eCvTargetTab
	void FillTargetTable( std::vector<_string> *pvTab, CvTargetTab eCvTarget );

	int GetTACVCtrlTypeList(
			CRankEx *pList,
			CTADatabase::CvTargetTab eCvTarget = CTADatabase::eForSSelCv,
			bool bStrictCtrlTypeFit = true,
			CDB_ControlProperties::CV2W3W eCv2W3W = CDB_ControlProperties::LastCV2W3W,
			_string strSelTypeID = L"",
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the control types from the control valve list given in 'pSrcList'.
	int GetTACVCtrlTypeList(
			CRankEx *pSrcList, 
			CRankEx *pDestList, 
			_string strTypeID = L"" );
#endif
	
	// Used into Hydromod scan REGVALV_TAB, TRVALV_TAB, CTRLVALV_TAB
	// Used into TabCDialogSselCV scan only CTRLVALV_TAB
	// Used into Hydromod(PICV) & TabCDialogSselPICV scan only PICTRLVALV_TAB
	int GetTaCVTypeList( 
			CRankEx *pList, 
			CTADatabase::CvTargetTab eCvTarget = CTADatabase::eForSSelCv, 
			CDB_ControlProperties::CV2W3W eCv2W3W = CDB_ControlProperties::LastCV2W3W, 
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC, 
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU, 
			FilterSelection eFilterSelection = NoFiltering, 
			int iDNMin = 0, 
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the types from the control valve list given in 'pSrcList'.
	int GetTaCVTypeList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList );
#endif

	int GetTaCVFamList( 
			CRankEx *pList, 
			CvTargetTab eCvTarget = CTADatabase::eForSSelCv, 
			CDB_ControlProperties::CV2W3W eCv2W3W = CDB_ControlProperties::LastCV2W3W, 
			_string strSelTypeID = L"", 
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC, 
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU, 
			FilterSelection eFilterSelection = NoFiltering, 
			int iDNMin = 0, 
			int iDNMax = INT_MAX,
			bool bDpControllerCanBeDisabled = false );		// HYS-1162

#ifndef TACBX
	// Same as above but extract all the families from the control valve list given in 'pSrcList'.
	int GetTACVFamList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList, 
			_string strTypeID = L"",
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU );
#endif
	
	int GetTaCVBdyMatList( 
			CRankEx *pList,
			CTADatabase::CvTargetTab eCvTarget = CTADatabase::eForSSelCv,
			CDB_ControlProperties::CV2W3W eCv2W3W = CDB_ControlProperties::LastCV2W3W,
			_string strSelTypeID = L"",
			_string strSelFamID = L"",
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC,
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the body materials from the control valve list given in 'pSrcList'.
	int GetTaCVBdyMatList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList, 
			_string strTypeID = L"",
			_string strFamilyID = L"",
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU );
#endif

	int GetTaCVConnList(
			CRankEx *pList,
			CTADatabase::CvTargetTab eCvTarget = CTADatabase::eForSSelCv,
			CDB_ControlProperties::CV2W3W eCv2W3W = CDB_ControlProperties::LastCV2W3W,
			_string strSelTypeID = L"",
			_string strSelFamID = L"",
			_string strSelBdyMatID=L"",
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC,
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the connections from the control valve list given in 'pSrcList'.
	int GetTaCVConnList(
			CRankEx *pSrcList, 
			CRankEx *pDestList, 
			_string strTypeID = L"",
			_string strFamilyID = L"",
			_string strBodyMaterialID = L"",
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU );
#endif

	int GetTaCVVersList(
			CRankEx *pList,
			CTADatabase::CvTargetTab eCvTarget = CTADatabase::eForSSelCv,
			CDB_ControlProperties::CV2W3W eCv2W3W = CDB_ControlProperties::LastCV2W3W,
			_string strSelTypeID = L"",
			_string strSelFamID = L"",
			_string strSelBdyMatID=L"",
			_string strSelConnectID = L"",
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC,
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the versions from the control valve list given in 'pSrcList'.
	int GetTaCVVersList(
			CRankEx *pSrcList, 
			CRankEx *pDestList, 
			_string strTypeID = L"",
			_string strFamilyID = L"",
			_string strBodyMaterialID = L"",
			_string strConnectID = L"",
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU );
#endif

	int GetTaCVPNList(
			CRankEx *pList,
			CTADatabase::CvTargetTab eCvTarget = CTADatabase::eForSSelCv,
			CDB_ControlProperties::CV2W3W eCv2W3W = CDB_ControlProperties::LastCV2W3W,
			_string strSelTypeID = L"",
			_string strSelFamID = L"",
			_string strSelBdyMatID=L"",
			_string strSelConnectID = L"",
			_string strSelVersionID = L"",
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC,
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

#ifndef TACBX
	// Same as above but extract all the PN from the control valve list given in 'pSrcList'.
	int GetTaCVPNList(
			CRankEx *pSrcList, 
			CRankEx *pDestList, 
			_string strTypeID = L"",
			_string strFamilyID = L"",
			_string strBodyMaterialID = L"",
			_string strConnectID = L"",
			_string strVersionID = L"",
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU );
#endif

	// HYS-1877: Add a way to get 6-way valve set and the PIBCV set linked.
	int GetTaCVList(
			CRankEx *pList,
			CTADatabase::CvTargetTab eCvTarget = CTADatabase::eForSSelCv,
			bool bRetIfFound = false,
			CDB_ControlProperties::CV2W3W eCv2W3W = CDB_ControlProperties::LastCV2W3W,
			_string strSelTypeID = L"",
			_string strSelFamID = L"",
			_string strSelBdyMatID=L"",
			_string strSelConnectID = L"",
			_string strSelVersionID = L"",
			_string strSelPNID = L"",
			CDB_ControlProperties::eCVFUNC eCVFunc = CDB_ControlProperties::LastCVFUNC,
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bForHubStation = false,
			CData *pProd = NULL,
			bool bOnlySet = false );

#ifndef TACBX //TACBX////////////////////////////////////////////////////////

	// HYS-1079: Fail-safe function is redefined and now we use default return position 
	int GetTaCVActuatorPowerSupplyList(
			CRankEx *pPowerSupplyList,
			CRankEx *pCVList,
			CDB_ControlProperties::CvCtrlType eCvCtrlType,
			bool bOnlyPackage = false,
			FilterSelection eFilterSelection = NoFiltering );

	int GetTaCVActuatorInputSignalList(
			CRankEx *pInputSignalList,
			CRankEx *pCVList,
			CDB_ControlProperties::CvCtrlType eCvCtrlType,
			bool bOnlyPackage = false,
			_string strSelPowerSupplyID = L"",
			FilterSelection eFilterSelection = NoFiltering );

	int GetTaCVActuatorFailSafeValues(
			CRankEx *pCVList,
			CDB_ControlProperties::CvCtrlType eCvCtrlType,
			bool bOnlyPackage = false,
			_string strSelPowerSupplyID = L"", 
			_string strSelInputSignalID = L"",
			FilterSelection eFilterSelection = NoFiltering );

	int GetTaCVActuatorList(
			CRankEx *pActuatorList,
			CDB_ControlValve *pclControlValve,
			bool bOnlyPackage = false,
			_string strSelPowerSupplyID = L"",
			_string strSelInputSignalID = L"",
			int iFailSafe = -1,
			_string SetDRPID = L"",
			FilterSelection eFilterSelection = NoFiltering );

	int GetTaCVDRPFunctionList(
			CRankEx *pDRPFctList,
			CDB_ControlProperties::CvCtrlType eCvCtrlType,
			CRankEx *pCVList,
			bool bOnlyPackage = false,
			_string strSelPowerSupplyID = L"",
			_string strSelInputSignalID = L"",
			int iFailSafeFct = -1,
			FilterSelection eFilterSelection = NoFiltering );

	// Return default return position function associated to the enum eDRPFunction.
	CString GetDRPFunctionStr( CDB_ControlValve::DRPFunction eDRPFunction );

	// Return a sorted list with sorted actuator from pTab
	// if pCV exist and CV is a part of a CVActSet, list is completed with Actuator from CV actuator set 
	int GetActuator(
			CRank *pList,
			CDB_ControlValve *pCV,
			bool bWorkingForASet = true,
			FilterSelection eFilterSelection = NoFiltering );

	// Extract actuators possibilities from a list of actuator.
	int GetActuator(
			CRank *pList,
			CTable *pTabActrLst,
			_string strSelPowerSupplyID = L"",
			_string strSelInputSignalID = L"",
			int iFailSafe = -1,
			CDB_ElectroActuator::DefaultReturnPosition eDRP = CDB_ElectroActuator::DefaultReturnPosition::Undefined,
			FilterSelection eFilterSelection = NoFiltering );

	int GetActuator(
			CRank *pList,

			CRank *pRankActrLst,

			_string strSelPowerSupplyID = L"",

			_string strSelInputSignalID = L"", int iFailSafe = -1,

			CDB_ElectroActuator::DefaultReturnPosition eDRP = CDB_ElectroActuator::DefaultReturnPosition::Undefined,
			FilterSelection eFilterSelection = NoFiltering );

	// Extract power possibilities from a list of actuator.
	int GetActuatorPowerSupplyList( CRank *pList, CTable *pTabActrLst, CDB_ControlProperties::CvCtrlType eCvCtrlType, FilterSelection eFilterSelection = NoFiltering );		// Target list, supply list
	int GetActuatorPowerSupplyList( CRank *pList, CRank *pRankActrLst, CDB_ControlProperties::CvCtrlType eCvCtrlType, FilterSelection eFilterSelection = NoFiltering );
	
	// Extract input signal possibilities from a list of actuator, could be filtered with power supply.
	int GetActuatorInputSignalList( CRank *pList, CTable *pTabActrLst, CDB_ControlProperties::CvCtrlType eCvCtrlType, _string strSelPowerSupplyID = L"", FilterSelection eFilterSelection = NoFiltering );
	int GetActuatorInputSignalList( CRank *pList, CRank *pRankActrLst, CDB_ControlProperties::CvCtrlType eCvCtrlType, _string strSelPowerSupplyID = L"", FilterSelection eFilterSelection = NoFiltering );
	
	int GetActuatorFailSafeValues( CTable *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, _string strSelPowerSupplyID = L"", _string strSelInputSignalID = L"", 
		FilterSelection eFilterSelection = NoFiltering );
	int GetActuatorFailSafeValues( CRank *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, _string strSelPowerSupplyID = L"", _string strSelInputSignalID = L"", 
		FilterSelection eFilterSelection = NoFiltering );

	// Extract fail safe possibilities from a list of actuator, could be filtered with power supply and input signal.
	int GetActuatorDRPList(
			CRank *pList,
			CTable *pTabActrLst,
			CDB_ControlProperties::CvCtrlType eCvCtrlType,
			CDB_ControlValve *pCV,
			_string strSelPowerSupplyID = L"",
			_string strSelInputSignalID = L"", 
			int iFailSafeFct = -1,
			FilterSelection eFilterSelection = NoFiltering );
	
	int GetActuatorDRPList(
			CRank *pList,
			CRank *pRankActrLst,
			CDB_ControlProperties::CvCtrlType eCvCtrlType, 
			CDB_ControlValve *pCV,
			_string strSelPowerSupplyID = L"",
			_string strSelInputSignalID = L"",
			int iFailSafeFct = -1,
			FilterSelection eFilterSelection = NoFiltering );

	// Retrieve all adapters common to a control valve.
	int GetAdapterList(
			CDB_ControlValve *pclControlValve, 
			CRank *pclAdapterList, 
			CDB_RuledTableBase **ppclRuledTable, 
			bool bDowngradeActuatorFunctionality = true, 
			CString strPowerSupplyToMatchID = _T(""), 
			CString strInputSignalToMatchID = _T(""),
			int iFailSafeFctToMatch = -1,
			CDB_ControlValve::DRPFunction eDRP = CDB_ControlValve::DRPFunction::drpfUndefined,
			CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU, 
			bool bForSet = false,
			FilterSelection eFilterSelection = NoFiltering );
#endif

	int GetSvTypeList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetSvFamilyList(
			CRankEx *pList,
			_string strSelTypeID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetSvBdyMatList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetSvConnList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetSvVersList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetSvPNList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );
	
	int GetSvList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamilyID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL );

#ifndef TACBX
	int GetSvActuatorList(
			CRank *pActuatorList,
			CDB_ShutoffValve *pclShutOffValve,
			bool bOnlyPackage = false,
			FilterSelection eFilterSelection = NoFiltering );
#endif

#ifndef TACBX
	// HYS-1877: To update combo box when we are in set selection
	int Get6WayValveConnectionList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
		    bool bOnlyForSet = false );

	// Same as above but extract all the families from the 6-way valve list given in 'pSrcList'.
	int Get6WayValveConnectionList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList );

	int Get6WayValveVersList(
			CRankEx *pList,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX );

	// Same as above but extract all the versions from the 6-way valve list given in 'pSrcList'.
	int Get6WayValveVersList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strSelConnectID );

	int Get6WayValveList(
			CRankEx *pList,
			_string strSelConnectID = L"",
			_string strSelVersionID = L"",
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL );

	// HYS-1877: To update PIBCV input signal combo box when we are in set selection
	int Get6WayValveActrInputSignalList(
		CRankEx *pInputSignalList,
		CRankEx *pCVList,
		CDB_ControlProperties::CvCtrlType eCvCtrlType,
		_string strSelFamID,
		bool bFastConnection = false,
		FilterSelection eFilterSelection = NoFiltering,
		bool bOnlyForSet = false );

	int Get6WayValveSecondaryTypeList(
		CRankEx *pList,
		FilterSelection eFilterSelection = NoFiltering,
		int iDNMin = 0,
		int iDNMax = INT_MAX );

	// HYS-1877: To update combo box when we are in set selection
	int Get6WayValvePIBCvFamilyList(
		CRankEx *pList,
		e6WayValveSelectionMode Selectionmode,
		FilterSelection eFilterSelection = NoFiltering,
		int iDNMin = 0,
		int iDNMax = INT_MAX,
		bool bOnlyPackage = false );

	int Get6WayValveBvFamilyList(
		CRankEx *pList,
		_string strSelTypeID,
		FilterSelection eFilterSelection = NoFiltering,
		int iDNMin = 0,
		int iDNMax = INT_MAX );

	int Get6WayValvePICVTypeList( 
		CRankEx * pList, 
		FilterSelection eFilterSelection = NoFiltering, 
		int iDNMin = 0,
		int iDNMax = INT_MAX );
#endif

	int GetSmartControlValveTypeList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

	// Same as above but extract all the types from the smart control valve list given in 'pSrcList'.
	int GetSmartControlValveTypeList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList );

	int GetSmartControlValveFamilyList(
			CRankEx *pList,
			_string strSelTypeID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the families from the smart control valve list given in 'pSrcList'.
	int GetSmartControlValveFamilyList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID );
#endif
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view - Add set information
	int GetSmartControlValveBdyMatList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
		    bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the materials from the smart control valve list given in 'pSrcList'.
	int GetSmartControlValveBdyMatList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID );
#endif
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view - Add set information
	int GetSmartControlValveConnList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
		    bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the connections from the smart control valve list given in 'pSrcList'.
	int GetSmartControlValveConnList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID );
#endif
	
	int GetSmartControlValveVersList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the versions from the smart control valve list given in 'pSrcList'.
	int GetSmartControlValveVersList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID,
			_string strConnectID );
#endif
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view - Add set information
	int GetSmartControlValvePNList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
		    bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the PN from the smart control valve list given in 'pSrcList'.
	int GetSmartControlValvePNList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID,
			_string strConnectID,
			_string strVersionID );
#endif
		
	int GetSmartControlValveList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamilyID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			_string strSelPNID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL,
			bool bIsForTools = false,
			bool bIsForSet = false );

	int GetSmartDpCTypeList(
			CRankEx *pList,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

	// Same as above but extract all the types from the smart differential pressure controller list given in 'pSrcList'.
	int GetSmartDpCTypeList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList );

	int GetSmartDpCFamilyList(
			CRankEx *pList,
			_string strSelTypeID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the families from the smart differential pressure controller list given in 'pSrcList'.
	int GetSmartDpCFamilyList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID );
#endif
	
	int GetSmartDpCBdyMatList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the materials from the smart differential pressure controller list given in 'pSrcList'.
	int GetSmartDpCBdyMatList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID );
#endif

	int GetSmartDpCConnList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the connections from the smart differential pressure controller list given in 'pSrcList'.
	int GetSmartDpCConnList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID );
#endif
	
	int GetSmartDpCVersList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the versions from the smart differential pressure controller list given in 'pSrcList'.
	int GetSmartDpCVersList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID,
			_string strConnectID );
#endif

	int GetSmartDpCPNList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			bool bIsForSet = false );

#ifndef TACBX
	// Same as above but extract all the PN from the smart differential pressure controller list given in 'pSrcList'.
	int GetSmartDpCPNList( 
			CRankEx *pSrcList, 
			CRankEx *pDestList,
			_string strTypeID,
			_string strFamilyID,
			_string strMaterialID,
			_string strConnectID,
			_string strVersionID );
#endif
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view - Add set information
	int GetSmartDpCList(
			CRankEx *pList,
			_string strSelTypeID,
			_string strSelFamilyID,
			_string strSelBdyMatID,
			_string strSelConnectID,
			_string strSelVersionID,
			_string strSelPNID,
			FilterSelection eFilterSelection = NoFiltering,
			int iDNMin = 0,
			int iDNMax = INT_MAX,
			CData *pProd = NULL,
			bool bIsForTools = false,
		    bool bIsForSet = false );

#ifndef TACBX
	// Fill a list with all TAProduct that match CBIType, CBISize
	// return true if at least one TA product is added into the list
	bool GetTAPListFromCBIDef( CRank *pList, CString CBIType, CString CBISize );

	// HYS-1617/HYS-1618 (Linked to HYS-1108): We can't bypass crossing table accessories when we are in direct selection.
	// If we select an actuator alone, we can show all accessories without regarding to the crossing table (bApplyCrossingTable = false)
	// But if we select an actuator alone, we need to apply the same behavior as in the individual selection and take into account the
	// crossing table ((bApplyCrossingTable = true).
	int GetAccessories( CRank *pList, CTable *pTab, FilterSelection eFilterSelection = NoFiltering, bool bApplyCrossingTable = true );

	int GetSetDpCMeasValve( CRank *pList, CDB_DpController *pDpC );

	int GetPipeSerieList( CRank *pList, FilterSelection eFilterSelection = NoFiltering );
	int GetPipeSizeList( CRank *pList, CTable *pPipeSerieTab, FilterSelection eFilterSelection = NoFiltering );

	// Function that return the DNMin and DNMax parameters of a TAProductList
	// The return value is true if DNMin < DNmax
	bool GetTAProdDNRange( CRankEx *pList, CDB_StringID **pStrIDDNMin, CDB_StringID **pStrIDDNMax );

	double GetPrice( CString strArticleNumber );


	// Returns if the selection can be edited or not.
	bool CanEditSelection( CData *pclSelection );

#endif //TACBX////////////////////////////////////////////////////////
	// This method returns 'true' if object can not be use in the specific context selection (individual, batch or direct selection, or in the hydraulic network calculation).
	bool MustExclude( CDB_Thing *pclDBThing, FilterSelection eFilterSelection );
};
