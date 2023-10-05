#pragma once


class CRViewSSelPICv : public CRViewSSelCtrl
{
public:
	CRViewSSelPICv();
	virtual ~CRViewSSelPICv();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public virtual methods.

	// Called by 'DlgIndSelXXX' the user click 'Suggest' button.
	// Param: 'pclProductSelectionParameters' contains all needed data.
	virtual void Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam = NULL );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CMultiSpreadBase protected pure virtual methods.
	
	// Returns 'true' if current method has displayed a tooltip.
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
			TCHAR *pstrTipText, BOOL *pbShowTip );

	// End of overriding CMultiSpreadBase protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelCltr protected pure virtual methods.

	// Param: 'fDowngradeActuatorFunctionality' set to 'true' if actuator with the '3 points' input signal type can work with the 'On/Off control type,
	//        and if actuator with the 'Proportional' input signal type can work with the 'On/Off' and the '3 points' control type.
	virtual bool IsActuatorFit( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, 
			bool bDowngradeActuatorFunctionality, bool bAcceptAllFailSafe );

	// End of overriding CRViewSSelCltr protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelCltr protected virtual methods.

	// Return row where is located edited control valve.
	virtual long FillControlValveRows( CDB_ControlValve *pEditedControlValve = NULL );
	virtual void GetAdapterList( CDB_ControlValve *pclSelectedControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable );
	virtual void GetSetAccessoryList( CDB_ControlValve *pclSelectedControlValve, CDB_Actuator *pclSeletedActuator, 
			CRank *pclSetAccessoryList, CDB_RuledTableBase **ppclRuledTable );
	// End of overriding CRViewSSelCltr protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private members.
private:
	long _SetRemarksKTH( CSheetDescription *pclSheetDescriptionPICv, long lRow );

// Private variables.
private:
	CIndSelPIBCVParams *m_pclIndSelPIBCVParams;
	CDB_RuledTableBase *m_pclAdapterRuledTable;
};

extern CRViewSSelPICv *pRViewSSelPICv;
