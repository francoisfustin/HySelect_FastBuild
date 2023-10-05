#include "StdAfx.h"
#include "EnBitmapComponent.h"

const COLORREF WHITE = RGB(255,255,255);

/////////////////////////////////////////////////////////////
/// This class manage All Component.
/// It returns a CEnBitmapComponent or a HBITMAP.
/////////////////////////////////////////////////////////////
CEnBitmapCompMngr::~CEnBitmapCompMngr( void )
{
	ClearMapComponent();
}

void CEnBitmapCompMngr::AddComponent( CEnBitmapComponent *pComponent )
{
	if( NULL == pComponent )
	{
		return;
	}

	m_mapComponent.SetAt( (void *)pComponent->GetID(), pComponent );
}

CEnBitmapComponent *CEnBitmapCompMngr::GetComponent( ULONG ulKey )
{
	CEnBitmapComponent *pComponent = 0;

	BOOL bReturn = m_mapComponent.Lookup( (void *)ulKey, (void*&)pComponent );

	if( FALSE == bReturn )
	{
		return NULL;
	}

	return pComponent;
}

///////////////////////////////////////////////////////
/// Build the Component's name with Func and Struc
///////////////////////////////////////////////////////
// TODO AL: CEnBitmapCompMngr::BuildKey()
ULONG CEnBitmapCompMngr::BuildKey( CAnchorPt::eFunc func, CAnchorPt::eStruct struc )
{
	ULONG ulRet = func;
	return ulRet;
}

void CEnBitmapCompMngr::ClearMapComponent()
{
	CEnBitmapComponent *pComponent;
	void *rKey;

	if( 0 == m_mapComponent.GetCount() )
	{
		return;	
	}
	
	POSITION pos = m_mapComponent.GetStartPosition();
	
	while( !pos )
	{
		m_mapComponent.GetNextAssoc( pos, rKey, (void*&)pComponent );
		delete pComponent;
	}
}

///////////////////////////////////////////////////
/// Store all informations about a component
///
/// This class is abstract so YOU MUST INHERIT of this
/// and define the method GetImage()
///////////////////////////////////////////////////

CEnBitmapComponent::CEnBitmapComponent()
{
	Init();
}

void CEnBitmapComponent::Init( void )
{
	m_ulID = 0;
}

_Pt *CEnBitmapComponent::GetpPt( eConnectionPoints eConnPt )
{
	_Pt *pPt = NULL;

	for( auto &iter : m_vecAnchoringPointList )
	{
		if( iter.m_eType == eConnPt )
		{
			pPt = &iter.m_ptPos;
			break;
		}
	}

	return pPt;
}

double CEnBitmapComponent::GetAngle( eConnectionPoints eConnPt )
{
	double dAngle = 0.0;

	for( auto &iter : m_vecAnchoringPointList )
	{
		if( iter.m_eType == eConnPt )
		{
			dAngle = iter.m_dAngle;
			break;
		}
	}

	return dAngle;
}

int CEnBitmapComponent::GetCtrlPointDistance( eConnectionPoints eConnPt )
{
	int iCtrlPointDistance = 0;

	for( auto &iter : m_vecAnchoringPointList )
	{
		if( iter.m_eType == eConnPt )
		{
			iCtrlPointDistance = iter.m_iCtrlPointDistance;
			break;
		}
	}

	return iCtrlPointDistance;
}

void CEnBitmapComponent::SetPt( eConnectionPoints eConnPt, _Pt Point )
{
	for( auto &iter : m_vecAnchoringPointList )
	{
		if( iter.m_eType == eConnPt )
		{
			iter.m_ptPos = Point;
			break;
		}
	}
}

void CEnBitmapComponent::SetAngle( eConnectionPoints eConnPt, double dValue )
{
	for( auto &iter : m_vecAnchoringPointList )
	{
		if( iter.m_eType == eConnPt )
		{
			iter.m_dAngle = dValue;
			break;
		}
	}
}

_Pt CEnBitmapComponent::GetCoord( CAnchorPt::eConnectType eType )
{
	_Pt AnchoringPoint( 0, 0 );

	switch( eType )
	{
		case CAnchorPt::Beg:

			if( NULL != GetpPt( ecpIn ) )
			{
				AnchoringPoint = *GetpPt( ecpIn );
			}

			break;
		
		case CAnchorPt::End:

			if( NULL != GetpPt( ecpOut ) )
			{
				AnchoringPoint = *GetpPt( ecpOut );
			}

			break;

		//TODO Anchoring on the middle point is incorrect when the valve is horizontal
		default:
		{
			if( NULL != GetpPt( ecpIn ) && NULL != GetpPt( ecpBypass ) )
			{
				AnchoringPoint = _Pt( GetpPt( ecpIn )->first, GetpPt( ecpBypass )->second );
			}
			else
			{
				AnchoringPoint = _Pt( GetpPt( ecpIn )->first, GetpPt( ecpIn )->second + ( GetpPt( ecpOut )->second - GetpPt( ecpIn )->second ) / 2 );
			}
		}
	}

	return AnchoringPoint;
}

