
#pragma once

#include <string>
#include <vector>
#include <map>
#include "EnBitmapAnchorPt.h"

// Redefine enum for an easy read
/*
#define CProdPic::PicTASymb		CProdPic::CProdPic::PicTASymb
#define PicImage		CProdPic::Pic
#define CProdPic::PicLocSymb		CProdPic::CProdPic::PicLocSymb
#define CProdPic::LastPicType		CProdPic::CProdPic::LastPicType
*/

using namespace std;

typedef std::pair<short,short> _Pt;

enum ePosition
{
	epUndef,
	epHorizontal,
	epVertical,
	epLast
};

enum eConnectionPoints
{
	ecpIn = 0,			// Input point
	ecpOut,				// Output point
	ecpBypass,			// Bypass point
	ecpCapilPrim,		// For MV	Primary capillary point is UPSTREAM			// For DPC	Primary capillary point is P+
	ecpCapilSec,		//			Secondary capillary point is DOWNSTREAM		//			Secondary capillary point is P-
	ecpActr,			// Actuator
	ecpTempSensor,		// For temperature sensor cable (Ex: TA-Smart)
	ecpInputSignal,		// For input signal cable (Ex: TA-Smart Dp)
	ecpOutputSignal,	// For output signal cable (Ex: Dp sensor)
	ecpLast
};

enum eConnectionAngles
{
	ecaCapilPrim,	// Primary capillary angle
	ecaCapilSec,	// Secondary capillary angle
	ecaActr,		// Actuator angle
	ecaLast
};

/*
	CProdPic :  used to describe a product image 
	Usage : 
		CProdPic TA_2WCV_01(GIF_2WCV_01,CProdPic::PicTASymb, epVertical, epVertical, epVertical,_Pt(33,1),_Pt(33,123));
*/
class CProdPic
{
public:
	enum eProdPicType
	{
		PicTASymb, 		// 0
		Pic,			// 1
		PicLocSymb,		// 2 
		LastPicType		// 3
	};

	typedef struct _AnchoringPointDef
	{
		eConnectionPoints m_eType;
		_Pt m_ptPos;
		double m_dAngle;
		int m_iCtrlPointDistance;			// If defined, we use this distance in pixel to compute the first and last point for
											// the bezier curves (See the "CEnBitmapPatchWork::CalculateCtrlPoint" method in the "EnBitmapPatchWork.cpp" file).

		struct _AnchoringPointDef( int iType, _Pt ptPos, double dAngle = 0.0, int iCtrlPointDistance = 0 ) 
				{ m_eType = (eConnectionPoints)iType, m_ptPos = ptPos; m_dAngle = dAngle; m_iCtrlPointDistance = iCtrlPointDistance; }
	}AnchoringPointDef;

	CProdPic();													// Default constructor

	CProdPic(	UINT iResID,
				eProdPicType etype,
				ePosition ePos,	
				std::vector<AnchoringPointDef> vecAnchPointList
			);

	~CProdPic() {}

	UINT GetResID() { return m_iResID; }
	eProdPicType GetProdPicType() { return m_ePicType; }
	ePosition GetProdPosition() { return m_ePosition; }
	const AnchoringPointDef *GetOneAnchoringPoint( eConnectionPoints ConnPt );
	const std::vector<AnchoringPointDef> GetAllAnchoringPoints() { return m_vecAnchoringPointList; }

// Private methods.
private:
	void _Init();

// Private variables.
private:
	UINT m_iResID;					// Ressource image ID
	eProdPicType m_ePicType;		// Product image type
	ePosition m_ePosition;
	std::vector<AnchoringPointDef>	m_vecAnchoringPointList;
};

typedef CProdPic* (*pGetProdPic)(LPCTSTR, CProdPic::eProdPicType);
typedef bool (*pVerifyImgFound)(LPCTSTR);


/*
	CDynCircSch :  used to describe a Dynamic scheme background 

	UINT			m_iBckgndResID;				// Ressource ID used for the background
	vector <CAnchorPt> m_vAnchorPt;		// vector with all Anchoring points

	Usage:
		// new Dynamic Hydronic Scheme
		CDynCircSch DynCircuit(GIF_CIRCSHCEME_SP);
		DynCircuit.Reset(GIF_CIRCSHCEME_SP);
		// Define Anchoring points
		DynCircuit.AddAnchorPt(APT(_Pt(78,330),APT::Mid,_T('N'),APT::MV,APT::s2W_Straight,CAnchorPt::Pplus,1));
		DynCircuit.AddAnchorPt(APT(_Pt(439,330),APT::Mid,_T('S'),APT::DpC,APT::s2W_Straight,CAnchorPt::PNU,1));	


		Anchoring points are used to describe component position and orientation;
		Spline points are used to describe capilary connection between components, a spline connection can start
		from an anchoring point in respect of the component placement, rotation, ...  In this case don't forget to fit group value and spline Index
		
*/

class CDynCircSch
{
public:
	
	CDynCircSch( UINT uiResourceID );			// Constructor with the resource ID used.

	// Remove all anchoring points and put a new resource ID
	void Reset( UINT );

	UINT GetBackgroundResourceID() { return m_uiBckgndResID; }
	CDynCircSch& AddAnchorPt( CAnchorPt cAnchorPt );
	CAnchorPt* GetAnchoringPt( unsigned uiIndex );
	UINT GetNbrAnchoringPt();

protected:
	// variables
	UINT					m_uiBckgndResID;		// Resource ID used for the background.
	vector <CAnchorPt>		m_vAnchorPt;			// vector with all Anchoring points.
};

typedef CDynCircSch* (*pGetDynCircSch)(LPCTSTR);