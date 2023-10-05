#pragma once


#include <utility>
#include <string>
using namespace std;

class CAnchorPt
{
public:
	// Be carefull each modification of eFunc should be ported into function CAnchorPt::TranslateTADBAnchorTxt()
	// that make the translation for anchoring points coming from TADB.txt
	// This enum is also used into the HM construction, check EnBitmapPatchWork and PanelCir2 make a search on "CAnchorPt::"

	enum eFunc
	{
		eFuncNU,
		BV_P,
		BV_Byp,
		BV_S,
		ControlValve,
		DpC,
		PICV,
		PUMP,
		ShutoffValve,				// Shut-off valve.
		DPCBCV,						// Combined Dp controller, control and balancing valve.
		SmartControlValve,			// Smart control valve.
		TempSensor,					// External temperature housing.
		SmartDpC,					// Smart differential pressure controller.
		DpSensor,					// Differential pressure measuring tool.
		eFuncLast,
		Optional = 0x8000,
	};

	enum PipeType
	{
		PipeType_Undefined,
		PipeType_Distribution,
		PipeType_Circuit,
		PipeType_NotApplicable
	};

	enum PipeLocation
	{
		PipeLocation_Undefined,
		PipeLocation_Supply,
		PipeLocation_Return,
		PipeLocation_ByPass,
		PipeLocation_NotApplicable
	};

	enum CircuitSide
	{
		CircuitSide_Undefined,
		CircuitSide_Primary,
		CircuitSide_Secondary,
		CircuitSide_NotApplicable
	};

	class _AnchorPtFunc
	{
	public:
		_AnchorPtFunc()
		{
			m_eFunction = eFuncNU;
			*m_tcName = L'\0';
			m_bOptional = false;
			m_ePipeType = PipeType_Undefined;
			m_ePipeLocation = PipeLocation_Undefined;
			m_eCircuitSide = CircuitSide_Undefined;
		};

		eFunc m_eFunction;
		TCHAR m_tcName[30];
		bool m_bOptional;
		PipeType m_ePipeType;
		PipeLocation m_ePipeLocation;
		CircuitSide m_eCircuitSide;
	};

	typedef class _AnchorPtFunc AnchorPtFunc;

#ifndef TACBX
	enum eStruct
	{
		sNU,						// Not used
		s2W_Straight,
		s2W_angle,
		s3W,
		sLastStruct
	};

	enum ePressSign
	{
		Pminus,
		Pplus,
		PUndef,						// let the engine determine where it will connect the capillary point
		PNU,						// Not used
		LastPressSign
	};

	enum TempConnection
	{
		TC_None,
		TC_Yes
	};

	enum IOConnection
	{
		IOC_None,
		IOC_Yes
	};

	enum eConnectType
	{
		Beg,
		Mid,
		End,
		Spline,
		LastConnType = Spline
	};

	enum eValveOrientation
	{
		voCCW = 0,							// CounterClockWise
		voCW								// ClockWise
	};

public:
	CAnchorPt();
	CAnchorPt( 		pair<int,int>	pt, 
					eConnectType	ConnectType, 
					TCHAR			tcDirection,				// N,W,S,E	
					eFunc			eFunction,
					eStruct			eStructure, 
					bool			fClockwise = voCCW,
					ePressSign		ePressureSign = PNU,
					TempConnection	eTempConnection = TC_None,
					IOConnection	eIOConnection = IOC_None,
					unsigned short	usGroup = 0 );

	CAnchorPt( 		pair<int,int>	pt, 
					eConnectType	ConnectType, 
					eFunc			eFunction,
					eStruct			eStructure, 
					ePressSign		ePressureSign,
					TempConnection	eTempConnection,
					IOConnection	eIOConnection,
					unsigned short	usGroup );

	CAnchorPt( 		pair<int,int>	pt, 
					eConnectType	ConnectType, 
					unsigned short	usGroup );

	CAnchorPt(		pair<int,int>	pt); 


	virtual ~CAnchorPt() {}
	
	void Reset();

	enum class TranslateError
	{
		OK = 0,
		BadSyntax = -1,
		BadAnchorPt = -2,
		BadPipeType = -3,
		BadPipeLocation = -4,
		BadCircuitSide = -5
	};

	static TranslateError TranslateTADBAnchorTxt( wstring str, CAnchorPt::AnchorPtFunc &rAnchorPtFunc );

