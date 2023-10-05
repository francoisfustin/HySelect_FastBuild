#pragma once

#define _USE_MATH_DEFINES
#include "math.h"

#include "TASelect.h"
#include "EnBitmap.h"
#include "EnBitmapAnchorPt.h"
#include "HydronicPic.h"

class CEnBitmapComponent;

/////////////////////////////////////////////////////////////
/// This class manage all components.
/// It returns a CEnBitmapComponent or a HBITMAP.
/////////////////////////////////////////////////////////////
class  CEnBitmapCompMngr
{
public:
	CEnBitmapCompMngr( void ) {}
	virtual ~CEnBitmapCompMngr( void );

	void AddComponent( CEnBitmapComponent *pComponent );
	CEnBitmapComponent *GetComponent( ULONG ulKey );

	// Establish a link between function-structure and a Product Image.
	ULONG BuildKey( CAnchorPt::eFunc func, CAnchorPt::eStruct struc );

// Protected members.
protected:
	void ClearMapComponent();

// Private variables.
private:
	CMapPtrToPtr m_mapComponent;              // A hash table with all components.
};

///////////////////////////////////////////////////
/// Store all informations about a component
///
/// This class is abstract so YOU MUST INHERIT of this
/// and define the method GetImage()
///////////////////////////////////////////////////
class  CEnBitmapComponent : public CEnBitmap
{
public:
	CEnBitmapComponent();
	virtual ~CEnBitmapComponent( void ) {}

	void Init();	
	
	// Abstract method.
	virtual CEnBitmap *GetImage() = 0;
	
	_Pt GetCoord( CAnchorPt::eConnectType eType );

	// Adding Complementary component as connection, actuators, testpoints, ...
	bool AddComplementaryProducts( eConnectionPoints eProdLoc, CEnBitmapComponent *pComponent );
	bool AddComplementaryProducts( CPoint ConnectPt, double dDeg, CEnBitmapComponent *pCmpt );
	
	// Execute a rotation all members coordinates will be modified angle is expressed in degree, counterclockwise.
	void Rotate( int iDeg );
	
	// Compute rotation/translation for a point.
	bool RotateAndTranslatePt( _Pt *pXY, double dRad, CPoint clTrans = CPoint( 0, 0 ) );
	
	// Proceed for all members points.
	bool RotateAndTranslateAllPt( double dRad, CPoint Trans );
	
	// Properties.
	const ULONG	GetID() { return m_ulID; }
	const CString GetText() { return m_text; }

	_Pt *GetpPt( eConnectionPoints eConnPt );
	double GetAngle( eConnectionPoints eConnPt );
	int GetCtrlPointDistance( eConnectionPoints eConnPt );

	void SetPt( eConnectionPoints eConnPt, _Pt Point );
	void SetAngle( eConnectionPoints eConnPt, double dValue );

	bool CapilSecIsUsed( CAnchorPt::ePressSign PressSign );

	// For MV	Primary capillary point is UPSTREAM
	//			Secondary capillary point is DOWNSTREAM
	// For DPC	Primary capillary point is P+
	//			Secondary capillary point is P-
	// GetptCapil(CAnchorPt::ePressSign) return capilary point corresponding to the anchoring point pressure sign.
	_Pt *GetptCapil( CAnchorPt::ePressSign PressSign );

	// GetAngleCapil(CAnchorPt::ePressSign) return capilary connection angle corresponding to the anchoring point pressure sign
	double GetAngleCapil(CAnchorPt::ePressSign PressSign );

protected:
	ULONG m_ulID;					// Component ID
	CString m_text;                 // Comment
	std::vector<CProdPic::AnchoringPointDef> m_vecAnchoringPointList;
};

class  CEnBitmapComponentFile : public CEnBitmapComponent
{
public:
	CEnBitmapComponentFile();
	CEnBitmapComponentFile( CProdPic *pProdPic, ULONG ulID );
	virtual ~CEnBitmapComponentFile( void ) {}

public:
	virtual CEnBitmap *GetImage();
	UINT GetuiResID() { return m_uiResID; }
	HBITMAP GethBitmap() { return m_hBitmap; }

	void FillExistingComponentFile( CProdPic *pProdPic, ULONG ulID );

// Protected variables.
protected:
	CString m_strFilePath;
	UINT m_uiResID;
	HBITMAP m_hBitmap;                   // Image handle of the component
};