bool CEnBitmapComponent::CapilSecIsUsed( CAnchorPt::ePressSign PressSign )
{
	if( GetpPt( ecpCapilSec ) )			// Secondary _Pt exist, use PressSign to find correct connection point
	{
		switch( PressSign )
		{
			case CAnchorPt::ePressSign::Pminus:
				return true;
				break;

			case CAnchorPt::ePressSign::Pplus:
			default:
				return false;
				break;
		}
	}
	return false;
}

_Pt *CEnBitmapComponent::GetptCapil( CAnchorPt::ePressSign PressSign )
{
	if( true == CapilSecIsUsed( PressSign ) )
	{
		return GetpPt( ecpCapilSec );
	}

	return GetpPt( ecpCapilPrim );
 }

double CEnBitmapComponent::GetAngleCapil( CAnchorPt::ePressSign PressSign )
{
	if( true == CapilSecIsUsed( PressSign ) )
	{
		return GetAngle( ecpCapilSec );
	}

	return GetAngle( ecpCapilPrim );
}

bool CEnBitmapComponent::RotateAndTranslateAllPt( double dRad, CPoint Trans )
{
	RotateAndTranslatePt( GetpPt( ecpIn ), dRad, Trans );
	RotateAndTranslatePt( GetpPt( ecpBypass ), dRad, Trans );			// Middle point.
	RotateAndTranslatePt( GetpPt( ecpOut ), dRad, Trans );				// Exit point.
	RotateAndTranslatePt( GetpPt( ecpCapilPrim ), dRad, Trans );		// Primary capillary point.
	RotateAndTranslatePt( GetpPt( ecpCapilSec ), dRad, Trans );			// Secondary capillary point.

	double dDeg = dRad / M_PI * 180.0;

	SetAngle( ecpCapilPrim, GetAngle( ecpCapilPrim) + dDeg );			// Primary capillary point connection angle.
	SetAngle( ecpCapilSec, GetAngle( ecpCapilSec) + dDeg );				// Secondary capillary point connection angle.
	
	return true;
}

bool CEnBitmapComponent::RotateAndTranslatePt( _Pt *pXY, double dRad, CPoint clTrans )
{
	if( NULL == pXY )
	{
		return false;
	}
	
	// Rotation.
	if( dRad > 0.0 )
	{
		// Compute radius.
		double dradius = sqrt( pow( (double)pXY->first, 2 ) + pow( (double)pXY->second, 2 ) );
		
		// Compute original angle.
		double dPhi = atan2( (double)pXY->second, (double)pXY->first );
		
		// Add rotation.
		dPhi += dRad;
		pXY->first = (short)EB_round( dradius * cos( dPhi ) );
		pXY->second = (short)EB_round( dradius * sin( dPhi ) );
	}
	
	// Translation.
	pXY->first += (short)clTrans.x;
	pXY->second += (short)clTrans.y;
	return true;
}

void CEnBitmapComponent::Rotate( int iDeg )
{
	while( iDeg > 360 )
	{
		iDeg -= 360;
	}
	
	while( iDeg <- 360 )
	{
		iDeg +=360;
	}

	CRect RectBefRot = GetSizeImage();

	// Rotate component, connection points are not affected.
	// Use background color and color weighting.
	if( iDeg != 0 )
	{
		RotateImage( iDeg );
	}

	CRect RectAftRot = GetSizeImage();
	
	// Radian conversion.
	double dRad = iDeg / 180 * M_PI;

	_Pt RefPt1( RectBefRot.Width(), 0 );	// Take the right point as second point 
	_Pt RefPt2( RectBefRot.Width(), RectBefRot.Height() );
	_Pt RefPt3( 0, RectBefRot.Height() );

	RotateAndTranslatePt( &RefPt1, dRad );
	RotateAndTranslatePt( &RefPt2, dRad );
	RotateAndTranslatePt( &RefPt3, dRad );

	CPoint Transl;
	Transl.x = min( 0, RefPt1.first );
	Transl.x = min( Transl.x, RefPt2.first );
	Transl.x = min( Transl.x, RefPt3.first );

	Transl.y = min( 0, RefPt1.second );
	Transl.y = min( Transl.y, RefPt2.second );
	Transl.y = min( Transl.y, RefPt3.second );

	Transl.x = Transl.x * -1;
	Transl.y = Transl.y * -1;
	RotateAndTranslateAllPt( dRad, Transl );
}