	CAnchorPt* SetGroup( unsigned short usGroup ) { m_usGroup = usGroup; return this; }
	void SetPoint( pair<int, int> *pt );
	void SetPoint( pair<int, int>	pt, 
					eConnectType	ConnecType = Spline, 
					TCHAR			tcDirection = 'N',				// N,W,S,E	
					eFunc			eFunction = eFuncLast,
					eStruct			eStructure = sLastStruct, 
					bool			fClockwise = voCCW,
					ePressSign		ePressureSign = PNU,
					TempConnection	eTempConnection = TC_None,
					IOConnection	eIOConnection = IOC_None,
					unsigned short	usGroup = 0 );
	void SetFunction( eFunc Func ) { m_eFunction = Func; }
	CAnchorPt *SetPressureSign( ePressSign ePressureSign ) { m_ePressureSign = ePressureSign; return this; }
	void SetTempConnection( TempConnection eTempConnection ) { m_eTempConnection = eTempConnection; }
	void SetIOConnection( IOConnection eIOConnection ) { m_eIOConnection = eIOConnection; }
	CAnchorPt *SetConnectType( eConnectType eType ) { m_eConnectType = eType; return this; }
	void SetDirection( TCHAR tcDirection );
	void SetDirection( int iDirection );
	void SetClockwise( eValveOrientation ValveOrientation ) { m_fClockwise = ( eValveOrientation::voCCW == ValveOrientation ) ? false : true; } 
	void SetVirtualPoint( bool bVirtualPoint ) { m_bVirtualPoint = bVirtualPoint; }
	void SetAngle( double dAngle ) { m_dAngle = dAngle; }
	void SetCtrlPointDistance( int iCtrlPointDistance ) { m_iCtrlPointDistance = iCtrlPointDistance; }

	const unsigned short GetGroup() { return m_usGroup; }
	const pair<int, int> GetPoint() { return m_pairCoordinate; }
	const eFunc GetFunction() { return m_eFunction; }
	const ePressSign GetPressureSign() { return m_ePressureSign; }
	const bool IsPressureSignDefined() { return ( ePressSign::Pplus == m_ePressureSign || ePressSign::Pminus == m_ePressureSign ) ? true : false; }
	const TempConnection GetTempConnection() { return m_eTempConnection; }
	const bool IsTempConnectionDefined() { return ( TempConnection::TC_Yes == m_eTempConnection ) ? true : false; }
	const IOConnection GetIOConnection() { return m_eIOConnection; }
	const bool IsIOConnectionDefined() { return ( IOConnection::IOC_Yes == m_eIOConnection ) ? true : false; }
	const eConnectType GetConnectType() { return m_eConnectType; }
	const bool IsConnectionDefined() { return ( true == IsPressureSignDefined() || true == IsTempConnectionDefined() || true == IsIOConnectionDefined() ) ? true : false; }
	const eStruct GetStructure() { return m_eStruct; }
	const int GetDirection() { return m_iDirection; }
	const bool IsClockwise() { return m_fClockwise; }
	const bool IsVirtualPoint() { return m_bVirtualPoint; }
	const double GetAngle() { return m_dAngle; }
	const int GetCtrlPointDistance() { return m_iCtrlPointDistance; }

	pair<int, int> GetPairCompoSize() const { return m_pairCompoSize; }
	void SetPairCompoSize(pair<int, int> val) { m_pairCompoSize = val; }


protected:
	unsigned short		m_usGroup;
	pair<int, int>		m_pairCoordinate;
	eFunc				m_eFunction;				// eFuncNU, BV_P, BV_Byp, BV_S, ControlValve, DpC, PICV or PUMP.
	ePressSign			m_ePressureSign;			// Pplus, PMinus available anchoring point pressure.
	TempConnection		m_eTempConnection;			// TC_None or TC_Yes.
	IOConnection		m_eIOConnection;			// IOC_None or IOC_Yes.
	eConnectType		m_eConnectType;				// Beg, Mid, End or Spline.
	eStruct				m_eStruct;					// sNU,	s2W_Straight, s2W_angle or s3W.
	int					m_iDirection;				// 0° for east; 270° for south
	bool				m_fClockwise;				// false = CounterClockwise, true = Clockwise
	pair<int, int>		m_pairCompoSize;			// Used during working to know what is the size of component attached in pairCoordinate

	// Theses variables help us to introduce a point in the schematic where we want to end a spline curve.
	// Remark: usually the spline curve is defined between two components. But if we have curve that starts from a component and must end
	//         on a pipe we don't have a component. This is why we have now a virtual point. And because the ctrl point distance and angle
	//         is defined in ProdPic (for the component) we add here 'm_dAngle' and 'm_iCtrlPointDistance' that will be used in the same
	//         way.
	bool				m_bVirtualPoint;
	double				m_dAngle;
	int					m_iCtrlPointDistance;		// If defined, we use this distance in pixel to compute the first and last point for
													// the bezier curves (See the "CEnBitmapPatchWork::CalculateCtrlPoint" method in the "EnBitmapPatchWork.cpp" file).
#endif
};
