#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "Utilities.h"
#include "Units.h"

#include "DlgWizPMRightViewInputBase.h"
#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"
#include "RViewSSelPM.h"

#include "DlgSelectionBase.h"
#include "DlgWizardSelBase.h"
#include "DlgWizardSelPM.h"

#include "DlgConfSel.h"
#include "DlgIndSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgWizardSelPM::CDlgWizardSelPM( CWnd *pParent )
	: CDlgWizardSelBase( m_clWizardSelPMParams, CDlgWizardSelPM::IDD, pParent )
{
	m_clWizardSelPMParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_ExpansionVessel;
	m_clWizardSelPMParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Wizard;

	m_eWizardPMEngineState = WPMES_ProjectTypeChoice;
	m_iCurrentButton = -1;
	m_bResultsAlreadyDisplayed = false;

	m_hIconButtonToDo = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_ARROWRIGHTGRAYED ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	m_hIconButtonDoing = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_ARROWRIGHT ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	m_hIconButtonDone = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_ARROWOK ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	m_hIconButtonLeft = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_ARROWLEFT ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	m_hIconButtonError = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_ARROWERROR ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );


	// Definition of all of these group of buttons must respect the order defined by the 'WizardPMEngineState' enum.

	////////////////////////////////////////////////////////////////////////////////////////////////
	// For choice of the project type.
	COneGroupDefinition *pclOneGroupDefinition = new COneGroupDefinition();

	// Button definitions.
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_HEATING, BIP_Right, BS_Enable, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_ProjectTypeChoice_GoInHeating ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_COOLING, BIP_Right, BS_Enable, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_ProjectTypeChoice_GoInCooling ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_SOLAR, BIP_Right, BS_Enable, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_ProjectTypeChoice_GoInSolar ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_ONLYWATERMAKEUP, BIP_Right, 
			BS_Disable, m_hIconButtonToDo, &CDlgWizardSelPM::_ProjectTypeChoice_GoInWaterMakeUpOnly ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_ONLYDEGASSING, BIP_Right, 
			BS_Disable, m_hIconButtonToDo, &CDlgWizardSelPM::_ProjectTypeChoice_GoInDegassingOnly ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For values input in heating mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_BACKTOPROJECTYPE, BIP_Left, BS_Doing, 
			m_hIconButtonLeft, &CDlgWizardSelPM::_BackToProjectType ) );

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_TEMPERATURES, BIP_Right, BS_Doing, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingTemperature ) );

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSURES, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingPressure ) );

	COneButtonDefinition *pclOnButtonDefinition = pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SYSTEMVOLUMEPOWER, BIP_Right, BS_ToDo,
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower ) );

	// Add dialog ID for the SWKI HE301-01 norm.
	pclOnButtonDefinition->AddDlgWizePMRightViewInputID( PressurisationNorm::PN_SWKIHE301_01, CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePowerSWKI );

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSMAINTTYPE, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingPressurisationMaintenanceType ) );

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingRequirement ) );

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT2, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingRequirement2 ) );

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_RESULTS, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_GoToResultsStep ) );

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SELECT, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_Select ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For values input in cooling mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_BACKTOPROJECTYPE, BIP_Left, BS_Doing, 
			m_hIconButtonLeft, &CDlgWizardSelPM::_BackToProjectType ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_TEMPERATURES, BIP_Right, BS_Doing, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingTemperature ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSURES, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingPressure ) );
	
	pclOnButtonDefinition = pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SYSTEMVOLUMEPOWER, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower ) );

	// Add dialog ID for the SWKI HE301-01 norm.
	pclOnButtonDefinition->AddDlgWizePMRightViewInputID( PressurisationNorm::PN_SWKIHE301_01, CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePowerSWKI );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSMAINTTYPE, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingPressurisationMaintenanceType ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingRequirement ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT2, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingRequirement2 ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_RESULTS, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_GoToResultsStep ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SELECT, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_Select ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For values input in solar mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_BACKTOPROJECTYPE, BIP_Left, BS_Doing,
			m_hIconButtonLeft, &CDlgWizardSelPM::_BackToProjectType ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_TEMPERATURES, BIP_Right, BS_Doing, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarTemperature ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSURES, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarPressure ) );
	
	pclOnButtonDefinition = pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SYSTEMVOLUMEPOWER, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePower ) );

	// Add dialog ID for the SWKI HE301-01 norm.
	pclOnButtonDefinition->AddDlgWizePMRightViewInputID( PressurisationNorm::PN_SWKIHE301_01, CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKI );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSMAINTTYPE, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarPressurisationMaintenanceType ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarRequirement ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT2, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarRequirement2 ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_RESULTS, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_GoToResultsStep ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SELECT, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_Select ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For values input in water make-up only mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_BACKTOPROJECTYPE, BIP_Left, BS_Doing, 
			m_hIconButtonLeft, &CDlgWizardSelPM::_BackToProjectType ) );
	/* TODO
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_TEMPERATURES, BIP_Right, BS_Doing, 
		m_hIconButtonDoing, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarTemperature ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSURES, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarPressure ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SYSTEMVOLUMEPOWER, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePower ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSMAINTTYPE, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarPressurisationMaintenanceType ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarRequirement ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT2, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarRequirement ) );
	*/
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_RESULTS, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_GoToResultsStep ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SELECT, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_Select ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For values input in degassing only mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_BACKTOPROJECTYPE, BIP_Left, BS_Doing, 
			m_hIconButtonLeft, &CDlgWizardSelPM::_BackToProjectType ) );
	/* TODO
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_TEMPERATURES, BIP_Right, BS_Doing, 
		m_hIconButtonDoing, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarTemperature ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSURES, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarPressure ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SYSTEMVOLUMEPOWER, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePower ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSMAINTTYPE, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarPressurisationMaintenanceType ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarRequirement ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT2, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarRequirement ) );
	*/
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_RESULTS, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_GoToResultsStep ) );
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SELECT, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_Select ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For choice of the project type in edition mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	// Button definitions.
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_HEATING, BIP_Right, BS_Enable, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_ProjectTypeChoice_GoInHeatingEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_COOLING, BIP_Right, BS_Enable, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_ProjectTypeChoice_GoInCoolingEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_SOLAR, BIP_Right, BS_Enable, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_ProjectTypeChoice_GoInSolarEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_ONLYWATERMAKEUP, BIP_Right, 
			BS_Disable, m_hIconButtonToDo, &CDlgWizardSelPM::_ProjectTypeChoice_GoInWaterMakeUpOnlyEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PROJECTYPE_ONLYDEGASSING, BIP_Right, 
			BS_Disable, m_hIconButtonToDo, &CDlgWizardSelPM::_ProjectTypeChoice_GoInDegassingOnlyEdition ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For values input in heating mode for the edition mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_BACKTOPROJECTYPE, BIP_Left, BS_Doing, 
			m_hIconButtonLeft, &CDlgWizardSelPM::_BackToProjectTypeEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_TEMPERATURES, BIP_Right, BS_Doing, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingTemperature ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSURES, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingPressure ) );
	
	pclOnButtonDefinition = pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SYSTEMVOLUMEPOWER, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower ) );

	// Add dialog ID for the SWKI HE301-01 norm.
	pclOnButtonDefinition->AddDlgWizePMRightViewInputID( PressurisationNorm::PN_SWKIHE301_01, CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePowerSWKI );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSMAINTTYPE, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingPressurisationMaintenanceType ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingRequirement ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT2, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::HeatingRequirement2 ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_RESULTS, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_GoToResultsStep ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_APPLY, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_ApplyEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_CANCEL, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_CancelEdition ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For values input in cooling mode for the edition mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_BACKTOPROJECTYPE, BIP_Left, BS_Doing, 
			m_hIconButtonLeft, &CDlgWizardSelPM::_BackToProjectTypeEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_TEMPERATURES, BIP_Right, BS_Doing, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingTemperature ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSURES, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingPressure ) );
	
	pclOnButtonDefinition = pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SYSTEMVOLUMEPOWER, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower ) );

	// Add dialog ID for the SWKI HE301-01 norm.
	pclOnButtonDefinition->AddDlgWizePMRightViewInputID( PressurisationNorm::PN_SWKIHE301_01, CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePowerSWKI );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSMAINTTYPE, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingPressurisationMaintenanceType ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingRequirement ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT2, BIP_Right, BS_ToDo, 
		m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::CoolingRequirement2 ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_RESULTS, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_GoToResultsStep ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_APPLY, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_ApplyEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_CANCEL, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_CancelEdition ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// For values input in solar mode for the edition mode.
	pclOneGroupDefinition = new COneGroupDefinition();

	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_BACKTOPROJECTYPE, BIP_Left, BS_Doing, 
			m_hIconButtonLeft, &CDlgWizardSelPM::_BackToProjectTypeEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_TEMPERATURES, BIP_Right, BS_Doing, 
			m_hIconButtonDoing, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarTemperature ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSURES, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarPressure ) );
	
	pclOnButtonDefinition = pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_SYSTEMVOLUMEPOWER, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePower ) );

	// Add dialog ID for the SWKI HE301-01 norm.
	pclOnButtonDefinition->AddDlgWizePMRightViewInputID( PressurisationNorm::PN_SWKIHE301_01, CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKI );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_PRESSMAINTTYPE, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarPressurisationMaintenanceType ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarRequirement ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_REQUIREMENT2, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_RightViewValuesInputDialogToChange, CDlgWizardPM_RightViewInput_Base::SolarRequirement2 ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_RESULTS, BIP_Right, BS_ToDo, 
			m_hIconButtonToDo, &CDlgWizardSelPM::_GoToResultsStep ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_APPLY, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_ApplyEdition ) );
	
	pclOneGroupDefinition->AddOneButtonDefinition( new COneButtonDefinition( IDS_DLGWIZARDPM_CANCEL, BIP_None, BS_ToDo, 0, 
			&CDlgWizardSelPM::_CancelEdition ) );

	m_clAllGroupDefinitions.AddOneGroupDefinition( pclOneGroupDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////

	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];

	m_arpButtonText.Add( &m_clButtonText1 );
	m_arpButtonText.Add( &m_clButtonText2 );
	m_arpButtonText.Add( &m_clButtonText3 );
	m_arpButtonText.Add( &m_clButtonText4 );
	m_arpButtonText.Add( &m_clButtonText5 );
	m_arpButtonText.Add( &m_clButtonText6 );
	m_arpButtonText.Add( &m_clButtonText7 );
	m_arpButtonText.Add( &m_clButtonText8 );
	m_arpButtonText.Add( &m_clButtonText9 );
	m_arpButtonText.Add( &m_clButtonText10 );

	m_arpButtonImg.Add( &m_clButtonImg1 );
	m_arpButtonImg.Add( &m_clButtonImg2 );
	m_arpButtonImg.Add( &m_clButtonImg3 );
	m_arpButtonImg.Add( &m_clButtonImg4 );
	m_arpButtonImg.Add( &m_clButtonImg5 );
	m_arpButtonImg.Add( &m_clButtonImg6 );
	m_arpButtonImg.Add( &m_clButtonImg7 );
	m_arpButtonImg.Add( &m_clButtonImg8 );
	m_arpButtonImg.Add( &m_clButtonImg9 );
	m_arpButtonImg.Add( &m_clButtonImg10 );

	m_arpSeparator.Add( &m_clSeparator1 );
	m_arpSeparator.Add( &m_clSeparator2 );
	m_arpSeparator.Add( &m_clSeparator3 );
	m_arpSeparator.Add( &m_clSeparator4 );
	m_arpSeparator.Add( &m_clSeparator5 );
	m_arpSeparator.Add( &m_clSeparator6 );
	m_arpSeparator.Add( &m_clSeparator7 );
	m_arpSeparator.Add( &m_clSeparator8 );
	m_arpSeparator.Add( &m_clSeparator9 );
}