bool CEnBitmapComponent::AddComplementaryProducts( CPoint ConnectPt, double dDeg, CEnBitmapComponent *pCmpt )
{
	if( ConnectPt.x < 0 || ConnectPt.y < 0 )
	{
		return false;
	}
	
	// Normalize rotation.
	while( dDeg > 360 )
	{
		dDeg -= 360;
	}

	while( dDeg <- 360 )
	{
		dDeg +=360;
	}

	double dRad = dDeg / 180 * M_PI;

	RGBX TransparentCmptColor( pCmpt->GetPixel( 0, 0 ) );

	// Apply rotation on component.
	pCmpt->Rotate( EB_round( dDeg ) );
	
	// Images size.
	CRect rectCmpt = pCmpt->GetSizeImage();
	CRect rectBody = GetSizeImage();

	// Work with new reference axes;
	// The reference point is the connection point.
	// Apply an offset to keep new composed object inside the first quadrant.
	CPoint Offset = CPoint( rectBody.Width() + rectCmpt.Width(), rectBody.Height() + rectCmpt.Height() );
	CPoint NewRefConnPt = Offset;

	// Compute (0,0) points for both axis (Body and Cmpt).
	CPoint NewRefBodyPt00 = NewRefConnPt - ConnectPt;
	CPoint NewRefCmptPt00;
	NewRefCmptPt00.x = NewRefConnPt.x - pCmpt->GetpPt( ecpOut )->first;
	NewRefCmptPt00.y = NewRefConnPt.y - pCmpt->GetpPt( ecpOut )->second;

	// Total size of the new bitmap.
	int iWidth = max( NewRefBodyPt00.x + rectBody.Width(), NewRefCmptPt00.x + rectCmpt.Width() ) - min( NewRefBodyPt00.x, NewRefCmptPt00.x );
	int iHeight = max( NewRefBodyPt00.y + rectBody.Height(), NewRefCmptPt00.y + rectCmpt.Height() ) - min( NewRefBodyPt00.y, NewRefCmptPt00.y );
	CSize NewSize = CSize( iWidth, iHeight );

	CPoint NewRefPt00 = CPoint( min( NewRefBodyPt00.x, NewRefCmptPt00.x ), min( NewRefBodyPt00.y, NewRefCmptPt00.y ) );
	NewRefBodyPt00 -= NewRefPt00;
	NewRefCmptPt00 -= NewRefPt00;
	
	// Enlarge base bitmap and translate the Body.
	ShiftImage( NewSize, CSize( NewRefBodyPt00.x, NewRefBodyPt00.y ) );

	// Paste component.
	PasteCEnBitmap( NewRefCmptPt00.x, NewRefCmptPt00.y, pCmpt, &TransparentCmptColor );

	// Correct connection points.
	// For the body, no rotation only translation.
	RotateAndTranslateAllPt( 0, NewRefBodyPt00 );
	
	// For component added
	// Only translation rotation was done before.
	pCmpt->RotateAndTranslateAllPt( 0, NewRefCmptPt00 );

	return true;
}

