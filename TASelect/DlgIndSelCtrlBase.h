#pragma once


#include "ProductSelectionParameters.h"
#include "DlgIndSelCtrlBase.h"

// Base class for the 'CDlgIndSelBCV', 'CDlgIndSelCv', 'CDlgIndSelDpCBCV' and 'CDlgIndSelPICv' classes.
// Remark: Before we had both 'FillCombo2w3w' and 'FillComboCtrlType' methods in the 'CDlgIndSelBase' class.
//         But it means that we had the need of the 'm_eCv2W3W', 'm_eCvCtrlType' variables for example also in the
//		   'CIndividualSelectionParameters'. Variables that are not common to all inherited classes of 'CDlgIndSelBase'.
class CDlgIndSelCtrlBase : public CDlgIndSelBase
{
public:

	CDlgIndSelCtrlBase( CIndSelCtrlParamsBase &clIndSelCtrlParams, UINT nID = 0, CWnd* pParent = NULL );

	virtual ~CDlgIndSelCtrlBase() {}

// Protected methods.
protected:
	virtual void FillCombo2w3w( CString strTableID, CDB_ControlProperties::CV2W3W eCv2w3w = CDB_ControlProperties::CV2W3W::CV2W );
	virtual void FillComboCtrlType( CTADatabase::CvTargetTab eTargetTab, CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvProportional );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	// HYS-1263 : Added to update application type if it is necessary
	virtual void ActivateLeftTabDialog();
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CIndSelCtrlParamsBase *m_pclIndSelCtrlParams;
};