CDlgWizardSelPM::~CDlgWizardSelPM()
{
	if( INVALID_HANDLE_VALUE != m_hIconButtonToDo )
	{
		DeleteObject( m_hIconButtonToDo );
	}

	if( INVALID_HANDLE_VALUE != m_hIconButtonDoing )
	{
		DeleteObject( m_hIconButtonDoing );
	}

	if( INVALID_HANDLE_VALUE != m_hIconButtonDone )
	{
		DeleteObject( m_hIconButtonDone );
	}

	if( INVALID_HANDLE_VALUE != m_hIconButtonLeft )
	{
		DeleteObject( m_hIconButtonLeft );
	}

	if( INVALID_HANDLE_VALUE != m_hIconButtonError )
	{
		DeleteObject( m_hIconButtonError );
	}
}

void CDlgWizardSelPM::SetApplicationBackground( COLORREF cBackColor )
{
	CDlgWizardSelBase::SetApplicationBackground( cBackColor );

	if( NULL != pRViewWizardSelPM )
	{
		pRViewWizardSelPM->SetApplicationBackground( cBackColor );
	}
}

bool CDlgWizardSelPM::IsRightViewResetNeeded()
{
	bool bReturn = true;

	if( true == m_bResultsAlreadyDisplayed && NULL != m_clWizardSelPMParams.m_pclSelectPMList 
			&& NULL != m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		// Remark: Why 'CompareTo'? Because if user was already gone in the result page (m_bResultsAlreadyDisplayed = true)
		//         but after that he changed some values, we need to reset the RView.

		// Check if data have changed since the last time the results were displayed.
		bReturn = !m_clPMInputUserSaved.CompareTo( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() ); 
	}

	return bReturn;
}

void CDlgWizardSelPM::SaveSelectionParameters()
{
	CDlgWizardSelBase::SaveSelectionParameters();

	CDS_WizardSelParameter *pclWizardSelParameter = m_clWizardSelPMParams.m_pTADS->GetpWizardSelParameter();

	if( NULL == pclWizardSelParameter )
	{
		return;
	}
	// HYS-1537 : Copy wizard PM preferences in Individual selection
	CDlgIndSelPressureMaintenance* pclPMDlg = dynamic_cast<CDlgIndSelPressureMaintenance*>( pDlgLeftTabSelManager->GetLeftTabDialog( ProductSubCategory::PSC_PM_ExpansionVessel,
		ProductSelectionMode_Individual ) );

	if( this == pDlgLeftTabSelManager->GetCurrentLeftTabDialog() )
	{
		pclPMDlg->UpdateWithWizardAllPrefInAllMode();
	}
	// Save the last data input by the user in the current right input value dialog.
	if( NULL != pRViewWizardSelPM )
	{
		pRViewWizardSelPM->FillPMInputUser();
	}
}

void CDlgWizardSelPM::OnRViewWizSelPMButtonBackClicked()
{
	if( 0 == m_iCurrentButton )
	{
		_BackToProjectType( -1 );
	}
	else
	{
		// Remark: do not change 'm_iCurrentButton' here. It's the called method that has to do it at the right moment.
		pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton - 1]->m_ptrfActionToDo;
 		(this->*fActionToDo)( m_iCurrentButton - 1 );
	}

	_ChangeButtonStates();
	_UpdateAllStatics();
}

void CDlgWizardSelPM::OnRViewWizSelPMButtonNextClicked()
{
	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	if( m_iCurrentButton == iVisibleButtonNumber )
	{
		return;
	}

	// Remark: do not change 'm_iCurrentButton' here. It's the called method that have to do it at the right moment.
 	pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton + 1]->m_ptrfActionToDo;
 	(this->*fActionToDo)( m_iCurrentButton + 1 );

	_ChangeButtonStates();
	_UpdateAllStatics();
}

void CDlgWizardSelPM::OnRViewWizSelPMActivateStepWithCurrentRightView( int iCurrentRightView )
{
	// This method will not activate the right view by calling 'pfActionToDo' but just the focus on the step that corresponds to the
	// current right view. Typically it's for example the case where user has introduced data in heating generators/consumers, he is in the 
	// 'Pressure maintenance type' step and click on the 'Back' button. 'CRViewWizardSelPM::OnButtonBackNextClicked' is called. This method
	// checks if user has introduced data. Here it is the case thus the method will display the 'Heating - Consumers' dialog (that is the second
	// one in advanced mode). But we need to activate the 'Water volume/Power' step.

	// Search the corresponding step.
	for( int i = 0; i < m_pclCurrentGroupDefinition->m_arOneGroupDefinition.GetCount(); i++ )
	{
		COneButtonDefinition *pOneButtonDefinition = m_pclCurrentGroupDefinition->m_arOneGroupDefinition.GetAt( i );

		if( iCurrentRightView == pOneButtonDefinition->GetDlgWizePMRightViewInputID( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetNorm() ) )
		{
			m_iCurrentButton = i;
			_ChangeButtonStates();
			_UpdateAllStatics();
			return;
		}
	}
}

void CDlgWizardSelPM::OnRViewWizSelPMSwitchNormalAdvancedMode( int iWhichInputDialog )
{
	if( NULL == pRViewWizardSelPM )
	{
		return;
	}

	pRViewWizardSelPM->DetachCurrentInputDialog();

	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	bool bBackEnable = true;
	bool bNextEnable = true;

	if( m_iCurrentButton == iVisibleButtonNumber )
	{
		bNextEnable = false;
	}

	// When leaving the advanced mode for system volume and installed power dialog, we need to know on which dialog to go.
	// For example: we are in the 'CDlgWizardPM_RightViewInput_CoolingConsumers' dialog and leave the advanced mode.
	// We need to know if we must go in the 'CDlgWizardPM_RightViewInput_CoolingSystemVolumePower' or 'CDlgWizardPM_RightViewInput_CoolingSystemVolumePowerSWKI' dialog.
	COneButtonDefinition *pclOneButtonDefinition = m_pclCurrentGroupDefinition->GetButtonDefinitionWithDlgID( iWhichInputDialog );

	if( NULL != pclOneButtonDefinition )
	{
		iWhichInputDialog = pclOneButtonDefinition->GetDlgWizePMRightViewInputID( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetNorm() );
	}

	pRViewWizardSelPM->AttachNewInputDialog( &m_clWizardSelPMParams, iWhichInputDialog, bBackEnable, bNextEnable );
}

void CDlgWizardSelPM::OnRViewWizSelPMInputChange( bool bErrorStatus )
{
	m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_eStatus = ( true == bErrorStatus ) ? BS_Error : BS_ToDefine;
	_ChangeButtonStates();
	_UpdateAllStatics();
}

void CDlgWizardSelPM::OnRViewWizSelPMVerifyFluidCharacteristics( bool &bEnable, CString &strMsg )
{
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg );
}

int CDlgWizardSelPM::OnRViewWizSelPMGetDlgWizePMRightViewInputID( int iDlgWizePMRightViewInputID )
{
	COneButtonDefinition *pclOneButtonDefinition = m_pclCurrentGroupDefinition->GetButtonDefinitionWithDlgID( iDlgWizePMRightViewInputID );

	if( NULL != pclOneButtonDefinition )
	{
		iDlgWizePMRightViewInputID = pclOneButtonDefinition->GetDlgWizePMRightViewInputID( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetNorm() );
	}

	return iDlgWizePMRightViewInputID;
}

void CDlgWizardSelPM::OnRViewSSelSelectProduct()
{
	// This method is called by the RView when user choose 'Select' item from the context menu.

	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	if( CMainFrame::eRVSSelPresureMaintenance == pMainFrame->GetCurrentRightView() )
	{
		if( false == m_clWizardSelPMParams.m_bEditModeRunning )
		{
			_Select( -1 );
		}
		else
		{
			_ApplyEdition( -1 );
		}
	}
}

void CDlgWizardSelPM::OnRViewSSelSelectionChanged( bool bSelectionActive )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	if( IDS_DLGWIZARDPM_RESULTS == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_iIDSText )
	{
		int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

		if( false == m_clWizardSelPMParams.m_bEditModeRunning )
		{
			m_pclCurrentGroupDefinition->ChangeButtonStatus( IDS_DLGWIZARDPM_SELECT, ( true == bSelectionActive ) ? BS_Doing : BS_ToDo );
		}
		else
		{
			m_pclCurrentGroupDefinition->ChangeButtonStatus( IDS_DLGWIZARDPM_APPLY, ( true == bSelectionActive ) ? BS_Doing : BS_ToDo );
		}

		_ChangeButtonStates();
		_UpdateAllStatics();
	}
}

void CDlgWizardSelPM::OnRViewSSelLostFocusWithTabKey( bool bShiftPressed )
{
	// TODO.
}

void CDlgWizardSelPM::OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey )
{
	// This method is called by the RView when user hit the 'Enter' key.

	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() || VK_RETURN != iKeyboardVirtualKey )
	{
		return;
	}

	if( CMainFrame::eRVSSelPresureMaintenance == pMainFrame->GetCurrentRightView() && NULL != pRViewSSelPM 
			&& true == pRViewSSelPM->IsSelectionAvailable() )
	{
		if( false == m_clWizardSelPMParams.m_bEditModeRunning )
		{
			_Select( -1 );
		}
		else
		{
			_ApplyEdition( -1 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgWizardSelPM, CDlgWizardSelBase )
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE( WM_USER_MODIFYSELECTEDWIZPM, OnModifySelectedPMProduct )
END_MESSAGE_MAP()

void CDlgWizardSelPM::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardSelBase::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT1, m_clButtonText1 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT2, m_clButtonText2 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT3, m_clButtonText3 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT4, m_clButtonText4 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT5, m_clButtonText5 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT6, m_clButtonText6 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT7, m_clButtonText7 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT8, m_clButtonText8 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT9, m_clButtonText9 );
	DDX_Control( pDX, IDC_WIZPMBUTTONTEXT10, m_clButtonText10 );

	DDX_Control( pDX, IDC_WIZPMBUTTONIMG1, m_clButtonImg1 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG2, m_clButtonImg2 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG3, m_clButtonImg3 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG4, m_clButtonImg4 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG5, m_clButtonImg5 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG6, m_clButtonImg6 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG7, m_clButtonImg7 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG8, m_clButtonImg8 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG9, m_clButtonImg9 );
	DDX_Control( pDX, IDC_WIZPMBUTTONIMG10, m_clButtonImg10 );

	DDX_Control( pDX, IDC_SEPARATOR1, m_clSeparator1 );
	DDX_Control( pDX, IDC_SEPARATOR2, m_clSeparator2 );
	DDX_Control( pDX, IDC_SEPARATOR3, m_clSeparator3 );
	DDX_Control( pDX, IDC_SEPARATOR4, m_clSeparator4 );
	DDX_Control( pDX, IDC_SEPARATOR5, m_clSeparator5 );
	DDX_Control( pDX, IDC_SEPARATOR6, m_clSeparator6 );
	DDX_Control( pDX, IDC_SEPARATOR7, m_clSeparator7 );
	DDX_Control( pDX, IDC_SEPARATOR8, m_clSeparator8 );
	DDX_Control( pDX, IDC_SEPARATOR9, m_clSeparator9 );
}

