#pragma once


#include "afxwin.h"
#include "XGroupBox.h"
#include "MyToolTipCtrl.h"

class CDlgHydroCalcQKvDp;
class CDlgHydroCalcQPDT;
class CDlgHydroCalcRedDisk;
class CHydroCalcMFCTabCtrl : public CMFCTabCtrl
{
public:
	// Overrides some 'CMFCBaseTabCtrl' to avoid drag to be moved.
	virtual DROPEFFECT OnDragEnter( COleDataObject *pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual DROPEFFECT OnDragOver( COleDataObject *pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual void SwapTabs( int nFisrtTabID, int nSecondTabID ) {}
	virtual void MoveTab( int nSource, int nDest ) {}
};

class CDlgHydroCalc : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgHydroCalc )

public:
	enum { IDD = IDD_DLGHYDROCALC };

	enum TabIDs
	{
		QKvDp = 0,
		QPDT,
		QRedDisk,
		Undefined
	};

	CDlgHydroCalc( CWnd *pParent = NULL );

	virtual ~CDlgHydroCalc();

	CWaterChar GetWaterChar(){ return m_clWaterChar; }
	double GetKvValue() { return m_dKvValue; }
    double GetDpValue() { return m_dDpValue; }
    double GetFlowValue() { return m_dFlowValue; }
    double GetDTValue() { return m_dDTValue; }
    double GetPowerValue() { return m_dPowerValue; }
    double GetSettingValue() { return m_dSettingValue; }
    CDB_TAProduct* GetValve() { return m_pValve; }
	int GetRadioState( TabIDs eTab ) { return m_iRadioState[eTab]; }
	int GetPreviousTabID() { return m_ePreviousTabID; }

    void SetKvValue( double dKvValue ) { m_dKvValue = dKvValue; }
    void SetDpValue( double dDpValue ) { m_dDpValue = dDpValue; }
    void SetFlowValue( double dFlowValue ) { m_dFlowValue = dFlowValue; }
    void SetDTValue( double dDTValue ) { m_dDTValue = dDTValue; }
    void SetPowerValue( double dPowerValue ) { m_dPowerValue = dPowerValue; }
    void SetSettingValue( double dSettingValue ) { m_dSettingValue = dSettingValue; }
    void SetValve( CDB_TAProduct *pValve ) { m_pValve = pValve; }
	void SetRadioState( TabIDs eTab, int iState ) { m_iRadioState[eTab] = iState; }
	void SetPreviousTabID( TabIDs eTab ) { m_ePreviousTabID = eTab; }

	void OnCancelFromMFCTab( void ) { EndDialog( IDCANCEL ); }

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnBnClickedOk();
	afx_msg void OnButtonModWater();
	afx_msg LRESULT OnChangeActiveTab( WPARAM wp, LPARAM lp );

// Private variables.
private:
	enum TabIndex
	{
		etiQKvDp = 0,
		etiQPDT,
		etiRedDisk,
		etiLast
	};

	CTADatastruct *m_pTADS;
	CHydroCalcMFCTabCtrl m_WndTabs;
	CDlgHydroCalcQKvDp *m_pclDlgHydroCalcQKvDp;
	CDlgHydroCalcQPDT *m_pclDlgHydroCalcQPDT;
	CDlgHydroCalcRedDisk *m_pclDlgHydroCalcRedDisk;
	CImageList m_TabCtrlImageList;
	CStatic m_GroupHydroCalc;
	CXGroupBox m_GroupWater;
	CMFCButton m_ButtonModWater;
	CButton m_ButtonOK;
	CMyToolTipCtrl m_ToolTip;

	// Variables common to all three CDlgHydroCalcXXX
	CWaterChar m_clWaterChar;
	ProjectType m_eCurrentProjectType;
	CDB_TAProduct *m_pValve;
	double m_dKvValue;
	double m_dDpValue;
	double m_dFlowValue;
	double m_dDTValue;
	double m_dPowerValue;
	double m_dSettingValue;
	int m_iRadioState[3];
	TabIDs m_ePreviousTabID;
};
