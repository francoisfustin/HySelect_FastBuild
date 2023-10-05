#pragma once


#include "MyToolTipCtrl.h"
#include "ExtComboBox.h"
#include "XGroupBox.h"

class CSSheet;
class CViewDescription;
class CDlgPipeDp : public CDialogEx
{
public:
	enum { IDD = IDD_DLGPIPEDP };

	CDlgPipeDp( CWnd* pParent = NULL );
	virtual ~CDlgPipeDp();

	// Opens the dialog window.
	// Returns the same value as domodal().
	void Display();

protected:
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonModWater();
	afx_msg void OnChangeEditFlow();
	afx_msg void OnSelChangeComboPipeName();

// Private members
private:
	// Fill the combo pipe.
	// Para: ptcsPipe specify the default selection. If NULL, the "Generic ...".
	void _FillComboPipe( LPCTSTR ptcsPipe = NULL );
	
	// Retrieve the pipe series name which is selected in the pipe name Combo
	LPCTSTR _GetSelPipeName();
	
	// Retrieve the IDPtr's of the pipe and of the pipe family which are selected
	bool _GetSelPipeIDPtr( IDPTR& PipeFamIDPtr );
	
	// Update all fields.
	void _UpdateAll();

	// Methods for CSSheet.
	void _InitializeSSheet( void );

	// Use for std::sort in '_UpdateAll'.
	struct rPipe;
	static bool _ComparePipe( rPipe i, rPipe j );

// Private variables.
private:
	enum SheetDescription
	{
		SD_PipeResult = 1
	};
	
	enum RowDescription_PipeResult
	{
		RD_PipeResult_FirstRow = 1,
		RD_PipeResult_ColName,
		RD_PipeResult_Unit,
		RD_PipeResult_FirstAvailRow
	};
	
	enum ColumnDescription_PipeResult
	{
		CD_PipeResult_FirstColumn = 1,
		CD_PipeResult_Icon,
		CD_PipeResult_Name,
		CD_PipeResult_LinearDp,
		CD_PipeResult_Velocity,
		CD_PipeResult_FlowRegime,
		CD_PipeResult_LastColumn
	};

	struct rPipe
    {
    	bool fBest;
    	CString strPipeName;
    	double dDiameter;
    	double dLinearDp;
    	double dVelocity;
		CString strFlowRegime;
    };

	CTADatastruct *m_pTADS;			// Data structure constituting the document
	CDS_TechnicalParameter *m_pTechParam;
	CWaterChar m_clWaterChar;
	ProjectType m_eCurrentProjectType;
	CViewDescription m_ViewDescription;
	CEdit m_EditFlow;
	CExtNumEditComboBox m_ComboPipeName;
	CMFCButton m_ButtonModWater;
	CXGroupBox m_GroupPipe;
	CXGroupBox m_GroupWater;
	CXGroupBox m_GroupQ;

	CMyToolTipCtrl m_ToolTip;			// ToolTipCtrl
	double m_dFlow;
};
