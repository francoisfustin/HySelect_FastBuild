#pragma once


#include "SSheet.h"
#include "hydromod.h"
#include "MessageManager.h"

class CSSheetPanelCirc1 : public CSSheet
{
public:
	CSSheetPanelCirc1();
	virtual ~CSSheetPanelCirc1();
	
	// HYS-1579: ePC1TU to have all available values in panel cir1 for terminal unit
	enum class ePC1TU
	{
		ePC1TU_None,
		ePC1TU_TermUnit,
		ePC1TU_Radiator
	};

	// HYS-1579: ePC1CVType to have all available values in panel cir1 for control valve type
	enum class ePC1CVType
	{
		ePC1CV_NoControl,
		ePC1CV_Standard,
		ePC1CV_Adjustable,
		ePC1CV_AdjustableMeas,
		ePC1CV_PressInd,
		ePC1CV_Smart
	};

	// Initialize pictures thumbnail.
	void Init( ePC1TU eTU, bool bShowDistributionPipe, CDS_HydroMod::ReturnType eReturnType, DWORD dwBalType, CDB_ControlProperties::eCVFUNC eCVType, 
			CDB_ControlProperties::CvCtrlType eCVCtrlType, int iCVTypeFamily, CDS_HydroMod *pHM = NULL );

	// Remark: it's to have exactly the same selection of circuit scheme for here and for the 'DlgHydronicSchView' dialog test.
	void Init_helper( std::vector<CDB_CircuitScheme *> *pvecCircuitShemeList, ePC1TU eTU, bool bShowDistributionPipe, CDS_HydroMod::ReturnType eReturnType, DWORD dwBalType, 
			CDB_ControlProperties::eCVFUNC eCVType, CDB_ControlProperties::CvCtrlType eCVCtrlType, int iCVTypeFamily, CDS_HydroMod *pHM = NULL );

	// It's also an helper for the 'DlgHydronicSchView' dialog test.
	ePC1CVType ConvertCvFunc2PC1CVType( CDB_ControlProperties::eCVFUNC eCVType );
	
	bool IsTuChecked() { return (ePC1TU::ePC1TU_TermUnit == m_eTU); }
	CDB_CircuitScheme *GetSelectedSch();
	bool IsAnchorPtExistInSelectedSCH( CAnchorPt::eFunc func );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	
	// 'MessageManager' message handler.
	afx_msg LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam );

	void CleanSchArray();

// Private methods.
private:
	void _OnLButtonDown( long lLButtonDownCol, long lLButtonDownRow );
	void _OnLButtonDblClk( long lLButtonDlbClkCol, long lLButtonDlbClkRow );

	// Allow to insert circuit schematic in a fixed order.
	void _InsertCircuitScheme( CRank *pclRank, int iCircSchemeCategoryOrder, CDB_CircuitScheme *pclCircuitScheme );
	
// Protected variables.
protected:	
	typedef struct sSchDesc
	{
		CDB_CircuitScheme *pSch;
		CArray<CAnchorPt::eFunc> *paFunc;
	};
	
	CArray<sSchDesc> m_arSchemes;
	int m_iSelectedScheme;
	long m_lSelectedCol;
	long m_lSelectedRow;
	ePC1TU m_eTU;
};
