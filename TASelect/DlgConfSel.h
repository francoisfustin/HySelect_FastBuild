#pragma once


#include "select.h"
#include "Global.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "EnBitmap.h"
#include "CustomEdits.h"
#include "ExtComboBox.h"
#include "ExtListCtrl.h"
#include "XGroupBox.h"
#include "ProductSelectionParameters.h"

class CDS_HmHub;
class CDlgConfSel : public CDialogEx
{
public:
	CDlgConfSel( CProductSelelectionParameters *pclProdSelParams = NULL, CWnd *pParent = NULL );
	~CDlgConfSel();
	enum { IDD = IDD_DLGCONFSEL };
	
	enum enumIFrom
	{
		FromNone,
		FromSSelAcc,
		FromSSelAct,
		FromSSelBCv,
		FromSSelBV,
		FromSSelCv,
		FromSSelDpController,
		FromSSelDpReliefValve,
		FromSSelPICv,
		FromSSelH,
		FromSSelTrv,
		FromSSelAirVentSep,
		FromSSelPMaint,
		FromSSelDpCBCV,
		FromSSelSv,
		FromSSelSafetyValve,
		FromSSel6WayValve,
		FromSSelTapWaterControl,
		FromSSelSmartControlValve,
		FromSSelSmartDpController,  // HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
		FromSSelFloorHeatingManifold,
		FromSSelFloorHeatingValve,
		FromSSelFloorHeatingController,
		FromSSelPWQAccAndServices,              // HYS-1741
		FromSSelTALinkAlone
	};

	void Display( CDS_HmHub *pHmHub );
	void Display( CDS_SSelBCv *pSelBCv );
	void Display( CDS_SSelBv *pSelBv );
	void Display( CDS_SSelCv *pSelCv );
	void Display( CDS_SSelDpC *pSelDpController );
	void Display( CDS_SSelDpReliefValve *pSelDpReliefValve );
	void Display( CDS_SSelPICv *pSelPICv );
	void Display( CDS_SSelRadSet *pSelTrv );
	void Display( CDS_SSelAirVentSeparator *pSelAirVentSeparator );
	void Display( CDS_SSelPMaint *pSelPressureMaint );
	void Display( CDS_SSelDpCBCV *pSelDpCBCV );
	void Display( CDS_SSelSv *pSelSv );
	void Display( CDS_SSelSafetyValve *pSelSafetyValve );
	void Display( CDS_SSel6WayValve *pSel6WayValve );
	void Display( CDS_SSelTapWaterControl *pSelTapWaterControl );
	void Display( CDS_SSelSmartControlValve *pSelSmartControlValve );
	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	void Display( CDS_SSelSmartDpC* pSelSmartDpController );
	void Display( CDS_SSelFloorHeatingManifold *pSelFloorHeatingManifold );
	void Display( CDS_SSelFloorHeatingValve *pSelFloorHeatingValve );
	void Display( CDS_SSelFloorHeatingController *pSelFloorHeatingController );
	// HYS-1741
	void Display( CDS_SelPWQAccServices *pSelPWQAccAndServices );

	// From a CDS_Actuator.
	void Display( CDS_Actuator *pSelAct );

	// From a CDS_Accessory.
	void Display( CDS_Accessory *pSelAcc );

	// From an 'CData' object. The method will try to find correct the 'CDS_...' object.
	// Remark: It is an helper for the 'CDlgDirSel::ConfirmSelection' method.
	void Display( CData *pSSel );

