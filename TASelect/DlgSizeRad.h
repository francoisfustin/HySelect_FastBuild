#pragma once


#include "MyToolTipCtrl.h"
#include "ExtNumEdit.h"
#include "ExtComboBox.h"
#include "Global.h"
#include "XGroupBox.h"

class CDlgSizeRad : public CDialogEx
{
public:
	enum { IDD = IDD_DLGSIZERAD };

	CDlgSizeRad( CIndSelTRVParams *pclIndSelTrvParams, CWnd *pParent = NULL );

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	void RefreshUnits();

	afx_msg void OnMove( int x, int y );
	afx_msg void OnPaint();
	afx_msg void OnBnClickedCalculateRecommendedPower();
	afx_msg void OnBnClickedCalculateRequiredFlow();
	afx_msg void OnBnClickedAddNominalConditions();
	afx_msg void OnBnClickedDeleteNominalConditions();
	afx_msg void OnBnClickedQDpReady();
	afx_msg void OnKillFocusEditAvailableDp();
	afx_msg void OnKillFocusEditRequiredHeatOut();
	afx_msg void OnKillFocusEditRoomTemperature();
	afx_msg void OnKillFocusEditSupplyTemperature();
	afx_msg void OnKillfocusEditRequiredDT();
	afx_msg void OnKillFocusEditInstalledPower();
	afx_msg void OnCbnSelChangeNominalConditions();
	afx_msg void OnChangeEditAvailableDp();
	afx_msg void OnChangeEditRequiredHeatOut();
	afx_msg void OnChangeEditInstalledPower();
	afx_msg void OnChangeEditRequiredDT();
	afx_msg void OnChangeEditRoomTemperature();
	afx_msg void OnChangeEditSupplyTemperature();
	afx_msg void OnBnCheckClickedAvailableDp();
	afx_msg LRESULT OnUnitChange( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

	// Display the calcrad results.
	void DisplayRadResults();
	
	// Display the calcflow results.
	void DisplayFlowResults();
	
	// Call this function any time it is necessary to reset calcrad results.
	void ResetRadResults( bool bResetReqdT = true );
	
	// Call this function any time it is necessary to reset calcflow results.
	void ResetFlowResults();
	
	// Reset all result editors.
	// Enable button calc Rad and Disable buttons calcflow and Q2Sheet.
	void ResetResults();
	
	// Reset content of ComboNomCond with USERDB RADNOMCOND_TAB content; if NCID exist select corresponding Item.
	void InitComboNC( CString strNCID = _T("") );
	
	CString GetStringFromNC( CDB_RadNomCond *pNC );

	double GetMaxQWithFlowLimitation();

	// Find out the Kvs bounds by sweeping over supply and return valves according to parameters set in the combo boxes.
	void FindKvsBounds( double *pdLowestTrvKvs, double *pdHighestTrvKvs, double *pdLowestRvKvs, double *pdHighestRvKvs );

// Private methods.
private:
	void _Init( void );

// Private variables.
public:
	CIndSelTRVParams *m_pclIndSelTRVParams;
	CWnd *m_pParent;
	CUserDatabase *m_pUSERDB;
	CString m_strSectionName;
	bool m_bInitialized;
	CMyToolTipCtrl m_ToolTip;
	CDB_RadNomCond *m_pNC;

	CButton m_CheckAvDp;
	CExtNumEditComboBox m_ComboNomCond;
	CExtNumEdit m_EditAvDp;
	CStatic m_StaticRadInfoBmp;
	CButton m_ButtonSummary;
	CXGroupBox m_GroupNote;
	CXGroupBox m_GroupRadCond;
	CXGroupBox m_GroupRad;
	CExtNumEdit m_EditTs;
	CEdit m_EditTr;
	CExtNumEdit m_EditTi;
	CEdit m_EditReqdT;
	CEdit m_EditRecPow;
	CEdit m_EditMinPow;
	CEdit m_EditMindT;
	CEdit m_EditInstPow;
	CExtNumEdit m_EditHeatOut;
	CEdit m_EditFlow;
	CButton m_ButtonQ2Sheet;
	CButton m_ButtonDelete;
	CButton m_ButtonCalcRad;
	CButton m_ButtonCalcFlow;
	CButton m_ButtonAdd;
};
