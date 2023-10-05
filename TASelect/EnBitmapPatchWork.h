#ifndef _SCHEMEBITMAP_H_
#define _SCHEMEBITMAP_H_

#include "EnBitmapAnchorPt.h"
#include "EnBitmapComponent.h"


#pragma once

class  CEnBitmapPatchWork : public CEnBitmap
{
public:
	CEnBitmapPatchWork( CEnBitmapCompMngr *pComponentManager = 0 );
	virtual ~CEnBitmapPatchWork();

	// HYS-1579: To have an equivalent with CSheetPanelCirc1.
	enum class ePatchWorkTerminalUnit
	{
		eNone,
		eTermUnit,
		eRadiator
	};

	enum SplineType
	{
		ST_Capillary,
		ST_TempSensor,
		ST_InputOutput
	};

	// Setter.
	void SetComponentManager( const CEnBitmapCompMngr *pComponentManager );
	void SetBackgroundColor( COLORREF color );
	
	// Manage the double buffer.
	CDC *GetDoubleBufferDC( CDC *pDC );
	void CopyDC( CDC *pDCSrc, CDC *pDCDst, pair<int, int> *ptOrigin = 0 );
	
	// Append Top and Bottom Images (Terminal unit, distribution pipes.
	// This operation should be done AFTER the DrawPatchWork.
	// Anchoring points are no moved by this operation.
	void Append( CEnBitmap *pBmp, bool fTop );

	// CDC Manipulation
	void RotateDC( CDC *pDCDest, CEnBitmap *pEnBitmap, int degree = 0, pair<int, int> *pAnchor = 0, pair<int, int> *ptRotCenter = 0, COLORREF backgroundColor = -1 );	
	void MapImagePointToDC( CAnchorPt* pAnchor, CEnBitmapComponent *pComponent, pair<int, int> *pPtAnchor, pair<int, int> &pt );
	BOOL MapPointToAnchor( pair<int, int> *pAnchor, pair<int, int> *pPt );
	BOOL MapPointToDefault( pair<int, int> *pAnchor, pair<int, int> *pPt );
	double GetRadian( pair<int, int> *pPt );
	
	// Method to draw a Scheme Part.
	// HYS-1579: We have an enum class for Terminal unit values
	void GetHydronicScheme( CDB_CircuitScheme *pCircuitScheme, int eUseTermUnit, bool fCVModulating, CArray<CAnchorPt::eFunc>*paAnchorPt = NULL, bool fDistributionPipe = false, CDS_HydroMod::ReturnType eReturnType = CDS_HydroMod::ReturnType::Direct );
	void GetHydronicScheme( CDS_HydroMod *pHM );

	// Special to debug.
	void GetHydronicScheme( CString strCircuitSchemeID, std::map<CAnchorPt::eFunc, CString> *pmapProductList, bool bControlProportional = true, bool bTerminalUnit = true, 
			int iLevel = 1, CDS_HydroMod::ReturnType eReturnType = CDS_HydroMod::ReturnType::Direct );

	CDynCircSch *LoadBackGround(CDB_CircuitScheme *pCircuitScheme );
	void LoadAnchoringPoints( CDB_CircuitScheme *pCircuitScheme, CDynCircSch*pDynamicCircuitScheme, bool fMarkOptionalComponent /*=false*/ );

	// For DpC individual selection.
	CDB_CircuitScheme *FindSSelDpCHydronicScheme( eMvLoc MvLoc, eDpCLoc DpCLoc, eDpStab DpStab, bool bForSet = false );
	CDB_CircuitScheme *FindSSelDpCHydronicScheme( int index, CDS_SSelDpC **pSelDpC );
	void GetSSelDpCHydronicScheme( CDB_CircuitScheme *pCircSch, IDPTR ValveIDPtr, IDPTR DpCIDPtr, eDpCLoc DpCLoc = DpCLocDownStream) ;
	void AddSSelDpCArrows( CDB_CircuitScheme *pCircSch );

	// For combined Dp controller, control and balancing valve.
	CDB_CircuitScheme *FindSSelDpCBCVHydronicScheme( eDpStab eDpStabilized, ShutoffValveLoc eShutoffValveLocation );
	void GetSSelDpCBCVHydronicScheme( CDB_CircuitScheme *pCircuitScheme, IDPTR DpCBCVIDPtr, IDPTR SVIDPtr = _NULL_IDPTR );

	void Add_TU_DistrPipes( CString TU, COLORREF colTU, CString Pipes, COLORREF colBot, int iOffset = 0 );

	void DrawPatchwork( int MvLoc, int DpCLoc );
	void DrawScheme( CDC *pDC, pair<int, int> *ptOrigin = 0 );
	void DrawBackground( CDC *pDC, ULONG ulBcKID );
	
