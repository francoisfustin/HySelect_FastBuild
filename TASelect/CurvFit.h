//////////////////////////////////////////////////////////////////////////
//  CurvFit.h : header file
//	Version: 1.0							Date: 10/04/08
//////////////////////////////////////////////////////////////////////////

#ifndef CURVFIT_H
#define CURVFIT_H
#include "global.h"

//////////////////////////////////////////////////////////////////////////////
//
//							class CCurveFitter
//
//	This class is used to store (x,f(x)) points and to fit a curve.
//  Between two points X1 and X2, the curve is a cubic polynomial so that
//	the 0, first and second derivatives are continuous accross interface Xi.
//	Extremum are allowed to occur only on interfaces. So, you are sure that
//	the function f(x) is bordered by f(X1) and f(X2) when x is between X1 and X2. 
//
//////////////////////////////////////////////////////////////////////////////

struct PolyCoeff_struct			// For each segment,
{                           	// you have the 4 polynomial coefficients (a0..a3).
	double a0,a1,a2,a3;
};


class CCurveFitter
{
	
private:
	struct FdF_struct
	{
		double f;
		double df;
	};
	struct CurvePt_struct
	{
		double x;
		FdF_struct* y;
	};

	CurvePt_struct* m_PointArray;	// The array of points for the curve.
	int m_nFunc;					// The number of functions associated to the same abcissa	
	int m_nPoints;					// Number of points in m_PointArray[]. 
	bool m_bComputed; 				// Set to true if the curve is already computed, false otherwise.

public:
	// nFunc is the number of fonctions associated to the abcissa.
	CCurveFitter(int nFunc = 1);
	~CCurveFitter();

	void Copy( CCurveFitter *pclDestination );
	void SetFuncNumber( int nFunc ) { m_nFunc = nFunc; }

	// Add a CurvePt_struct (x,f(x)) to the list.
	// fx points to an array of doubles which contains the f values of the 
	// different functions. The size of this array should be at least m_nFunc.
	void AddPoint( double x, double* fx, bool fCheckIfExist = false );
	
	// Get the number of stored points.
	int GetSize() const { return m_nPoints; }

	// Get the number of fonctions associated to the abcissa.
	int GetFuncNumber( void ) { return m_nFunc; }

	// Get the specified point.
	const CurvePt_struct& GetPoint( int index ) { ASSERT( ( index >= 0 ) && ( index < m_nPoints ) ); return m_PointArray[index]; }
	
	// Get the X value for the specified point index.
	const double& GetPointX( int index ) { return GetPoint( index ).x; }
	
	// Get the Function value for the specified point index and function index.
	const double& GetPointF( int index, int nFunc = 0 ) { ASSERT( ( nFunc >= 0 ) && ( nFunc < m_nFunc ) ); return GetPoint( index ).y[nFunc].f; }
	
	// Get min/max values for x and f(x).
	// func is the index of the curve for which you want to retrieve 
	// the min/max value (0<func<=m_nFunc).
	double GetMinX() const;
	double GetMaxX() const;
	double GetMinFX(int func = 0) const;
	double GetMaxFX(int func = 0) const;
	
	// Get the function value for a given function func and a given x.
	double GetValue( double x, int func = 0 );

	// Get the root for a given function value.
	bool GetRoot( double fx, double *pdX, int func = 0 );
	
	// Purge the curve.
	void Purge();

private:
	// Find the segment to which x belongs.
	int GetSegmentIndex( double x, double *pdXLeft, double *pdXRight, bool *pfInterpolRequired );

	// Compute first derivatives for each segment.
	bool ComputedF();

	// Compute the polynomial coefficients for a given function and segment.
	void GetCoefficient( int func, int nSeg, PolyCoeff_struct *prPolyCoeff );
};

#endif // !defined(CURVFIT_H)