BOOL CDlgWizardSelPM::OnInitDialog() 
{
	CDlgWizardSelBase::OnInitDialog();

	for( int iLoop = 0; iLoop < m_arpButtonText.GetCount(); iLoop++ )
	{
		m_arpButtonText[iLoop]->SetFontSize( 12 );

		// Memorize width and top position for text and image buttons.
		CRect rectButtonTextPos;
		m_arpButtonText[iLoop]->GetWindowRect( &rectButtonTextPos );
		ScreenToClient( &rectButtonTextPos );
		m_arButtonTextWidthTop.Add( rectButtonTextPos );

		CRect rectButtonImgPos;
		m_arpButtonImg[iLoop]->GetWindowRect( &rectButtonImgPos );
		ScreenToClient( &rectButtonImgPos );
		m_arButtonImgWidthTop.Add( rectButtonImgPos );
	
		// Memorize width and top position for separators.
		if( iLoop < m_arpSeparator.GetCount() )
		{
			CRect rectSeparatorPos;
			m_arpSeparator[iLoop]->GetWindowRect( &rectSeparatorPos );
			ScreenToClient( &rectSeparatorPos );
			m_arSeparatorWidthTop.Add( rectSeparatorPos );
		}
	}

	return TRUE;
}

void CDlgWizardSelPM::OnSize( UINT nType, int cx, int cy )
{
	CDlgWizardSelBase::OnSize( nType, cx, cy );

	if( NULL == m_clButtonText1.GetSafeHwnd() )
	{
		// Not yet ready.
		return;
	}

	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;
	int iVisibleSeparatorNumber = iVisibleButtonNumber - 1;

	for( int iLoopStatic = 0; iLoopStatic < iVisibleButtonNumber; iLoopStatic++ )
	{
		CRect rectClient;
		GetClientRect( &rectClient );

		// If button is on the right, we move this one.
		if( BIP_Right == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eImagePosition )
		{
			int iNewX = rectClient.Width() - 4 - m_arButtonImgWidthTop[iLoopStatic].Width();
			m_arpButtonImg[iLoopStatic]->SetWindowPos( NULL, iNewX, m_arButtonImgWidthTop[iLoopStatic].top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
		}

		// Separator.
		if( iLoopStatic < iVisibleSeparatorNumber )
		{
			int iNewWidth = rectClient.Width() - m_arSeparatorWidthTop[iLoopStatic].left;
			m_arpSeparator[iLoopStatic]->SetWindowPos( NULL, -1, -1, iNewWidth, m_arSeparatorWidthTop[iLoopStatic].Height(), SWP_NOMOVE | SWP_NOZORDER );
		}
	}
}

void CDlgWizardSelPM::OnLButtonDown( UINT nFlags, CPoint point )
{
	CDlgWizardSelBase::OnLButtonDown( nFlags, point );

	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	for( int iLoopButton = 0; iLoopButton < iVisibleButtonNumber; iLoopButton++ )
	{
		if( point.y >= m_arButtonTextWidthTop[iLoopButton].top && point.y <= m_arButtonTextWidthTop[iLoopButton].bottom )
		{
			_OnBnClickedButton( iLoopButton );
			return;
		}
	}
}

LRESULT CDlgWizardSelPM::OnModifySelectedPMProduct( WPARAM wParam, LPARAM lParam )
{
	if( NULL == pDlgLTtabctrl || NULL == m_clWizardSelPMParams.m_pTADS || NULL == m_clWizardSelPMParams.m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( 0 );
	}

	CDS_SSelPMaint *pclSSelPMaint = dynamic_cast<CDS_SSelPMaint *>( ( CData * )lParam );
	ASSERT( NULL != pclSSelPMaint );

	if( NULL == pclSSelPMaint )
	{
		return 0;
	}

	CPMInputUser *pclPMInputUser = pclSSelPMaint->GetpInputUser();

	if( NULL == pclPMInputUser )
	{
		return 0;
	}

	// Verify that application type is available in the current database.
	if( pclPMInputUser->GetApplicationType() != ( m_clWizardSelPMParams.m_pTADS->GetpTechParams()->GetAvailablePMPanels() & pclPMInputUser->GetApplicationType() ) )
	{
		return 0;
	}

	// Keep a copy to detect changes by the user.
	m_clPMInputUserSaved.CopyFrom( pclPMInputUser );

	// Update the PM & WQ selection preferences structure in the current selection to edit.
	pMainFrame->UpdatePMWQSelectPrefsRibbon();

	m_bInitialised = false;

	// This methods will fire an active tab event and the 'ActivateLeftTabDialog' method will be called.
	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are in one other application mode, we need to change it.
	if( ProductSelectionMode_Wizard != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Wizard );
	}

	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_ExpansionVessel );

	// Select 'Heating, 'Cooling' or 'Solar'.
	pMainFrame->ChangeRbnProductSelectionApplicationType( pclPMInputUser->GetApplicationType() );

	m_clWizardSelPMParams.m_SelIDPtr = pclSSelPMaint->GetIDPtr();

	// Because we call the 'ClickRibbonSSelApplicationType' above, the application type is already known.
	CPMInputUser *pclGlobalPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, pclPMInputUser->GetApplicationType() );

	if( NULL != pclGlobalPMInputUser )
	{
		// Copy all variables from 'pclPMInputUser' that come from the current 'CDS_SSelPMaint' object to edit into the global corresponding
		// 'CPMInputUser' located in the 'CPMSElectionHelper'.
		pclGlobalPMInputUser->CopyFrom( pclPMInputUser );
	}

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, pclPMInputUser->GetApplicationType() );

	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( 0 );
	}
	
	// HYS-1537 : Update PM preferences
	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( &m_clWizardSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}

	m_clWizardSelPMParams.m_WC = *m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();

	// Copy also the water characteristic in the technical parameters because the 'CMainFrame::InitializeRibbonRichEditBoxesSSel' 
	// method uses it instead of the one in 'PMSelectionHelper'.
	if( NULL != m_clWizardSelPMParams.m_pTADS->GetpWCForProductSelection() )
	{
		CWaterChar *pTechDataWC = m_clWizardSelPMParams.m_pTADS->GetpWCForProductSelection()->GetpWCData();
		*pTechDataWC = m_clWizardSelPMParams.m_WC;
	}

	// Copy also additive name, PC weight, supply temperature and DT in the technical parameters because the main frame
	// uses these variables to update the fluid characteristics in the ribbon.
	CDS_TechnicalParameter *pclTechParams = m_clWizardSelPMParams.m_pTADS->GetpTechParams();
	pclTechParams->SetDefaultISTps( pclPMInputUser->GetSupplyTemperature() );

	if( ProjectType::Cooling == pclPMInputUser->GetApplicationType() )
	{
		pclTechParams->SetDefaultISDT( pclPMInputUser->GetReturnTemperature() - pclPMInputUser->GetSupplyTemperature() );
	}
	else
	{
		pclTechParams->SetDefaultISDT( pclPMInputUser->GetSupplyTemperature() - pclPMInputUser->GetReturnTemperature() );
	}

	pclTechParams->SetDefaultISAdditID( m_clWizardSelPMParams.m_WC.GetAdditID() );
	pclTechParams->SetDefaultISPcWeight( m_clWizardSelPMParams.m_WC.GetPcWeight() );

	// Remark: Water characteristics has been already copied above with 'pclGlobalPMInputUser->CopyFrom( pclPMInputUser )'.
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	SetModificationMode( true );

	// Go in the good step in regards to the object to edit.
	_UpdateForEditingSelection();

	// Need to be done before going to the result steps. Because 'RViewSSelPM::Suggest' will be called and this method
	// will call 'GetRViewNeededData' that needs the 'm_fInitialised' variable to be set to true.
	m_bInitialised = true;

	// Activate 'Apply changes' and 'Cancel button'.
	// Remark: do it before calling the '_GoToStep' method otherwise the two buttons will not be refreshed.
	_ChangeSelectOrApplyButtonsState( true );

	// Set to 'false' to force a refresh of the 'RViewSSelPM' view.
	m_bResultsAlreadyDisplayed = false;
	_GoToStep( IDS_DLGWIZARDPM_RESULTS );

	m_bResultsAlreadyDisplayed = true;

	return 0;
}

LRESULT CDlgWizardSelPM::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardSelBase::OnNewDocument( wParam, lParam );

	if( NULL == pRViewWizardSelPM || NULL == pRViewSSelPM )
	{
		ASSERTA_RETURN( -1 );
	}

	pRViewWizardSelPM->SetWizarSelectionParameters( &m_clWizardSelPMParams );
	pRViewWizardSelPM->RegisterNotificationHandler( this );

	pRViewSSelPM->UnregisterNotificationHandler( this );

	if( NULL == pDlgLeftTabSelManager || NULL == pDlgLeftTabSelManager->GetPMSelectionHelper() )
	{
		ASSERTA_RETURN( -1 );
	}

	// Retrieve the last application type used by the user.
	CDS_WizardSelParameter *pclWizardSelParameter = m_clWizardSelPMParams.m_pTADS->GetpWizardSelParameter();
	
	if( NULL == pclWizardSelParameter )
	{
		ASSERTA_RETURN( -1 );
	}

	ProjectType eProjectType = pclWizardSelParameter->GetPMApplicationType();

	// HYS-1350: We reset all to avoid previous errors.
	pDlgLeftTabSelManager->GetPMSelectionHelper()->Reset( ProductSelectionMode_Wizard, eProjectType );

	// HYS-1554 : On new document opening if we change the application type don't forget to adapt the wizard rview
	if( ( m_eWizardPMEngineState != WPMES_ProjectTypeChoice ) && ( m_eWizardPMEngineState != WPMES_ProjectTypeChoiceEdition ) 
		&& ( ( ProjectType::Heating == eProjectType &&  WPMES_ValueInputStepsHeating != m_eWizardPMEngineState )
		|| ( ProjectType::Cooling == eProjectType && WPMES_ValueInputStepsCooling != m_eWizardPMEngineState )
		|| ( ProjectType::Solar == eProjectType && WPMES_ValueInputStepsSolar != m_eWizardPMEngineState ) ) )
	{
		if( ProjectType::Heating == eProjectType )
		{
			m_eWizardPMEngineState = WPMES_ValueInputStepsHeating;
		}
		else if( ProjectType::Cooling == eProjectType )
		{
			m_eWizardPMEngineState = WPMES_ValueInputStepsCooling;
		}
		else if( ProjectType::Solar == eProjectType )
		{
			m_eWizardPMEngineState = WPMES_ValueInputStepsSolar;
		}
		m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];
		m_iCurrentButton = 1;

		// Call action on the first button to force to have the good right view input.
		pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_ptrfActionToDo;
		( this->*fActionToDo )( m_iCurrentButton );
	}

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, eProjectType );

	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( -1 );
	}

	// Copy also water characteristic in 'CPMInputUser'.
	*( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC() ) = m_clWizardSelPMParams.m_WC;

	m_bResultsAlreadyDisplayed = false;
	m_bInitialised = true;

	// HYS-1537 : Update PM preferences
	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( &m_clWizardSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}
	// HYS-1350: To reset values in input fields.
	_ApplyPMInputUserChangedInRightView();

	return 0;
}

