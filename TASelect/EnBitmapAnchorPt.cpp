#include "StdAfx.h"

#ifndef TACBX
#include "tchar.h"
#include <atlstr.h>
#include <regex>
#include <map>
#endif 

#include "global.h"
#include "EnBitmapAnchorPt.h"

#ifndef TACBX
CAnchorPt::CAnchorPt()
{
	Reset();
}

CAnchorPt::CAnchorPt(
						pair<int,int>	pt, 
						eConnectType	ConnectType,		// Beg, Mid, End or Spline.
						TCHAR			tcDirection,		// 'N', 'W', 'S', 'E'.
						eFunc			eFunction,			// eFuncNU, BV_P, BV_Byp, BV_S,	ControlValve, DpC, PICV, PUMP, ShutoffValve, DPCBCV, SmartControlValve or TempSensor.
						eStruct			eStructure,			// sNU, s2W_Straight, s2W_angle or s3W.
						bool			fClockwise,			// true or false.
						ePressSign		ePressureSign,		// Pminus, Pplus, PUndef or PNU.
						TempConnection	eTempConnection,	// TC_None or TC_Yes.
						IOConnection	eIOConnection,		// IOC_None or IOC_Yes.
						unsigned short	usGroup				// = 0
					)
{
	Reset();
	SetPoint( pt, ConnectType, tcDirection, eFunction, eStructure, fClockwise, ePressureSign, eTempConnection, eIOConnection, usGroup );
}

CAnchorPt::CAnchorPt( pair<int, int> pt, eConnectType ConnectType, eFunc eFunction, eStruct eStructure, ePressSign ePressureSign, TempConnection eTempConnection, 
	IOConnection eIOConnection, unsigned short usGroup )
{
	Reset();
	SetPoint( pt, ConnectType, 'N', eFunction, eStructure, 0, ePressureSign, eTempConnection, eIOConnection, usGroup );
}

CAnchorPt::CAnchorPt( pair<int, int> pt, eConnectType ConnectType, unsigned short usGroup )
{
	Reset();
	SetPoint( pt, ConnectType, 'N', CAnchorPt::eFuncLast, sLastStruct, 0, PNU, TC_None, IOC_None, usGroup );
}

CAnchorPt::CAnchorPt( pair<int,int> pt )
{
	Reset();
	SetPoint( &pt );
}

void CAnchorPt::Reset()
{
	m_usGroup = 0;
	m_pairCoordinate.first = -1;
	m_pairCoordinate.second = -1;
	m_eFunction = CAnchorPt::eFuncLast;
	m_ePressureSign = ePressSign::PUndef;
	m_eConnectType = LastConnType;
	SetDirection( 0 );
	m_eStruct = sLastStruct;
	m_fClockwise = 0;
	m_pairCompoSize.first = -1;
	m_pairCompoSize.second = -1;
	m_bVirtualPoint = false;
	m_dAngle = 0.0;
	m_iCtrlPointDistance = INT_MIN;
}

