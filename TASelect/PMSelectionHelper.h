#pragma once


#include "ProductSelectionParameters.h"
#include "SelectPM.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to help the exchange of a the 'CPMInputUser' between individual and wizard selection.
class CPMSelectionHelper
{
public:

	typedef enum _PMLastSelectionMode
	{
		LSM_NotYet,
		LSM_Individual,
		LSM_Wizard
	}PMLastSelectionMode;

	CPMSelectionHelper();
	virtual ~CPMSelectionHelper() {}

	// Allow to retrieve the 'CSelectPMList' that is linked to the current application type set in the ribbon.
	CSelectPMList *GetSelectPMList( ProductSelectionMode eProducSelectionMode );

	// Allow to retrieve the 'CSelectPMList' object for a specific application type (Heating, cooling or solar).
	CSelectPMList *GetSelectPMList( ProductSelectionMode eProducSelectionMode, ProjectType eProjectType );
	
	// Allow to retrieve the 'CPMInputUser' in the 'CSelectPMList' class for the current application type set in the ribbon.
	CPMInputUser *GetPMInputUser( ProductSelectionMode eProducSelectionMode );

	// Allow to retrieve the 'CPMInputUser' in the 'CSelectPMList' class for a specific application type.
	CPMInputUser *GetPMInputUser( ProductSelectionMode eProducSelectionMode, ProjectType eProjectType );

	// Allow to reset all data with their default values for all the application types.
	void ResetAll( void );

	// Allow to reset all data with their default values for the current application type set in the ribbon.
	void Reset( ProductSelectionMode eProducSelectionMode );

	// Allow to reset all data with their default values for a specific application type.
	void Reset( ProductSelectionMode eProducSelectionMode, ProjectType eProjectType );

	// Allow to set what is the last selection mode (Individual or wizard) that has worked on the variables.
	// Remark: It helps us to know how to react when going from one mode to other.
	void SetLastSelectionMode( PMLastSelectionMode eLastSelectionMode ) { m_eLastSelectionMode = eLastSelectionMode; }

	PMLastSelectionMode GetLastSelectionMode( void ) { return m_eLastSelectionMode; } 

	// HYS-1537 : Can be used with wizard param and ind sel param
	bool VerifyPMWQPrefs( CProductSelelectionParameters *pclSelPMParams );

// Private methods.
private:
	// Method to verify validity of some PM & WQ preferences.
	// HYS-1537 : Can be used with wizard param and ind sel param
	void _CheckDB( CProductSelelectionParameters *pclSelPMParams );

	void _SetValidateOptionFlagHelper( CDB_Product *pclProduct, int iDegFunction, int iWMUpFunctions, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet );

	// Return what type of water make-up (standard or soft/desal) is available with the current product.
	void _CheckWaterMakeUpType( CDB_Product *pclProduct, int *piFunctions );

	// Check a Pleno table to determine what kind of water make-up we have (standard, soft/desal or both).
	void _CheckWMUpTable( CDB_Product *pclProduct, int *piFunctions );

	// For optimization.
	// HYS-1537 : Can be used with wizard param and ind sel param
	void _PrepareMapVentoAlone( CProductSelelectionParameters* pclSelPMParams );
	void _PrepareMapPlenoAlone( CProductSelelectionParameters* pclSelPMParams );
	void _PrepareMapPlenoTable( CProductSelelectionParameters* pclSelPMParams );

// Private variables.
private:
	bool m_bInitialized;
	std::map<ProductSelectionMode, std::map<ProjectType, CSelectPMList*>> m_mapSelectPMLists;
	CSelectPMList m_clIndividualSelectionHeatingSelectPMList;
	CSelectPMList m_clIndividualSelectionCoolingSelectPMList;
	CSelectPMList m_clIndividualSelectionSolarSelectPMList;
	CSelectPMList m_clWizardSelectionHeatingSelectPMList;
	CSelectPMList m_clWizardSelectionCoolingSelectPMList;
	CSelectPMList m_clWizardSelectionSolarSelectPMList;
	PMLastSelectionMode m_eLastSelectionMode;

	// Variables for validation of different PM & WQ preferences (allow to disable or not some of them in regards to the context).
	CPMWQPrefsValidation m_clPMWQPrefsValidation;

	typedef std::map<int, CPMWQPrefsValidation::PMWQPrefArray> mapIntPrefArray;

	// 'int' can be 'CPMWQPrefsValidation::DWFCT_WaterMakeUpStd' or 'CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal'.
	std::map<CString, mapIntPrefArray> m_mapPlenoByTableID;

	// 'int' can be '0' or 'CPMWQPrefsValidation::DWFCT_WaterMakeUpStd'.
	mapIntPrefArray m_mapVentoAlone;

	// 'int' can be 'CPMWQPrefsValidation::DWFCT_WaterMakeUpStd' or 'CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal'.
	mapIntPrefArray m_mapPlenoAlone;
};
