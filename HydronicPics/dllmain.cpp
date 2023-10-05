// dllmain.cpp : Defines the entry point for the DLL application.
#include "StdAfx.h"
#include "tchar.h"
#include "resource.h"
#include "HydronicPicMng.h"

////////////////////////////////////////////////////////////////////////////////////////////
// Define && MACRO
typedef CAnchorPt APT;
typedef std::pair<_string, CProdPic *> PicPair;
typedef std::pair<_string, CDynCircSch > CircSchPair;

#define ADDIMAGE( arg1, arg2 )				m_mmapPicsList.insert( PicPair( _T(#arg1 ), arg2 ) )
#define ADDCIRCSCH( arg1, arg2 )			m_mmapCircSchList.insert( CircSchPair( _T(#arg1 ), arg2 ) )
#define NEWPRODPIC static CProdPic
////////////////////////////////////////////////////////////////////////////////////////////


// Create a multimap to take into account that each Product Image can be a TASymbol, an Image or a LocalSymbol.
multimap <_string, CProdPic *> m_mmapPicsList;

// Create a multimap that will contain the circuits schemes.
multimap <_string, CDynCircSch > m_mmapCircSchList;

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	switch( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}

	// DLL main function is used many times.
	// But we want to load the frames only one time.
	int iNbr = m_mmapPicsList.size();
	int iNbrCircSch = m_mmapCircSchList.size();
	if( iNbr > 0 && iNbrCircSch > 0 )
		return TRUE;
	
	/*==============================================================================
	Revised: AL																17/03/10
			 AL																21/05/10

	Function: ADDIMAGE(IDDescriptor,CProdPic(IDRes,ImgType,ptIn,ptOut,...) )

	Description: Add images and there properties into the multimap

	Hydronic schemes specifications could be found into "Specs for dynamic hydronic schemes.doc" file

	NEWPRODPIC macro for static CProdPic describe a component image
		Use: 	alternative constructors exist see hydronicPic.h
				CProdPic(	UINT iResID,					//Image Resource ID defined when we add the picture
							eProdPicType etype,				//'CProdPic::PicTASymb' for TA Symbol, 'Pic' for Picture, 'CProdPic::PicLocSymb' for localized symbol
							_Pt In,							//Input (x,y)
							_Pt Out,						//Output (x.y)
							_Pt Bypass,						//Bypass (x,y)
							_Pt CapilPrim,					//Primary capillary point see below
							double AngleCapilPrim,			//Connection angle for primary capillary point
							_Pt CapilSec,
							double AngleCapilSec
						);
															// For MV	Primary capillary point is UPSTREAM
															//			Secondary capillary point is DOWNSTREAM
															// For DPC	Primary capillary point is P+
															//			Secondary capillary point is P-

	ADDIMAGE macro used to add an image into the picture database
		Use: ADDIMAGE(PICTURE_ID, &PRODPIC);
				PICTURE_ID  is the identifier stored into the TADB for each component (text)
				PRODPIC		is the product picture created with NEWPRODPIC macro for instance TA_BV


	ADDCIRCSCH macro used to add a circuit scheme into the schemes database
		Use: ADDCIRCSCH(SCHEME_ID, CDynCircSch);
				SCHEME_ID   is the identifier stored into the TADB for each scheme (text) CDB_CircuitScheme
				CDynCircSch	circuit scheme that contains base image and all anchoring points
				
	APT macro (shortcut) that replace CAnchorPt used to define the anchoring point on the schema
		Use:		
			CAnchorPt( 	pair<int,int>	pt, 
						eConnectType	type,				// Beg,	Mid, End, Spline
						TCHAR			dir,				// N,W,S,E	
						eFunc			func,				// BV-P,MV,BV-Byp,BV-S,CV-P,DpC,Pump
						eStruct			structure,			// sNU,	s2W_Straight,s2W_angle,	s3W,
						bool			bClockwise = voCCW,	// voCCW counterclockwise, voCW ClockWise
						ePressSign		Ps = PNU,			// Pminus,Pplus,PUndef (let the engine determine where it will connect the capillary point), PNU not used						// Not used
						unsigned short	Group = 0			// Just a figure, by default the group is 0, useful to connect other capillaries
						);
		Sample: see DC_BV, DC_DP, DC_DPF below to have a detailed sample of usage
	==============================================================================*/
	
	typedef CProdPic::AnchoringPointDef APDf;
	enum
	{
		_I = ecpIn,				// Input point
		_O = ecpOut,			// Output point
		_B = ecpBypass,			// Bypass point
		_CP = ecpCapilPrim,		// For MV	Primary capillary point is UPSTREAM			// For DPC	Primary capillary point is P+
		_CS = ecpCapilSec,		//			Secondary capillary point is DOWNSTREAM		//			Secondary capillary point is P-
		_A = ecpActr,			// Actuator
		_TS = ecpTempSensor,	// Temp sensor cable
		_IS = ecpInputSignal,	// Input signal for exampe TA-Smart 
		_OS = ecpOutputSignal,	// Output signal for exampe TA-Link 
	};

	// TA Symbol that can be added
	// WARNING for schema look TA_BV symbol is vertical mirrored
	NEWPRODPIC TA_BV( GIF_TA_BV, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 52, 1 ) ), APDf( _O, _Pt( 52, 123 ) ), APDf( _CP, _Pt( 93, 33 ), 0.0 ), APDf( _CS, _Pt( 93, 92 ), 0.0 ) } );
	NEWPRODPIC TA_DPC( GIF_TA_DPC, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 46, 1 ) ), APDf( _O, _Pt( 46, 123 ) ), APDf( _CP, _Pt( 134, 62 ), 0.0 ) } );
	
	// Control valve.
	//   - _01 for OnOff.
	//   - _M for Modulating.
	//   - _P for pressetable.
	//   - _PPT for pressetable and measuring points.
	NEWPRODPIC TA_2WCV_01( GIF_TA_2WCV_01, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 33, 1 ) ), APDf( _O, _Pt( 33, 123 ) ) } );
	NEWPRODPIC TA_2WCV_01_P( GIF_TA_2WCV_01_P, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 47, 2 ) ), APDf( _O, _Pt( 47, 123 ) ) } );
	NEWPRODPIC TA_2WCV_01_PPT( GIF_TA_2WCV_01_PPT, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 47, 2 ) ), APDf( _O, _Pt( 47, 123 ) ), APDf( _CP, _Pt( 3, 25 ), 180.0 ) } );
	NEWPRODPIC TA_2WCV_M( GIF_TA_2WCV_M, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 32, 1 ) ), APDf( _O, _Pt( 32, 123 ) ) } );
	NEWPRODPIC TA_2WCV_M_P( GIF_TA_2WCV_M_P, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 47, 2 ) ), APDf( _O, _Pt( 47, 123 ) ) } );
	NEWPRODPIC TA_2WCV_M_PPT( GIF_TA_2WCV_M_PPT, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 47, 2 ) ), APDf( _O, _Pt( 47, 123 ) ), APDf( _CP, _Pt( 2, 25 ), 180.0 ) } );
	NEWPRODPIC TA_3WCV_01( GIF_TA_3WCV_01, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 60, 1 ) ), APDf( _O, _Pt( 60, 121 ) ), APDf( _B, _Pt( 1, 61 ) ) } );
	NEWPRODPIC TA_3WCV_01_P( GIF_TA_3WCV_01_P, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 60, 2 ) ), APDf( _O, _Pt( 60, 119 ) ), APDf( _B, _Pt( 1, 61 ) ) } );
	NEWPRODPIC TA_3WCV_01_PPT( GIF_TA_3WCV_01_PPT, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 60, 2 ) ), APDf( _O, _Pt( 60, 119 ) ), APDf( _B, _Pt( 1, 61 ) ), APDf( _CP, _Pt( 17, 25 ), 90.0 ), APDf( _CS, _Pt( 17, 96 ), 270.0 ) } );
	NEWPRODPIC TA_3WCV_M( GIF_TA_3WCV_M, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 60, 2 ) ), APDf( _O, _Pt( 60, 119 ) ), APDf( _B, _Pt( 1, 61 ) ) } );
	NEWPRODPIC TA_3WCV_M_P( GIF_TA_3WCV_M_P, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 60, 2 ) ), APDf( _O, _Pt( 60, 123 ) ), APDf( _B, _Pt( 1, 62 ) ) } );
	NEWPRODPIC TA_3WCV_M_PPT( GIF_TA_3WCV_M_PPT, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 60, 2 ) ), APDf( _O, _Pt( 60, 123 ) ), APDf( _B, _Pt( 1, 62 ) ), APDf( _CP, _Pt( 17, 25 ), 90.0 ), APDf( _CS, _Pt( 17, 99 ), 270.0 ) } );
	
	// Pumps.
	//   - _CSP constant speed.
	//   - _VSP variable speed.
	NEWPRODPIC TA_CSP( GIF_TA_CSP, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 62, 1 ) ), APDf( _O, _Pt( 62, 124 ) ) } );
	NEWPRODPIC TA_VSP( GIF_TA_VSP, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 61, 1 ) ), APDf( _O, _Pt( 61, 124 ) ) } );

	NEWPRODPIC TA_PICV_01_P( GIF_TA_PICV_01_P, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 90, 1 ) ), APDf( _O, _Pt( 90, 122 ) ) } );
	NEWPRODPIC TA_PICV_01_PPT( GIF_TA_PICV_01_PPT, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 90, 1 ) ), APDf( _O, _Pt( 90, 122 ) ) } );
	NEWPRODPIC TA_PICV_M_P( GIF_TA_PICV_M_P, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 90, 1 ) ), APDf( _O, _Pt( 90, 122 ) ) } );
	NEWPRODPIC TA_PICV_M_PPT( GIF_TA_PICV_M_PPT, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 90, 1 ) ), APDf( _O, _Pt( 90, 122 ) ) } );

	NEWPRODPIC TA_CS( GIF_TA_CS, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 67, 12 ) ), APDf( _O, _Pt( 67, 166 ) ) } );
	NEWPRODPIC TA_CHKV( GIF_TA_CHKV, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 32, 1 ) ), APDf( _O, _Pt( 32, 123 ) ) } );
	NEWPRODPIC TA_DPCF( GIF_TA_DPCF, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 46, 1 ) ), APDf( _O, _Pt( 46, 123 ) ), APDf( _CP, _Pt( 135, 62 ), 0.0 ), APDf( _CS, _Pt( 98, 39 ), 180.0 ) } );
	NEWPRODPIC TA_DPRV( GIF_TA_DPRV, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 33, 1 ) ), APDf( _O, _Pt( 33, 124 ) ) } );
	NEWPRODPIC TA_DRV_L( GIF_TA_DRV_L, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 33, 1 ) ), APDf( _O, _Pt( 33, 123 ) ) } );
	NEWPRODPIC TA_DRV_S( GIF_TA_DRV_S, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 38, 1 ) ), APDf( _O, _Pt( 38, 123 ) ) } );
	NEWPRODPIC TA_FO( GIF_TA_FO, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 33, 10 ) ), APDf( _O, _Pt( 33, 43 ) ) } );
	NEWPRODPIC TA_RV( GIF_TA_RV, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 62, 1 ) ), APDf( _O, _Pt( 1, 60 ) ) } );
	NEWPRODPIC TA_SHV( GIF_TA_SHV, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 43, 1 ) ), APDf( _O, _Pt( 43, 123 ) ), APDf( _CP, _Pt( 1, 34 ), 180.0 ) } );
	NEWPRODPIC TA_TRV( GIF_TA_TRV, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 38, 1 ) ), APDf( _O, _Pt( 38, 122 ) ) } );
	NEWPRODPIC TA_VV( GIF_TA_VV, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 32, 1 ) ), APDf( _O, _Pt( 32, 154 ) ) } );
	NEWPRODPIC TA_RVT( GIF_TA_RVT, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 32, 1 ) ), APDf( _O, _Pt( 32, 122 ) ) } );

	NEWPRODPIC TA_DPCBCV_M( GIF_TA_DPCBCV_M, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 99, 1 ) ), APDf( _O, _Pt( 99, 122 ) ), APDf( _CP, _Pt( 189, 62 ), 0.0, 50 ) } );
	NEWPRODPIC TA_DPCBCV_01( GIF_TA_DPCBCV_01, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 99, 1 ) ), APDf( _O, _Pt( 99, 122 ) ), APDf( _CP, _Pt( 189, 62 ), 0.0, 50 ) } );

	// Safety valves.
	NEWPRODPIC TA_DSV( GIF_TA_DSV, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 50, 99 ) ), APDf( _O, _Pt( 1, 49 ) ) } );

	// TA-Smart.
	NEWPRODPIC TA_SMART_MV( GIF_TA_SMART, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 31, 2 ) ), APDf( _O, _Pt( 31, 208 ) ), APDf( _TS, _Pt( 90, 90 ), 260.0, 40 ) } );

	// Temperature sensor.
	NEWPRODPIC TA_TEMP_SENSOR( GIF_TEMP_SENSOR, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 24, 20 ) ), APDf( _O, _Pt( 24, 20 ) ), APDf( _TS, _Pt( 57, 20 ), 300.0, 50 ) } );

	// TA-Smart Dp
	NEWPRODPIC TA_SMART_DP( GIF_TA_SMART_DP, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 31, 2 ) ), APDf( _O, _Pt( 31, 208 ) ), APDf( _IS, _Pt( 90, 2 ), 90.0, 40 ) } );

	// TA-Link.
	NEWPRODPIC TA_LINK( GIF_TA_LINK, CProdPic::PicTASymb, epHorizontal, { APDf( _I, _Pt( 32, 1 ) ), APDf( _O, _Pt( 32, 137 ) ), APDf( _CP, _Pt( 1, 103 ), 180.0, 40 ), 
			APDf( _CS, _Pt( 1, 80 ), 180.0, 40 ), APDf( _OS, _Pt( 36, 1 ), 90.0, 40 ) } );

	// ADD TA Symbols.
	ADDIMAGE( TA_BV, &TA_BV );
	ADDIMAGE( TA_DPC, &TA_DPC );
	ADDIMAGE( TA_2WCV_01, &TA_2WCV_01 );
	ADDIMAGE( TA_2WCV_01_P, &TA_2WCV_01_P );
	ADDIMAGE( TA_2WCV_01_PPT, &TA_2WCV_01_PPT );
	ADDIMAGE( TA_2WCV_M, &TA_2WCV_M );
	ADDIMAGE( TA_2WCV_M_P, &TA_2WCV_M_P );
	ADDIMAGE( TA_2WCV_M_PPT, &TA_2WCV_M_PPT );
	ADDIMAGE( TA_3WCV_01, &TA_3WCV_01 );
	ADDIMAGE( TA_3WCV_01_P, &TA_3WCV_01_P );
	ADDIMAGE( TA_3WCV_01_PPT, &TA_3WCV_01_PPT );
	ADDIMAGE( TA_3WCV_M, &TA_3WCV_M );
	ADDIMAGE( TA_3WCV_M_P, &TA_3WCV_M_P );
	ADDIMAGE( TA_3WCV_M_PPT, &TA_3WCV_M_PPT );
	ADDIMAGE( TA_CSP, &TA_CSP );
	ADDIMAGE( TA_VSP, &TA_VSP );
	ADDIMAGE( TA_CS, &TA_CS );
	ADDIMAGE( TA_CHKV, &TA_CHKV );
	ADDIMAGE( TA_DPCF, &TA_DPCF );
	ADDIMAGE( TA_DPRV, &TA_DPRV );
	ADDIMAGE( TA_PICV_01_P, &TA_PICV_01_P );
	ADDIMAGE( TA_PICV_01_PPT, &TA_PICV_01_PPT );
	ADDIMAGE( TA_PICV_M_P, &TA_PICV_M_P );
	ADDIMAGE( TA_PICV_M_PPT, &TA_PICV_M_PPT );
	ADDIMAGE( TA_DRV_L, &TA_DRV_L );
	ADDIMAGE( TA_DRV_S, &TA_DRV_S );
	ADDIMAGE( TA_FO, &TA_FO );
	ADDIMAGE( TA_RV, &TA_RV );
	ADDIMAGE( TA_SHV, &TA_SHV );
	ADDIMAGE( TA_TRV, &TA_TRV );
	ADDIMAGE( TA_VV, &TA_VV );
	ADDIMAGE( TA_RVT, &TA_RVT );
	ADDIMAGE( TA_DPCBCV_M, &TA_DPCBCV_M );
	ADDIMAGE( TA_DPCBCV_01, &TA_DPCBCV_01 );
	ADDIMAGE( TA_DSV, &TA_DSV );
	ADDIMAGE( TA_SMART_MV, &TA_SMART_MV );
	ADDIMAGE( TA_TEMP_SENSOR, &TA_TEMP_SENSOR );
	ADDIMAGE( TA_SMART_DP, &TA_SMART_DP );
	ADDIMAGE( TA_LINK, &TA_LINK );

	// STAD.
	ADDIMAGE( PX_STAD25A_BV, &TA_BV );
	NEWPRODPIC STAD25A_BV( GIF_STAD25A_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 18, 4 ) ), APDf( _O, _Pt( 18, 95 ) ), APDf( _CP, _Pt( 71, 5 ), 58.0 ) } );
	ADDIMAGE( PX_STAD25A_BV, &STAD25A_BV );

	ADDIMAGE( PX_STAD25B_BV, &TA_BV );
	NEWPRODPIC STAD25B_BV( GIF_STAD25B_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 17, 22 ) ), APDf( _O, _Pt( 17, 108 ) ), APDf( _CP, _Pt( 40, 1 ), 90.0 ) } );
	ADDIMAGE( PX_STAD25B_BV, &STAD25B_BV );

	ADDIMAGE( PX_STAD25C_BV, &TA_BV );
	NEWPRODPIC STAD25C_BV( GIF_STAD25C_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 17, 25 ) ), APDf( _O, _Pt( 17, 110 ) ), APDf( _CP, _Pt( 40, 1 ), 90.0 ) } );
	ADDIMAGE( PX_STAD25C_BV, &STAD25C_BV );

	ADDIMAGE( PX_STADA25A_BV, &TA_BV );
	NEWPRODPIC STADA25A_BV( GIF_STADA25A_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 18, 1 ) ), APDf( _O, _Pt( 18, 118 ) ) } );
	ADDIMAGE( PX_STADA25A_BV, &STADA25A_BV );

	ADDIMAGE( PX_STADA25B_BV, &TA_BV );
	NEWPRODPIC STADA25B_BV( GIF_STADA25B_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 17, 10 ) ), APDf( _O, _Pt( 17, 121 ) ), APDf( _CP, _Pt( 40, 1 ), 90.0 ) } );
	ADDIMAGE( PX_STADA25B_BV, &STADA25B_BV );

	ADDIMAGE( PX_STADA25C_BV, &TA_BV );
	NEWPRODPIC STADA25C_BV( GIF_STADA25C_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 17, 13 ) ), APDf( _O, _Pt( 17, 124 ) ), APDf( _CP, _Pt( 40, 1 ), 90.0 ) } );
	ADDIMAGE( PX_STADA25C_BV, &STADA25C_BV );

	ADDIMAGE( PX_STADC25A_BV, &TA_BV );
	NEWPRODPIC STADC25A_BV( GIF_STADC25A_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 19, 1 ) ), APDf( _O, _Pt( 19, 99 ) ) } );
	ADDIMAGE( PX_STADC25A_BV, &STADC25A_BV );

	ADDIMAGE( PX_STADC25B_BV, &TA_BV );
	ADDIMAGE( PX_STADC25B_BV, &CProdPic( GIF_STADC25B_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 15, 1 ) ), APDf( _O, _Pt( 15, 97 ) ) } ) );

	ADDIMAGE( PX_STA-DR25_BV, &TA_BV );
	NEWPRODPIC STADR25_BV( GIF_STADR_25_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 38, 1 ) ), APDf( _O, _Pt( 38, 75 ) ), APDf( _CP, _Pt( 70, 21 ), 0.0 ) } );
	ADDIMAGE( PX_STA-DR25_BV, &STADR25_BV );

	// STAD with capillary pipe for DpC set.
	ADDIMAGE( PX_STADSTAR25_DPCSET, &TA_BV );
	NEWPRODPIC STADSTAR25_DPCSET( GIF_STADSTART25_DPCSET, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 17, 8 ) ), APDf( _O, _Pt( 17, 101 ) ), APDf( _CP, _Pt( 36, 2 ), 45.0 ) } );
	ADDIMAGE( PX_STADSTAR25_DPCSET, &STADSTAR25_DPCSET );

	// TA_MULTI.
	ADDIMAGE(PX_TAMULTI20_CV, &TA_2WCV_01);//TODO
	NEWPRODPIC TAMULTI20_CV(GIF_TAMULTI20_CV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 20, 0 ) ), APDf( _O, _Pt( 20, 97 ) ), APDf( _CP, _Pt( 62, 6 ), 45.0 ), APDf( _CS, _Pt( 62, 91 ), 315.0 ) } );
	ADDIMAGE(PX_TAMULTI20_CV, &TAMULTI20_CV);

	ADDIMAGE(PX_TAMULTI20_BV, &TA_BV);
	NEWPRODPIC TAMULTI20_BV(GIF_TAMULTI20_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 20, 0 ) ), APDf( _O, _Pt( 20, 97 ) ), APDf( _CP, _Pt( 62, 6 ), 45.0 ), APDf( _CS, _Pt( 62, 91 ), 315.0 ) } );
	ADDIMAGE(PX_TAMULTI20_BV, &TAMULTI20_BV);

	ADDIMAGE(PX_TAMULTI20_DPC, &TA_DPC);
	NEWPRODPIC TAMULTI20_DPC(GIF_TAMULTI20_DPC, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 20, 22 ) ), APDf( _O, _Pt( 20, 119 ) ), APDf( _CP, _Pt( 131, 24 ), 90.0 ) } );
	ADDIMAGE(PX_TAMULTI20_DPC, &TAMULTI20_DPC);

	// STAF.
	ADDIMAGE( PX_STAF-R80_BV, &TA_BV );
	NEWPRODPIC STAF_R80_BV( GIF_STAF_R80_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 35, 1 ) ), APDf( _O, _Pt( 35, 106 ) ), APDf( _CP, _Pt( 73, 3 ), 90.0 ), APDf( _CS, _Pt( 73, 104 ), 270.0 ) } );
	ADDIMAGE( PX_STAF-R80_BV, &STAF_R80_BV );

	ADDIMAGE( PX_STAF-SG025_BV, &TA_BV );
	NEWPRODPIC STAF_SG025_BV( GIF_STAF_SG025_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 46, 1 ) ), APDf( _O, _Pt( 46, 126 ) ), APDf( _CP, _Pt( 101, 6 ), 90.0 ), APDf( _CS, _Pt( 101, 121 ), 270.0 ) } );
	ADDIMAGE( PX_STAF-SG025_BV, &STAF_SG025_BV );

	ADDIMAGE( PX_STAF-SG080A_BV, &TA_BV );
	NEWPRODPIC STAF_SG080A_BV( GIF_STAF_SG080A_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 38, 1 ) ), APDf( _O, _Pt( 38, 122 ) ), APDf( _CP, _Pt( 81, 4 ), 90.0 ), APDf( _CS, _Pt( 81, 119 ), 270.0 ) } );
	ADDIMAGE( PX_STAF-SG080A_BV, &STAF_SG080A_BV );

	ADDIMAGE( PX_STAF-SG200A_BV, &TA_BV );
	NEWPRODPIC STAF_SG200A_BV( GIF_STAF_SG200A_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 45, 1 ) ), APDf( _O, _Pt( 45, 155 ) ), APDf( _CP, _Pt( 80, 26 ), 0.0 ), APDf( _CS, _Pt( 78, 137 ), 0.0 ) } );
	ADDIMAGE( PX_STAF-SG200A_BV, &STAF_SG200A_BV );

	// STAG.
	ADDIMAGE( PX_STAG80_BV, &TA_BV );
	NEWPRODPIC STAG80_BV( GIF_STAG80_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 19, 1 ) ), APDf( _O, _Pt( 19, 138 ) ), APDf( _CP, _Pt( 48, 20 ), 90.0 ), APDf( _CS, _Pt( 48, 115 ), 240.0 ) } );
	ADDIMAGE( PX_STAG80_BV, &STAG80_BV );

	ADDIMAGE( PX_STAG200_BV, &TA_BV );
	NEWPRODPIC STAG200_BV( GIF_STAG200_BV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 32, 1 ) ), APDf( _O, _Pt( 32, 126 ) ), APDf( _CP, _Pt( 61, 14 ), 90.0 ), APDf( _CS, _Pt( 61, 114 ), 270.0 ) } );
	ADDIMAGE( PX_STAG200_BV, &STAG200_BV );

	// STAP.
	ADDIMAGE( PX_STAP25_10-60, &TA_DPC );
	NEWPRODPIC STAP25_10_60( GIF_STAP25_10_60, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 19, 18 ) ), APDf( _O, _Pt( 19, 89 ) ), APDf( _CP, _Pt( 49, 95 ), 260.0 ) } );
	ADDIMAGE( PX_STAP25_10-60, &STAP25_10_60 );

	ADDIMAGE( PX_STAP80_20-80, &TA_DPC );
	NEWPRODPIC STAP80_20_80( GIF_STAP80_20_80, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 35, 1 ) ), APDf( _O, _Pt( 35, 105 ) ), APDf( _CP, _Pt( 101, 37 ), 60.0 ) } );
	ADDIMAGE( PX_STAP80_20-80, &STAP80_20_80 );

	ADDIMAGE( PX_DPCF, &TA_DPCF);

	// DA50.
	ADDIMAGE( PX_DA50_32A, &TA_DPC );
	NEWPRODPIC DA50_32A( GIF_DA50_32A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 30, 16 ) ), APDf( _O, _Pt( 30, 69 ) ), APDf( _CP, _Pt( 123, 36 ), 270.0 ) } );
	ADDIMAGE( PX_DA50_32A, &DA50_32A );

	ADDIMAGE( PX_DA50_80A, &TA_DPC );
	NEWPRODPIC DA50_80A( GIF_DA50_80A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 44, 1 ) ), APDf( _O, _Pt( 44, 90 ) ), APDf( _CP, _Pt( 148, 39 ), 270.0 ) } );
	ADDIMAGE( PX_DA50_80A, &DA50_80A );

	ADDIMAGE( PX_DA50_200A, &TA_DPC );
	NEWPRODPIC DA50_200A( GIF_DA50_200A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 57, 1 ) ), APDf( _O, _Pt( 57, 125 ) ), APDf( _CP, _Pt( 169, 57 ), 270.0 ) } );
	ADDIMAGE( PX_DA50_200A, &DA50_200A );

	// DAF50.
	ADDIMAGE( PX_DAF50_32A, &TA_DPCF );
	NEWPRODPIC DAF50_32A( GIF_DA50_32A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 30, 16 ) ), APDf( _O, _Pt( 30, 69 ) ), APDf( _CP, _Pt( 123, 36 ), 270.0 ), APDf( _CS, _Pt( 108, 34 ), 180.0 ) } );
	ADDIMAGE( PX_DAF50_32A, &DAF50_32A );

	ADDIMAGE( PX_DAF50_80A, &TA_DPCF );
	NEWPRODPIC DAF50_80A( GIF_DA50_80A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 44, 1 ) ), APDf( _O, _Pt( 44, 90 ) ), APDf( _CP, _Pt( 148, 39 ), 270.0 ), APDf( _CS, _Pt( 133, 37 ), 180.0 ) } );
	ADDIMAGE( PX_DAF50_80A, &DAF50_80A );

	ADDIMAGE( PX_DAF50_200A, &TA_DPCF );
	NEWPRODPIC DAF50_200A( GIF_DA50_200A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 57, 1 ) ), APDf( _O, _Pt( 57, 125 ) ), APDf( _CP, _Pt( 169, 57 ), 270 ), APDf( _CS, _Pt( 156, 57 ), 180 ) } );
	ADDIMAGE( PX_DAF50_200A, &DAF50_200A );


	// PILOT_R.
	ADDIMAGE( PX_PILOT_R_80, &TA_DPC );
	NEWPRODPIC PILOT_R_80( GIF_PILOT_R_80, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 43, 1 ) ), APDf( _O, _Pt( 43, 88 ) ), APDf( _CP, _Pt( 85, 41 ), 0 ) } );
	ADDIMAGE( PX_PILOT_R_80, &PILOT_R_80 );

	ADDIMAGE( PX_PILOT_R_150, &TA_DPC );
	NEWPRODPIC PILOT_R_150( GIF_PILOT_R_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 52, 0 ) ), APDf( _O, _Pt( 52, 99 ) ), APDf( _CP, _Pt( 107, 47 ), 0 ) } );
	ADDIMAGE( PX_PILOT_R_150, &PILOT_R_150 );

	// DA516.
	ADDIMAGE( PX_DA516_25A, &TA_DPC );
	NEWPRODPIC DA516_25A( GIF_DA516_25A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 31, 1 ) ), APDf( _O, _Pt( 31, 83 ) ), APDf( _CP, _Pt( 67, 17 ), 0 ) } );
	ADDIMAGE( PX_DA516_25A, &DA516_25A );

	ADDIMAGE( PX_DA516_80A, &TA_DPC );
	NEWPRODPIC DA516_80A( GIF_DA516_80A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 44, 1 ) ), APDf( _O, _Pt( 44, 63 ) ), APDf( _CP, _Pt( 83, 6 ), 0 ) } );
	ADDIMAGE( PX_DA516_80A, &DA516_80A );

	// DAF516.
	ADDIMAGE( PX_DAF516_25A, &TA_DPCF );
	NEWPRODPIC DAF516_25A( GIF_DA516_25A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 31, 1 ) ), APDf( _O, _Pt( 31, 83 ) ), APDf( _CP, _Pt( 67, 17 ), 0.0 ), APDf( _CS, _Pt( 31, 27 ), 180.0 ) } );
	ADDIMAGE( PX_DAF516_25A, &DAF516_25A );

	ADDIMAGE( PX_DAF516_80A, &TA_DPCF );
	NEWPRODPIC DAF516_80A( GIF_DA516_80A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 44, 1 ) ), APDf( _O, _Pt( 44, 63 ) ), APDf( _CP, _Pt( 83, 6 ), 0.0 ), APDf( _CS, _Pt( 83, 6 ), 0.0 ) } );
	ADDIMAGE( PX_DAF516_80A, &DAF516_80A );

	// TBV.
	ADDIMAGE( PX_TBV20NF_TBV, &TA_BV );
	NEWPRODPIC TBV20NF_TBV( GIF_TBV20NF_TBV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 19, 8 ) ), APDf( _O, _Pt( 19, 96 ) ), APDf( _CP, _Pt( 72, 6 ), 30.0 ), APDf( _CS, _Pt( 84, 27 ), 30.0 ) } );
	ADDIMAGE( PX_TBV20NF_TBV, &TBV20NF_TBV );

	// TBV-C
	ADDIMAGE( PX_TBV-C20, &TA_2WCV_01_PPT );
	NEWPRODPIC TBV_C20( GIF_TBV_C_20, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 18, 8 ) ), APDf( _O, _Pt( 18, 90 ) ), APDf( _CP, _Pt( 68, 6 ), 30.0 ), APDf( _CS, _Pt( 79, 25 ), 30.0 ) } );
	ADDIMAGE( PX_TBV-C20, &TBV_C20 );

	// HYS-1381 : Set image for BR12WT
	NEWPRODPIC BR12WT_SET( GIF_BR12WT_SET, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( SET_2WCV_BR12WT, &BR12WT_SET );

	// TBV-CM share the same picture than TBV-C.
	ADDIMAGE( PX_TBV-CM20, &TA_2WCV_M_PPT );
	ADDIMAGE( PX_TBV-CM20, &TBV_C20 );

	// TBV-CMP.
	ADDIMAGE( PX_TBV-CMP_20, &TA_PICV_M_PPT );
	NEWPRODPIC TBV_CMP_20( GIF_TBV_CMP_20, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 47, 1 ) ), APDf( _O, _Pt( 47, 78 ) ) } );
	ADDIMAGE( PX_TBV-CMP_20, &TBV_CMP_20 );

	// TA-FUSION-C 
	ADDIMAGE(PX_FUSION_C50, &TA_2WCV_M_PPT );
	NEWPRODPIC FUSION_C32( GIF_FUSION_C50A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 23, 2 ) ), APDf( _O, _Pt( 23, 106 ) ), APDf( _CP, _Pt( 77, 5 ), 55.0 ), APDf( _CS, _Pt( 85, 18 ), 55.0 ) } );
	ADDIMAGE( PX_FUSION_C50, &FUSION_C32 );

	ADDIMAGE(PX_FUSION_C80, &TA_2WCV_M_PPT );
	NEWPRODPIC FUSION_C80( GIF_FUSION_C80A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 42, 1 ) ), APDf( _O, _Pt( 42, 76 ) ), APDf( _CP, _Pt( 90, 7 ), 0.0 ), APDf( _CS, _Pt( 90, 70 ), 0.0 ) } );
	ADDIMAGE( PX_FUSION_C80, &FUSION_C80 );

	ADDIMAGE( PX_FUSION_C150, &TA_2WCV_M_PPT );
	NEWPRODPIC FUSION_C150( GIF_FUSION_C150A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 54, 2 ) ), APDf( _O, _Pt( 51, 85 ) ), APDf( _CP, _Pt( 106, 6 ), 0.0 ), APDf( _CS, _Pt( 106, 80 ), 0.0 ) } );
	ADDIMAGE( PX_FUSION_C150, &FUSION_C150 );

	// TA-FUSION-P 
	ADDIMAGE(PX_FUSION_P50, &TA_PICV_M_PPT );
	NEWPRODPIC FUSION_P32( GIF_FUSION_P50A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 42, 2 ) ), APDf( _O, _Pt( 42, 139 ) ) } );
	ADDIMAGE( PX_FUSION_P50, &FUSION_P32 );

	ADDIMAGE(PX_FUSION_P80, &TA_PICV_M_PPT );
	NEWPRODPIC FUSION_P80( GIF_FUSION_P80A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 54, 1 ) ), APDf( _O, _Pt( 54, 115 ) ) } );
	ADDIMAGE( PX_FUSION_P80, &FUSION_P80 );

	ADDIMAGE( PX_FUSION_P150, &TA_PICV_M_PPT );
	NEWPRODPIC FUSION_P150( GIF_FUSION_P150A, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 60, 1 ) ), APDf( _O, _Pt( 60, 149 ) ) } );
	ADDIMAGE( PX_FUSION_P150, &FUSION_P150 );

	// KT 512
	ADDIMAGE( PX_KT512_20, &TA_PICV_01_P );
	NEWPRODPIC KT512_20_DPSCV( GIF_KT512_20, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 31, 1 ) ), APDf( _O, _Pt( 31, 77 ) ) } );
	ADDIMAGE( PX_KT512_20, &KT512_20_DPSCV );

	// KTH 512.
	ADDIMAGE( PX_KTH512_25, &TA_PICV_M_P );
	NEWPRODPIC KTH512_25_DPSCV( GIF_KTH512_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 37, 1 ) ), APDf( _O, _Pt( 37, 88 ) ) } );
	ADDIMAGE( PX_KTH512_25, &KTH512_25_DPSCV );

	// KTM 512.
	ADDIMAGE( PX_KTM512_25, &TA_PICV_M_P );
	NEWPRODPIC KTM512_25_DPSCV( GIF_KTM512_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 37, 1 ) ), APDf( _O, _Pt( 37, 88 ) ) } );
	ADDIMAGE( PX_KTM512_25, &KTM512_25_DPSCV );
	ADDIMAGE( PX_KTM512_80, &TA_PICV_M_P );
	NEWPRODPIC KTM512_80_DPSCV( GIF_KTM512_80, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 28, 1 ) ), APDf( _O, _Pt( 28, 78 ) ) } );
	ADDIMAGE( PX_KTM512_80, &KTM512_80_DPSCV );

	// KTCM 512.
	ADDIMAGE( PX_KTCM512_20, &TA_PICV_M_P );
	NEWPRODPIC KTCM512_20_DPSCV( GIF_KTCM512_20, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 22, 1 ) ), APDf( _O, _Pt( 22, 61 ) ), APDf( _CP, _Pt( 20, 12 ),0 ), APDf( _CS, _Pt( 22, 26 ), 0 ) } );
	ADDIMAGE( PX_KTCM512_20, &KTCM512_20_DPSCV );

	// All separators.
	NEWPRODPIC ZEKS_80( GIF_SEP_ZEK_80_S, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 161, 1 ) ), APDf( _O, _Pt( 161, 168 ) ) } );
	ADDIMAGE( PX_ZEKS_80, &ZEKS_80 );
	NEWPRODPIC ZEKF_80( GIF_SEP_ZEK_80_F, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 161, 1 ) ), APDf( _O, _Pt( 161, 172 ) ) } );
	ADDIMAGE( PX_ZEKF_80, &ZEKF_80 );
	NEWPRODPIC ZEKS_200( GIF_SEP_ZEK_200_S, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 174, 1 ) ), APDf( _O, _Pt( 174, 199 ) ) } );
	ADDIMAGE( PX_ZEKS_200, &ZEKS_200 );
	NEWPRODPIC ZEKF_200( GIF_SEP_ZEK_200_F, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 174, 1 ) ), APDf( _O, _Pt( 174, 203 ) ) } );
	ADDIMAGE( PX_ZEKF_200, &ZEKF_200 );

	NEWPRODPIC ZIK_80_S( GIF_SEP_ZIK_80_S, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 161, 1 ) ), APDf( _O, _Pt( 161, 168 ) ) } );
	ADDIMAGE( PX_ZIK_80_S, &ZIK_80_S );
	NEWPRODPIC ZIK_80_F( GIF_SEP_ZIK_80_F, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 161, 1 ) ), APDf( _O, _Pt( 161, 174 ) ) } );
	ADDIMAGE( PX_ZIK_80_F, &ZIK_80_F );
	NEWPRODPIC ZIK_200_S( GIF_SEP_ZIK_200_S, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 174, 1 ) ), APDf( _O, _Pt( 174, 199 ) ) } );
	ADDIMAGE( PX_ZIK_200_S, &ZIK_200_S );
	NEWPRODPIC ZIK_200_F( GIF_SEP_ZIK_200_F, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 174, 1 ) ), APDf( _O, _Pt( 174, 203 ) ) } );
	ADDIMAGE( PX_ZIK_200_F, &ZIK_200_F );

	NEWPRODPIC ZIO_80_S( GIF_SEP_ZIO_80_S, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 161, 1 ) ), APDf( _O, _Pt( 161, 168 ) ) } );
	ADDIMAGE( PX_ZIO_80_S, &ZIO_80_S );
	NEWPRODPIC ZIO_80_F( GIF_SEP_ZIO_80_F, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 161, 1 ) ), APDf( _O, _Pt( 161, 174 ) ) } );
	ADDIMAGE( PX_ZIO_80_F, &ZIO_80_F );
	NEWPRODPIC ZIO_200_S( GIF_SEP_ZIO_200_S, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 174, 1 ) ), APDf( _O, _Pt( 174, 199 ) ) } );
	ADDIMAGE( PX_ZIO_200_S, &ZIO_200_S );
	NEWPRODPIC ZIO_200_F( GIF_SEP_ZIO_200_F, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 174, 1 ) ), APDf( _O, _Pt( 174, 203 ) ) } );
	ADDIMAGE( PX_ZIO_200_F, &ZIO_200_F );
	NEWPRODPIC FCTYPE80( GIF_SEP_FCTYPE_80, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( PX_FC_SEP_80, &FCTYPE80 );
	NEWPRODPIC FCTYPE150( GIF_SEP_FCTYPE_150, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( PX_FC_SEP_150, &FCTYPE150 );
	NEWPRODPIC FCTYPE273( GIF_SEP_FCTYPE_273, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( PX_FC_SEP_273, &FCTYPE273 );
	
	NEWPRODPIC ZTM( GIF_SEP_ZTM, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 45, 101 ) ), APDf( _O, _Pt( 115, 101 ) ) } );
	ADDIMAGE( PX_ZTM, &ZTM );
	NEWPRODPIC ZTKM( GIF_SEP_ZTKM, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 45, 101 ) ), APDf( _O, _Pt( 115, 101 ) ) } );
	ADDIMAGE( PX_ZTKM, &ZTKM );

	// Two inlets and two outlets -> HOW TO DO THAT ???
	NEWPRODPIC ZUK_25( GIF_SEP_ZUK_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 87, 1 ) ), APDf( _O, _Pt( 87, 68 ) ) } );
	ADDIMAGE( PX_ZUK_25, &ZUK_25 );
	NEWPRODPIC ZUKM_25( GIF_SEP_ZUKM_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 93, 1 ) ), APDf( _O, _Pt( 93, 80 ) ) } );
	ADDIMAGE( PX_ZUKM_25, &ZUKM_25 );

	NEWPRODPIC ZUV_25( GIF_SEP_ZUV_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 34, 1 ) ), APDf( _O, _Pt( 34, 67 ) ) } );
	ADDIMAGE( PX_ZUV_25, &ZUV_25 );
	NEWPRODPIC ZUVL_25( GIF_SEP_ZUVL_25, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 18, 117 ) ), APDf( _O, _Pt( 18, 177 ) ) } );
	ADDIMAGE( PX_ZUVL_25, &ZUVL_25 );

	// Two inlets and two outlets -> HOW TO DO THAT ???
	NEWPRODPIC ZUC_25( GIF_SEP_ZUC_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 82, 1 ) ), APDf( _O, _Pt( 82, 67 ) ) } );
	ADDIMAGE( PX_ZUC_25, &ZUC_25 );
	NEWPRODPIC ZUCM_25( GIF_SEP_ZUCM_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 86, 1 ) ), APDf( _O, _Pt( 86, 68 ) ) } );
	ADDIMAGE( PX_ZUCM_25, &ZUCM_25 );
	NEWPRODPIC ZUR_25( GIF_SEP_ZUR_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 95, 1 ) ), APDf( _O, _Pt( 95, 67 ) ) } );
	ADDIMAGE( PX_ZUR_25, &ZUR_25 );

	NEWPRODPIC ZUD_25( GIF_SEP_ZUD_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 82, 1 ) ), APDf( _O, _Pt( 82, 67 ) ) } );
	ADDIMAGE( PX_ZUD_25, &ZUD_25 );
	NEWPRODPIC ZUDL_25( GIF_SEP_ZUDL_25, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 18, 1 ) ), APDf( _O, _Pt( 18, 62 ) ) } );
	ADDIMAGE( PX_ZUDL_25, &ZUDL_25 );

	NEWPRODPIC ZUM_25( GIF_SEP_ZUM_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 86, 1 ) ), APDf( _O, _Pt( 86, 68 ) ) } );
	ADDIMAGE( PX_ZUM_25, &ZUM_25 );
	NEWPRODPIC ZUML_25( GIF_SEP_ZUML_25, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 17, 1 ) ), APDf( _O, _Pt( 17, 61 ) ) } );
	ADDIMAGE( PX_ZUML_25, &ZUML_25 );

	NEWPRODPIC ZG_65_F(GIF_SEP_ZG_65_F, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt(180, 1) ), APDf( _O, _Pt(180, 226) ) } );
	ADDIMAGE(PX_ZG_65_F, &ZG_65_F);

	NEWPRODPIC ZG_65_S(GIF_SEP_ZG_65_S, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt(180, 1) ), APDf( _O, _Pt(180, 194) ) } );
	ADDIMAGE(PX_ZG_65_S, &ZG_65_S);

	// All air vents.
	NEWPRODPIC ZUP( GIF_AIRV_ZUP, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 1, 27 ) ), APDf( _O, _Pt( 1, 27 ) ) } );
	ADDIMAGE( PX_ZUP, &ZUP );
	
	NEWPRODPIC ZUT( GIF_AIRV_ZUT25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 1, 26 ) ), APDf( _O, _Pt( 1, 26 ) ) } );
	ADDIMAGE( PX_ZUT, &ZUT );
	
	NEWPRODPIC ZUTX( GIF_AIRV_ZUTX, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 1, 26 ) ), APDf( _O, _Pt( 1, 26 ) ) } );
	ADDIMAGE( PX_ZUTX, &ZUTX );
	
	NEWPRODPIC ZTV( GIF_AIRV_ZTV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 1, 80 ) ), APDf( _O, _Pt( 76, 80 ) ) } );
	ADDIMAGE( PX_ZTV, &ZTV );

	// TA-COMPACT.
	ADDIMAGE( PX_COMPACTP_15, &TA_PICV_01_PPT );
	NEWPRODPIC COMPACT_15( GIF_TA_COMPACT_15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 74, 1 ) ), APDf( _O, _Pt( 74, 73 ) ) } );
	ADDIMAGE( PX_COMPACTP_15, &COMPACT_15 );

	// TA-MODULATOR 40-50.
	ADDIMAGE( PX_MODULATOR_40, &TA_PICV_01_PPT );
	NEWPRODPIC MODULATOR_40( GIF_TA_MODULATOR_40, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 33, 1 ) ), APDf( _O, _Pt( 33, 140 ) ) } );
	ADDIMAGE( PX_MODULATOR_40, &MODULATOR_40 );

	// TA-MODULATOR 65-80.
	ADDIMAGE( PX_MODULATOR_65, &TA_PICV_01_PPT );
	NEWPRODPIC MODULATOR_65( GIF_TA_MODULATOR_65, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 36, 1 ) ), APDf( _O, _Pt( 36, 140 ) ) } );
	ADDIMAGE( PX_MODULATOR_65, &MODULATOR_65 );

	// Combined Dp controller, control and balancing valve (DpCBCV).
	ADDIMAGE( PX_COMPACTDP_15, &TA_DPCBCV_M );
	NEWPRODPIC TA_COMPACTDP_15( GIF_TA_COMPACTDP_15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 67, 1 ) ), APDf( _O, _Pt( 67, 97 ) ), APDf( _CP, _Pt( 133, 54 ), 320 ) } );
	ADDIMAGE( PX_COMPACTDP_15, &TA_COMPACTDP_15 );

	// Shut-off valve.
	ADDIMAGE( PX_STS, &TA_SHV );
	NEWPRODPIC TA_STS_15( GIF_STS15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 13, 8 ) ), APDf( _O, _Pt( 18, 87 ) ), APDf( _CP, _Pt( 61, 6 ), 30.0 ) } );
	ADDIMAGE( PX_STS, &TA_STS_15 );

	// Safety valves.
	ADDIMAGE( PX_DSV_DGH_15_32, &TA_DSV );
	NEWPRODPIC TA_DSV_DGH_15_32( GIF_DSV_DGH_15_32, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 1, 66 ) ), APDf( _O, _Pt( 68, 140 ) ) } );
	ADDIMAGE( PX_DSV_DGH_15_32, &TA_DSV_DGH_15_32 );

	ADDIMAGE( PX_DSV_DGH_40_50, &TA_DSV );
	NEWPRODPIC TA_DSV_DGH_40_50( GIF_DSV_DGH_40_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 1, 67 ) ), APDf( _O, _Pt( 123, 165 ) ) } );
	ADDIMAGE( PX_DSV_DGH_40_50, &TA_DSV_DGH_40_50 );

	ADDIMAGE( PX_DSV_H_F_SOL, &TA_DSV );
	NEWPRODPIC TA_DSV_H_F_SOL( GIF_DSV_H_F_SOL, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 1, 50 ) ), APDf( _O, _Pt( 82, 141 ) ) } );
	ADDIMAGE( PX_DSV_H_F_SOL, &TA_DSV_H_F_SOL );

	// CV206 GG DN15.
	ADDIMAGE( PX_CV206GG_15, &TA_2WCV_M );
	NEWPRODPIC CV206_GG_15( GIF_CV206_GG_15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 57, 1 ) ), APDf( _O, _Pt( 57, 92 ) ) } );
	ADDIMAGE( PX_CV206GG_15, &CV206_GG_15 );

	// CV206 GG DN20.
	ADDIMAGE( PX_CV206GG_20, &TA_2WCV_M );
	NEWPRODPIC CV206_GG_20( GIF_CV206_GG_20, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 56, 1 ) ), APDf( _O, _Pt( 56, 93 ) ) } );
	ADDIMAGE( PX_CV206GG_20, &CV206_GG_20 );

	// CV206 GG DN25.
	ADDIMAGE( PX_CV206GG_25, &TA_2WCV_M );
	NEWPRODPIC CV206_GG_25( GIF_CV206_GG_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 56, 1 ) ), APDf( _O, _Pt( 56, 92 ) ) } );
	ADDIMAGE( PX_CV206GG_25, &CV206_GG_25 );

	// CV206 GG DN32.
	ADDIMAGE( PX_CV206GG_32, &TA_2WCV_M );
	NEWPRODPIC CV206_GG_32( GIF_CV206_GG_32, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 63, 1 ) ), APDf( _O, _Pt( 63, 93 ) ) } );
	ADDIMAGE( PX_CV206GG_32, &CV206_GG_32 );

	// CV206 GG DN40.
	ADDIMAGE( PX_CV206GG_40, &TA_2WCV_M );
	NEWPRODPIC CV206_GG_40( GIF_CV206_GG_40, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 62, 1 ) ), APDf( _O, _Pt( 62, 99 ) ) } );
	ADDIMAGE( PX_CV206GG_40, &CV206_GG_40 );

	// CV206 GG DN50-65.
	ADDIMAGE( PX_CV206GG_50_65, &TA_2WCV_M );
	NEWPRODPIC CV206_GG_50_65( GIF_CV206_GG_50_65, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 60, 1 ) ), APDf( _O, _Pt( 60, 110 ) ) } );
	ADDIMAGE( PX_CV206GG_50_65, &CV206_GG_50_65 );

	// CV206 GG DN80-100.
	ADDIMAGE( PX_CV206GG_80_100, &TA_2WCV_M );
	NEWPRODPIC CV206_GG_80_100( GIF_CV206_GG_80_100, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 60, 1 ) ), APDf( _O, _Pt( 60, 112 ) ) } );
	ADDIMAGE( PX_CV206GG_80_100, &CV206_GG_80_100 );

	// CV306 GG DN15-25.
	ADDIMAGE( PX_CV306GG_15_25, &TA_2WCV_M );
	NEWPRODPIC CV306_GG_15_25( GIF_CV306_GG_15_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 50, 1 ) ), APDf( _O, _Pt( 50, 93 ) ), APDf( _B, _Pt( 1, 47 ) ) } );
	ADDIMAGE( PX_CV306GG_15_25, &CV306_GG_15_25 );

	// CV306 GG DN32.
	ADDIMAGE( PX_CV306GG_32, &TA_2WCV_M );
	NEWPRODPIC CV306_GG_32( GIF_CV306_GG_32, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 50, 1 ) ), APDf( _O, _Pt( 50, 94 ) ), APDf( _B, _Pt( 1, 47 ) ) } );
	ADDIMAGE( PX_CV306GG_32, &CV306_GG_32 );

	// CV306 GG DN40-100.
	ADDIMAGE( PX_CV306GG_40_100, &TA_2WCV_M );
	NEWPRODPIC CV306_GG_40_100( GIF_CV306_GG_40_100, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 48, 1 ) ), APDf( _O, _Pt( 48, 112 ) ), APDf( _B, _Pt( 1, 56 ) ) } );
	ADDIMAGE( PX_CV306GG_40_100, &CV306_GG_40_100 );

	// CV306 GG DN125-150.
	ADDIMAGE( PX_CV306GG_125_150, &TA_2WCV_M );
	NEWPRODPIC CV306_GG_125_150( GIF_CV306_GG_125_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 49, 1 ) ), APDf( _O, _Pt( 49, 112 ) ), APDf( _B, _Pt( 1, 56 ) ) } );
	ADDIMAGE( PX_CV306GG_125_150, &CV306_GG_125_150 );

	// CV216 DN125.
	ADDIMAGE( PX_CV216_125, &TA_2WCV_M );
	NEWPRODPIC CV216_125( GIF_CV216_125, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 33, 1 ) ), APDf( _O, _Pt( 33, 85 ) ) } );
	ADDIMAGE( PX_CV216_125, &CV216_125 );

	// CV216 DN150.
	ADDIMAGE( PX_CV216_150, &TA_2WCV_M );
	NEWPRODPIC CV216_150( GIF_CV216_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 32, 1 ) ), APDf( _O, _Pt( 32, 88 ) ) } );
	ADDIMAGE( PX_CV216_150, &CV216_150 );

	// CV216 DN200.
	ADDIMAGE( PX_CV216_200, &TA_2WCV_M );
	NEWPRODPIC CV216_200( GIF_CV216_200, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 38, 1 ) ), APDf( _O, _Pt( 38, 97 ) ) } );
	ADDIMAGE( PX_CV216_200, &CV216_200 );

	// CV216 DN250-300.
	ADDIMAGE( PX_CV216_250_300, &TA_2WCV_M );
	NEWPRODPIC CV216_250_300( GIF_CV216_250_300, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 36, 1 ) ), APDf( _O, _Pt( 36, 103 ) ) } );
	ADDIMAGE( PX_CV216_250_300, &CV216_250_300 );

	// CV316 DN125.
	ADDIMAGE( PX_CV316_125, &TA_2WCV_M );
	NEWPRODPIC CV316_125( GIF_CV316_125, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 55, 1 ) ), APDf( _O, _Pt( 55, 86 ) ), APDf( _B, _Pt( 1, 44 ) ) } );
	ADDIMAGE( PX_CV316_125, &CV316_125 );

	// CV316 DN150.
	ADDIMAGE( PX_CV316_150, &TA_2WCV_M );
	NEWPRODPIC CV316_150( GIF_CV316_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 66, 1 ) ), APDf( _O, _Pt( 66, 112 ) ), APDf( _B, _Pt( 1, 57 ) ) } );
	ADDIMAGE( PX_CV316_150, &CV316_150 );

	// CV316 DN200.
	ADDIMAGE( PX_CV316_200, &TA_2WCV_M );
	NEWPRODPIC CV316_200( GIF_CV316_200, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 74, 1 ) ), APDf( _O, _Pt( 74, 107 ) ), APDf( _B, _Pt( 1, 54 ) ) } );
	ADDIMAGE( PX_CV316_200, &CV316_200 );

	// CV316 DN250-300.
	ADDIMAGE( PX_CV316_250_300, &TA_2WCV_M );
	NEWPRODPIC CV316_250_300( GIF_CV316_250_300, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 76, 1 ) ), APDf( _O, _Pt( 76, 121 ) ), APDf( _B, _Pt( 1, 61 ) ) } );
	ADDIMAGE( PX_CV316_250_300, &CV316_250_300 );

	// CV216 GG DN15.
	ADDIMAGE( PX_CV216GG_15, &TA_2WCV_M );
	NEWPRODPIC CV216GG_15( GIF_CV216_GG_15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 54, 1 ) ), APDf( _O, _Pt( 54, 78 ) ) } );
	ADDIMAGE( PX_CV216GG_15, &CV216GG_15 );

	// CV216 GG DN20.
	ADDIMAGE( PX_CV216GG_20, &TA_2WCV_M );
	NEWPRODPIC CV216GG_20( GIF_CV216_GG_20, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 56, 1 ) ), APDf( _O, _Pt( 56, 86 ) ) } );
	ADDIMAGE( PX_CV216GG_20, &CV216GG_20 );

	// CV216 GG DN25-32.
	ADDIMAGE( PX_CV216GG_25_32, &TA_2WCV_M );
	NEWPRODPIC CV216GG_25_32( GIF_CV216_GG_25_32, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 54, 1 ) ), APDf( _O, _Pt( 54, 87 ) ) } );
	ADDIMAGE( PX_CV216GG_25_32, &CV216GG_25_32 );

	// CV216 GG DN40-50.
	ADDIMAGE( PX_CV216GG_40_50, &TA_2WCV_M );
	NEWPRODPIC CV216GG_40_50( GIF_CV216_GG_40_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 61, 1 ) ), APDf( _O, _Pt( 61, 92 ) ) } );
	ADDIMAGE( PX_CV216GG_40_50, &CV216GG_40_50 );

	// CV216 GG DN65.
	ADDIMAGE( PX_CV216GG_65, &TA_2WCV_M );
	NEWPRODPIC CV216GG_65( GIF_CV216_GG_65, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 49, 1 ) ), APDf( _O, _Pt( 49, 93 ) ) } );
	ADDIMAGE( PX_CV216GG_65, &CV216GG_65 );
	
	// CV216 GG DN80-100.
	ADDIMAGE( PX_CV216GG_80_100, &TA_2WCV_M );
	NEWPRODPIC CV216GG_80_100( GIF_CV216_GG_80_100, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 46, 1 ) ), APDf( _O, _Pt( 46, 90 ) ) } );
	ADDIMAGE( PX_CV216GG_80_100, &CV216GG_80_100 );

	// CV216 GG DN125-200.
	ADDIMAGE( PX_CV216GG_125_200, &TA_2WCV_M );
	NEWPRODPIC CV216GG_125_200( GIF_CV216_GG_125_200, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 43, 1 ) ), APDf( _O, _Pt( 43, 89 ) ) } );
	ADDIMAGE( PX_CV216GG_125_200, &CV216GG_125_200 );

	// CV316 GG DN15.
	ADDIMAGE( PX_CV316GG_15, &TA_2WCV_M );
	NEWPRODPIC CV316GG_15( GIF_CV316_GG_15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 27, 1 ) ), APDf( _O, _Pt( 27, 56 ) ), APDf( _B, _Pt( 1, 29 ) ) } );
	ADDIMAGE( PX_CV316GG_15, &CV316GG_15 );

	// CV316 GG DN20.
	ADDIMAGE( PX_CV316GG_20, &TA_2WCV_M );
	NEWPRODPIC CV316GG_20( GIF_CV316_GG_20, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 27, 1 ) ), APDf( _O, _Pt( 27, 56 ) ), APDf( _B, _Pt( 1, 29 ) ) } );
	ADDIMAGE( PX_CV316GG_20, &CV316GG_20 );

	// CV316 GG DN25.
	ADDIMAGE( PX_CV316GG_25, &TA_2WCV_M );
	NEWPRODPIC CV316GG_25( GIF_CV316_GG_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 27, 1 ) ), APDf( _O, _Pt( 27, 56 ) ), APDf( _B, _Pt( 1, 29 ) ) } );
	ADDIMAGE( PX_CV316GG_25, &CV316GG_25 );

	// CV316 GG DN32.
	ADDIMAGE( PX_CV316GG_32, &TA_2WCV_M );
	NEWPRODPIC CV316GG_32( GIF_CV316_GG_32, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 49, 1 ) ), APDf( _O, _Pt( 49, 94 ) ), APDf( _B, _Pt( 1, 47 ) ) } );
	ADDIMAGE( PX_CV316GG_32, &CV316GG_32 );

	// CV316 GG DN40.
	ADDIMAGE( PX_CV316GG_40, &TA_2WCV_M );
	NEWPRODPIC CV316GG_40( GIF_CV316_GG_40, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 50, 1 ) ), APDf( _O, _Pt( 50, 99 ) ), APDf( _B, _Pt( 1, 50 ) ) } );
	ADDIMAGE( PX_CV316GG_40, &CV316GG_40 );

	// CV316 GG DN50.
	ADDIMAGE( PX_CV316GG_50, &TA_2WCV_M );
	NEWPRODPIC CV316GG_50( GIF_CV316_GG_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 49, 1 ) ), APDf( _O, _Pt( 49, 109 ) ), APDf( _B, _Pt( 1, 55 ) ) } );
	ADDIMAGE( PX_CV316GG_50, &CV316GG_50 );

	// CV316 GG DN65.
	ADDIMAGE( PX_CV316GG_65, &TA_2WCV_M );
	NEWPRODPIC CV316GG_65( GIF_CV316_GG_65, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 49, 1 ) ), APDf( _O, _Pt( 49, 116 ) ), APDf( _B, _Pt( 1, 58 ) ) } );
	ADDIMAGE( PX_CV316GG_65, &CV316GG_65 );
	
	// CV316 GG DN80-100.
	ADDIMAGE( PX_CV316GG_80_100, &TA_2WCV_M );
	NEWPRODPIC CV316GG_80_100( GIF_CV316_GG_80_100, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 39, 1 ) ), APDf( _O, _Pt( 39, 90 ) ), APDf( _B, _Pt( 1, 46 ) ) } );
	ADDIMAGE( PX_CV316GG_80_100, &CV316GG_80_100 );

	// CV316 GG DN125-200.
	ADDIMAGE( PX_CV316GG_125_200, &TA_2WCV_M );
	NEWPRODPIC CV316GG_125_200( GIF_CV316_GG_125_200, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 37, 1 ) ), APDf( _O, _Pt( 37, 89 ) ), APDf( _B, _Pt( 1, 45 ) ) } );
	ADDIMAGE( PX_CV316GG_125_200, &CV316GG_125_200 );

	// CV216 MZ.
	ADDIMAGE( PX_CV216MZ, &TA_2WCV_M );
	NEWPRODPIC CV216_MZ( GIF_CV216_MZ, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 25, 1 ) ), APDf( _O, _Pt( 25, 79 ) ) } );
	ADDIMAGE( PX_CV216MZ, &CV216_MZ );

	// CV316 MZ.
	ADDIMAGE( PX_CV316MZ, &TA_2WCV_M );
	NEWPRODPIC CV316_MZ( GIF_CV316_MZ, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 40, 1 ) ), APDf( _O, _Pt( 40, 91 ) ), APDf( _B, _Pt( 1, 46 ) ) } );
	ADDIMAGE( PX_CV316MZ, &CV316_MZ );
	
	// CV216 RGA DN15-25.
	ADDIMAGE( PX_CV216RGA_15_25, &TA_2WCV_M );
	NEWPRODPIC CV216_RGA_15_25( GIF_CV216_RGA_15_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 39, 1 ) ), APDf( _O, _Pt( 39, 91 ) ) } );
	ADDIMAGE( PX_CV216RGA_15_25, &CV216_RGA_15_25 );

	// CV216 RGA DN32-50.
	ADDIMAGE( PX_CV216RGA_32_50, &TA_2WCV_M );
	NEWPRODPIC CV216_RGA_32_50( GIF_CV216_RGA_32_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 37, 1 ) ), APDf( _O, _Pt( 37, 96 ) ) } );
	ADDIMAGE( PX_CV216RGA_32_50, &CV216_RGA_32_50 );

	// CV316 RGA DN15-25.
	ADDIMAGE( PX_CV316RGA_15_25, &TA_2WCV_M );
	NEWPRODPIC CV316_RGA_15_25( GIF_CV316_RGA_15_25, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 40, 1 ) ), APDf( _O, _Pt( 40, 91 ) ), APDf( _B, _Pt( 1, 45 ) ) } );
	ADDIMAGE( PX_CV316RGA_15_25, &CV316_RGA_15_25 );

	// CV316 RGA DN32-50.
	ADDIMAGE( PX_CV316RGA_32_50, &TA_2WCV_M );
	NEWPRODPIC CV316_RGA_32_50( GIF_CV316_RGA_32_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 45, 1 ) ), APDf( _O, _Pt( 45, 92 ) ), APDf( _B, _Pt( 1, 47 ) ) } );
	ADDIMAGE( PX_CV316RGA_32_50, &CV316_RGA_32_50 );
	
	// CV225/240S/240E DN15.
	ADDIMAGE( PX_CV225_240S_E_15, &TA_2WCV_M );
	NEWPRODPIC CV225_240S_E_15( GIF_CV225_240S_E_15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 24, 1 ) ), APDf( _O, _Pt( 24, 61 ) ) } );
	ADDIMAGE( PX_CV225_240S_E_15, &CV225_240S_E_15 );

	// CV225 DN20-40
	ADDIMAGE( PX_CV225_20_40, &TA_2WCV_M );
	NEWPRODPIC CV225_20_40( GIF_CV225_20_40, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 20, 1 ) ), APDf( _O, _Pt( 20, 57 ) ) } );
	ADDIMAGE( PX_CV225_20_40, &CV225_20_40 );

	// CV225 DN50
	ADDIMAGE( PX_CV225_50, &TA_2WCV_M );
	NEWPRODPIC CV225_50( GIF_CV225_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 28, 1 ) ), APDf( _O, _Pt( 28, 73 ) ) } );
	ADDIMAGE( PX_CV225_50, &CV225_50 );

	// CV225 DN65
	ADDIMAGE( PX_CV225_65, &TA_2WCV_M );
	NEWPRODPIC CV225_65( GIF_CV225_65, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 30, 1 ) ), APDf( _O, _Pt( 30, 89 ) ) } );
	ADDIMAGE( PX_CV225_65, &CV225_65 );

	// CV225 DN80-125
	ADDIMAGE( PX_CV225_80_125, &TA_2WCV_M );
	NEWPRODPIC CV225_80_125( GIF_CV225_80_125, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 35, 1 ) ), APDf( _O, _Pt( 35, 89 ) ) } );
	ADDIMAGE( PX_CV225_80_125, &CV225_80_125 );

	// CV225 DN150
	ADDIMAGE( PX_CV225_150, &TA_2WCV_M );
	NEWPRODPIC CV225_150( GIF_CV225_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 34, 1 ) ), APDf( _O, _Pt( 34, 95 ) ) } );
	ADDIMAGE( PX_CV225_150, &CV225_150 );

	// CV225 DN200
	ADDIMAGE( PX_CV225_200, &TA_2WCV_M );
	NEWPRODPIC CV225_200( GIF_CV225_200, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 39, 1 ) ), APDf( _O, _Pt( 39, 99 ) ) } );
	ADDIMAGE( PX_CV225_200, &CV225_200 );

	// CV225 DN250-300 (This picture has been taken from the CV240S DN250-300 because the drawing doesn't exist).
	ADDIMAGE( PX_CV225_250_300, &TA_2WCV_M );
	NEWPRODPIC CV225_250_300( GIF_CV225_250_300, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 45, 1 ) ), APDf( _O, _Pt( 45, 118 ) ) } );
	ADDIMAGE( PX_CV225_250_300, &CV225_250_300 );

	// CV325 DN15.
	ADDIMAGE( PX_CV325_15, &TA_2WCV_M );
	NEWPRODPIC CV325_15( GIF_CV325_15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 42, 1 ) ), APDf( _O, _Pt( 42, 44 ) ), APDf( _B, _Pt( 1, 23 ) ) } );
	ADDIMAGE( PX_CV325_15, &CV325_15 );

	// CV325 DN20-40
	ADDIMAGE( PX_CV325_20_40, &TA_2WCV_M );
	NEWPRODPIC CV325_20_40( GIF_CV325_20_40, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 42, 1 ) ), APDf( _O, _Pt( 42, 48 ) ), APDf( _B, _Pt( 1, 25 ) ) } );
	ADDIMAGE( PX_CV325_20_40, &CV325_20_40 );

	// CV325 DN50
	ADDIMAGE( PX_CV325_50, &TA_2WCV_M );
	NEWPRODPIC CV325_50( GIF_CV325_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 58, 1 ) ), APDf( _O, _Pt( 58, 82 ) ), APDf( _B, _Pt( 1, 42 ) ) } );
	ADDIMAGE( PX_CV325_50, &CV325_50 );

	// CV325 DN65
	ADDIMAGE( PX_CV325_65, &TA_2WCV_M );
	NEWPRODPIC CV325_65( GIF_CV325_65, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 48, 1 ) ), APDf( _O, _Pt( 48, 81 ) ), APDf( _B, _Pt( 1, 41 ) ) } );
	ADDIMAGE( PX_CV325_65, &CV325_65 );
	
	// CV325 DN80
	ADDIMAGE( PX_CV325_80, &TA_2WCV_M );
	NEWPRODPIC CV325_80( GIF_CV325_80, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 55, 1 ) ), APDf( _O, _Pt( 55, 80 ) ), APDf( _B, _Pt( 1, 41 ) ) } );
	ADDIMAGE( PX_CV325_80, &CV325_80 );

	// CV325 DN100-125
	ADDIMAGE( PX_CV325_100_125, &TA_2WCV_M );
	NEWPRODPIC CV325_100_125( GIF_CV325_100_125, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 51, 1 ) ), APDf( _O, _Pt( 51, 80 ) ), APDf( _B, _Pt( 1, 41 ) ) } );
	ADDIMAGE( PX_CV325_100_125, &CV325_100_125 );

	// CV325 DN150
	ADDIMAGE( PX_CV325_150, &TA_2WCV_M );
	NEWPRODPIC CV325_150( GIF_CV325_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 54, 1 ) ), APDf( _O, _Pt( 54, 91 ) ), APDf( _B, _Pt( 1, 47 ) ) } );
	ADDIMAGE( PX_CV325_150, &CV325_150 );

	// CV325 DN200
	ADDIMAGE( PX_CV325_200, &TA_2WCV_M );
	NEWPRODPIC CV325_200( GIF_CV325_200, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 58, 1 ) ), APDf( _O, _Pt( 58, 86 ) ), APDf( _B, _Pt( 1, 44 ) ) } );
	ADDIMAGE( PX_CV325_200, &CV325_200 );
	
	// CV325 DN250-300 (This picture has been taken from the CV340S DN250-300 because the drawing doesn't exist).
	ADDIMAGE( PX_CV325_250_300, &TA_2WCV_M );
	NEWPRODPIC CV325_250_300( GIF_CV325_250_300, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 58, 1 ) ), APDf( _O, _Pt( 58, 89 ) ), APDf( _B, _Pt( 1, 45 ) ) } );
	ADDIMAGE( PX_CV325_250_300, &CV325_250_300 );

	// CV240S/240E DN20-40
	ADDIMAGE( PX_CV240S_E_20_40, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_20_40( GIF_CV240S_E_20_40, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 26, 1 ) ), APDf( _O, _Pt( 26, 68 ) ) } );
	ADDIMAGE( PX_CV240S_E_20_40, &CV240S_E_20_40 );

	// CV240S/240E DN50
	ADDIMAGE( PX_CV240S_E_50, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_50( GIF_CV240S_E_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 27, 1 ) ), APDf( _O, _Pt( 27, 73 ) ) } );
	ADDIMAGE( PX_CV240S_E_50, &CV240S_E_50 );

	// CV240S/240E DN65
	ADDIMAGE( PX_CV240S_E_65, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_65( GIF_CV240S_E_65, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 29, 1 ) ), APDf( _O, _Pt( 29, 86 ) ) } );
	ADDIMAGE( PX_CV240S_E_65, &CV240S_E_65 );

	// CV240S/240E DN80
	ADDIMAGE( PX_CV240S_E_80, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_80( GIF_CV240S_E_80, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 33, 1 ) ), APDf( _O, _Pt( 33, 91 ) ) } );
	ADDIMAGE( PX_CV240S_E_80, &CV240S_E_80 );

	// CV240S/240E DN100
	ADDIMAGE( PX_CV240S_E_100, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_100( GIF_CV240S_E_100, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 35, 1 ) ), APDf( _O, _Pt( 35, 81 ) ) } );
	ADDIMAGE( PX_CV240S_E_100, &CV240S_E_100 );

	// CV240S/240E DN125
	ADDIMAGE( PX_CV240S_E_125, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_125( GIF_CV240S_E_125, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 35, 1 ) ), APDf( _O, _Pt( 35, 92 ) ) } );
	ADDIMAGE( PX_CV240S_E_125, &CV240S_E_125 );

	// CV240S/240E DN150
	ADDIMAGE( PX_CV240S_E_150, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_150( GIF_CV240S_E_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 34, 1 ) ), APDf( _O, _Pt( 34, 95 ) ) } );
	ADDIMAGE( PX_CV240S_E_150, &CV240S_E_150 );

	// CV240S/240E DN200
	ADDIMAGE( PX_CV240S_E_200, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_200( GIF_CV240S_E_200, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 41, 1 ) ), APDf( _O, _Pt( 41, 110 ) ) } );
	ADDIMAGE( PX_CV240S_E_200, &CV240S_E_200 );

	// CV240S/240E DN250-300
	ADDIMAGE( PX_CV240S_E_250_300, &TA_2WCV_M );
	NEWPRODPIC CV240S_E_250_300( GIF_CV240S_E_250_300, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 45, 1 ) ), APDf( _O, _Pt( 45, 118 ) ) } );
	ADDIMAGE( PX_CV240S_E_250_300, &CV240S_E_250_300 );

	// CV340S/340E DN15
	ADDIMAGE( PX_CV340S_E_15, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_15( GIF_CV340S_E_15, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 55, 1 ) ), APDf( _O, _Pt( 55, 56 ) ), APDf( _B, _Pt( 1, 29 ) ) } );
	ADDIMAGE( PX_CV340S_E_15, &CV340S_E_15 );

	// CV340S/340E DN20-40
	ADDIMAGE( PX_CV340S_E_20_40, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_20_40( GIF_CV340S_E_20_40, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 55, 1 ) ), APDf( _O, _Pt( 55, 64 ) ), APDf( _B, _Pt( 1, 32 ) ) } );
	ADDIMAGE( PX_CV340S_E_20_40, &CV340S_E_20_40 );

	// CV340S/340E DN50
	ADDIMAGE( PX_CV340S_E_50, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_50( GIF_CV340S_E_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 52, 1 ) ), APDf( _O, _Pt( 52, 74 ) ), APDf( _B, _Pt( 1, 38 ) ) } );
	ADDIMAGE( PX_CV340S_E_50, &CV340S_E_50 );

	// CV340S/340E DN65
	ADDIMAGE( PX_CV340S_E_65, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_65( GIF_CV340S_E_65, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 51, 1 ) ), APDf( _O, _Pt( 51, 85 ) ), APDf( _B, _Pt( 1, 43 ) ) } );
	ADDIMAGE( PX_CV340S_E_65, &CV340S_E_65 );

	// CV340S/340E DN80
	ADDIMAGE( PX_CV340S_E_80, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_80( GIF_CV340S_E_80, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 59, 1 ) ), APDf( _O, _Pt( 59, 84 ) ), APDf( _B, _Pt( 1, 43 ) ) } );
	ADDIMAGE( PX_CV340S_E_80, &CV340S_E_80 );

	// CV340S/340E DN100
	ADDIMAGE( PX_CV340S_E_100, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_100( GIF_CV340S_E_100, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 52, 1 ) ), APDf( _O, _Pt( 52, 84 ) ), APDf( _B, _Pt( 1, 43 ) ) } );
	ADDIMAGE( PX_CV340S_E_100, &CV340S_E_100 );

	// CV340S/340E DN125
	ADDIMAGE( PX_CV340S_E_125, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_125( GIF_CV340S_E_125, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 56, 1 ) ), APDf( _O, _Pt( 56, 86 ) ), APDf( _B, _Pt( 1, 44 ) ) } );
	ADDIMAGE( PX_CV340S_E_125, &CV340S_E_125 );

	// CV340S/340E DN150
	ADDIMAGE( PX_CV340S_E_150, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_150( GIF_CV340S_E_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 55, 1 ) ), APDf( _O, _Pt( 55, 93 ) ), APDf( _B, _Pt( 1, 47 ) ) } );
	ADDIMAGE( PX_CV340S_E_150, &CV340S_E_150 );

	// CV340S/340E DN200
	ADDIMAGE( PX_CV340S_E_200, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_200( GIF_CV340S_E_200, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 59, 1 ) ), APDf( _O, _Pt( 59, 89 ) ), APDf( _B, _Pt( 1, 45 ) ) } );
	ADDIMAGE( PX_CV340S_E_200, &CV340S_E_200 );

	// CV340S/340E DN250-300
	ADDIMAGE( PX_CV340S_E_250_300, &TA_2WCV_M );
	NEWPRODPIC CV340S_E_250_300( GIF_CV340S_E_250_300, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 59, 1 ) ), APDf( _O, _Pt( 59, 89 ) ), APDf( _B, _Pt( 1, 45 ) ) } );
	ADDIMAGE( PX_CV340S_E_250_300, &CV340S_E_250_300 );

	// Vento V Connect.
	NEWPRODPIC VENTO_V_CONNECT( GIF_VENTO_V_CONNECT, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( PX_VENTO_V_CONNECT, &VENTO_V_CONNECT );

	// Vento VI Connect.
	NEWPRODPIC VENTO_VI_CONNECT( GIF_VENTO_VI_CONNECT, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( PX_VENTO_VI_CONNECT, &VENTO_VI_CONNECT );

	// Simply Vento 2.1 S Connect.
	NEWPRODPIC SIMPLY_VENTO_V2_1_S( GIF_SIMPLY_VENTO_V2_1_S, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( PX_SIMPLY_V2_1_S, &SIMPLY_VENTO_V2_1_S );

	// Simply Vento 2.1 SWME Connect.
	NEWPRODPIC SIMPLY_VENTO_V2_1_SWME( GIF_SIMPLY_VENTO_V2_1_SWME, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( PX_SIMPLY_V2_1_SWME, &SIMPLY_VENTO_V2_1_SWME );

	// Vento V F EcoEfficient.
	NEWPRODPIC VENTO_V_F_ECOEFFICIENT( GIF_VENTO_V_F_ECOEFFICIENT, CProdPic::Pic, epVertical, { APDf( _I, _Pt( 0, 0 ) ), APDf( _O, _Pt( 0, 0 ) ) } );
	ADDIMAGE( PX_VENTO_V_F_ECOEFF, &VENTO_V_F_ECOEFFICIENT );

	// TA-Smart DN 20-50.
	ADDIMAGE( PX_TASMART_20_50, &TA_SMART_MV );
	NEWPRODPIC TASMART_20_50( GIF_TASMART20_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 33, 1 ) ), APDf( _O, _Pt( 33, 208 ) ), APDf( _TS, _Pt( 106, 107), 295.0, 75 ) } );
	ADDIMAGE( PX_TASMART_20_50, &TASMART_20_50 );

	// TA-Smart DN 65-150.
	ADDIMAGE( PX_TASMART_65_150, &TA_SMART_MV );
	NEWPRODPIC TASMART_65_150( GIF_TASMART65_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 45, 1 ) ), APDf( _O, _Pt( 45, 138 ) ), APDf( _TS, _Pt( 115, 66), 295.0, 75 ) } );
	ADDIMAGE( PX_TASMART_65_150, &TASMART_65_150 );

	// External temperature housing
	ADDIMAGE( PX_TEMP_SENSOR, &TA_TEMP_SENSOR );
	NEWPRODPIC TA_EXTERNAL_TEMP_HOUSING( GIF_EXTERNAL_TEMP_HOUSING, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 30, 2 ) ), APDf( _O, _Pt( 30, 45 ) ), APDf( _TS, _Pt( 57, 28), 90.0, 100 ) } );
	ADDIMAGE( PX_TEMP_SENSOR, &TA_EXTERNAL_TEMP_HOUSING );

	// TA-Smart Dp DN 20-50.
	ADDIMAGE( PX_TASMARTDP_20_50, &TA_SMART_DP );
	NEWPRODPIC TASMART_DP_20_50( GIF_TASMART20_50, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 33, 1 ) ), APDf( _O, _Pt( 33, 208 ) ), APDf( _IS, _Pt( 112, 22), 90.0, 40 ) } );
	ADDIMAGE( PX_TASMARTDP_20_50, &TASMART_DP_20_50 );

	// TA-Smart Dp DN 65-150.
	ADDIMAGE( PX_TASMARTDP_65_150, &TA_SMART_DP );
	NEWPRODPIC TASMART_DP_65_150( GIF_TASMART65_150, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 45, 1 ) ), APDf( _O, _Pt( 45, 138 ) ), APDf( _IS, _Pt( 117, 17), 90.0, 40 ) } );
	ADDIMAGE( PX_TASMARTDP_65_150, &TASMART_DP_65_150 );

	// TA-Link.
	ADDIMAGE( PX_TALINK, &TA_LINK );
	NEWPRODPIC TA_LINK_IMG( GIF_TA_LINK_IMG, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 55, 1 ) ), APDf( _O, _Pt( 55, 133 ) ), APDf( _CP, _Pt( 4, 113 ), 180.0, 40 ), 
			APDf( _CS, _Pt( 4, 93 ), 180.0, 40 ), APDf( _OS, _Pt( 55, 1 ), 90.0, 40 ) } );
	ADDIMAGE( PX_TALINK, &TA_LINK_IMG );


	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Create all circuit schemes objects
	// Remark: The default size for a circuit schematic is 518x660 pixels (See all the "CircSheme XXX.gif").

	// Top/Bottom pipes (arrows).
	CDynCircSch DynCircuit( GIF_TOPARROWS );
	ADDCIRCSCH( SCH_PIPESTOPBOTARROWS, DynCircuit );

	// Top/Bottom pipes (Pending).
	DynCircuit.Reset( GIF_TOPPENDING );
	ADDCIRCSCH( SCH_PIPESTOPPENDING, DynCircuit );

	// One pipe arrow.
	DynCircuit.Reset( GIF_ONEPIPETOPARROW );
	ADDCIRCSCH( SCH_ONEPIPETOPBOTARROW, DynCircuit );

	// Bottom distribution pipes.
	DynCircuit.Reset( GIF_BOTTOMDISTR );
	ADDCIRCSCH( SCH_DISTRIBPIPES, DynCircuit );

	DynCircuit.Reset( GIF_BOTTOMDISTR_REV );
	ADDCIRCSCH( SCH_DISTRIBPIPES_REV, DynCircuit );
	
	// Terminal unit.
	DynCircuit.Reset( GIF_CIRCSCHEME_TU );
	ADDCIRCSCH( SCH_TERMINALUNIT, DynCircuit );
	
	// Straight pipes circuit.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	ADDCIRCSCH( CIRSCH_SP, DynCircuit );

	// HYS-1579: Separate Pump and Pump + BV
	// Pump circuit.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_PUMP, DynCircuit );

	// Pump circuit with a balancing valve.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_PUMP_BV, DynCircuit );

	// DC_BV circuit, Reset DynCircuit with a new base image.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, no capillary point needed.
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_DC_BV, DynCircuit );

	// DC_DP DpC stabilized on branch.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is pplus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );	
	ADDCIRCSCH( CIRSCH_DC_DP, DynCircuit );

	// DC_DPF DpC stabilized on branch, measuring valve in primary.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points for DpC (group 0).
	{
		APT DpCF = APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pminus );
		DynCircuit.AddAnchorPt( DpCF );
		// Add second DpC anchoring point connected on the Pplus (group 1).
		DpCF.SetConnectType( APT::Spline )->SetGroup( 1 )->SetPressureSign( APT::Pplus );	
		DynCircuit.AddAnchorPt( DpCF );
		// Add Measuring valve will be connected to the P+ (group 0).
		DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
		// Add connection of DpCF P- to pipe (group 1).
		DynCircuit.AddAnchorPt( APT( _Pt( 130, 250 ), APT::Spline, 1 ) );
		DynCircuit.AddAnchorPt( APT( _Pt( 101, 250 ), APT::Spline, 1 ) );
	}
	ADDCIRCSCH( CIRSCH_DC_DPF, DynCircuit );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1952: Smart differential pressure controller. The smart differential pressure controller is located on the supply.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );

	// Define the smart valve Dp component and set that we have an input signal cable connection.
	// When drawing the component, the "CEnBitmapPatchWork::DrawComponent" method will automatically add the start point and the first control point
	// for the spline curve from the input signal cable connection point of the smart valve Dp.
	APT SmartValveDpAP = APT( _Pt( 101, 430 ), APT::Mid, _T('N'), APT::SmartDpC, APT::s2W_Straight, APT::voCW );
	SmartValveDpAP.SetIOConnection( APT::IOC_Yes );
	DynCircuit.AddAnchorPt( SmartValveDpAP );

	// Define the Dp sensor component and set that we have an output signal cable connection.
	// When drawing the component, the "CEnBitmapPatchWork::DrawComponent" method will automatically add the last control point and the last point
	// for the spline curve to the output signal cable connection point of Dp sensor.
	APT DpSensorAP = APT( _Pt( 258, 230 ), APT::Mid, _T('W'), APT::DpSensor, APT::sNU, APT::voCW );
	APT DpSensorAPCopy = DpSensorAP;
	DpSensorAP.SetIOConnection( APT::IOC_Yes );
	DynCircuit.AddAnchorPt( DpSensorAP );

	// Add spline points for the input/output signal cable (Group = 0) between the TA-Link and the TA-Smart Dp.
	DynCircuit.AddAnchorPt( APT( _Pt( 140, 280 ), APT::Spline, 0 ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 200, 370 ), APT::Spline, 0 ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 310, 550 ), APT::Spline, 0 ) );

	// Here we add the Dp sensor component but in virtual mode. It means that we will not draw again the component but we will prepare
	// a new spline curve. This one belongs to the group 1 (To differentiate with the input/ouput signal cable spline curve).
	// Because point is not set (=INT_MIN) and we define the pressure sign, internally we will gather point information from the
	// 'TA_LINK' CProdPic object defines above (with NEWPRODPIC TA_LINK) -> Control point distance and angle are thus defined in this 'TA_LINK' object.
	// When drawning the component, the "CEnBitmapPatchWork::DrawComponent" method will automatically add the first point and the first control point
	// for the spline curve from the '_CP' point of the Dp sensor component.
	// Remark: point coordinate is retrieve from the 'TA_LINK' definition. And because point here is not defined (=INT_MIN), we know that we will
	//         need to convert this coordinate from the Dp sensor picture to the hydraulic circuit coordinate system and apply correct translation and rotation
	//          to appply the point on the final bitmap.
	DpSensorAP = DpSensorAPCopy;
	DpSensorAP.SetGroup( 1 );
	DpSensorAP.SetPressureSign( APT::Pminus );
	DpSensorAP.SetVirtualPoint( true );
	DynCircuit.AddAnchorPt( DpSensorAP );

	// We add again the Dp sensor component in virtual mode to allow to finish spline curve definition.
	// Here it's a special case because we don't link the end of spline curve on an existing component. We add a virtual point on the supply pipe.
	// We will thus not retrieve information from a component definition but we will set the physical point, control point distance and angle directly
	// in the anchoring point.
	// When drawning the component, the "CEnBitmapPatchWork::DrawComponent" method will automatically add the last control point and the last point
	// for the spline curve to the virtual point.
	// Remark: point coordinate is defined in this anchoring point. We know that coordinate are already in the hydraulic circuit coordinate system.
	//         We will thus only need to translate or/and rotate to appply the point on the final bitmap.
	DpSensorAP = DpSensorAPCopy;

	// Mandatory to call the 'SetPoint' method with a pointer (Otherwise it's the other "SetPoint" that is called and this one clear all the "CAnchorPt" content).
	pair<int, int> ptVirtual = { 101, 120 };
	DpSensorAP.SetPoint( &ptVirtual );
	DpSensorAP.SetGroup( 1 );
	DpSensorAP.SetVirtualPoint( true );
	DpSensorAP.SetCtrlPointDistance( 50 );
	DpSensorAP.SetAngle( 270.0 );
	DynCircuit.AddAnchorPt( DpSensorAP );

	// We do the same as above but now with the Pminus point of the TA-Link component.
	DpSensorAP = DpSensorAPCopy;
	DpSensorAP.SetGroup( 2 );
	DpSensorAP.SetPressureSign( APT::Pplus );
	DpSensorAP.SetVirtualPoint( true );
	DynCircuit.AddAnchorPt( DpSensorAP );

	// Same as the group 1, we finish the spline curve definition.
	DpSensorAP = DpSensorAPCopy;

	// Mandatory to call the 'SetPoint' method with a pointer (Otherwise it's the other "SetPoint" that is called and this one clear all the "CAnchorPt" content).
	ptVirtual = { 416, 120 };
	DpSensorAP.SetPoint( &ptVirtual );
	DpSensorAP.SetGroup( 2 );
	DpSensorAP.SetVirtualPoint( true );
	DpSensorAP.SetCtrlPointDistance( 50 );
	DpSensorAP.SetAngle( 90.0 );
	DynCircuit.AddAnchorPt( DpSensorAP );
		
	ADDCIRCSCH( CIRSCH_DC_SMARTDP1, DynCircuit );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1952: Smart differential pressure controller. The smart differential pressure controller is located on the return.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );

	// Define the smart valve Dp component and set that we have an input signal cable connection.
	// When drawing the component, the "CEnBitmapPatchWork::DrawComponent" method will automatically add the start point and the first control point
	// for the spline curve from the input signal cable connection point of the smart valve Dp.
	SmartValveDpAP = APT( _Pt( 416, 430 ), APT::Mid, _T('S'), APT::SmartDpC, APT::s2W_Straight, APT::voCW );
	SmartValveDpAP.SetIOConnection( APT::IOC_Yes );
	DynCircuit.AddAnchorPt( SmartValveDpAP );

	// Define the Dp sensor component and set that we have an output signal cable connection.
	// When drawing the component, the "CEnBitmapPatchWork::DrawComponent" method will automatically add the last control point and the last point
	// for the spline curve to the output signal cable connection point of Dp sensor.
	DpSensorAP = APT( _Pt( 258, 230 ), APT::Mid, _T('W'), APT::DpSensor, APT::sNU, APT::voCCW );
	DpSensorAPCopy = DpSensorAP;
	DpSensorAP.SetIOConnection( APT::IOC_Yes );
	DynCircuit.AddAnchorPt( DpSensorAP );

	// Here we add the Dp sensor component but in virtual mode. It means that we will not draw again the component but we will prepare
	// a new spline curve. This one belongs to the group 1 (To differentiate with the input/ouput signal cable spline curve).
	// Because we define the pressure sign, internally we will gather point information from the 'TA_LINK' CProdPic object defines above 
	// (with NEWPRODPIC TA_LINK) -> Control point distance and angle are thus defined in this 'TA_LINK' object.
	// When drawning the component, the "CEnBitmapPatchWork::DrawComponent" method will automatically add the first point and the first control point
	// for the spline curve from the '_CP' point of the Dp sensor component.
	// Remark: point coordinate is retrieve from the 'TA_LINK' definition. And because point here is not defined (=INT_MIN), we know that we will
	//         need to convert this coordinate from the TA-Link picture to the hydraulic circuit coordinate system and apply correct translation and rotation
	//          to appply the point on the final bitmap.
	DpSensorAP = DpSensorAPCopy;
	DpSensorAP.SetGroup( 1 );
	DpSensorAP.SetPressureSign( APT::Pminus );
	DpSensorAP.SetVirtualPoint( true );

	// Because a bug in the library (And I have not time and the energy to correct) we have now the possibility to set a control point distance
	// for a virtual point to bypass the one defined in the corresponding point in the component.
	DpSensorAP.SetCtrlPointDistance( -40 );
	DynCircuit.AddAnchorPt( DpSensorAP );

	// We add again the Dp sensor component in virtual mode to allow to finish spline curve definition.
	// Here it's a special case because we don't link the end of spline curve on an existing component. We add a virtual point on the supply pipe.
	// We will thus not retrieve information from a component definition but we will set the physical point, control point distance and angle directly
	// in the anchoring point.
	// When drawning the component, the "CEnBitmapPatchWork::DrawComponent" method will automatically add the last control point and the last point
	// for the spline curve to the virtual point.
	// Remark: point coordinate is defined in this anchoring point. We know that coordinate are already in the hydraulic circuit coordinate system.
	//         We will thus only need to translate or/and rotate to appply the point on the final bitmap.
	DpSensorAP = DpSensorAPCopy;

	// Mandatory to call the 'SetPoint' method with a pointer (Otherwise it's the other "SetPoint" that is called and this one clear all the "CAnchorPt" content).
	ptVirtual = { 416, 120 };
	DpSensorAP.SetPoint( &ptVirtual );
	DpSensorAP.SetGroup( 1 );
	DpSensorAP.SetVirtualPoint( true );
	DpSensorAP.SetCtrlPointDistance( 50 );
	DpSensorAP.SetAngle( 270.0 );
	DynCircuit.AddAnchorPt( DpSensorAP );

	// We do the same as above but now with the Pminus point of the Dp sensor component.
	DpSensorAP = DpSensorAPCopy;
	DpSensorAP.SetGroup( 2 );
	DpSensorAP.SetPressureSign( APT::Pplus );
	DpSensorAP.SetVirtualPoint( true );

	// Because a bug in the library (And I have not time and the energy to correct) we have now the possibility to set a control point distance
	// for a virtual point to bypass the one defined in the corresponding point in the component.
	DpSensorAP.SetCtrlPointDistance( -40 );
	DynCircuit.AddAnchorPt( DpSensorAP );

	// Same as the group 1, we finish the spline curve definition.
	DpSensorAP = DpSensorAPCopy;

	// Mandatory to call the 'SetPoint' method with a pointer (Otherwise it's the other "SetPoint" that is called and this one clear all the "CAnchorPt" content).
	ptVirtual = { 101, 120 };
	DpSensorAP.SetPoint( &ptVirtual );
	DpSensorAP.SetGroup( 2 );
	DpSensorAP.SetVirtualPoint( true );
	DpSensorAP.SetCtrlPointDistance( 50 );
	DpSensorAP.SetAngle( 90.0 );
	DynCircuit.AddAnchorPt( DpSensorAP );
	ADDCIRCSCH( CIRSCH_DC_SMARTDP2, DynCircuit );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////////
	// 2Way Circuits
	/////////////////////////////////////
	
	// 2WD_BV Control valve & Balancing valve.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_2WD_BVC, DynCircuit );

	// 2Ways circuit, DpC on branch + MV on primary + CV - Control only
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points for valves.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 540 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	ADDCIRCSCH( CIRSCH_2WD_DP1X, DynCircuit );
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );

	// 2Ways circuit, DpC on CV + MV on primary + CV - Control only.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points for valves.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 120 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 540 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	// Define additional points for capillary curve number should be a multiple of 3.
	DynCircuit.AddAnchorPt( APT( _Pt( 283, 160 ) ) )
			  .AddAnchorPt( APT( _Pt( 283, 330 ) ) )
			  .AddAnchorPt( APT( _Pt( 283, 550 ) ) );
	ADDCIRCSCH( CIRSCH_2WD_DP2X, DynCircuit );
	
	// 2Ways circuit, DpC on branch + PressetPT CV on secondary 
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW , APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 530 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	// Define additional points for capillary curve number should be a multiple of 3.
	ADDCIRCSCH( CIRSCH_2WD_DP3X, DynCircuit );

	// 2Ways circuit, DpC on CV + PressetPT CV on secondary 
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points for valves.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 120 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW , APT::Pplus ) );
	ADDCIRCSCH( CIRSCH_2WD_DP4X, DynCircuit );

	// PICV stabilized on branch.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is pplus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::PICV, APT::sNU, APT::voCCW ) );	
	ADDCIRCSCH( CIRSCH_2WD_PICV, DynCircuit );

	// DpCBCV stabilized on branch.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, SV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of SvLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is plus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::DPCBCV, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::ShutoffValve, APT::sNU, APT::voCCW, APT::Pplus ) );	
	ADDCIRCSCH( CIRSCH_2WD_DPCBCV, DynCircuit );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1671: Smart control valve. The smart control valve is located on the supply.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );

	// Define anchoring point for the smart control valve.
	APT SmartValveAP = APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::SmartControlValve, APT::s2W_Straight, APT::voCW );
	SmartValveAP.SetTempConnection( APT::TC_Yes );
	DynCircuit.AddAnchorPt( SmartValveAP );
	
	// Define anchoring point for the temperature sensor.
	APT TempSensorAP = APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::TempSensor, APT::sNU, APT::voCW );
	TempSensorAP.SetTempConnection( APT::TC_Yes );
	DynCircuit.AddAnchorPt( TempSensorAP );

	// Now add 3 spline intermediate points.
	DynCircuit.AddAnchorPt( APT( _Pt( 211, 310 ) ) )
			  .AddAnchorPt( APT( _Pt( 264, 392 ) ) )
			  .AddAnchorPt( APT( _Pt( 308, 402 ) ) );

	ADDCIRCSCH( CIRSCH_2WD_SMART1, DynCircuit );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1671: Smart control valve. The smart control valve is located on the return.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );

	// Define anchoring point for the temperature sensor.
	TempSensorAP = APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::TempSensor, APT::sNU, APT::voCW );
	TempSensorAP.SetTempConnection( APT::TC_Yes );
	DynCircuit.AddAnchorPt( TempSensorAP );

	// Define anchoring point for the smart control valve.
	SmartValveAP = APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::SmartControlValve, APT::s2W_Straight, APT::voCW  );
	SmartValveAP.SetTempConnection( APT::TC_Yes );
	DynCircuit.AddAnchorPt( SmartValveAP );

	// Now add 3 spline intermediate points.
	DynCircuit.AddAnchorPt( APT( _Pt( 211, 258 ) ) )
			  .AddAnchorPt( APT( _Pt( 254, 270 ) ) )
			  .AddAnchorPt( APT( _Pt( 307, 319 ) ) );

	ADDCIRCSCH( CIRSCH_2WD_SMART2, DynCircuit );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////
	// 2Way Injection Circuits
	/////////////////////////////////////

	// 2WD_BV_INJ Control valve & Balancing valve.
	DynCircuit.Reset( GIF_CIRCSCHEME_2W_INJ );
	// Define Anchoring points.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 422 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 422 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_2WD_BVC_INJ, DynCircuit );

	// 2Ways injection circuit, with Dp controller on cv (control only); MV on secondary
	DynCircuit.Reset( GIF_CIRCSCHEME_2W_INJ );
	// Define Anchoring points for valves.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 280 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 422 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 575 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	// Define additional points for capillary curve number should be a multiple of 3.
	DynCircuit.AddAnchorPt( APT( _Pt( 283, 320 ) ) )
		.AddAnchorPt( APT( _Pt( 283, 420 ) ) )
		.AddAnchorPt( APT( _Pt( 283, 585 ) ) );
	ADDCIRCSCH( CIRSCH_2WD_DP_INJ, DynCircuit );

	// 2Ways injection circuit, DpC on CV + PressetPT CV on secondary
	DynCircuit.Reset( GIF_CIRCSCHEME_2W_INJ );
	// Define Anchoring points for valves.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 280 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 422 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
