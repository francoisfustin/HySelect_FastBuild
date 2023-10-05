#pragma once


#include "tchart.h"

#define DLGDIVFACTOR_FAKESHEETCOLS		50
#define DLGDIVFACTOR_FAKESHEETROWS		20
class CSelProdPageDiversityFactor : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_DF = 1,
	};

	CSelProdPageDiversityFactor( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageDiversityFactor( void );
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return false; }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_DIVERSITYFACTOR; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam );

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _OnMouseMove( long lMouseMoveCol, long lMouseMoveRow );
	void _OnLButtonDown( long lLButtonDownCol, long lLButtonDownRow );

	bool _CreateTitleZone( CSheetDescription* pclSheetDescription );

	bool _CreateGraphZone( CSheetDescription* pclSheetDescription );
	void _FillDFValvesGraph( void );
	void _FillDFPipesGraph( void );
	void _PutGraphInSheet( CSheetDescription* pclSheetDescription );

	void _InitDFTextHeader( CSheetDescription* pclSheetDescription );
	void _FillDFValvesText( CSheetDescription* pclSheetDescription );
	void _FillDFPipesText( CSheetDescription* pclSheetDescription );

	void _SetLastRow( CSheetDescription* pclSheetDescription, long lLastRow );

// Private variables.
private:
	CTableHM* m_pclSelectionTableHM;

	// For valves graph.
	typedef std::map< bool, short > mapValveBeforeAfter;
	typedef mapValveBeforeAfter::iterator mapValveBeforeAfterIter;
	typedef std::map< int, mapValveBeforeAfter > mapIntValveBeforeAfter;
	typedef mapIntValveBeforeAfter::iterator mapValveGraphIter;
	mapIntValveBeforeAfter m_mapValvesGraph;

	// For pipes graph.
	typedef std::map< bool, double > mapPipeBeforeAfter;
	typedef mapPipeBeforeAfter::iterator mapPipeBeforeAfterIter;
	typedef std::map< int, mapPipeBeforeAfter > mapIntPipeBeforeAfter;
	typedef mapIntPipeBeforeAfter::iterator mapPipeGraphIter;
	mapIntPipeBeforeAfter m_mapPipesGraph;

	// For valves text.
	typedef std::map< int, short > mapValveCounter;
	typedef mapValveCounter::iterator mapValveCounterIter;
	typedef std::map< int, mapValveCounter > mapIntValveCounter;
	typedef mapIntValveCounter::iterator mapTextValveIter;
	typedef mapIntValveCounter::reverse_iterator mapTextValveRevIter;
	mapIntValveCounter m_mapValvesText;
		
	// For pipes text.
	typedef std::map< int, double > mapPipeCounter;
	typedef mapPipeCounter::iterator mapPipeCounterIter;
	typedef std::map< int, mapPipeCounter > mapIntPipeCounter;
	typedef mapIntPipeCounter::iterator mapTextPipeIter;
	typedef mapIntPipeCounter::reverse_iterator mapTextPipeRevIter;
	mapIntPipeCounter m_mapPipesText;

	std::map< int, CString > m_mapSizeString;

	CTChart m_TChartValves;
	CTChart m_TChartPipes;
	HICON m_hHandOpened;
	bool m_fCursorChanged;
	CEnBitmap m_clChartValvesBmp;
	CEnBitmap m_clChartPipesBmp;
		
	enum RowDescription
	{
		RD_Empty = 1,
		RD_Title,
		RD_Empty2,
		RD_DFDetails,
		RD_DFClick,
		RD_Empty3,
		RD_Graphs,
		RD_Empty4,
		RD_Text_ColumnHeader,
		RD_Text_FirstData
	};

	enum DFColumnDescription
	{
		CD_HeaderCol = 1,
		CD_ValveNumberBefore,
		CD_ValveNameBefore,
		CD_ValveSeparator,
		CD_ValveNumberAfter,
		CD_ValveNameAfter,
		CD_Separator,
		CD_PipeNumberBefore,
		CD_PipeNameBefore,
		CD_PipeSeparator,
		CD_PipeNumberAfter,
		CD_PipeNameAfter,
		CD_FooterCol,
		CD_Pointer,
	};
};
