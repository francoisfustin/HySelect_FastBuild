#pragma once


class CRViewSSelBCv :public CRViewSSelCtrl
{
public:
	CRViewSSelBCv();
	virtual ~CRViewSSelBCv();

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
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pfShowTip );
	
	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelCltr protected pure virtual methods.

	// Param: 'fDowngradeActuatorFunctionality' set to 'true' if actuator with the '3 points' input signal type can work with the 'On/Off control type,
	//        and if actuator with the 'Proportional' input signal type can work with the 'On/Off' and the '3 points' control type.
	virtual bool IsActuatorFit( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, bool bDowngradeActuatorFunctionality, bool bAcceptAllFailSafe );

	// End of overriding CRViewSSelCltr protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelCltr protected virtual methods.

	// Return row where is located edited control valve.
	virtual long FillControlValveRows( CDB_ControlValve *pclSelectedControlValve = NULL );
	virtual void GetAdapterList( CDB_ControlValve *pclSelectedControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable );
	virtual void GetSetAccessoryList( CDB_ControlValve *pclSelectedControlValve, CDB_Actuator *pclSeletedActuator, CRank *pclSetAccessoryList, CDB_RuledTableBase **ppclRuledTable );

	// End of overriding CRViewSSelCltr protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// This method is called to gray incompatible adapter (at now only for BCV and PICV).
	// When use select a control valve, it's possible that there is some TA actuators available for it. But it's also possible to put actuator from other
	// manufacturer. To do that we just need an adapter between control valve and actuator. So when user select a control valve, we show all adapters and also
	// TA actuators. If user click on an adapter, we need to check if this one is compatible with displayed TA actuators or not. If it's not the case then
	// we grayed all actuators that are not compatible. In the same way if user click on an TA actuator, we have to run list of all displayed adapters to check
	// which ones can be used with control valve. Then all incompatible adapters will be also grayed.
//	virtual void GrayUncompatibleAccessoriesCheckbox( CDB_Accessory *pclAdapter, bool fCheck );

	// Private methods.
private:
	void _SetRemarkFullOpening( CSheetDescription *pclSheetDescriptionBCV, long lRow );

// Private variables.
private:
	CIndSelBCVParams *m_pclIndSelBCVParams;
	CDB_RuledTableBase *m_pclAdapterRuledTable;
};

extern CRViewSSelBCv *pRViewSSelBCv;
