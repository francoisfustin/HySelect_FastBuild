//
// CurvFit.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "global.h"
#include "CurvFit.h"

#ifndef TACBX
#include "HySelectException.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//
//					Class CCurveFitter
//
//////////////////////////////////////////////////////////////////////////////

CCurveFitter::CCurveFitter( int nFunc )
	: m_nFunc( nFunc )
{
	ASSERT( m_nFunc > 0 );
	m_nPoints = 0;
	m_bComputed = false;
	m_PointArray = NULL;
}

CCurveFitter::~CCurveFitter()
{
	Purge();
}	

void CCurveFitter::Copy( CCurveFitter *pclDestination )
{
	pclDestination->Purge();
	pclDestination->SetFuncNumber( m_nFunc );
	double *pardFx = new double[m_nFunc];

	for( int iLoop = 0; iLoop < m_nPoints; iLoop++ )
	{
		for( int i = 0; i < m_nFunc; i++ )
		{
			pardFx[i] = m_PointArray[iLoop].y[i].f;
		}

		pclDestination->AddPoint( m_PointArray[iLoop].x, pardFx );
	}
	
	delete[] pardFx;
}

// Add a point (x,f(x)) to the list. The x values are increasingly ordered.	
void CCurveFitter::AddPoint( double x, double *pFx, bool bCheckIfExist )
{
	HYSELECT_TRY
	{
		if( true == bCheckIfExist )
		{
			for( int iLoop = 0; iLoop < m_nPoints; iLoop++ )
			{
				if( m_PointArray[iLoop].x == x )
				{
					for( int i = 0; i < m_nFunc; i++ )
					{
						m_PointArray[iLoop].y[i].f = pFx[i];
					}

					return;
				}
			}
		}
	
		// Do allocation at the start of the method because if there is any problem, we can easily go out.
		FdF_struct *pNewFdF_Struct = (FdF_struct *)malloc( m_nFunc * sizeof( FdF_struct ) );

		if( NULL == pNewFdF_Struct )
		{
			HYSELECT_THROW( _T("Internal error: 'pNewFdF_Struct' allocation error.") );
		}

		CurvePt_struct *ptRealloc = (CurvePt_struct *)realloc(m_PointArray, ( m_nPoints + 1 ) * sizeof( CurvePt_struct ) );

		if( NULL == ptRealloc )
		{
			free( pNewFdF_Struct );
			HYSELECT_THROW( _T("Internal error: 'ptRealloc' reallocation error.") );
		}
		else
		{
			m_PointArray = ptRealloc;
		}

		m_nPoints++;

		// Insert new point.
		int i;
		for( i = m_nPoints - 2; ( i >= 0) && ( m_PointArray[i].x > x ); i--);
	
		int pos = i + 1;

		for( i = m_nPoints - 1; i > pos; i-- )
		{
			m_PointArray[i] = m_PointArray[i - 1];
		}
	
		m_PointArray[pos].x = x;
		m_PointArray[pos].y = pNewFdF_Struct;

		for( i = 0; i < m_nFunc; i++ )
		{
			m_PointArray[pos].y[i].f = pFx[i];
		}

		// Validity check.
#if defined( _DEBUG )
		for( i = 1; i < m_nPoints; i++ )
		{
			if( m_PointArray[i].x < m_PointArray[i - 1].x )
			{
				HYSELECT_THROW( _T("Invalid point (%i) in the curve: %i value can't be lower than the previous value %i."), i, m_PointArray[i].x, m_PointArray[i - 1].x );
			}
		}
#endif
	
		// Reset compute 
		m_bComputed = false;
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CCurveFitter::AddPoint' method.") )
}		

// Return	min X value
//			max X value
//			min f(x) value
//			max f(x) value
double CCurveFitter::GetMinX() const
{
	if( m_nPoints < 1 )
		return 0.0;
	return m_PointArray[0].x;
}
 
double CCurveFitter::GetMaxX() const
{
	if( m_nPoints < 1 )
		return 0.0;
	return m_PointArray[m_nPoints - 1].x;
}

double CCurveFitter::GetMinFX( int func ) const
{
	if( func < 0 || func >= m_nFunc )
	{
		ASSERT( 0 );
		return 0.0;
	}
	
	if( m_nPoints < 1 )
		return 0.0;

	double mini = DBL_MAX;
	for( int i = 0; i < m_nPoints; i++ )
		mini = min( mini, m_PointArray[i].y[func].f );
	return mini;
}
	
double CCurveFitter::GetMaxFX( int func ) const
{
	if( func < 0 || func >= m_nFunc )
	{
		ASSERT( 0 );
		return 0.0;
	}

	if( m_nPoints < 1 )
		return 0.0;

	double maxi = -DBL_MAX;
	for( int i = 0; i < m_nPoints; i++ )
		maxi = max( maxi, m_PointArray[i].y[func].f );
	return maxi;
}	

// Compute the value of the function. If the x value is out of the stored x range,
// then the function is assumed to be the nearest available value.
// Compute the curve first derivatives if they are not yet computed.
double CCurveFitter::GetValue( double x, int func )
{
	if( func < 0 || func >= m_nFunc )
	{
		ASSERT( 0 );
		return 0.0;
	}

	// If there is only one point, no need for any calculation.
	if( 1 == m_nPoints )
	{
		return m_PointArray[0].y[func].f;
	}

	// If can't compute the curve first derivatives...
	if( false == ComputedF() )
	{
		return 0.0;
	}
	
	double xLeft=0.0, xRight=0.0;
	bool bInterpolRequired=false;
	int iLeft = GetSegmentIndex( x, &xLeft, &xRight, &bInterpolRequired );

	// Check range validity (+ coincidence with 1st or last point of the range).
	if( -1 == iLeft )
	{
		return m_PointArray[0].y[func].f;
	}
	
	if( iLeft == m_nPoints )
	{
		return m_PointArray[m_nPoints - 1].y[func].f;
	}
		
	if( false == bInterpolRequired )
	{
		return m_PointArray[iLeft].y[func].f;
	}

	// Compute z = (x-Xi-1)/(Xi-Xi-1).
	double z = ( x - xLeft ) / ( xRight - xLeft );
	ASSERT( (z >= 0 ) && ( z <= 1 ) );
	
	// Compute function value.
	PolyCoeff_struct c;
	GetCoefficient( func, iLeft + 1, &c );
	double d = c.a0 + z * ( c.a1 + z * ( c.a2 + z * c.a3 ) );

	return d;
}

// Calculate the x value for which the function is equal to fx.
// Return true if successful, false otherwise.
// Compute the curve first derivatives, if they are not yet computed.
// Cannot give several x Values. Gives the first one from the left
// (negative x direction).
bool CCurveFitter::GetRoot( double fx, double *pdX, int func )
{
	if( func < 0 || func >= m_nFunc || NULL == pdX )
	{
		ASSERT( 0 );
		return false;
	}

	// Precision to be obtained in iterative processes.
	const double Precision = 1.0e-6;

	// If there is only one point, no need for any calculation.
	if( 1 == m_nPoints )
	{
		*pdX = m_PointArray[0].x;
		return true;
	}
	
	// If can't compute the curve first derivatives...
	if( false == ComputedF() )
		return false;

	//-----------------------------------------------------------------------
	// First locate the segment Xi-1..Xi in which the function equals fx.
	//-----------------------------------------------------------------------
	double fLeft;
	double fRight = m_PointArray[0].y[func].f - fx;
	int iCurv = 0;
	do
	{   
		iCurv++;
		fLeft = fRight;
		fRight = m_PointArray[iCurv].y[func].f - fx;
	}
	while( ( ( fLeft * fRight ) > 0 ) && ( iCurv < ( m_nPoints - 1 ) ) );

	if( ( fLeft * fRight ) > 0 )
		return false;

	// If we fall exactly on 1 of the stored points...
	if( 0.0 == fLeft )
	{
		*pdX = m_PointArray[iCurv - 1].x;
		return true;
	}

	//-----------------------------------------------------------------------
	// Find the z value ( z=(x-Xi-1)/(Xi-Xi-1) ).
	// Improved dichotomic algorithm is used.
	//-----------------------------------------------------------------------
	PolyCoeff_struct c;
	GetCoefficient( func, iCurv, &c );
	double fMid;
	double zLeft = 0.0;
	double zRight = 1.001;
	double zMid = zRight;
	double zOld;
	int i = (int)( -3.0 * log( Precision ) );
	do
	{
		zOld = zMid;
		zMid = zLeft + ( zRight - zLeft ) / ( 1.0 + fabs( fRight / fLeft ) );
		fMid = c.a0 + zMid * ( c.a1 + zMid * ( c.a2 + zMid * c.a3 ) ) - fx;
		if( 0.0 == fMid )
		{
			zOld = zMid;
		}
		else if( ( fMid * fLeft ) > 0 )
		{
			zLeft = zMid;
			fLeft = fMid;
		}
		else
		{
			zRight = zMid;
			fRight = fMid;
		}
	}
	while( ( fabs( zMid - zOld ) > Precision ) && ( i-- > 0 ) );

	// If no convergence...
	if( i < 0 )
	{
		TRACE( _T("No convergence in GetRoot()") );
		return false;
	}
	
	//-----------------------------------------------------------------------
	// return the x Value.
	//-----------------------------------------------------------------------
	*pdX = m_PointArray[iCurv - 1].x + zMid * ( m_PointArray[iCurv].x - m_PointArray[iCurv - 1].x );
	return true;
}

// Purge the curve.  
void CCurveFitter::Purge()
{
	for( int i = 0; i < m_nPoints; i++ )
		free( m_PointArray[i].y );
	free( m_PointArray );
	m_PointArray = 0;
	m_nPoints = 0;
	m_bComputed = false;
}

/////////////////////////////////////////////////////////////////////////

// Find the segment to which x belongs.
// Return the index of the origin of the segment, which is also the 
// 0-based index of the segment.
// Return -1 if x is out of the x-range on the left OR if x is just on the 1st point of the range; 
// return nPoints if x is out of the x-range on the right OR if x is just on the last point of the range 
int CCurveFitter::GetSegmentIndex( double x, double *pdXLeft, double *pdXRight, bool *pfInterpolRequired )
{
	if( NULL == pdXLeft || NULL == pdXRight || NULL == pfInterpolRequired )
		return -1;
	
	*pfInterpolRequired = true;
	int iLeft = 0;
	int iRight = m_nPoints - 1;
	int iMid;
	*pdXLeft = m_PointArray[iLeft].x;
	*pdXRight = m_PointArray[iRight].x;
	// Check range validity
	if( x <= *pdXLeft )
	{
		*pdXRight = m_PointArray[iLeft + 1].x;
		return -1;
	}
	
	if( x >= *pdXRight )
	{
		*pdXLeft = m_PointArray[iRight - 1].x;
		return m_nPoints;
	}

	//-----------------------------------------------------------------------
	// Look for the stored points in between the x value is.
	// Dichotomic algorithm is used. iLeft will contain the 0-based segment index.
	//-----------------------------------------------------------------------
	double xMid;
	do
	{
		iMid = ( iLeft + iRight ) / 2;
		xMid = m_PointArray[iMid].x;
		if( xMid > x )
		{
			iRight = iMid;
			*pdXRight = xMid;
		}
		else if( xMid < x )
		{
			iLeft = iMid;
			*pdXLeft = xMid;
		}
		else		// We fall exactly on 1 of the stored points
		{
			iLeft = iMid;
			iRight = iMid;
			*pdXLeft = xMid;
			*pdXRight = xMid;
			*pfInterpolRequired = false;
		}
	}
	while( ( iRight - iLeft ) > 1 );
	ASSERT( x >= *pdXLeft && x <= *pdXRight );
	return iLeft;
}

// Compute first derivatives for each segment.
// In a segment (Xi-1..Xi), the function is a cubic polynom.
// 0 and first derivations are conservative through interfaces Xi.
// No extremum is allowed to occur elsewhere than at a discrete point Xi.
// Return true if successful, false otherwise (You have less than 2 points).
bool CCurveFitter::ComputedF()
{
	int i,ifct;
	double Li, Lip1, dfi, dfip1;
	
	if( m_bComputed )
		return true;
	
	if( m_nPoints < 2 )
		return false;
	
	// If you only have 2 points, compute a linear function...
	if( 2 == m_nPoints )
	{
		for( ifct = 0; ifct < m_nFunc; ifct++ )
		{
			m_PointArray[0].y[ifct].df = ( m_PointArray[1].y[ifct].f - m_PointArray[0].y[ifct].f ) / ( m_PointArray[1].x - m_PointArray[0].x );
			m_PointArray[1].y[ifct].df = m_PointArray[0].y[ifct].df;
		}
		m_bComputed = true;
		return true;
	}

	//------------------------------------------------------------------
	// Compute the slopes for points from i=1 to i=m_nPoints-2
	// Special treatment when the point is a maximum or minimum :
	// the slope is set to 0.0
	//------------------------------------------------------------------
	for( ifct = 0; ifct < m_nFunc; ifct++ )
	{
		Li = m_PointArray[m_nPoints - 1].x - m_PointArray[m_nPoints - 2].x;
		dfi = ( m_PointArray[m_nPoints - 1].y[ifct].f - m_PointArray[m_nPoints - 2].y[ifct].f ) / Li;
		for( i = m_nPoints - 2; i; i-- )
		{
			Lip1 = Li;
			dfip1 = dfi;
			Li = m_PointArray[i].x - m_PointArray[i-1].x;
			dfi = ( m_PointArray[i].y[ifct].f - m_PointArray[i - 1].y[ifct].f ) / Li;
			m_PointArray[i].y[ifct].df = ( dfi * dfip1 > 0.0 ) ? ( Lip1 * dfi + Li * dfip1 ) / ( Lip1 + Li ) : 0.0;
		}
	}
	
	//-----------------------------------------------------------------------
	// Compute the slopes for i=0 and i=m_nPoints-1 (first and last points)  	
	// Special treatment when the point is a maximum or minimum :
	// the slope is set to 0.0
	//-----------------------------------------------------------------------
	for( ifct = 0; ifct < m_nFunc; ifct++ )
	{
		if( m_PointArray[1].y[ifct].f == m_PointArray[0].y[ifct].f )
		{
			m_PointArray[0].y[ifct].df = 0.0;
		}
		else
		{	
			// HYS-2013: Correct the index.
			Li = m_PointArray[1].x - m_PointArray[0].x;
			Lip1 = m_PointArray[2].x - m_PointArray[1].x;
			m_PointArray[0].y[ifct].df = ( 1.0 / Li + 1.0 / Lip1 ) * ( m_PointArray[1].y[ifct].f - m_PointArray[0].y[ifct].f )
								  		- Li / ( Lip1 * ( Li + Lip1 ) ) * ( m_PointArray[2].y[ifct].f - m_PointArray[0].y[ifct].f );
		}

		if( m_PointArray[m_nPoints - 1].y[ifct].f == m_PointArray[m_nPoints - 2].y[ifct].f )
		{
			m_PointArray[m_nPoints - 1].y[ifct].df = 0.0;
		}
		else
		{
			Li = m_PointArray[m_nPoints - 2].x - m_PointArray[m_nPoints - 3].x;
			Lip1 = m_PointArray[m_nPoints - 1].x - m_PointArray[m_nPoints - 2].x;
			m_PointArray[m_nPoints - 1].y[ifct].df = ( 1.0 / Li + 1.0 / Lip1 ) * ( m_PointArray[m_nPoints - 1].y[ifct].f - m_PointArray[m_nPoints - 2].y[ifct].f )
													- Lip1 / ( Li * ( Li + Lip1 ) ) * ( m_PointArray[m_nPoints - 1].y[ifct].f - m_PointArray[m_nPoints - 3].y[ifct].f );
		}
	}

	// Set compute flag		
	m_bComputed = true;
	return true;
}

// Compute the polynomial coefficients for a given segment.
void CCurveFitter::GetCoefficient( int func, int nSeg, PolyCoeff_struct *prPolyCoeff )
{
	if( func < 0 || func >= m_nFunc || nSeg <= 0 || nSeg >= m_nPoints || NULL == prPolyCoeff )
	{
		ASSERT( 0 );
		return;
	}

	double l = m_PointArray[nSeg].x - m_PointArray[nSeg - 1].x;
	double f0 = m_PointArray[nSeg - 1].y[func].f;
	double f1 = m_PointArray[nSeg].y[func].f;
	double df0 = m_PointArray[nSeg - 1].y[func].df;
	double df1 = m_PointArray[nSeg].y[func].df;
	
	prPolyCoeff->a0 = f0;
	prPolyCoeff->a1 = l * df0;
	prPolyCoeff->a2 = 3.0 * ( f1 - f0 ) - l * ( 2.0 * df0 + df1 );
	prPolyCoeff->a3 = 2.0 * ( f0 - f1 ) + l * ( df0 + df1 );

	//------------------------------------------------------------------
	// avoid non monotonic curve.
	// Find a kind of z point where the first derivate is null
	// and check these points are not in the segment.
	//------------------------------------------------------------------
 	double x1, x2;
	if( prPolyCoeff->a3 == 0.0 )
	{
		if( prPolyCoeff->a2 == 0.0 )
			x1 = x2 = -1.0;
		else
		{
			x1 = -1.5 * prPolyCoeff->a1 / prPolyCoeff->a2;
			x2 = -1;
		}
	}
	else
	{
		double b = prPolyCoeff->a2 / prPolyCoeff->a3;
		double delta = b * b - 3.0 * prPolyCoeff->a1 / prPolyCoeff->a3;
		if( delta >= 0.0 )
		{
			delta = sqrt( delta );
			x1 = -b + delta;
			x2 = -b - delta;
		}
		else
			x1 = x2 = -1.0;
	}
	if( ( ( x1 > 0.0 ) && ( x1 < 3.0 ) ) || ( ( x2 > 0.0 ) && ( x2 < 3.0 ) ) )
	{
		prPolyCoeff->a0 = m_PointArray[nSeg - 1].y[func].f;
		prPolyCoeff->a1 = m_PointArray[nSeg].y[func].f - prPolyCoeff->a0;
		prPolyCoeff->a2 = prPolyCoeff->a3 = 0.0;
	}
}