LRESULT CDlgWizardSelPM::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == pDlgLeftTabSelManager || this != pDlgLeftTabSelManager->GetCurrentLeftTabDialog() 
			|| NULL == pRViewWizardSelPM )
	{
		return 0;
	}

	CDlgWizardSelBase::OnTechParChange( wParam, lParam );

	if( WPMES_ProjectTypeChoice == m_eWizardPMEngineState || WPMES_ProjectTypeChoiceEdition == m_eWizardPMEngineState 
			|| WPMES_ValueInputStepsWaterMakeUp == m_eWizardPMEngineState || WPMES_ValueInputStepsDegassing == m_eWizardPMEngineState )
	{
		return 0;
	}

	/*
	if( CMainFrame::eRVSSelPresureMaintenance == pMainFrame->GetCurrentRightView() )
	{
		// If user was in the result page (with the 'RViewSSelPM' view active) and change the application type with the ribbon, we need
		// to reset to the 'RViewWizardSelPM' view.
		pMainFrame->ActiveFormView( CMainFrame::eRVWizardSelPM );

		// Don't want to receive notification from the 'RViewSSelPM' view when this one is no more the active one.
		if( NULL != pRViewSSelPM )
		{
			pRViewSSelPM->UnregisterNotificationHandler( this );
		}
	}

	ChangeApplicationType( m_pclWizardSelParams->m_pTADS->GetpTechParams()->GetProductSelectionApplicationType() );
	*/

	// Before calling the following method, we set this variable to 'false' to force a refresh of the result if we are at this step.
	// Remark: if we are at the result page, the result are cleared by 'CDlgIndSelPressureMaintenance::OnWaterChange'.
	m_bResultsAlreadyDisplayed = false;

	_ApplyPMInputUserChangedInRightView();

	return 0;
}

LRESULT CDlgWizardSelPM::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == pDlgLeftTabSelManager || this != pDlgLeftTabSelManager->GetCurrentLeftTabDialog() )
	{
		return 0;
	}

	if( NULL == m_pclCurrentGroupDefinition )
	{
		return 0;
	}

	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	// Call base class.
	CDlgWizardSelBase::OnWaterChange( wParam, lParam );

	// Check if user has modified the water characteristic.
	bool bUpdateWaterChar = false;

	if( WMUserWaterCharLParam::WM_UWC_LWP_Change == lParam )
	{
		bUpdateWaterChar = true;
	}

	bool bUpdateEditField = false;
	CDS_TechnicalParameter *pTechParam = m_clWizardSelPMParams.m_pTADS->GetpTechParams();
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		switch( m_eWizardPMEngineState )
		{
			case WPMES_ValueInputStepsHeating:
			case WPMES_ValueInputStepsHeatingEdition:
				pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, ProjectType::Heating );
				pclPMInputUser->SetSupplyTemperature( pTechParam->GetDefaultISHeatingTps() );
				pclPMInputUser->SetReturnTemperature( pTechParam->GetDefaultISHeatingTps() - pTechParam->GetDefaultISHeatingDT() );
				*pclPMInputUser->GetpWC() = m_clWizardSelPMParams.m_WC;
				bUpdateEditField = true;
				break;

			case WPMES_ValueInputStepsCooling:
			case WPMES_ValueInputStepsCoolingEdition:
				pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, ProjectType::Cooling );
				pclPMInputUser->SetSupplyTemperature( pTechParam->GetDefaultISCoolingTps() );
				pclPMInputUser->SetReturnTemperature( pTechParam->GetDefaultISCoolingTps() + pTechParam->GetDefaultISCoolingDT() );
				*pclPMInputUser->GetpWC() = m_clWizardSelPMParams.m_WC;

				if( false == pclPMInputUser->GetCheckMinTemperature() )
				{
					pclPMInputUser->SetMinTemperature( m_clWizardSelPMParams.m_WC.GetTfreez() );
				}

				bUpdateEditField = true;
				break;

			case WPMES_ValueInputStepsSolar:
			case WPMES_ValueInputStepsSolarEdition:
				pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, ProjectType::Solar );
				pclPMInputUser->SetSupplyTemperature( pTechParam->GetDefaultISSolarTps() );
				pclPMInputUser->SetReturnTemperature( pTechParam->GetDefaultISSolarTps() - pTechParam->GetDefaultISSolarDT() );
				*pclPMInputUser->GetpWC() = m_clWizardSelPMParams.m_WC;
				bUpdateEditField = true;
				break;
		}
	}

	if( true == bUpdateEditField )
	{
		m_bResultsAlreadyDisplayed = false;
		_ApplyPMInputUserChangedInRightView( bUpdateWaterChar );
	}

	return 0;
}

void CDlgWizardSelPM::LeaveLeftTabDialog()
{
	// Don't want to receive anymore notification from the 'RViewSSelPM' view.
	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->UnregisterNotificationHandler( this );
	}

	// Save the last data input by the user in the current right input value dialog.
	if( NULL != pRViewWizardSelPM )
	{
		pRViewWizardSelPM->FillPMInputUser();
	}

	if( true == m_bResultsAlreadyDisplayed && NULL != m_clWizardSelPMParams.m_pclSelectPMList 
			&& false == m_clPMInputUserSaved.CompareTo( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() ) )
	{
		// Check if data has changed since the last time the results were displayed.
		m_bResultsAlreadyDisplayed = false;
	}

	// And keep a copy.
	if( NULL != m_clWizardSelPMParams.m_pclSelectPMList )
	{
		m_clPMInputUserSaved.CopyFrom( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() );
	}

	// Set that we use variables in 'CPMSelectionHelper' with the wizard selection mode.
	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->GetPMSelectionHelper()->SetLastSelectionMode( CPMSelectionHelper::LSM_Wizard );
	}
}

void CDlgWizardSelPM::ActivateLeftTabDialog()
{
	CDlgWizardSelBase::ActivateLeftTabDialog();

	if( CPMSelectionHelper::LSM_Individual == pDlgLeftTabSelManager->GetPMSelectionHelper()->GetLastSelectionMode() )
	{
		_UpdateWithIndividualSelPMState();
		// Copy all individual PM prefs in all mode
		_UpdateWithIndividualAllPrefInAllMode();
	}

	if( ( WPMES_ProjectTypeChoice != m_eWizardPMEngineState && WPMES_ProjectTypeChoiceEdition != m_eWizardPMEngineState ) || -1 == m_iCurrentButton )
	{
		m_eWizardPMEngineState = ( false == m_clWizardSelPMParams.m_bEditModeRunning ) ? WPMES_ProjectTypeChoice : WPMES_ProjectTypeChoiceEdition;
		ProjectType eIndSelPMProjectType = m_clWizardSelPMParams.m_pTADS->GetpIndSelParameter()->GetPMApplicationType();
		ChangeApplicationType( eIndSelPMProjectType );
		m_clPMInputUserSaved.CopyFrom( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() );
	}
}

void CDlgWizardSelPM::ChangeApplicationType( ProjectType eProjectType )
{
	CDlgWizardSelBase::ChangeApplicationType( eProjectType );

	if( ProjectType::Heating == eProjectType && 
			( WPMES_ValueInputStepsHeating == m_eWizardPMEngineState || WPMES_ValueInputStepsHeatingEdition == m_eWizardPMEngineState ) )
	{
		// If we are already in heating mode, we do nothing.
		return;
	}
	else if( ProjectType::Cooling == eProjectType && 
			( WPMES_ValueInputStepsCooling == m_eWizardPMEngineState || WPMES_ValueInputStepsCoolingEdition == m_eWizardPMEngineState ) )
	{
		// If we are already in cooling mode, we do nothing.
		return;
	}
	else if( ProjectType::Solar == eProjectType && 
			( WPMES_ValueInputStepsSolar == m_eWizardPMEngineState || WPMES_ValueInputStepsSolarEdition == m_eWizardPMEngineState ) )
	{
		// If we are already in solar mode, we do nothing.
		return;
	}

	if( WPMES_ProjectTypeChoice != m_eWizardPMEngineState && WPMES_ProjectTypeChoiceEdition != m_eWizardPMEngineState 
			&& IDS_DLGWIZARDPM_RESULTS == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_iIDSText )
	{
		// If user was in the result page (with the 'RViewSSelPM' view active) and change the application type, we need
		// to reset to the 'RViewWizardSelPM' view.
		pMainFrame->ActiveFormView( CMainFrame::eRVWizardSelPM );

		// Don't want to receive anymore notification from the 'RViewSSelPM' view.
		if( NULL != pRViewSSelPM )
		{
			pRViewSSelPM->UnregisterNotificationHandler( this );
		}
	}
	
	int iNewGroupDefinition = -1;
	pfActionToDo fnActionToDo = NULL;

	if( false == m_clWizardSelPMParams.m_bEditModeRunning )
	{
		if( ProjectType::Heating == eProjectType && WPMES_ValueInputStepsHeating != m_eWizardPMEngineState )
		{
			iNewGroupDefinition = WPMES_ValueInputStepsHeating;
			fnActionToDo = &CDlgWizardSelPM::_ProjectTypeChoice_GoInHeating;
		}
		else if( ProjectType::Cooling == eProjectType && WPMES_ValueInputStepsCooling != m_eWizardPMEngineState )
		{
			iNewGroupDefinition = WPMES_ValueInputStepsCooling;
			fnActionToDo = &CDlgWizardSelPM::_ProjectTypeChoice_GoInCooling;
		}
		else if( ProjectType::Solar == eProjectType && WPMES_ValueInputStepsSolar != m_eWizardPMEngineState )
		{
			iNewGroupDefinition = WPMES_ValueInputStepsSolar;
			fnActionToDo = &CDlgWizardSelPM::_ProjectTypeChoice_GoInSolar;
		}
	}
	else
	{
		if( ProjectType::Heating == eProjectType && WPMES_ValueInputStepsHeatingEdition != m_eWizardPMEngineState )
		{
			iNewGroupDefinition = WPMES_ValueInputStepsHeatingEdition;
			fnActionToDo = &CDlgWizardSelPM::_ProjectTypeChoice_GoInHeatingEdition;
		}
		else if( ProjectType::Cooling == eProjectType && WPMES_ValueInputStepsCoolingEdition != m_eWizardPMEngineState )
		{
			iNewGroupDefinition = WPMES_ValueInputStepsCoolingEdition;
			fnActionToDo = &CDlgWizardSelPM::_ProjectTypeChoice_GoInCoolingEdition;
		}
		else if( ProjectType::Solar == eProjectType && WPMES_ValueInputStepsSolarEdition != m_eWizardPMEngineState )
		{
			iNewGroupDefinition = WPMES_ValueInputStepsSolarEdition;
			fnActionToDo = &CDlgWizardSelPM::_ProjectTypeChoice_GoInSolarEdition;
		}
	}
	
	if( NULL != fnActionToDo )
	{
		// Must be the first call because these method will save data input by user.
		pRViewWizardSelPM->DetachCurrentInputDialog();

		// Even in the switching of the context of 'm_pclSelectPMList' is done when calling below the 'fnActionToDo' method,
		// we need at least to point on the good variables to call 'IsAtLeastOneError'.
		m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, eProjectType );
	
		if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
		{
			ASSERT_RETURN;
		}

		// Copy also water characteristic from 'CPMInputUser' to the local 'm_WC' variable.
		m_clWizardSelPMParams.m_WC = *m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();
		// HYS-1537 : Update PM preferences
		if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( &m_clWizardSelPMParams ) )
		{
			pMainFrame->UpdatePMWQSelectPrefsRibbon();
		}

		// Check now if there are errors following this update.
		// Because when calling 'fbActionToDo' we will call the 'CRViewWizardSelPM::AttachNewInputDialog' method that will use errors
		// to check if we need to go in advanced mode or not.
		COneGroupDefinition *pclNewGroupDefinition = m_clAllGroupDefinitions[iNewGroupDefinition];
		PressurisationNorm eNorm = m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetNorm();

		for( int iLoop = 0; iLoop < pclNewGroupDefinition->m_iVisibleButtonNumber; iLoop++ )
		{
			int iRightViewInputID = pclNewGroupDefinition->m_arOneGroupDefinition[iLoop]->GetDlgWizePMRightViewInputID( eNorm );

			if( -1 == iRightViewInputID )
			{
				continue;
			}

			ButtonStatus eButtonStatus = ( true == pRViewWizardSelPM->IsAtLeastOneError( iRightViewInputID ) ) ? BS_Error : BS_ToDefine;
			pclNewGroupDefinition->m_arOneGroupDefinition[iLoop]->m_eStatus = eButtonStatus;
		}

		// Switch now to the right application type.
		(this->*fnActionToDo)( -1 );

		// Change buttons in the left.
		_ChangeButtonStates();
		_UpdateAllStatics();
	}

	// Update the 'CDS_WizardSelParameter' in technical parameters to keep a trace of the user choice when he will 
	// restart HySelect.
	m_clWizardSelPMParams.m_pTADS->GetpWizardSelParameter()->SetPMApplicationType( eProjectType );
}

