#pragma once


#include "tchart.h"
#include "export.h"


#define DLGPMTCHARTHELPER_WALLBACKCOLOR						RGB( 255, 255, 255 )
#define DLGPMTCHARTHELPER_PANELCOLOR						RGB( 255, 255, 255 )
#define DLGPMTCHARTHELPER_HEADERFONTCOLOR					RGB( 0, 0, 0 )
#define DLGPMTCHARTHELPER_GRIDPENCOLOR						RGB( 226, 226, 226 )
#define DLGPMTCHARTHELPER_MINORGRIDCOLOR					RGB( 128, 128, 128 )
#define DLGPMTCHARTHELPER_AXISPENCOLOR						RGB( 64, 64, 64 )
#define DLGPMTCHARTHELPER_WORKPOINTLEGENDITEMCOLOR			RGB( 100, 100, 100 )
#define DLGPMTCHARTHELPER_WORKPOINTLEGENDSYMBOLCOLOR		RGB( 135, 135, 135 )

#define DLGPMTCHARTHELPER_STATICO_P0						RGB( 74, 144, 226 )		// p0 (minimum pressure) - dark blue
#define DLGPMTCHARTHELPER_STATICO_PT						RGB( 0, 0, 0 )			// p(t) - black
#define DLGPMTCHARTHELPER_STATICO_PA						RGB( 8, 148, 161 )		// pa (initial pressure) - light blue
#define DLGPMTCHARTHELPER_STATICO_PAMIN						RGB( 34, 94, 130 )		// pa,min (minimum initial pressure) - blue
#define DLGPMTCHARTHELPER_STATICO_PFILL						RGB( 71, 171, 108 )		// pfill (filling pressure) - green
#define DLGPMTCHARTHELPER_STATICO_PE						RGB( 245, 166, 35 )		// pe (final pressure) - orange
#define DLGPMTCHARTHELPER_STATICO_PSV						RGB( 208, 2, 27 )		// PSV - light red

#define DLGPMTCHARTHELPER_COMPRESSO_CURVE1					RGB( 141, 162, 199)		// Polo blue (Approximative)
#define DLGPMTCHARTHELPER_COMPRESSO_CURVE2					RGB( 247, 189, 122)		// Rajah (Light orange) (Approximative)
#define DLGPMTCHARTHELPER_COMPRESSO_CURVE3					RGB( 91, 159, 174 )		// Hippie blue (Approximative)
#define DLGPMTCHARTHELPER_COMPRESSO_CURVE4					RGB( 243, 105, 58)		// Flamingo (Near orange) (Approximative)
#define DLGPMTCHARTHELPER_COMPRESSO_CURVE5					RGB( 59, 78, 118 )		// East bay (Dark blue) (Approximative)

#define DLGPMTCHARTHELPER_COMPRESSOCX_CURVE1				RGB( 59, 78, 118 )		// East bay (Dark blue) (Approximative)
#define DLGPMTCHARTHELPER_COMPRESSOCX_CURVE2				RGB( 243, 105, 58)		// Flamingo (Near orange) (Approximative)
#define DLGPMTCHARTHELPER_COMPRESSOCX_CURVE3				RGB( 141, 162, 199)		// Polo blue (Approximative)

#define DLGPMTCHARTHELPER_TRANSFERO_CURVE1					RGB( 141, 162, 199)		// Polo blue (Approximative)
#define DLGPMTCHARTHELPER_TRANSFERO_CURVE2					RGB( 247, 189, 122)		// Rajah (Light orange) (Approximative)
#define DLGPMTCHARTHELPER_TRANSFERO_CURVE3					RGB( 91, 159, 174 )		// Hippie blue (Approximative)
#define DLGPMTCHARTHELPER_TRANSFERO_CURVE4					RGB( 243, 105, 58)		// Flamingo (Near orange) (Approximative)
#define DLGPMTCHARTHELPER_TRANSFERO_CURVE5					RGB( 59, 78, 118 )		// East bay (Dark blue) (Approximative)

#define DLGPMTCHARTHELPER_STATICO_LEGENDWIDTH				80
#define DLGPMTCHARTHELPER_COMPRESSO_LEGENDWIDTH				140
#define DLGPMTCHARTHELPER_COMPRESSOCX_LEGENDWIDTH			180
#define DLGPMTCHARTHELPER_TRANSFERO_LEGENDWIDTH				120

class CSelectedVssl;
class CDlgPMTChartHelper : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgPMTChartHelper )

public:
	enum { IDD = IDD_DLGPMTCHARTHELPER };
	CDlgPMTChartHelper( CWnd *pParent = nullptr );
	virtual ~CDlgPMTChartHelper();

	void DrawExpansionVesselCurves( CSelectedVssl *pclSelectedVessel, CPMInputUser *pclPMInputUser );
	void DrawTransferoCurves( CDB_TecBox *pclTransferoReference, double dPmanSI, double dqNSI, CRankEx &clTransferoTecBoxList, CPMInputUser *pclPMInputUser );
	void DrawCompressoCurves( CDB_TecBox *pclCompressoReference, double dPmanSI, double dqNSI, CRankEx &clCompressoTecBoxList, CPMInputUser *pclPMInputUser );
	void DrawCompressoCXCurves( CDB_TecBox *pclCompressoReference, double dPmanSI, double dqNSI, CRankEx &clCompressoTecBoxList, CPMInputUser *pclPMInputUser, int iNbrOfDevices );

	CExport GetExport() { return m_TChart.GetExport(); }

protected:
	DECLARE_EVENTSINK_MAP()
	void OnGetLegendPosTchart1( long lItemIndex, long *plX, long *plY, long *plXColor );
	virtual BOOL OnInitDialog();

private:
	CTChart m_TChart;
	
	// HYS-1022: 'pa' becomes 'pa,min' and 'pa,opt' becomes 'pa'.
	// Change the order to allow 'pt' to be overwritten by bubbles.
	enum { PC_First, PC_p0 = PC_First, PC_pt, PC_pa, PC_pamin, PC_pfill, PC_pe, PC_PSV, PC_Last };
	std::map<int, int> m_mapPCTypeToSeries;

	typedef struct _LegendPosInfo
	{
		std::map<int, long> m_mapTopPos;
		std::map< int, int> m_mapIndexOrder;
	}LegendPosInfo;
	
	LegendPosInfo m_rLegendPosInfo;
	bool m_bAnnotationTextDisplayed;

	enum CurrentDeviceType
	{
		Undefined,
		Vessel,
		Compresso,
		CompressoCX,
		Transfero
	};
	CurrentDeviceType m_eCurrentDeviceType;

	bool m_bStaticoLeftAxisInitialized;
	double m_dYMinPressSI;
	double m_dYMinPressCU;
	double m_dYMaxPressSI;
	double m_dYMaxPressCU;

	double m_dWorkingPointqNSI;
	double m_dWorkingPointPmanSI;

	long m_lLegendLeft;
	int m_iNextLegendItemYPos;
};