bool CEnBitmapComponent::AddComplementaryProducts( eConnectionPoints eProdLoc, CEnBitmapComponent *pCmpt )
{
	if( NULL == pCmpt )
	{
		return false;
	}
	
	CRect rectBody = GetSizeImage();

	// Find connection point.
	if( NULL == pCmpt->GetpPt( ecpOut ) || pCmpt->GetpPt( ecpOut )->first < 0 || pCmpt->GetpPt( ecpOut )->second < 0 )
	{
		return false;
	}

	switch( eProdLoc )
	{
		case ecpIn: 

			if( NULL == GetpPt( ecpIn ) || GetpPt( ecpIn )->first < 0 || GetpPt( ecpIn )->second < 0 )
			{
				return false;
			}

			// Body IN _Pt connected to Out _Pt of complementary product.
			// New body In _Pt will becomes the In _Pt of complementary product.

			if( true == AddComplementaryProducts( CPoint( GetpPt( ecpIn )->first, GetpPt( ecpIn )->second ), 0, pCmpt ) )
			{
				// Replace body input point by the new cmpt input point.
				_Pt ptIn( pCmpt->GetpPt( ecpIn )->first, pCmpt->GetpPt( ecpIn )->second );
				SetPt( ecpIn, ptIn );
			}		
			
			break;

		case ecpOut:

			if( NULL == GetpPt( ecpOut ) || GetpPt( ecpOut )->first < 0 || GetpPt( ecpOut )->second < 0 )
			{
				return false;
			}

			// Body Out _Pt connected to Out _Pt of complementary product.
			// New body Out _Pt will becomes the In _Pt of complementary product.

			if( true == AddComplementaryProducts( CPoint( GetpPt( ecpOut )->first, GetpPt( ecpOut )->second ), 180, pCmpt ) )
			{
				// Replace body input point by the new cmpt input point.
				_Pt ptOut( pCmpt->GetpPt( ecpIn )->second, pCmpt->GetpPt( ecpIn )->first );
				SetPt( ecpOut, ptOut );
			}		

			break;

		case ecpBypass:

			if( NULL == GetpPt( ecpCapilPrim ) || GetpPt( ecpCapilPrim )->first < 0 || GetpPt( ecpCapilPrim )->second < 0 )
			{
				return false;
			}

			// Body CapilPrim _Pt connected to Out _Pt of complementary product.
			// New body CapilPrim _Pt will becomes the In _Pt of complementary product.

			if( true == AddComplementaryProducts( CPoint( GetpPt( ecpCapilPrim )->first, GetpPt( ecpCapilPrim )->second ), GetAngle( ecpCapilPrim ), pCmpt ) )
			{
				// Replace body input point by the new cmpt input point.
				_Pt ptCapilPrim( pCmpt->GetpPt( ecpIn )->second, pCmpt->GetpPt( ecpIn)->first );
				SetPt( ecpCapilPrim, ptCapilPrim );
			}		

			break;
	
		case ecpActr: 
			break;

		case ecpCapilPrim:

			if( NULL == GetpPt( ecpCapilPrim ) || GetpPt( ecpCapilPrim )->first < 0 || GetpPt( ecpCapilPrim )->second < 0 )
			{
				return false;
			}

			// Body CapilPrim _Pt connected to Out _Pt of complementary product.
			// New body CapilPrim _Pt will becomes the In _Pt of complementary product.

			if( true == AddComplementaryProducts( CPoint( GetpPt( ecpCapilPrim )->first, GetpPt( ecpCapilPrim )->second ), GetAngle( ecpCapilPrim ), pCmpt ) )
			{
				// Replace body input point by the new cmpt input point.
				_Pt ptCapilPrim( pCmpt->GetpPt( ecpIn )->second, pCmpt->GetpPt( ecpIn )->first );
				SetPt( ecpCapilPrim, ptCapilPrim );
			}		
			
			break;

		case ecpCapilSec: 
			
			if( NULL == GetpPt( ecpCapilSec ) || GetpPt( ecpCapilSec )->first < 0 || GetpPt( ecpCapilSec )->second < 0 )
			{
				return false;
			}

			// Body CapilPrim _Pt connected to Out _Pt of complementary product.
			// New body CapilPrim _Pt will becomes the In _Pt of complementary product.

			if( true == AddComplementaryProducts( CPoint( GetpPt( ecpCapilSec )->first, GetpPt( ecpCapilSec )->second ), GetAngle( ecpCapilSec ), pCmpt ) )
			{
				// Replace body input point by the new cmpt input point.
				_Pt ptCapilSec( pCmpt->GetpPt( ecpIn )->second, pCmpt->GetpPt( ecpIn )->first );
				SetPt( ecpCapilSec, ptCapilSec );
			}		
			
			break;

		default:
			break;
	}
	
	return false;
}

CEnBitmapComponentFile::CEnBitmapComponentFile() : CEnBitmapComponent()
{
	m_uiResID = 0;
	m_hBitmap = 0;
}

CEnBitmapComponentFile::CEnBitmapComponentFile( CProdPic *pProdPic, ULONG ulID )
	:CEnBitmapComponent()
{
	m_uiResID = 0;
	m_hBitmap = 0;
	FillExistingComponentFile( pProdPic, ulID );
}

CEnBitmap *CEnBitmapComponentFile::GetImage()
{
	if( NULL == GetSafeHandle() )
	{
		if( m_uiResID > 0 )
		{
			LoadImage( m_uiResID, _T("GIF"), TASApp.GetHMProdPics() );
		}
		else
		{
			LoadImage( m_strFilePath );
		}

		if( NULL == GetSafeHandle() )
		{
			return 0;
		}
		
		COLORREF crBck = GetPixel( 0, 0 );
		SetBackgroundColor( crBck );
	}
	
	// Draw Image.
	CEnBitmap *pEnBitmap = new CEnBitmap();

	if( NULL == pEnBitmap )
	{
		return 0;
	}

	pEnBitmap->CopyImage( this );

	return pEnBitmap;
}

void CEnBitmapComponentFile::FillExistingComponentFile( CProdPic *pProdPic, ULONG ulID )
{
	if( NULL == pProdPic )
	{
		return;
	}
	
	Init();
	
	m_uiResID = pProdPic->GetResID();
	m_hBitmap = 0;
	m_vecAnchoringPointList = pProdPic->GetAllAnchoringPoints();
	m_ulID = ulID;

	if( m_uiResID > 0 )
	{
		LoadImage( m_uiResID, _T("GIF"), TASApp.GetHMProdPics() );
		COLORREF crBck = GetPixel( 0, 0);
		SetBackgroundColor( crBck );
	}
}