void CDlgWizardSelPM::OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg, pclDlgWaterChar );
}

bool CDlgWizardSelPM::LeftTabKillFocus( bool bNext )
{
	bool bReturn = false;

	if( CMainFrame::eRVWizardSelPM == GetRViewID() )
	{
		// TAB -> go to the top control only if right view is empty.
		if( NULL != pRViewWizardSelPM && TRUE == pRViewWizardSelPM->IsWindowVisible() && false == pRViewWizardSelPM->IsEmpty() )
		{
			// Set the focus on the right view.
			pRViewWizardSelPM->SetFocus();
			// Set focus on the appropriate group in the right view.
			pRViewWizardSelPM->SetFocusW( bNext );
			bReturn = true;
		}
	}
	else
	{
		// TAB -> go to the top control only if right view is empty.
		if( NULL != pRViewSSelPM && TRUE == pRViewSSelPM->IsWindowVisible() && false == pRViewSSelPM->IsEmpty() )
		{
			// Set the focus on the right view.
			pRViewSSelPM->SetFocus();
			// Set focus on the appropriate group in the right view.
			pRViewSSelPM->SetFocusW( bNext );
			bReturn = true;
		}
	}

	return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgWizardSelPM::_UpdateForEditingSelection()
{
	if( NULL == pDlgLeftTabSelManager || NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Remark: if notification handler is already registered, the 'CRViewSSelSS' class does nothing.
	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );
	}

	if( ProjectType::Heating == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() )
	{
		if( false == m_clWizardSelPMParams.m_bEditModeRunning )
		{
			_ProjectTypeChoice_GoInHeating( -1 );
		}
		else
		{
			_ProjectTypeChoice_GoInHeatingEdition( -1 );
		}
	}
	else if( ProjectType::Cooling == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() )
	{
		if( false == m_clWizardSelPMParams.m_bEditModeRunning )
		{
			_ProjectTypeChoice_GoInCooling( -1 );
		}
		else
		{
			_ProjectTypeChoice_GoInCoolingEdition( -1 );
		}
	}
	else if( ProjectType::Solar == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() )
	{
		if( false == m_clWizardSelPMParams.m_bEditModeRunning )
		{
			_ProjectTypeChoice_GoInSolar( -1 );
		}
		else
		{
			_ProjectTypeChoice_GoInSolarEdition( -1 );
		}
	}
}

void CDlgWizardSelPM::_OnBnClickedButton( int iWhichButton )
{
	if( WPMES_ProjectTypeChoice != m_eWizardPMEngineState && WPMES_ProjectTypeChoiceEdition != m_eWizardPMEngineState 
			&& iWhichButton == m_iCurrentButton )
	{
		// Except for the first step where we have to choose between heating, solar and so on, for other steps we do nothing
		// if user clicks on the same button as the current one.
		return;
	}
	
	if( WPMES_ProjectTypeChoice != m_eWizardPMEngineState && WPMES_ProjectTypeChoiceEdition != m_eWizardPMEngineState
			&& IDS_DLGWIZARDPM_RESULTS == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_iIDSText )
	{
		if( iWhichButton < m_iCurrentButton )
		{
			// If user was in the result page (with the 'RViewSSelPM' view active) and clicks on button above to change input value, we need
			// to reset to the 'RViewWizardSelPM' view.
			// We have also the case when user is in edition mode and clicks on the 'Cancel' button that is after the 'Results' button. This is why we add 
			// the test with 'IDS_DLGWIZARDPM_CANCEL'. And 'IDS_DLGWIZARDPM_APPLY' in the case user clicks on the 'Apply' button.
			pMainFrame->ActiveFormView( CMainFrame::eRVWizardSelPM );

			// Don't want to receive anymore notification from the 'RViewSSelPM' view.
			if( NULL != pRViewSSelPM )
			{
				pRViewSSelPM->UnregisterNotificationHandler( this );
			}

			_ChangeSelectOrApplyButtonsState( false );
		}
		else if( IDS_DLGWIZARDPM_CANCEL == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iWhichButton]->m_iIDSText 
			|| IDS_DLGWIZARDPM_APPLY == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iWhichButton]->m_iIDSText )
		{
		}
	}

	pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iWhichButton]->m_ptrfActionToDo;
	(this->*fActionToDo)( iWhichButton );

	_ChangeButtonStates();
	_UpdateAllStatics();
}