	// For single TAlink
	void Display( CDS_SSelDpSensor *pSSel );

// Protected members.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//HYS-987: Get x, y position of the cursor
	CPoint GetListCtrlCursorPosition() const;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelChangeRemark();
	afx_msg void OnCbnEditChangeRemark();
	afx_msg void OnEnChangeRemark();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDelete();
	//HYS-987
	afx_msg void OnClickedCheckBox();
	afx_msg void OnNMClickList( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnEnKillFocusEditQty( );
	// Called when the accessory quantity field is modified
	afx_msg LRESULT OnNotifyDescriptionEdited( WPARAM wParam, LPARAM lParam );


// Private members
private:
	void _Display( int iFrom );

	// Initialize list control according m_iFrom.
	void _InitListCtrl();

	// Fill list control for control valve.
	void _FillListCtrlForControlValve( CSelectedInfos *pSelInfo, CDS_SSelCtrl *pSelCtrl );

	// Fill list control for Dp relief valve.
	void _FillListCtrlForDpReliefValve( CSelectedInfos *pSelInfo, CDS_SSelDpReliefValve *pclSSelDpReliefValve );
	
	// Fill list control for radiator set.
	void _FillListCtrlForRadiatorSet( CSelectedInfos *pSelInfo, CDS_SSelRadSet *pSelRadSet );

	// Fill list control for air vents & separators.
	void _FillListCtrlForAirVentSep( CSelectedInfos *pSelInfo, CDS_SSelAirVentSeparator *pSelAirVentSep );

	// Fill list control for pressure maintenance product.
	void _FillListCtrlForPressMaint( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint );

	// Fill list control for shut-off valves.
	void _FillListCtrlForShutOffValve( CSelectedInfos *pSelInfo, CDS_SSelSv *pclSSelSv );

	// Fill list control for safety valves.
	void _FillListCtrlForSafetyValve( CSelectedInfos *pSelInfo, CDS_SSelSafetyValve *pclSSelSafetyValve );

	// Fill list control for 6-way valves.
	void _FillListCtrlFor6WayValve( CSelectedInfos *pSelInfo, CDS_SSel6WayValve *pclSSel6WayValve );

	// Fill list control for smart control valves.
	void _FillListCtrlForSmartControlValve( CSelectedInfos *pSelInfo, CDS_SSelSmartControlValve *pclSSelSmartControlValve );

	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	// Fill list control for smart Dp controller.
	void _FillListCtrlForSmartDpC( CSelectedInfos *pSelInfo, CDS_SSelSmartDpC *pclSSelSmartDpController );
	void _FillListCtrlForTALinkAlone( CSelectedInfos *pSelInfo, CDS_SSelDpSensor *pclSSelDpSensor );

	// Helpers to fill pressure maintenance product.
	void _FillPMVesselHelper( CDB_Vessel *pclVessel, CDS_SSelPMaint *pclSelPressMaint, CString& strRemark, 
		CDB_TecBox::TBIntegratedVesselType eIntegratedVesselType = CDB_TecBox::ePMIntVesselType_None, bool bShowAccessories = true );

	void _FillPMCompressoTransferoHelper( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint, CString& strRemark );
	void _FillPMPlenoHelper( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint, CString& strRemark );
	// HYS-1121: To manage Pleno with Pleno protection water make-up
	void _FillPlenoProtectionHelper( CSelectedInfos * pSelInfo, CDS_SSelPMaint * pclSelPressMaint, CString & strRemark );
	void _FillPMPlenoRefillHelper( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint, CString& strRemark );
	void _FillPMVentoHelper( CSelectedInfos *pSelInfo, CDS_SSelPMaint *pclSelPressMaint, CString& strRemark );

	// Fill list control for tap water control products.
	void _FillListCtrlForTapWaterControl( CSelectedInfos *pSelInfo, CDS_SSelTapWaterControl *pclSSelTapWaterControl );

	// Fill list control for floor heating manifold products.
	void _FillListCtrlForFloorHeatingManifold( CSelectedInfos *pSelInfo, CDS_SSelFloorHeatingManifold *pclSSelFloorHeatingManifold );

	// Fill list control for floor heating control valve products.
	void _FillListCtrlForFloorHeatingValve( CSelectedInfos *pSelInfo, CDS_SSelFloorHeatingValve *pclSSelFloorHeatingValve );

	// Fill list control for floor heating controller products.
	void _FillListCtrlForFloorHeatingController( CSelectedInfos *pSelInfo, CDS_SSelFloorHeatingController *pclSSelFloorHeatingController );
	// HYS-1741
	void _FillPWQAccAndServices( CSelectedInfos *pSelInfo, CDS_SelPWQAccServices *pclSSelPWQAccAndSevices );

	// Initialize drop down with glossary DB.
	void _InitGlossaryDD( bool bClearEdRem, CString str = _T("") );

	void _InitRemark( CString str );

	void _InitRefsAndRemarks( CSelectedInfos *pSelInfo );

	void _SaveRefsAndRemarks( CSelectedInfos *pSelInfo );

	// HYS-987: Add parameters iGroupQuantity and iDistributedQty
	long _AddAccessoriesInfos( int iRow, int iIDS, CAccessoryList *pclAccessoryList, int iGroupQuantity, std::vector<CData *> vecBuiltInAccessories,
			int iDistributedQty = 1, CAccessoryList::AccessoryType eAccessoryType = CAccessoryList::AccessoryType::_AT_Undefined, bool bAddBlankLine = true,
			COLORREF clColor = _BLACK );

	long _AddAccessoriesInfos( int iRow, CString strTitle, CAccessoryList *pclAccessoryList, int iGroupQuantity, std::vector<CData *> vecBuiltInAccessories,
			int iDistributedQty = 1, CAccessoryList::AccessoryType eAccessoryType = CAccessoryList::AccessoryType::_AT_Undefined, bool bAddBlankLine = true,
			COLORREF clColor = _BLACK );

	// Allow to fill specific name and description in regards to the situation.
	void _AddAccessoriesInfosHelper_GetNameDescription( CDB_Product *pclAccessory, CString &strName, CString &strDescription );
	void _AddAccessoriesInfosHelper_GetErrorMessage( CDB_Product *pclAccessory, CString &strError );

	long _AddTAProductInfos( long lRow, CDB_TAProduct *pTAP );
	
	// HYS-987: Add parameter pSelInfo to get the quantity of actuator
	void _FillActuators( CSelectedInfos *pSelInfo, CDB_Actuator *pActr, CDB_ControlValve *pCv, CString strTitle = _T(""), COLORREF clColor = _BLACK,
			bool bFillFailSafeDRP = true );

	long _FillActuatorAccessories( CAccessoryList *pclActuatorAccessoryList, CDB_Actuator *pclActuator, int iGlobalQuantity, long lRow,
			CString strTitle = _T(""), COLORREF clColor = _BLACK );
	
	// HYS-987: This function add quantity value for each product in the ListCtrl. Param lQty = Distributed quantity
	// lEditedQty = the quantity saved in AccessoryItem.
	void _AddQuantity( long lRow, unsigned int lQty = 1, unsigned int lGroupQty = 1, long lEditedQty = -1, bool bToBeSaved = false );

	// HYS-1968: Change editable quantity status.
	// lRowStart: first accessory row
	// lRowEnd: last accessory row + 1
	// bToBeSaved: Can we edit and save accessory quantity ?
	void _SetQuantityStatus( long lRowStart,  long lRowEnd, bool bToBeSaved );
	
	// HYS-987: This function is called when the group quantity is edited and sets all quantity field with the 
	// initial value multiply by lQuantity.
	void _SetAllEditableQuantity( unsigned int lQuantity );

	void _SetImage();
	
// Protected variables.
protected:
	CProductSelelectionParameters *m_pclProdSelParams;	

	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CUserDatabase *m_pUSERDB;
	CUnitDatabase *m_pUnitDB;

	CDS_HmHub *m_pHmHub;
	CDS_Accessory *m_pSelAcc;
	CDS_Actuator *m_pSelAct;
	CDS_SSelBCv *m_pSelBCv;
	CDS_SSelBv *m_pSelBv;
	CDS_SSelCv *m_pSelCv;
	CDS_SSelDpC *m_pSelDpController;
	CDS_SSelDpReliefValve *m_pSelDpReliefValve;
	CDS_SSelPICv *m_pSelPICv;
	CDS_SSelRadSet *m_pSelRadSet;
	CDS_SSelAirVentSeparator *m_pSelAirVentSeparator;
	CDS_SSelPMaint *m_pSelPressureMaint;
	CDS_SSelDpCBCV *m_pSelDpCBCV;
	CDS_SSelSv *m_pSelSv;
	CDS_SSelSafetyValve *m_pSelSafetyValve;
	CDS_SSel6WayValve *m_pSel6WayValve;
	CDS_SSelTapWaterControl *m_pSelTapWaterControl;
	CDS_SSelSmartControlValve *m_pSelSmartControlValve;
	CDS_SSelSmartDpC *m_pSelSmartDpC; 
	CDS_SSelDpSensor *m_pSelTALinkAlone; 
	CDS_SSelFloorHeatingManifold *m_pSelFloorHeatingManifold;
	CDS_SSelFloorHeatingValve *m_pSelFloorHeatingValve;
	CDS_SSelFloorHeatingController *m_pSelFloorHeatingController;
	CDS_SelPWQAccServices * m_pSelPWQAccAndServices; // HYS-1741

	bool m_bComboChanged;
	int m_iFrom;			
	long m_lQty;
	_string m_strComment;					// Comment from the CDB_TAProduct that will be initialize on the selection.

	CMFCButton m_ButtonAdd;
	CButton	m_ButtonOk;
	CMFCButton m_ButtonDelete;
	CEdit m_EditQty;
	CExtEditComboBox m_ComboRemark;
	CEdit m_Edit1stRef;
	CEdit m_Edit2ndRef;
	CEdit m_EditRemark;
	CStatic m_staticImg;
	CExtListCtrl m_ListCtrl;
	CProdPic *m_pProdPic;
	CEnBitmap m_Bmp; 
	CXGroupBox m_GroupInfo;
	CXGroupBox m_GroupRemark;
	CButton m_ButtonCheckBox;
	// To detect witch cells are editable.
	// map to store row and position of quantity which can be editable.
	// HYS-1968: Use map to keep lRow.
	map<long, RECT> m_mapEditableQuantity;
	// To store all static or editable quantity
	map<long, unsigned int*> m_mapQuantityRef;
	// To store accessories quantity
	typedef struct _SelectedAccessories
	{
		CAccessoryList *pclSelectedAccList = NULL;
		CDS_Actuator *pclSelectedActuator = NULL;
		CAccessoryList::AccessoryItem AccItemFromAccList;
		CDS_Actuator::AccessoryItem AccItemFromActuator;
	}SelectedAccessories;

	map<long, SelectedAccessories> m_mapSelectAccessories;
	bool m_bApplyDefaultQty;
	bool m_bApplyAccessoriesQty;

	CToolTipCtrl m_ToolTip;
};