	// Draw required components at all listed anchoring points.
	void DrawAnchor( CDC *pDC );

	void DrawComponent( CDC *pDC, CAnchorPt *anchor );
	void AddPtSpline( CDC *pDC, CAnchorPt *pAnchor, CEnBitmapComponent *pComponent = NULL );
	void DrawSpline( CDC *pDC, int Group );
	
	//////////////////////////////////////////////////////////////
	/// Other Methods
	//////////////////////////////////////////////////////////////
	const CString GetName() { return m_strName; }
	void SetName(CString name) { m_strName = name; }
	const ULONG GetBackground() { return m_ulBackground; }
	void SetBackground(ULONG bck) { m_ulBackground = bck; }
	CPtrArray *GetArrayAnchor() { return &m_ArrayAnchor; }

	// Add one or more AnchorPoint.
	void AddAnchor( CAnchorPt *pAnchoringPoint, int iAnchor = 1 );
	
	CAnchorPt *FindAnchoringPt( CAnchorPt::eFunc );
	
	// Reset Patchwork internal members.
	void Reset();
	
	void CleanPtList( CPtrArray *pPtArray );
	void CleanArrayAnchor() { m_ArrayAnchor.RemoveAll(); }

	// Allow to find the capillary point connection.
	// Once found, this one must be converted from the coordinate defined for the symbol (Or the picture)
	// to the coordinate in the hydraulic circuit.
	pair<int, int> *FindCapillPt( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent );

	// Allow to find the temperature sensor cable point connection.
	// Once found, this one must be converted from the coordinate defined for the symbol (Or the picture)
	// to the coordinate in the hydraulic circuit.
	pair<int, int> *FindTempSensorCablePt( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent );

	// Allow to find the input/output signal cable point connection.
	// Once found, this one must be converted from the coordinate defined for the symbol (Or the picture)
	// to the coordinate in the hydraulic circuit.
	pair<int, int> *FindInputOutputCablePt( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent );

	// Allow to convert a virtual point that is defined in the component coordinate system (In the picture defines with 'NEWPRODPIC' in the 'dllmain.cpp'
	// file from the 'HydrocPicsDLL' project) to the final bitmap coordinate system.
	void ConvertComponentCoordinateSystemToBitmapCoordinateSystem( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent, pair<int, int> *ptPoint );

	// Allow to convert a point that is defined in the hydraulic circuit coordinate system to the final bitmap coordinate system.
	void ConvertHydraulicCircuitCoordinateSystemToBitmapCoordinateSystem( CAnchorPt *pAnchor, CEnBitmapComponent *pComponent, pair<int, int> *ptPoint );

	pair<int, int> *CalculateCtrlPoint( pair<int, int> *pPtCurve, CAnchorPt* pAnchor, CEnBitmapComponent *pComponent );

protected:
	void PrepareDefaultComponents( CDB_CircuitScheme *pCircuitScheme, bool fCVModulating );
	void PrepareHydronicScheme( CDB_CircuitScheme *pCircuitScheme, bool fMarkOptionalComponent = false );
	bool MvShouldBeReturned( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor );
	_Pt *GetCapilPt( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor );
	
	double GetCapilAngle( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor );
	double GetTempSensorCableAngle( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor );
	double GetInputOutputCableAngle( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor );

	// To draw a bezier curve between two components (For example: TA-Smart and external temperature housing),
	// we have the start and end point with their angles. In the "CEnBitmapPatchWork::CalculateCtrlPoint" method,
	// control point for the bezier curve will be computed with a certain distance in pixel from the start or the end point.
	// By default, this distance is the maximum length between height and width of the current component from which we 
	// compute the control point. For TA-Smart symbol, the distance becomes to big and this does not allow us to 
	// correctly place intermediate points to well draw the curve. So, in the definition of anchoring point (See dllmain.cpp)
	// we can add a fixed distance for the first and last control point of the bezier curve.
	int GetCtrlPointDistance( CEnBitmapComponent *pComponent, CAnchorPt *pAnchor );
	
// Private variables.
private:
	CString				m_strName;					// Scheme's name
	ULONG				m_ulBackground;			// Background ID
	CPtrArray			m_ArrayAnchor;          // list of anchor point
	CEnBitmapCompMngr	*m_pComponentManager;   // ptr to a manager of all components
	COLORREF			m_BackgroundColor;      // background color
	typedef CMap<int,int,CPtrArray*,CPtrArray*> _GroupMap;
	_GroupMap			m_GroupMap;
	int					m_iMvLoc;
	int					m_iDpCLoc;
	CStringArray		m_arstrDefaultComponents;
};

#endif	//_SCHEMEBITMAP_H_