void CDlgWizardSelPM::_UpdateAllStatics( void )
{
	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;
	int iVisibleSeparatorNumber = iVisibleButtonNumber - 1;

	for( int iLoopStatic = 0; iLoopStatic < m_arpButtonText.GetCount(); iLoopStatic++ )
	{
		// Button text and image.
		if( iLoopStatic < iVisibleButtonNumber )
		{
			m_arpButtonText[iLoopStatic]->EnableWindow( TRUE );
			m_arpButtonText[iLoopStatic]->ShowWindow( SW_SHOW );

			CString strText = TASApp.LoadLocalizedString( m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_iIDSText );
			m_arpButtonText[iLoopStatic]->SetWindowText( strText );

			// For text color we have to do that in the 'OnCtlColor' method.

			if( BIP_None != m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eImagePosition )
			{
				m_arpButtonImg[iLoopStatic]->EnableWindow( TRUE );
				m_arpButtonImg[iLoopStatic]->ShowWindow( SW_SHOW );
				m_arpButtonImg[iLoopStatic]->SetIcon( m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_hIcon );
			}

			// Button image position.
			if( BIP_Right == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eImagePosition )
			{
				CRect rectClient;
				GetClientRect( &rectClient );

				int iNewX = rectClient.Width() - 4 - m_arButtonImgWidthTop[iLoopStatic].Width();
				m_arpButtonImg[iLoopStatic]->SetWindowPos( NULL, iNewX, m_arButtonImgWidthTop[iLoopStatic].top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
			}
			else if( BIP_Left == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eImagePosition )
			{
				m_arpButtonImg[iLoopStatic]->SetWindowPos( NULL, 4, m_arButtonImgWidthTop[iLoopStatic].top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
			}

			// Button text position.
			if( BIP_Right == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eImagePosition || 
				BIP_None == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eImagePosition )
			{
				// If image is at the right, text is at the left.
				m_arpButtonText[iLoopStatic]->SetWindowPos( NULL, 4, m_arButtonTextWidthTop[iLoopStatic].top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
			}
			else
			{
				// If image is at the left, text is at the right.
				int iNewX = 4 + m_arButtonImgWidthTop[iLoopStatic].Width() + 4;
				m_arpButtonText[iLoopStatic]->SetWindowPos( NULL, iNewX, m_arButtonTextWidthTop[iLoopStatic].top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
			}
		}
		else
		{
			m_arpButtonText[iLoopStatic]->EnableWindow( FALSE );
			m_arpButtonText[iLoopStatic]->ShowWindow( SW_HIDE );
			m_arpButtonText[iLoopStatic]->SetWindowText( _T("") );

			m_arpButtonImg[iLoopStatic]->EnableWindow( FALSE );
			m_arpButtonImg[iLoopStatic]->ShowWindow( SW_HIDE );
		}

		// Separator.
		if( iLoopStatic < m_arpSeparator.GetCount() )
		{
			if( iLoopStatic < iVisibleSeparatorNumber )
			{
				m_arpSeparator[iLoopStatic]->EnableWindow( TRUE );
				m_arpSeparator[iLoopStatic]->ShowWindow( SW_SHOW );
			}
			else
			{
				m_arpSeparator[iLoopStatic]->EnableWindow( FALSE );
				m_arpSeparator[iLoopStatic]->ShowWindow( SW_HIDE );
			}
		}
	}

	// If new buttons or separators are now visible, we need to adapt width.
	CRect rect;
	GetClientRect( &rect );

	SendMessage( WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM( rect.Width(), rect.Height() ) );
	
	_UpdateDialogHeight();

	// To force a refresh.
	Invalidate( TRUE );
	UpdateWindow();
}

void CDlgWizardSelPM::_ChangeButtonStates()
{
	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	for( int iLoopStatic = 0; iLoopStatic < iVisibleButtonNumber; iLoopStatic++ )
	{
		if( iLoopStatic < iVisibleButtonNumber )
		{
			if( BIP_None == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eImagePosition )
			{
				// If it's a button with only label and no image, we do nothing with it (it is managed elsewhere).
				// For example the 'Select' button.
				continue;
			}

			if( BS_Error == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eStatus )
			{
				// Do not change if the button is in an error state.
				m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_hIcon = m_hIconButtonError;
				continue;
			}

			if( WPMES_ProjectTypeChoice == m_eWizardPMEngineState || WPMES_ProjectTypeChoiceEdition == m_eWizardPMEngineState )
			{
				// For project type panel, all buttons are in 'Doing' state.
				// m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eStatus = BS_Doing;
				// m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_hIcon = m_hIconButtonToDo;
			}
			else
			{
				if( 0 == iLoopStatic )
				{
					// For Heating, Solar and so on, the first button is the back button to the project type choice.
					// We let it in 'To do' state.
					m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eStatus = BS_Doing;
					m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_hIcon = m_hIconButtonLeft;
				}
				else  if( iLoopStatic < m_iCurrentButton )
				{
					// For buttons previous to the current one, there are in 'Done' state.
					m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eStatus = BS_Done;
					m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_hIcon = m_hIconButtonDone;
				}
				else if( iLoopStatic == m_iCurrentButton )
				{
					// For the current one, this is the 'Doing' state.
					m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eStatus = BS_Doing;
					m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_hIcon = m_hIconButtonDoing;
				}
				else
				{
					// For buttons following the current one, there are in 'ToDo' state.
					m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_eStatus = BS_ToDo;
					m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoopStatic]->m_hIcon = m_hIconButtonToDo;
				}
			}
		}
	}
}

bool  CDlgWizardSelPM::_BackToProjectType( int iWhichButton )
{
	if( NULL == pRViewWizardSelPM )
	{
		return false;
	}

	// Remark: when destroying dialog, this one can yet receive OnKillFocus on an edit field that was in edition mode.
	// If for example it's a field concerning temperature, there is a verification of the value. And if there is an error,
	// a message will be sent to this view. That allows us to prevent 'CDlgWizardSelPM' that there is error for the current input view.
	pRViewWizardSelPM->DetachCurrentInputDialog();

	m_clWizardSelPMParams.m_pclSelectPMList = NULL;
	m_eWizardPMEngineState = WPMES_ProjectTypeChoice;
	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];
	m_iCurrentButton = 0;

	pRViewWizardSelPM->AttachNewInputDialog( &m_clWizardSelPMParams, CDlgWizardPM_RightViewInput_Base::ProjectType, false, false );

	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInHeating( int iWhichButton )
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERTA_RETURN( false );
	}

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, ProjectType::Heating );

	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// Copy also water characteristic from 'CPMInputUser' to the local 'm_WC' variable.
	m_clWizardSelPMParams.m_WC = *m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();

	m_eWizardPMEngineState = WPMES_ValueInputStepsHeating;
	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];
	m_iCurrentButton = 1;
	// HYS-1537 : Update PM preferences
	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( &m_clWizardSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}

	// Call action on the first button to force to have the good right view input.
	pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_ptrfActionToDo;
 	(this->*fActionToDo)( m_iCurrentButton );
	
	// Notify the mainframe to change value in the ribbon.
	// Why if 'iWhichButton' is different of -1? Because if -1 it means that the call comes from outside (when user 
	// change application type directly in the ribbon). In this case we don't need to send the message.
	if( -1 != iWhichButton )
	{
		// HYS-1554 : When the application type is changed by clicking on the left button, the new value is not saved
		// in datastruct.
		// Update the 'CDS_WizardSelParameter' 
		m_clWizardSelPMParams.m_pTADS->GetpWizardSelParameter()->SetPMApplicationType( ProjectType::Heating );
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_APPLICATIONTYPECHANGE, ( WPARAM )Heating );
	}

	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInCooling( int iWhichButton )
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERTA_RETURN( false );
	}

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, ProjectType::Cooling );
	
	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// Copy also water characteristic from 'CPMInputUser' to the local 'm_WC' variable.
	m_clWizardSelPMParams.m_WC = *m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();

	m_eWizardPMEngineState = WPMES_ValueInputStepsCooling;
	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];
	m_iCurrentButton = 1;

	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( &m_clWizardSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}

	// Call action on the first button to force to have the good right view input.
	pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_ptrfActionToDo;
 	(this->*fActionToDo)( m_iCurrentButton );
	
	// Notify the mainframe to change value in the ribbon.
	// Why if 'iWhichButton' is different of -1? Because if -1 it means that the call comes from outside (when user 
	// change application type directly in the ribbon). In this case we don't need to send the message.
	if( -1 != iWhichButton )
	{
		// HYS-1554 : When the application type is changed by clicking on the left button, the new value is not saved
		// in datastruct.
		// Update the 'CDS_WizardSelParameter'.
		m_clWizardSelPMParams.m_pTADS->GetpWizardSelParameter()->SetPMApplicationType( ProjectType::Cooling );
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_APPLICATIONTYPECHANGE, ( WPARAM )Cooling );
	}

	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInSolar( int iWhichButton )
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERTA_RETURN( false );
	}

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, ProjectType::Solar );

	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// Copy also water characteristic from 'CPMInputUser' to the local 'm_WC' variable.
	m_clWizardSelPMParams.m_WC = *m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();

	m_eWizardPMEngineState = WPMES_ValueInputStepsSolar;
	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];	
	m_iCurrentButton = 1;
    // HYS-1537 : Update PM preferences
	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( &m_clWizardSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}


	// Call action on the first button to force to have the good right view input.
	pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_ptrfActionToDo;
 	(this->*fActionToDo)( m_iCurrentButton );
	
	// Notify the mainframe to change value in the ribbon.
	// Why if 'iWhichButton' is different of -1? Because if -1 it means that the call comes from outside (when user 
	// change application type directly in the ribbon). In this case we don't need to send the message.
	if( -1 != iWhichButton )
	{
		// HYS-1554 : When the application type is changed by clicking on the left button, the new value is not saved
		// datastruct.
		// Update the 'CDS_WizardSelParameter'.
		m_clWizardSelPMParams.m_pTADS->GetpWizardSelParameter()->SetPMApplicationType( ProjectType::Solar );
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_APPLICATIONTYPECHANGE, ( WPARAM )Solar );
	}

	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInWaterMakeUpOnly( int iWhichButton )
{
	// TODO

	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInDegassingOnly( int iWhichButton )
{
	// TODO

	return true;
}

bool CDlgWizardSelPM::_BackToProjectTypeEdition( int iWhichButton )
{
	if( NULL == pRViewWizardSelPM )
	{
		return false;
	}

	// Remark: when destroying dialog, this one can yet receive OnKillFocus on an edit field that was in edition mode.
	// If for example it's a field concerning temperature, there is a verification of the value. And if there is an error,
	// a message will be sent to this view. That allows us to prevent 'CDlgWizardSelPM' that there is error for the current input view.
	pRViewWizardSelPM->DetachCurrentInputDialog();

	m_clWizardSelPMParams.m_pclSelectPMList = NULL;
	m_eWizardPMEngineState = WPMES_ProjectTypeChoiceEdition;
	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];
	m_iCurrentButton = 0;

	pRViewWizardSelPM->AttachNewInputDialog( &m_clWizardSelPMParams, CDlgWizardPM_RightViewInput_Base::ProjectType, false, false );

	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInHeatingEdition( int iWhichButton )
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERTA_RETURN( false );
	}

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, ProjectType::Heating );

	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// Copy also water characteristic from 'CPMInputUser' to the local 'm_WC' variable.
	m_clWizardSelPMParams.m_WC = *m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();

	m_eWizardPMEngineState = WPMES_ValueInputStepsHeatingEdition;
	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];
	m_iCurrentButton = 1;

	// Call action on the first button to force to have the good right view input.
	pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_ptrfActionToDo;
 	(this->*fActionToDo)( m_iCurrentButton );
	
	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInCoolingEdition( int iWhichButton )
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERTA_RETURN( false );
	}

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, ProjectType::Cooling );

	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// Copy also water characteristic from 'CPMInputUser' to the local 'm_WC' variable.
	m_clWizardSelPMParams.m_WC = *m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();

	m_eWizardPMEngineState = WPMES_ValueInputStepsCoolingEdition;
	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];
	m_iCurrentButton = 1;

	// Call action on the first button to force to have the good right view input.
	pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_ptrfActionToDo;
 	(this->*fActionToDo)( m_iCurrentButton );
	
	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInSolarEdition( int iWhichButton )
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERTA_RETURN( false );
	}

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, ProjectType::Solar );

	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// Copy also water characteristic from 'CPMInputUser' to the local 'm_WC' variable.
	m_clWizardSelPMParams.m_WC = *m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();

	m_eWizardPMEngineState = WPMES_ValueInputStepsSolarEdition;
	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];	
	m_iCurrentButton = 1;

	// Call action on the first button to force to have the good right view input.
	pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_ptrfActionToDo;
 	(this->*fActionToDo)( m_iCurrentButton );
	
	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInWaterMakeUpOnlyEdition( int iWhichButton )
{
	// TODO

	return true;
}

bool CDlgWizardSelPM::_ProjectTypeChoice_GoInDegassingOnlyEdition( int iWhichButton )
{
	// TODO

	return true;
}

bool CDlgWizardSelPM::_RightViewValuesInputDialogToChange( int iWhichButton )
{
	if( NULL == pRViewWizardSelPM )
	{
		return false;
	}

	pRViewWizardSelPM->DetachCurrentInputDialog();
	
	m_iCurrentButton = iWhichButton;
	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	bool bBackEnable = true;
	bool bNextEnable = true;

	if( m_iCurrentButton == iVisibleButtonNumber )
	{
		bNextEnable = false;
	}

	PressurisationNorm eNorm = m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetNorm();
	pRViewWizardSelPM->AttachNewInputDialog( &m_clWizardSelPMParams, 
			m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->GetDlgWizePMRightViewInputID( eNorm ), bBackEnable, bNextEnable );

	return true;
}

bool CDlgWizardSelPM::_GoToResultsStep( int iWhichButton )
{
	if( NULL == pRViewSSelPM || NULL == m_clWizardSelPMParams.m_pclSelectPMList )
	{
		return false;
	}

	// Check first if there is no error in input.
	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	for( int iLoop = 0; iLoop < iVisibleButtonNumber; iLoop++ )
	{
		if( BS_Error == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoop]->m_eStatus )
		{
			CString str = TASApp.LoadLocalizedString( IDS_DLGWIZARDSELPM_ERROREXIST );
			AfxMessageBox( str, MB_OK );
			return false;
		}
	}

	// HYS-1041: This case is not allowed for wizard selection
	if( false == _VerifyFluidCharacteristics( true ) )
	{
		return false;
	}

	// Detach the last input value dialog to be sure to have the last data input by user.
	if( NULL != pRViewWizardSelPM && ( NULL == pMainFrame->GetActiveView() ) )
	{
		pRViewWizardSelPM->DetachCurrentInputDialog();
	}

	// Need to be called just after the call to the 'DetachCurrentInputDialog' method above. Because if user is writing a
	// value, when detaching the dialog a kill focus will be fired and it's only at this moment that the value will be saved.
	bool bEqualInputUser = m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->CompareTo( &m_clPMInputUserSaved );
	m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetWC( m_clWizardSelPMParams.m_WC );

	if( false == bEqualInputUser
			|| false == m_bResultsAlreadyDisplayed )
	{
		// If user has changed values since he is in the wizard mode, we need to clear previous results and call
		// the 'Select' method.

		m_clPMInputUserSaved.CopyFrom( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() );
		int iDevFound = m_clWizardSelPMParams.m_pclSelectPMList->Select();

		if( 0 != iDevFound )
		{
			m_bRViewEmpty = false;
			VerifyModificationMode();
		
			// Remark: if notification handler is already registered, the 'CRViewSSelSS' class does nothing.
			pRViewSSelPM->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );

			pMainFrame->ActiveFormView( CMainFrame::eRVSSelPresureMaintenance );

			_CopyWizardSelParamsToIndSelParams();
			pRViewSSelPM->Suggest( &m_clIndSelPMParams );

			// Check if selection is done or not.
			// Remark: it is possible in the case of the edition mode. 'RViewSSelPM' will suggest selection and choose
			//         the right one if found. In this case we need to enable the 'Apply' .
			_ChangeSelectOrApplyButtonsState( pRViewSSelPM->IsSelectionAvailable() );
		}
		else
		{
			//TODO: display a message
		}
	}
	else
	{
		// Otherwise, there is no change. We just display the 'RViewSSelPM' page without modification.
		pRViewSSelPM->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );
		pMainFrame->ActiveFormView( CMainFrame::eRVSSelPresureMaintenance );

		// Check if selection is done or not.
		_ChangeSelectOrApplyButtonsState( pRViewSSelPM->IsSelectionAvailable() );
	}

	m_bResultsAlreadyDisplayed = true;
	m_iCurrentButton = iWhichButton;
	return true;
}

