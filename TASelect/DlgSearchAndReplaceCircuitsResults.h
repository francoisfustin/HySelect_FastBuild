#pragma once


#include "afxwin.h"
#include "DialogExt.h"
#include "SSheet.h"
#include "RViewDescription.h"
#include "MessageManager.h"

class CDlgSearchAndReplaceCircuitsResults : public CDialogExt, public CResizingColumnInfo::INotificationHandler
{
	DECLARE_DYNAMIC( CDlgSearchAndReplaceCircuitsResults )

public:
	enum { IDD = IDD_DLGSEARCHREPLACECIRCUITS_RESULTS };

	CDlgSearchAndReplaceCircuitsResults( CWnd *pParent = NULL );
	virtual ~CDlgSearchAndReplaceCircuitsResults();

	void SetInputParameters( CDB_CircuitScheme::eBALTYPE eBalType, bool bWorkWithIMIControlValve, CDB_ControlProperties::CvCtrlType eCtrlType, CDB_ControlProperties::eCVFUNC eCtrlValvType );
	void SetChangesList( std::vector<std::pair<CDlgSearchAndReplaceCircuits::CCircuit, CDlgSearchAndReplaceCircuits::CCircuit>> *pvecChangesList );

	// Overrides 'CResizingColumnInfo::INotificationHandler' notification handler.
	virtual void ResizingColumnInfoNH_OnSSheetColumnWidthChanged( UINT uiSheetDescriptionID, long lColumnID );

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
	// 'MessageManager' message handler.
	virtual afx_msg LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam );
	
	// Overides CWnd virtual method.
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );

// Private methods.
private:
	enum class SheetDescription
	{
		SD_Source = 1,
		SD_Target,
	};

	void _InitializeSSheet( SheetDescription eSheetDescription );
	void _FillSheet( SheetDescription eSheetDescription );
	void _OnCellClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow );
	void _OnKeyDown( CSheetDescription *pclSheetDescription );
	void _OnKeyUp( CSheetDescription *pclSheetDescription );
	void _OnKeyHome( CSheetDescription *pclSheetDescription );
	void _OnKeyEnd( CSheetDescription *pclSheetDescription );
	void _OnKeyPageDown( CSheetDescription *pclSheetDescription );
	void _OnKeyPageUp( CSheetDescription *pclSheetDescription );

private:
	CDB_CircuitScheme::eBALTYPE m_eBalType;
	bool m_bWorkWithIMIControlValve;
	CDB_ControlProperties::CvCtrlType m_eControlType;
	CDB_ControlProperties::eCVFUNC m_eCtrlValveType;

	std::vector<std::pair<CDlgSearchAndReplaceCircuits::CCircuit, CDlgSearchAndReplaceCircuits::CCircuit>> *m_pvecChangesList;
	CStatic m_StaticSourcesRect;
	CStatic m_StaticTargetsRect;

	enum class ColumnDefinition
	{
		CD_Index = 1,
		CD_CircuitName,
		CD_BalancingType,
		CD_CircuitType,
		CD_ValveType,
		CD_ControlType,
		CD_ControlValveType
	};

	enum class ColumWith
	{
		CW_Index = 3,
		CW_CircuitName = 15,
		CW_BalancingType = 17,
		CW_CircuitType = 17,
		CW_ValveType = 10,
		CW_ControlType = 11,
		CW_ControlValveType = 30
	};

	CViewDescription m_ViewDescription;
	CSSheet *m_pclSSheetSource;
	CSSheet *m_pclSSheetTarget;
	CResizingColumnInfo *m_pclResizingColumnnInfoSource;
	CResizingColumnInfo *m_pclResizingColumnnInfoTarget;
	CRect m_rectSource;
	CRect m_rectTarget;
};