CAnchorPt::TranslateError CAnchorPt::TranslateTADBAnchorTxt( wstring str, CAnchorPt::AnchorPtFunc &rAnchorPtFunc )
{
	std::map<CString, CAnchorPt::eFunc> mapAchPt = {
		{ _T("nu"), eFunc::eFuncNU },
		{ _T("bv_p"), eFunc::BV_P },
		{ _T("bv_byp"), eFunc::BV_Byp },
		{ _T("bv_s"), eFunc::BV_S },
		{ _T("controlvalve"), eFunc::ControlValve },
		{ _T("dpc"), eFunc::DpC },
		{ _T("picv"), eFunc::PICV },
		{ _T("pump"), eFunc::PUMP },
		{ _T("shutoffvalve"), eFunc::ShutoffValve },
		{ _T("dpcbcv"), eFunc::DPCBCV },
		{ _T("smartcontrolvalve"), eFunc::SmartControlValve },
		{ _T("tempsensor"), eFunc::TempSensor },
		{ _T("smartdpc"), eFunc::SmartDpC },
		{ _T("dpsensor"), eFunc::DpSensor } };

	std::map<CString, PipeType> mapPipeTypes = {
		{ _T("d"), PipeType_Distribution },
		{ _T("c"), PipeType_Circuit },
		{ _T("n"), PipeType_NotApplicable } };

	std::map<CString, PipeLocation> mapPipeLocations = {
		{ _T("s"), PipeLocation_Supply },
		{ _T("r"), PipeLocation_Return },
		{ _T("b"), PipeLocation_ByPass },
		{ _T("n"), PipeLocation_NotApplicable } };

	std::map<CString, CircuitSide> mapCircuitSides = {
		{ _T("p"), CircuitSide_Primary },
		{ _T("s"), CircuitSide_Secondary },
		{ _T("n"), CircuitSide_NotApplicable } };
	
	std::wsmatch match;
	std::wregex reg_ex( _T("\\[(\\w{3})\\]") );

	if( false == std::regex_search( str, match, reg_ex ) )
	{
		return TranslateError::BadSyntax;
	}

	if( 2 != match.size() ) 
	{
		return TranslateError::BadSyntax;
	}

	CString strAchPt = match.prefix().str().c_str();
	strAchPt.Trim();
	strAchPt.MakeLower();
	
	unsigned short i = 0;
	rAnchorPtFunc.m_bOptional = false; 

	if( _T('*') == strAchPt[0] )
	{
		strAchPt.Delete( 0, 1 );
		rAnchorPtFunc.m_bOptional = true;
	}
	
	if( 0 == mapAchPt.count( strAchPt ) )
	{
		return TranslateError::BadAnchorPt;
	}

	rAnchorPtFunc.m_eFunction = mapAchPt[strAchPt];
	_tcscpy( rAnchorPtFunc.m_tcName, match.prefix().str().c_str() );

	CString strLocation = match[1].str().c_str();
	strLocation.Trim();
	strLocation.MakeLower();

	if( 0 == mapPipeTypes.count( CString( strLocation[0] ) ) )
	{
		return TranslateError::BadPipeType;
	}

	rAnchorPtFunc.m_ePipeType = mapPipeTypes[CString( strLocation[0] )];

	if( 0 == mapPipeLocations.count( CString( strLocation[1] ) ) )
	{
		return TranslateError::BadPipeLocation;
	}

	rAnchorPtFunc.m_ePipeLocation = mapPipeLocations[CString( strLocation[1] )];

	if( 0 == mapCircuitSides.count( CString( strLocation[2] ) ) )
	{
		return TranslateError::BadCircuitSide;
	}

	rAnchorPtFunc.m_eCircuitSide = mapCircuitSides[CString( strLocation[2] )];

	return TranslateError::OK;
}

void CAnchorPt::SetPoint( pair<int, int> *pt )
{
	if( NULL == pt )
		return;

	m_pairCoordinate = *pt;
}

void CAnchorPt::SetPoint(	pair<int,int>	pt, 
							eConnectType	ConnecType,			// Beg, Mid, End or Spline.
							TCHAR			tcDirection,		// 'N', 'W', 'S', 'E'.
							eFunc			eFunction,			// eFuncNU, BV_P, BV_Byp, BV_S,	ControlValve, DpC, PICV, PUMP, ShutoffValve, DPCBCV, SmartControlValve or TempSensor.
							eStruct			eStructure,			// s2W_Straight, s2W_angle or s3W.
							bool			fClockwise,			// = 0
							ePressSign		ePressureSign,		// = PNU
							TempConnection	eTempConnection,	// = TC_None
							IOConnection	eIOConnection,		// = IOC_None
							unsigned short	usGroup				// = 0
							)
{
	m_pairCoordinate.first = pt.first;
	m_pairCoordinate.second = pt.second;
	m_eConnectType = ConnecType;
	SetDirection( tcDirection );
	m_eFunction = eFunction;
	m_eStruct = eStructure;
	m_fClockwise = fClockwise;
	m_ePressureSign = ePressureSign;
	m_eTempConnection = eTempConnection;
	m_eIOConnection = eIOConnection;
	m_usGroup = usGroup;
}

void CAnchorPt::SetDirection( TCHAR tcDirection )
{
	// 0° is east; 270° for south.
	int iDirection;
	switch( tcDirection )
	{
		case 'N':
		case 'n':
			iDirection = 90;
			break;

		case 'E':
		case 'e':
			iDirection = 0;
			break;

		case 'W':
		case 'w':
			iDirection = 180;
			break;

		case 'S':
		case 's':
			iDirection = 270;
			break;
	}
	iDirection = iDirection % 360;
	if( iDirection < 0 )
	{
		iDirection = 360 + iDirection;
	}
	m_iDirection = iDirection;
}

void CAnchorPt::SetDirection( int iDirection )
{
	iDirection += 90;
	iDirection = iDirection % 360;
	if( iDirection < 0 )
	{
		iDirection = 360 + iDirection;
	}
	m_iDirection = iDirection;
}
#endif