// 	// Define additional points for capillary curve number should be a multiple of 3.
// 	DynCircuit.AddAnchorPt( APT( _Pt( 283, 320 ) ) )
// 		.AddAnchorPt( APT( _Pt( 283, 420 ) ) )
// 		.AddAnchorPt( APT( _Pt( 283, 585 ) ) );
	ADDCIRCSCH( CIRSCH_2WD_DP_INJ_PT, DynCircuit );

	// PICV stabilized on branch.
	DynCircuit.Reset( GIF_CIRCSCHEME_2W_INJ );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is pplus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 422 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 422 ), APT::Mid, _T('S'), APT::PICV, APT::sNU, APT::voCCW ) );	
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_2WD_PICV_INJ, DynCircuit );


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1671: Smart control valve. The smart control valve is located on the supply.
	DynCircuit.Reset( GIF_CIRCSCHEME_2W_INJ );
	
	// Define anchoring point for the smart control valve.
	SmartValveAP = APT( _Pt( 101, 422 ), APT::Mid, _T( 'N' ), APT::SmartControlValve, APT::s2W_Straight, APT::voCW );
	SmartValveAP.SetTempConnection( APT::TC_Yes );
	DynCircuit.AddAnchorPt( SmartValveAP );

	// Define anchoring point for the temperature sensor.
	TempSensorAP = APT( _Pt( 416, 422 ), APT::Mid, _T( 'S' ), APT::TempSensor, APT::sNU, APT::voCW );
	TempSensorAP.SetTempConnection( APT::TC_Yes );
	DynCircuit.AddAnchorPt( TempSensorAP );

	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T( 'N' ), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T( 'S' ), APT::BV_S, APT::sNU, APT::voCW ) );

	// Now add 3 spline intermediate points.
	DynCircuit.AddAnchorPt( APT( _Pt( 211, 432 ) ) )
			  .AddAnchorPt( APT( _Pt( 254, 474 ) ) )
			  .AddAnchorPt( APT( _Pt( 308, 494 ) ) );

	ADDCIRCSCH( CIRSCH_2WD_SMART_INJ1, DynCircuit );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1671: Smart control valve. The smart control valve is located on the return.
	DynCircuit.Reset( GIF_CIRCSCHEME_2W_INJ );
	
	// Define anchoring point for the temperature sensor.
	TempSensorAP = APT( _Pt( 101, 422 ), APT::Mid, _T( 'N' ), APT::TempSensor, APT::sNU, APT::voCW );
	TempSensorAP.SetTempConnection( APT::TC_Yes );
	DynCircuit.AddAnchorPt( TempSensorAP );

	// Define anchoring point for the smart control valve.
	SmartValveAP = APT( _Pt( 416, 422 ), APT::Mid, _T( 'S' ), APT::SmartControlValve, APT::s2W_Straight, APT::voCW );
	SmartValveAP.SetTempConnection( APT::TC_Yes );
	DynCircuit.AddAnchorPt( SmartValveAP );

	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T( 'N' ), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T( 'S' ), APT::BV_S, APT::sNU, APT::voCW ) );

	// Now add 3 spline intermediate points.
	DynCircuit.AddAnchorPt( APT( _Pt( 211, 350 ) ) )
			  .AddAnchorPt( APT( _Pt( 254, 362 ) ) )
			  .AddAnchorPt( APT( _Pt( 307, 411 ) ) );

	ADDCIRCSCH( CIRSCH_2WD_SMART_INJ2, DynCircuit );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// HYS-1930: DC_DPAUTOADAPT DpC stabilized on BV bypass.
	// Remark: The circuit scheme GIF is the same as a 2-way injection, but obviously here it's not a 2-way control circuit !
	DynCircuit.Reset( GIF_CIRCSCHEME_2W_AUTOADAPTIVE );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is 0 and the DpC capillary connection point is pplus.
	APT DpCAP = APT( _Pt( 416, 491 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::ePressSign::Pminus );
	DynCircuit.AddAnchorPt( DpCAP );

	APT BvBypAP = APT( _Pt( 258, 257 ), APT::Mid, _T('W'), APT::BV_Byp, APT::sNU, APT::voCW, APT::Pplus );
	DynCircuit.AddAnchorPt( BvBypAP );
	
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );

	ADDCIRCSCH( CIRSCH_DC_AUTOADAPT, DynCircuit );


	/////////////////////////////////////
	// 3Way Circuits
	/////////////////////////////////////

	// 3W CV - BV & BV Bypass.
	DynCircuit.Reset( GIF_CIRCSCHEME_3W );
	// Define Anchoring points.
	DynCircuit.AddAnchorPt( APT( _Pt( 258, 258 ), APT::Mid, _T('E'), APT::BV_Byp, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 258 ), APT::Mid, _T('S'), APT::ControlValve, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 465 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_3WD_BVCVBVB, DynCircuit );

	// 3W MV (in primary)- DpC - CV - BVbyp.
	DynCircuit.Reset( GIF_CIRCSCHEME_3W );
	// Define Anchoring points.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 465 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 258, 258 ), APT::Mid, _T('E'), APT::BV_Byp, APT::sNU, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 258 ), APT::Mid, _T('S'), APT::ControlValve, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 465 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	ADDCIRCSCH( CIRSCH_3WD_DP, DynCircuit );

	// 3Ways mixing circuit with decoupling bypass, primary flow lower than secondary flow, CV located in primary
	DynCircuit.Reset( GIF_CIRCSCHEME_3W_INJ );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 580 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 438 ), APT::Mid, _T('S'), APT::ControlValve, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_3WINJ_BV_CC1, DynCircuit );

	// Dp Stabilized - 3Ways mixing circuit with decoupling bypass, primary flow lower than secondary flow
	DynCircuit.Reset( GIF_CIRCSCHEME_3W_INJ );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 580 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 580 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 438 ), APT::Mid, _T('S'), APT::ControlValve, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_3WINJ_DP_CC1, DynCircuit );

	// 3Ways mixing circuit with decoupling bypass, primary flow  higher than secondary flow CV in secondary
	DynCircuit.Reset( GIF_CIRCSCHEME_3W_INJ );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 580 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 258 ), APT::Mid, _T('N'), APT::ControlValve, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_3WINJ_BV_CC2, DynCircuit );

	// Dp Stabilized - 3Ways mixing circuit with decoupling bypass, primary flow  higher than secondary flow CV in secondary
	DynCircuit.Reset( GIF_CIRCSCHEME_3W_INJ );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 580 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 580 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 258 ), APT::Mid, _T('N'), APT::ControlValve, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_3WINJ_DP_CC2, DynCircuit );

	// 3Ways mixing circuit with primary balancing valve
	DynCircuit.Reset( GIF_CIRCSCHEME_3W );
	// Define Anchoring points.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 258 ), APT::Mid, _T('N'), APT::ControlValve, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 580 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_3WINJ_BV1, DynCircuit );

	// 3Ways mixing circuit Dp stab
	DynCircuit.Reset( GIF_CIRCSCHEME_3W );
	// Define Anchoring points.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 258 ), APT::Mid, _T('N'), APT::ControlValve, APT::s3W, APT::voCCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 465 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 465 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 87 ), APT::Mid, _T('N'), APT::PUMP, APT::sNU, APT::voCW ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 87 ), APT::Mid, _T('S'), APT::BV_S, APT::sNU, APT::voCW ) );
	ADDCIRCSCH( CIRSCH_3WINJ_DP, DynCircuit );

	/////////////////////////////////////
	// Scheme used for individual selection of Dp controller.	
	//
	/////////////////////////////////////

	// DC_DP DpC stabilized on branch primary.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is pplus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );	
	ADDCIRCSCH( CIRSCH_SSELDPC_11, DynCircuit );

	// DC_DP DpC stabilized on branch secondary.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is pplus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 150 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 530 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );	
	// Define additional points for capillary curve number should be a multiple of 3.
	DynCircuit.AddAnchorPt( APT( _Pt( 258, 160 ) ) )
			.AddAnchorPt( APT( _Pt( 258, 330 ) ) )
			.AddAnchorPt( APT( _Pt( 258, 550 ) ) );
	ADDCIRCSCH( CIRSCH_SSELDPC_12, DynCircuit );

	// HYS-1828: DC_DP DpC stabilized on branch primary for a set selection.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is pplus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	ADDCIRCSCH( CIRSCH_SSELDPCSET_11, DynCircuit );

	// HYS-1828: DC_DP DpC stabilized on branch secondary for a set selection.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is pplus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 150 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 530 ), APT::Mid, _T('S'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );	
	// Define additional points for capillary curve number should be a multiple of 3.
	DynCircuit.AddAnchorPt( APT( _Pt( 258, 160 ) ) )
			.AddAnchorPt( APT( _Pt( 258, 330 ) ) )
			.AddAnchorPt( APT( _Pt( 258, 550 ) ) );
	ADDCIRCSCH( CIRSCH_SSELDPCSET_12, DynCircuit );

	// DC_DP DpCF stabilized on branch primary.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	// Add anchoring point for DpCF.
	// Default group is O and the DpC capillary connection point is Pminus.
	{
		APT DpCF = APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pminus );	
		DynCircuit.AddAnchorPt( DpCF );
		// Add second DpC anchoring point connected on the Pplus (group 1 ) 
		DpCF.SetConnectType( APT::Spline )->SetGroup( 1 )->SetPressureSign( APT::Pplus );
		DynCircuit.AddAnchorPt( DpCF );
		// Add connection of DpCF P- to pipe (group 1 )
		DynCircuit.AddAnchorPt( APT( _Pt( 130, 250 ), APT::Spline, 1 ) );
		DynCircuit.AddAnchorPt( APT( _Pt( 101, 250 ), APT::Spline, 1 ) );
	}
	ADDCIRCSCH( CIRSCH_SSELDPCF_11, DynCircuit );

	// DC_DP DpCF stabilized on branch secondary.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, MV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of MvLoc and DpCLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is pminus.
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 150 ), APT::Mid, _T('S'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	{
		APT DpCF = APT( _Pt( 101, 530 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pminus );	
		DynCircuit.AddAnchorPt(DpCF);
		// Add second DpC anchoring point connected on the Pplus (group 1 ).
		DpCF.SetConnectType( APT::Spline )->SetGroup( 1 )->SetPressureSign( APT::Pplus );
		DynCircuit.AddAnchorPt(DpCF);
		// Add connection of DpCF P- to pipe (group 1 ).
		DynCircuit.AddAnchorPt( APT( _Pt( 130, 450 ), APT::Spline, 1 ) );
		DynCircuit.AddAnchorPt( APT( _Pt( 101, 450 ), APT::Spline, 1 ) );
	}
	// Define additional points for capillary curve number should be a multiple of 3.
	DynCircuit.AddAnchorPt( APT( _Pt( 250, 160 ) ) ).AddAnchorPt( APT( _Pt( 300, 330 ) ) ).AddAnchorPt( APT( _Pt( 350, 550 ) ) );
	ADDCIRCSCH( CIRSCH_SSELDPCF_12, DynCircuit );

	// 2WD DP CV stabilized by DpC.
	DynCircuit.Reset( GIF_CIRCSCHEME_ONEPIPE );
	// Define Anchoring points for valves.
	DynCircuit.AddAnchorPt( APT( _Pt( 201, 540 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 201, 120 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 201, 330 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW ) );
	// Define additional points for capillary curve number should be a multiple of 3.
	DynCircuit.AddAnchorPt( APT( _Pt( 400, 160 ) ) ).AddAnchorPt( APT( _Pt( 400, 330 ) ) ).AddAnchorPt( APT( _Pt( 400, 550 ) ) );
	ADDCIRCSCH( CIRSCH_SSELDPC_2X, DynCircuit );

	// HYS-1828: 2WD DP CV stabilized by DpC for a set selection.
	DynCircuit.Reset( GIF_CIRCSCHEME_ONEPIPE );
	// Define Anchoring points for valves.
	DynCircuit.AddAnchorPt( APT( _Pt( 201, 540 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 201, 120 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 201, 330 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW ) );
	// Define additional points for capillary curve number should be a multiple of 3.
	DynCircuit.AddAnchorPt( APT( _Pt( 400, 160 ) ) ).AddAnchorPt( APT( _Pt( 400, 330 ) ) ).AddAnchorPt( APT( _Pt( 400, 550 ) ) );
	ADDCIRCSCH( CIRSCH_SSELDPCSET_2X, DynCircuit );

	// 2WD DP CV stabilized by DpCF.
	DynCircuit.Reset( GIF_CIRCSCHEME_ONEPIPE );
	// Define Anchoring points for valves.
	DynCircuit.AddAnchorPt( APT( _Pt( 201, 120 ), APT::Mid, _T('N'), APT::BV_P, APT::sNU, APT::voCW, APT::PUndef ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 201, 330 ), APT::Mid, _T('N'), APT::ControlValve, APT::s2W_Straight, APT::voCCW ) );
	{
		APT DpCF = APT( _Pt( 201, 540 ), APT::Mid, _T('N'), APT::DpC, APT::sNU, APT::voCW, APT::Pminus );	
		DynCircuit.AddAnchorPt(DpCF);
		// Add second DpC anchoring point connected on the Pplus (group 1 ).
		DpCF.SetConnectType( APT::Spline )->SetGroup( 1 )->SetPressureSign( APT::Pplus );	
		DynCircuit.AddAnchorPt( DpCF );
		// Add connection of DpCF P- to pipe (group 1 ).
		DynCircuit.AddAnchorPt( APT( _Pt( 230, 460 ), APT::Spline, 1 ) );
		DynCircuit.AddAnchorPt( APT( _Pt( 201, 460 ), APT::Spline, 1 ) );
	}
	// Define additional points for capillary curve number should be a multiple of 3.
	DynCircuit.AddAnchorPt( APT( _Pt( 400, 160 ) ) ).AddAnchorPt( APT( _Pt( 400, 330 ) ) ).AddAnchorPt( APT( _Pt( 400, 550 ) ) );
	ADDCIRCSCH( CIRSCH_SSELDPCF_2X, DynCircuit );

	/////////////////////////////////////
	// Scheme used for SSel DpCBCV
	// (Combined Dp controller, control and balancing valve (ie: TA-COMPACT-DP)
	/////////////////////////////////////
	
	// DpCBCV stabilized on branch without STS.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, SV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of SvLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is plus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::DPCBCV, APT::sNU, APT::voCW, APT::Pplus ) );
	ADDCIRCSCH( CIRSCH_SSELDPCBCV, DynCircuit );

	// DpCBCV stabilized on branch with STS.
	DynCircuit.Reset( GIF_CIRCSCHEME_SP );
	// Define Anchoring points, SV capilary defined as APT::PUndef will be connected automatically 
	// by the engine in function of SvLoc coming from CDB_CircuitScheme.
	// Default group is O and the DpC capillary connection point is plus.
	DynCircuit.AddAnchorPt( APT( _Pt( 101, 330 ), APT::Mid, _T('N'), APT::DPCBCV, APT::sNU, APT::voCW, APT::Pplus ) );
	DynCircuit.AddAnchorPt( APT( _Pt( 416, 330 ), APT::Mid, _T('S'), APT::ShutoffValve, APT::sNU, APT::voCCW, APT::Pplus ) );	
	ADDCIRCSCH( CIRSCH_2WD_DPCBCV_STS, DynCircuit );

	return TRUE;
}