bool CDlgWizardSelPM::_Select( int iWhichButton )
{
	try
	{
		if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_pclCurrentGroupDefinition )
		{
			return false;
		}

		// If 'iWhichButton' is '-1' it means that it's not the user that intentionally press the button but either the user has
		// chosen the 'Select' option in the right view context menu or has hit the 'Enter' key.
		if( -1 != iWhichButton && BS_Disable == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iWhichButton]->m_eStatus )
		{
			// If button is not enabled, we can't select for now.
			return false;
		}

		IDPTR IDPtr;
		m_clWizardSelPMParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPMaint ) );
		CDS_SSelPMaint *pclSSelPMaint = ( CDS_SSelPMaint * )( IDPtr.MP );

		// Must be done before calling 'FillInSelected' because this method uses some variables in it.
		pclSSelPMaint->GetpInputUser()->CopyFrom( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() );

		*pclSSelPMaint->GetpSelectedInfos()->GetpWCData() = m_clWizardSelPMParams.m_WC;

		if( NULL != pRViewSSelPM )
		{
			pRViewSSelPM->FillInSelected( pclSSelPMaint );
		}

		// This variable must be set after the call to the 'FillInSelected' method because this one reset all.
		pclSSelPMaint->SetSelectionMode( ProductSelectionMode_Wizard );

		CDlgConfSel dlg( &m_clWizardSelPMParams );
		dlg.Display( pclSSelPMaint );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clWizardSelPMParams.m_pTADS->Get( _T("PRESSMAINT_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: can't retrieve the 'PRESSMAINT_TAB' table from the datastruct.") );
			}

			pTab->Insert( IDPtr );
			m_clWizardSelPMParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelPMaint ) );
		}
		else
		{
			m_clWizardSelPMParams.m_pTADS->DeleteObject( IDPtr );
		}

		if( NULL != dlg.GetSafeHwnd() )
		{
			dlg.DestroyWindow();
		}

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgWizardSelPM::_Select'."), __LINE__, __FILE__ );
		throw;
	}
}

bool CDlgWizardSelPM::_ApplyEdition( int iWhichButton )
{
	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_pclCurrentGroupDefinition )
	{
		return false;
	}

	// Verify pointer validity; user can delete object after entering in edition.
	if( _T('\0') != *m_clWizardSelPMParams.m_SelIDPtr.ID )
	{
		m_clWizardSelPMParams.m_SelIDPtr = m_clWizardSelPMParams.m_pTADS->Get( m_clWizardSelPMParams.m_SelIDPtr.ID );
	}

	CDS_SSelPMaint *pclSSelPMaint = dynamic_cast<CDS_SSelPMaint *>( m_clWizardSelPMParams.m_SelIDPtr.MP );

	// If object doesn't exist, it's an error.
	if( NULL == pclSSelPMaint )
	{
		return false;
	}

	// Must be done before calling 'FillInSelected' because this method uses some variables in it.
	pclSSelPMaint->GetpInputUser()->CopyFrom( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() );

	*pclSSelPMaint->GetpSelectedInfos()->GetpWCData() = m_clWizardSelPMParams.m_WC;

	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->FillInSelected( pclSSelPMaint );
	}

	// This variable must be set after the call to the 'FillInSelected' method because this one reset all.
	pclSSelPMaint->SetSelectionMode( ProductSelectionMode_Wizard );

	CDlgConfSel dlg( &m_clWizardSelPMParams );
	IDPTR IDPtr = pclSSelPMaint->GetIDPtr();
	dlg.Display( pclSSelPMaint );

	if( IDOK == dlg.DoModal() )
	{
		CTable *pTab = dynamic_cast<CTable *>( m_clWizardSelPMParams.m_pTADS->Get( _T("PRESSMAINT_TAB") ).MP );
		ASSERT( NULL != pTab );

		if( NULL != pTab )
		{
			m_clWizardSelPMParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelPMaint ) );
		}

		// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
		// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
		if( true == m_clWizardSelPMParams.m_bEditModeRunning )
		{
			_CancelEdition( -1 );
		}
	}
	else
	{
		if( _T('\0') == *m_clWizardSelPMParams.m_SelIDPtr.ID )
		{
			m_clWizardSelPMParams.m_pTADS->DeleteObject( IDPtr );
		}
	}

	if( NULL != dlg.GetSafeHwnd() )
	{
		dlg.DestroyWindow();
	}

	return true;
}

bool CDlgWizardSelPM::_CancelEdition( int iWhichButton )
{
	// User cancel edition of the wizard selection.
	SetModificationMode( false );
	
	if( WPMES_ValueInputStepsHeatingEdition == m_eWizardPMEngineState )
	{
		m_eWizardPMEngineState = WPMES_ValueInputStepsHeating;
		// _ProjectTypeChoice_GoInHeating( -1 );
	}
	else if( WPMES_ValueInputStepsCoolingEdition == m_eWizardPMEngineState )
	{
		m_eWizardPMEngineState = WPMES_ValueInputStepsCooling;
		// _ProjectTypeChoice_GoInCooling( -1 );
	}
	else if( WPMES_ValueInputStepsSolarEdition == m_eWizardPMEngineState )
	{
		m_eWizardPMEngineState = WPMES_ValueInputStepsSolar;
		// _ProjectTypeChoice_GoInSolar( -1 );
	}

	m_pclCurrentGroupDefinition = m_clAllGroupDefinitions[m_eWizardPMEngineState];
	m_iCurrentButton = 1;

	// Activate the 'Select' button.
	m_pclCurrentGroupDefinition->ChangeButtonStatus( IDS_DLGWIZARDPM_SELECT, BS_ToDo );

	// Set to 'false' to force a refresh of the 'RViewSSelPM' view in normal mode.
	m_bResultsAlreadyDisplayed = false;
	_GoToStep( IDS_DLGWIZARDPM_RESULTS );

	return true;
}

void CDlgWizardSelPM::_ApplyPMInputUserChangedInRightView( bool bWaterCharUpdated )
{
	if( NULL == pRViewWizardSelPM )
	{
		return;
	}

	bool bCurrentStepIsResult = false;

	if( WPMES_ProjectTypeChoice != m_eWizardPMEngineState && WPMES_ProjectTypeChoiceEdition != m_eWizardPMEngineState
			&& IDS_DLGWIZARDPM_RESULTS == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[m_iCurrentButton]->m_iIDSText )
	{
		bCurrentStepIsResult = true;
	}
	else
	{
		pRViewWizardSelPM->ApplyPMInputUserUpdated( bWaterCharUpdated );
	}

	// Check now if there are errors following this update.
	PressurisationNorm eNorm = m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetNorm();

	for( int iLoop = 0; iLoop < m_pclCurrentGroupDefinition->m_iVisibleButtonNumber; iLoop++ )
	{
		int iRightViewInputID = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoop]->GetDlgWizePMRightViewInputID( eNorm );

		if( -1 == iRightViewInputID )
		{
			continue;
		}

		ButtonStatus eButtonStatus = ( true == pRViewWizardSelPM->IsAtLeastOneError( iRightViewInputID ) ) ? BS_Error : BS_ToDefine;
		m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoop]->m_eStatus = eButtonStatus;
	}

	if( true == bCurrentStepIsResult )
	{
		bool bResultsAreDisplayed = _GoToResultsStep( m_iCurrentButton );
		
		if( false == bResultsAreDisplayed )
		{
			// If there are errors, we switch to the first steps where errors exist.
			int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

			for( int iLoop = 0; iLoop < iVisibleButtonNumber; iLoop++ )
			{
				if( BS_Error == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoop]->m_eStatus )
				{
					pMainFrame->ActiveFormView( CMainFrame::eRVWizardSelPM );

					if( NULL != pRViewSSelPM )
					{
						pRViewSSelPM->UnregisterNotificationHandler( this );
					}
					
					pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoop]->m_ptrfActionToDo;
 					(this->*fActionToDo)( iLoop );
					
					break;
				}
			}
		}
	}

	_ChangeButtonStates();
	_UpdateAllStatics();
}

bool CDlgWizardSelPM::_GoInResultsPreliminary()
{
	// Check first if there is no error in input.
	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	for( int iLoop = 0; iLoop < iVisibleButtonNumber; iLoop++ )
	{
		if( BS_Error == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoop]->m_eStatus )
		{
			CString str = TASApp.LoadLocalizedString( IDS_DLGWIZARDSELPM_ERROREXIST );
			AfxMessageBox( str, MB_OK );
			return false;
		}
	}

	// Detach the last input value dialog to be sure to have the last data input by user.
	if( NULL != pRViewWizardSelPM )
	{
		pRViewWizardSelPM->DetachCurrentInputDialog();
	}

	// Need to be called just after the call to the 'DetachCurrentInputDialog' method above. Because if user is writing a
	// value, when detaching the dialog a kill focus will be fired and it's only at this moment that the value will be saved.
	if( NULL != m_clWizardSelPMParams.m_pclSelectPMList )
	{
		m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetWC( m_clWizardSelPMParams.m_WC );
	}

	return true;
}

void CDlgWizardSelPM::_GoToStep( int iStepNameID )
{
	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	bool bCanStop = false;

	for( int iLoop = 0; iLoop < iVisibleButtonNumber && false == bCanStop; iLoop++ )
	{
		if( iStepNameID == m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoop]->m_iIDSText )
		{
			pfActionToDo fActionToDo = m_pclCurrentGroupDefinition->m_arOneGroupDefinition[iLoop]->m_ptrfActionToDo;
 			(this->*fActionToDo)( iLoop );

			_ChangeButtonStates();
			_UpdateAllStatics();

			bCanStop = true;
		}
	}
}

void CDlgWizardSelPM::_ChangeSelectOrApplyButtonsState( bool bEnable )
{
	if( false == m_clWizardSelPMParams.m_bEditModeRunning )
	{
		// We enable/disable the 'Select' button if we are in the normal mode.
		m_pclCurrentGroupDefinition->ChangeButtonStatus( IDS_DLGWIZARDPM_SELECT, ( true == bEnable ) ? BS_Enable :  BS_Disable );
	}
	else
	{
		// We enable/disable the 'Apply changes' button if we are in the edition mode.
		m_pclCurrentGroupDefinition->ChangeButtonStatus( IDS_DLGWIZARDPM_APPLY, ( true == bEnable ) ? BS_Enable :  BS_Disable );

		// The 'Cancel' button can be always enabled.
		m_pclCurrentGroupDefinition->ChangeButtonStatus( IDS_DLGWIZARDPM_CANCEL, BS_Enable );
	}
}

void CDlgWizardSelPM::_UpdateDialogHeight()
{
	// When changing the project type (or when going between normal and edit mode), we have not always the same button number that are 
	// visible. We resize the client area to match the new size. Here, we change the dialog container to this new size and we notify 
	// the 'DlgLeftTabSelManager' to change the 'IDC_STATICGROUP' control size. This allows 'DlgLeftTabBase' to perfectly draw the 
	// corporate logo at the bottom of the left tab.

	int iVisibleButtonNumber = m_pclCurrentGroupDefinition->m_iVisibleButtonNumber;

	CExtStatic *pLastStatic = m_arpButtonText[iVisibleButtonNumber - 1];
	CRect rectStatic;
	pLastStatic->GetWindowRect( &rectStatic );
	ScreenToClient( &rectStatic );

	if( NULL != pDlgLeftTabSelManager )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		rectClient.bottom = rectStatic.bottom + 4;
		SetWindowPos( NULL, -1, -1, rectClient.Width(), rectClient.Height(), SWP_NOMOVE | SWP_NOZORDER );
		pDlgLeftTabSelManager->ChangeStaticGroupSize( rectClient );
	}
}

