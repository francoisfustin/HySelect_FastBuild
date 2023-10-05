#pragma once


#include "DlgCtrlPropTree.h"
#include "DlgTPGeneral.h"
#include "DlgTPGeneralPressureMaintenance.h"
#include "DlgTPGeneralWaterQuality.h"
#include "DlgTPGeneralBalancingValves.h"
#include "DlgTPGeneralStdAndAdjustableControlValves.h"
#include "DlgTPGeneralPIBCV.h"
#include "DlgTPGeneralSmartValves.h"
#include "DlgTPGeneralDpControllers.h"
#include "DlgTPGeneralFixedOrifices.h"
#include "DlgTPGeneralRadiatorValves.h"
#include "DlgTPGeneralPipes.h"
#include "DlgTPGeneralDeviceSizes.h"
#include "DlgTPPageMainHC.h"
#include "DlgTPPageHC.h"
#include "DlgTPPageHCActuators.h"
#include "DlgTPPageHCPipes.h"
#include "DlgTPPageHCSmartCV.h"
#include "DlgTPPageHCPumps.h"


class CDlgTechParam : public CDlgCtrlPropTree
{
	DECLARE_DYNAMIC( CDlgTechParam )

public:
	typedef enum eOpeningPage
	{
		PageGeneral,								// General
		PageGeneralPressureMaintenance,				// General\Pressure maintenance
		PageGeneralWaterQuality,					// General\Water quality
		PageGeneralBalancingValves,					// General\Balancing valves
		PageGeneralStdAndAdjustableControlValves,	// General\Std. & adjustable control valves
		PageGeneralPIBCV,							// General\Press.-indep. bal. & control valves
		PageGeneralSmartValves,						// General\Smart ctrl. valves & smart diff. press. controllers.
		PageGeneralDpControllers,					// General\Dp controllers
		PageGeneralFixOrifice,						// General\Fixed orifices
		PageGeneralRadiatorValves,					// General\Radiator valves
		PageGeneralPipes,							// General\Pipes
		PageGeneralDeviceSizes,						// General\Device sizes

		PageMainHC,									// Hydronic circuit calculation
		PageHCBv,									// Hydronic circuit calculation\Balancing valves
		PageHCStandardControlValves,				// Hydronic circuit calculation\Standard control valves
		PageHCAdjustableControlValves,				// Hydronic circuit calculation\Adjustable control valves
		PageHCPICv,									// Hydronic circuit calculation\Press.-indep. ctrl valves
		PageHCActuators,							// Hydronic circuit calculation\Actuators
		PageHCDpC,									// Hydronic circuit calculation\Dp controllers
		PageHCDpCBCV,								// Hydronic circuit calculation\Combined Dp controller, control and balancing valves.
		PageHCSmartCV,								// HYS-1763: Hydronic circuit calculation\Smart control valves.
		PageHCPipes,								// Hydronic circuit calculation\Pipes
		PageHCPumps									// HYS-1930: New hydronic circuit: auto-adapting variable flow decoupling circuit: Hydronic circuit calculation\Pumps
	};

	CDlgTechParam( eOpeningPage ePage = PageGeneral, CWnd *pParent = NULL );
	virtual ~CDlgTechParam();
	void SaveAllPages();

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedResetToDefault();

private:
	void _InitPages();
	bool _CheckPage( HTREEITEM hItem );

	// Protected variables.
protected:
	int m_iCurrentPage;
	
	CDlgTPGeneral m_PageGeneral;														// General
	CDlgTPGeneralPressureMaintenance m_PageGeneralPressureMaintenance;					// General\Pressure maintenance
	DlgTPGeneralWaterQuality m_PageGeneralWaterQuality;									// General\Water quality
	CDlgTPGeneralBalancingValves m_PageGeneralBalancingValves;							// General\Balancing valves
	CDlgTPGeneralStdAndAdjustableControlValves m_PageStdAndAdjustableControlValves;		// General\Std. & adjustable control valves
	CDlgTPGeneralPIBCV m_PageGeneralPIBCV;												// General\Press.-indep. bal. & control valves
	CDlgTPGeneralSmartValves m_PageGeneralSmartValves;									// General\Smart ctrl. valves & smart diff. press. controllers
	CDlgTPGeneralDpControllers m_PageGeneralDpControllers;								// General\Dp controllers
	CDlgTPGeneralFixedOrifices m_PageGeneralFixOrifice;									// General\Fixed orifices
	CDlgTPGeneralRadiatorValves m_PageGeneralRadiatorValves;							// General\Radiator valves
	CDlgTPGeneralPipes m_PageGeneralPipes;												// General\Pipes
	CDlgTPGeneralDeviceSizes m_PageGeneralDeviceSizes;									// General\Device sizes

	CDlgTPPageMainHC m_PageMainHC;														// Hydronic circuit calculation
	CDlgTPPageHC m_PageHCBalancingValves;												// Hydronic circuit calculation\Balancing valves
	CDlgTPPageHC m_PageHCStandardControlValves;											// Hydronic circuit calculation\Standard control valves
	CDlgTPPageHC m_PageHCAdjustableControlValves;										// Hydronic circuit calculation\Adjustable control valves
	CDlgTPPageHC m_PageHCPIBCV;															// Hydronic circuit calculation\Press.-indep. bal. & control valves
	CDlgTPPageHCActuators m_PageHCActuators;											// Hydronic circuit calculation\Actuators
	CDlgTPPageHC m_PageHCDpC;															// Hydronic circuit calculation\Dp controllers
	CDlgTPPageHC m_PageHCDpCBCV;														// Hydronic circuit calculation\Combined Dp controller, control and balancing valves.
	CDlgTPPageHCPipes m_PageHCPipes;													// Hydronic circuit calculation\Pipes
	// HYS-1763: for Smart control valve page
	CDlgTPPageHCSmartCV m_PageHCSmartCV;												// Hydronic circuit calculation\Smart control valves
	// HYS-1930: New hydronic circuit: auto-adapting variable flow decoupling circuit: add default secondary pump Hmin.
	CDlgTPPageHCPumps m_PageHCPumps;													// Hydronic circuit calculation\Pumps

	CArray <CDlgCtrlPropPage *> m_ArpPage;

	BYTE m_bmodified;
	bool m_bRefreshResults;

	IDPTR m_IDPtrTechParamSaved;
	CDS_TechnicalParameter *m_pclTechParamSaved;
	double m_dPipeSafetyfactorSaved;
	bool m_bCheckDeletedAvailableSaved;
};