void CDlgWizardSelPM::_CopyWizardSelParamsToIndSelParams()
{
	// This method will just copy the variables that are in the 'CProductSelectionParameters' base class.
	m_clIndSelPMParams.CopyFrom( &m_clWizardSelPMParams );

	// We need to do it manually for all variables of 'm_clWizardSelPMParams' that are needed by the 'RViewSSelPM'.

	// All variables in 'CWizardSelPMParams'.
	// Remark: no need to do a copy. The 'm_pclSelectedPMList' variable in the 'CWizardSelPMParams' class is a pointer on the
	//         global 'CSelectPMList' obtains thanks to the 'CPMSelectionHelper' class. Thus we can simply update the pointer
	//         in 'm_clIndSelPMParams'.
	m_clIndSelPMParams.m_pclSelectPMList = m_clWizardSelPMParams.m_pclSelectPMList;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 'COneButtonDefinition' class.
////////////////////////////////////////////////////////////////////////////////////////////////
CDlgWizardSelPM::COneButtonDefinition::COneButtonDefinition( int iIDSText, ButtonImgPos eImagePosition, ButtonStatus eStatus, HICON hIcon, 
		pfActionToDo ptrfActionToDo, int iDlgWizePMRightViewInputID )
{
	m_iIDSText = iIDSText;
	m_eImagePosition = eImagePosition;
	m_eStatus = eStatus;
	m_hIcon = hIcon;
	m_ptrfActionToDo = ptrfActionToDo;
	m_iDlgWizePMRightViewInputID = iDlgWizePMRightViewInputID;
}

void CDlgWizardSelPM::COneButtonDefinition::AddDlgWizePMRightViewInputID( PressurisationNorm eNorm, int iDlgWizePMRightViewInputID )
{
	if( m_mapiDlgWizePMRightViewInputID.end() == m_mapiDlgWizePMRightViewInputID.find( eNorm ) )
	{
		m_mapiDlgWizePMRightViewInputID.insert( std::pair<PressurisationNorm, int>( eNorm, iDlgWizePMRightViewInputID ) );
	}
}

int CDlgWizardSelPM::COneButtonDefinition::GetDlgWizePMRightViewInputID( PressurisationNorm eNorm )
{
	int iDlgWizePMRightViewInputID = m_iDlgWizePMRightViewInputID;

	if( m_mapiDlgWizePMRightViewInputID.size() > 0 )
	{
		std::map<PressurisationNorm, int>::iterator iter = m_mapiDlgWizePMRightViewInputID.find( eNorm );

		if( iter != m_mapiDlgWizePMRightViewInputID.end() )
		{
			iDlgWizePMRightViewInputID = iter->second;
		}
	}

	return iDlgWizePMRightViewInputID;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 'COneGroupDefinition' class.
////////////////////////////////////////////////////////////////////////////////////////////////
CDlgWizardSelPM::COneGroupDefinition::~COneGroupDefinition()
{
	for( int iLoop = 0; iLoop < m_arOneGroupDefinition.GetCount(); iLoop++ )
	{
		if( NULL != m_arOneGroupDefinition[iLoop] )
		{
			delete m_arOneGroupDefinition[iLoop];
		}
	}
}

CDlgWizardSelPM::COneButtonDefinition *CDlgWizardSelPM::COneGroupDefinition::AddOneButtonDefinition( COneButtonDefinition *pclOneButtonDefinition )
{
	m_arOneGroupDefinition.Add( pclOneButtonDefinition );
	m_arOneGroupDefinition[m_iVisibleButtonNumber]->m_iButtonNumber = m_iVisibleButtonNumber;
	m_iVisibleButtonNumber++;
	return pclOneButtonDefinition;
}

void CDlgWizardSelPM::COneGroupDefinition::ChangeButtonStatus( int iIDSText, ButtonStatus eStatus )
{
	COneButtonDefinition *pButtonDefinition = GetButtonDefinitionWithTextID( iIDSText );

	if( NULL != pButtonDefinition )
	{
		pButtonDefinition->m_eStatus = eStatus;
	}
}

CDlgWizardSelPM::COneButtonDefinition *CDlgWizardSelPM::COneGroupDefinition::GetButtonDefinitionWithDlgID( int iDlgWizePMRightViewInputID )
{
	COneButtonDefinition *pButtonDefinition = NULL;

	for( int iLoop = 0; iLoop < m_arOneGroupDefinition.GetCount() && NULL == pButtonDefinition; iLoop++ )
	{
		if( NULL == m_arOneGroupDefinition[iLoop] )
		{
			continue;
		}

		if( iDlgWizePMRightViewInputID == m_arOneGroupDefinition[iLoop]->m_iDlgWizePMRightViewInputID )
		{
			pButtonDefinition = m_arOneGroupDefinition[iLoop];
		}
		else if( m_arOneGroupDefinition[iLoop]->m_mapiDlgWizePMRightViewInputID.size() > 0 )
		{
			for( auto &iter : m_arOneGroupDefinition[iLoop]->m_mapiDlgWizePMRightViewInputID )
			{
				if( iter.second == iDlgWizePMRightViewInputID )
				{
					pButtonDefinition = m_arOneGroupDefinition[iLoop];
					break;
				}
			}
		}
	}

	return pButtonDefinition;
}

CDlgWizardSelPM::COneButtonDefinition *CDlgWizardSelPM::COneGroupDefinition::GetButtonDefinitionWithTextID( int iIDSText )
{
	COneButtonDefinition *pButtonDefinition = NULL;

	for( int iLoop = 0; iLoop < m_arOneGroupDefinition.GetCount() && NULL == pButtonDefinition; iLoop++ )
	{
		if( NULL != m_arOneGroupDefinition[iLoop] && iIDSText == m_arOneGroupDefinition[iLoop]->m_iIDSText )
		{
			pButtonDefinition = m_arOneGroupDefinition[iLoop];
		}
	}

	return pButtonDefinition;
}

CDlgWizardSelPM::COneButtonDefinition *CDlgWizardSelPM::COneGroupDefinition::operator[] ( int iIndex )
{
	if( iIndex < 0 || iIndex > m_arOneGroupDefinition.GetCount() - 1 )
	{
		return NULL;
	}

	return m_arOneGroupDefinition.GetAt( iIndex );
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 'CAllGroupDefinitions' class.
////////////////////////////////////////////////////////////////////////////////////////////////
CDlgWizardSelPM::CAllGroupDefinitions::~CAllGroupDefinitions()
{
	for( int iLoop = 0; iLoop < m_arAllGroupDefinitions.GetCount(); iLoop++ )
	{
		if( NULL != m_arAllGroupDefinitions[iLoop] )
		{
			delete m_arAllGroupDefinitions[iLoop];
		}
	}
}

void CDlgWizardSelPM::CAllGroupDefinitions::AddOneGroupDefinition( COneGroupDefinition *pclOneGroupDefinition )
{
	m_arAllGroupDefinitions.Add( pclOneGroupDefinition );
}

CDlgWizardSelPM::COneGroupDefinition *CDlgWizardSelPM::CAllGroupDefinitions::operator[] ( int iIndex )
{
	if( iIndex < 0 || iIndex > m_arAllGroupDefinitions.GetCount() - 1 )
	{
		return NULL;
	}

	return m_arAllGroupDefinitions.GetAt( iIndex );
}

bool CDlgWizardSelPM::_VerifyFluidCharacteristics( bool bShowErrorMsg )
{
	bool bEnable = true;
	CString strMsg = _T("");
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg );

	if( false == bEnable && true == bShowErrorMsg )
	{
		AfxMessageBox( strMsg );
	}

	return bEnable;
}

void CDlgWizardSelPM::_VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CString strAdditiveFamilyID = _T("");
	
	if( NULL != pclDlgWaterChar )
	{
		strAdditiveFamilyID = pclDlgWaterChar->GetAdditiveFamilyID();
	}
	else
	{
		if( NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC() )
		{
			ASSERT_RETURN;
		}

		strAdditiveFamilyID = m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC()->GetAdditFamID();
	}

	bEnable = true;
	strMsg = _T("");

	// HYS-1041: Disable suggest button when the additive is not a Glycol.
	if( ( 0 != StringCompare( strAdditiveFamilyID, _T("GLYCOL_ADDIT" ) ) ) 
			&& ( 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT" ) ) ) )
	{
		strMsg = TASApp.LoadLocalizedString( AFXMSG_BAD_ADDITIVE );
		bEnable = false;
	}

	// HYS-1104: Disable the 'Suggest' button if we are in EN12953 and not with water.
	if( m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12953 ) 
			&& ( 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT" ) ) ) )
	{
		strMsg = TASApp.LoadLocalizedString( IDS_DLGINDSELPM_EN12953ONLYWATER );
		bEnable = false;
	}
}

void CDlgWizardSelPM::_UpdateWithIndividualSelPMState()
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase *pclDlgSelectionBase = pDlgLeftTabSelManager->GetLeftTabDialog( ProductSubCategory::PSC_PM_ExpansionVessel, 
			ProductSelectionMode::ProductSelectionMode_Individual );

	if( NULL == pclDlgSelectionBase || NULL == dynamic_cast<CDlgIndSelPressureMaintenance *>( pclDlgSelectionBase ) )
	{
		return;
	}

	CDlgIndSelPressureMaintenance *pDlgIndividualSelPM = dynamic_cast<CDlgIndSelPressureMaintenance*>( pclDlgSelectionBase );

	m_clWizardSelPMParams.m_SelIDPtr = pDlgIndividualSelPM->GetEditedObject();
	SetModificationMode( pDlgIndividualSelPM->IsEditModeRunning() );

	m_clWizardSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard );
	
	if( NULL == m_clWizardSelPMParams.m_pclSelectPMList || NULL == m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}
		
	CSelectPMList *pclIndividualSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, 
			m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() );
	
	if( NULL == pclIndividualSelectPMList || NULL == pclIndividualSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}
	
	m_clWizardSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->CopyFrom( pclIndividualSelectPMList->GetpclPMInputUser() );
}

void CDlgWizardSelPM::_UpdateWithIndividualAllPrefInAllMode()
{
	// Copy all individual selection PM prefs in Heating, Cooling and Solar mode
	CSelectPMList* pclHeatingIndSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Heating );
	CSelectPMList* pclHeatingWizSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, Heating );
	if( NULL != pclHeatingIndSelPMList && NULL != pclHeatingIndSelPMList->GetpclPMInputUser() )
	{
		CPMWQPrefs* pclPrefHeating = pclHeatingIndSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		if( NULL != pclHeatingWizSelPMList && NULL != pclHeatingWizSelPMList->GetpclPMInputUser() )
		{
			CPMWQPrefs* pclPrefHeatingWiz = pclHeatingWizSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
			if( true != pclPrefHeating->CompareTo( pclPrefHeatingWiz ) )
			{
				pclPrefHeatingWiz->CopyFrom( pclPrefHeating );
			}
		}
	}

	CSelectPMList* pclCoolingIndSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Cooling );
	CSelectPMList* pclCoolingWizSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, Cooling );
	if( NULL != pclCoolingIndSelPMList && NULL != pclCoolingIndSelPMList->GetpclPMInputUser() )
	{
		CPMWQPrefs* pclPrefCooling = pclCoolingIndSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		if( NULL != pclCoolingWizSelPMList && NULL != pclCoolingWizSelPMList->GetpclPMInputUser() )
		{
			CPMWQPrefs* pclPrefCoolingWiz = pclCoolingWizSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
			if( true != pclPrefCooling->CompareTo( pclPrefCoolingWiz ) )
			{
				pclPrefCoolingWiz->CopyFrom( pclPrefCooling );
			}
		}
	}

	CSelectPMList* pclSolarIndSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Solar );
	CSelectPMList* pclSolarWizSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, Solar );
	if( NULL != pclSolarIndSelPMList && NULL != pclSolarIndSelPMList->GetpclPMInputUser() )
	{
		CPMWQPrefs* pclPrefSolar = pclSolarIndSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		if( NULL != pclSolarWizSelPMList && NULL != pclSolarWizSelPMList->GetpclPMInputUser() )
		{
			CPMWQPrefs* pclPrefSolarWiz = pclSolarWizSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
			if( true != pclPrefSolar->CompareTo( pclPrefSolarWiz ) )
			{
				pclPrefSolarWiz->CopyFrom( pclPrefSolar );
			}
		}
	}
}
