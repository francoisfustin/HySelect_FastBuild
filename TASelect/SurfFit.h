//////////////////////////////////////////////////////////////////////////
//  SurfFit.h : header file
//	Version: 1.0							Date: 10/04/08
//////////////////////////////////////////////////////////////////////////

#ifndef SURFFIT_H__INCLUDED_
#define SURFFIT_H__INCLUDED_
#include <vector>
#include <map>
#include "global.h"

////////////////////////////////////////////////////////////////////////////
//
//							class CSurfaceFitter
//
//	This class stores (x,y,y_min(x),y_max(x),f_k(x,y)) points and fits a 
//	3D surface.
//
//	Functions y_min(x) and y_max(x) limit the extent of the (x,y) cartesian 
//	grid on which the 3D surface is built. Function y_min(x) and y_max(x) 
//	are fit each as a curve: between two points X1 and X2, the curve is a 
//	cubic polynomial so that the 0, first and second derivatives are 
//	continuous accross interface Xi. Extrema are allowed to occur only on 
//	interfaces. So, you are sure that function y_min(x) (the same for 
//	y_max(x)) is bordered by y_min(X1) and y_min(X2) when x is between X1 
//	and X2. 
//
//	An important restriction on y_min(x) and y_max(x) is that they cannot
//	intersect anywhere a same cartesian cell of the (x,y) grid. This needs
//	to be asserted during data reading (normally in a CDB_... class).
//
//	Functions f_k(x,y) are fit as 3D surfaces built on the cartesian grid 
//  made of the (x,y) points. This cartesian grid is limited by functions 
//	y_min(x) and y_max(x), so that any interpolation attempted beyond the 
//	data limits is prevented. Interpolation is achieved by bilinear 
//	interpolation on the grid square in which a given point (x,y) falls.
//	When a grid square is truncated by one of the functions y_min(x) or 
//	y_max(x), the interpolation takes place in the resulting triangle or 
//	quadrilateral.
//
////////////////////////////////////////////////////////////////////////////

enum SurfValReturn_enum
{
	SV_OUTOFXYRANGE,	// (x,y) arguments are out of range, no value
	SV_OUTOFXRANGE,		// x is out of range, return value for the first or last x 
	SV_OUTOFYRANGE,		// y is out of range, return value for the first or last y
	SV_BELOWYMIN,		// y < y_min, return value for y=y_min
	SV_ABOVEYMAX,		// y > y_max, return value for y=y_max
	SV_OK,				// return value from bilinear interpolation
	SV_UNKNOWERROR,
	SV_LAST
};

struct SurfPt_struct
{
	double* f;
	SurfPt_struct()
	{
		Reset();
	}
	void Reset()
	{
		f = NULL;
	}
};

class CSurfaceFitter
{
private:

	double *m_pX,*m_pY;				// Vectors of x and y coordinates.
	CCurveFitter* m_pLimitYCurve;	// Limit y_min(x) & y_max(x) functions.
	SurfPt_struct** m_pPointArray;	// The array of points for the surface(s).
	const int m_nX,m_nY;			// Number of points in each direction.
	const int m_nFunc;				// The number of functions associated to the same coordinates.
	const bool m_bYmin,m_bYmax;		// Flags for the existence of limit functions y_min & y_max.

public:
	// Constructor/destructor
	// nFunc is the number of fonctions associated to the cartesian grid.
	CSurfaceFitter(int nX, int nY, int nFunc = 1, bool bYmin = false, bool bYmax = false);
	~CSurfaceFitter();

public:
	// Set the value of the ith component of the x vector
	void SetX(int i, double x);
	// Set the value of the ith component of the y vector
	void SetY(int i, double y);
	// Add a y_min and/or y_max value to m_LimitYCurve. yLim points to a vector of 
	// 1 or 2 components depending on the existence of y_min and y_max.
	void AddYminYmax(double x, double *yLim);
	// Add a SurfPt_struct (fk(x,y)) to the array at position (i,j).
	// fxy points to a vector which contains the values of the different functions.
	// The size of this array should be equal to m_nFunc.
	void AddPoint(int i, int j, double* fxy);
	
	// Get the number of stored points in x direction.
	int GetSizeX() const {return m_nX;};
	// Get the number of stored points in y direction.
	int GetSizeY() const {return m_nY;};
	// Get the Ylim value for the specified x index.
	double GetYlimValue(double x,int func=0) {return m_pLimitYCurve->GetValue(x,func);}; 
	// Get the Ymin value for the specified x index.
	const double GetYmin(int index) {return m_pLimitYCurve->GetPointF(index,0);}; 
	// Get the Ymax value for the specified x index.
	const double GetYmax(int index) {return m_pLimitYCurve->GetPointF(index,1);}; 
	// Get the X value for the specified point index.
	const double GetPointX(int index) {ASSERT(index>=0 && index<m_nX); return m_pX[index];}; 
	// Get the Y value for the specified point index.
	const double GetPointY(int index) {ASSERT(index>=0 && index<m_nY); return m_pY[index];}; 
	// Get the Function value for the specified point indices and function index.
	const double GetPointF(int xIndex, int yIndex, int func=0) {ASSERT(func>=0 && func<m_nFunc); return m_pPointArray[xIndex][yIndex].f[func];}; 
	// Get the X segment index in which x falls 
	int GetXSegment(double x) const {double z; return GetSegment(m_pX,m_nX,x,&z);};
	// Get the Y segment index in which y falls 
	int GetYSegment(double y) const {double z; return GetSegment(m_pY,m_nY,y,&z);};
	
	// Get the function values for a given coordinate (x,y).
	// Returns a SurfValReturn_enum 
	SurfValReturn_enum GetValue(double x, double y, SurfPt_struct* value);
	
	// Get minimum X value
	const double GetXmin(){return GetPointX(0);};
	// Get maximum X/Y value
	const double GetXmax(){return GetPointX(max(0,m_nX-1));};
	const double GetYmax(){return GetPointY(max(0,m_nY-1));};

private:
	// Get the segment index of vector vec in which x falls 
	int GetSegment(const double* vec, int vecSize, double x, double* z) const;
};

#define NUMBER_OF_FUNCTION	4			// Rho, Nu, Cp, Vapor pressure
enum eFunctType 
{
	efnRho = 0,
	efnNu = 1,
	efnCp = 2,
	efnVp = 3
};

class CAdditFitter
{
private:
	typedef std::map<double/*temperature*/, std::vector<double>/*Functions*/ > typeMapTempFn;
	// One CConcenPt by concentration
	class CCurvTemp
	{
	public:
		CCurvTemp();
		~CCurvTemp();
		CCurveFitter *CreateTempCurve();
		SurfValReturn_enum GetPointOnTempCurve(double &dTemp, std::vector<double> &vecFn);
		void AddPoint(double dTemp, double dRho, double dNu, double dCp, double dVaporPressure);
		void AddPoint( double dTemp, std::vector<double> *pVecFn );
		double GetTmin();
		double GetTmax();
		int GetFunctionNumber();

		void Write( OUTSTREAM outf );
		bool Read ( INPSTREAM inpf );
	private:
		CCurveFitter *m_pCurveFnFctTemp;
		typeMapTempFn m_mapTemp;
		std::vector<double> m_vecFn;
	};
	typedef std::map<double/*Value of concentration*/, CCurvTemp* /*pointer on curve & array of points (temp, fn)*/> typeMapConcentration;
	typeMapConcentration m_mapConcentration;
	CCurveFitter *pCurvTminTmax;
	// Variables globaly added to accelerate process 
	std::vector<double> m_vecFn;
	std::vector<double> m_vecFnBlw;
	std::vector<double> m_vecFnAbv;

public:
	CAdditFitter();
	~CAdditFitter();

	void AddPoint (	double dConcentration, double dTemp, double dRho, double dNu, double dCp, double dVaporPressure);
	
	// Create curves with Tmin and Tmax, should be called at the end of completion process (all concentration curves have been inputed).
	bool CreateCurveTminTmax();
	
	// Return number of concentration curves.
	int GetNbrOfConcentrationCurves();
	
	// Return Concentration, Tmin and Tamx associated at point PointX (should be included [0, GetNbrOfConcentrationCurves()[ )
	bool GetPointX( UINT iPointX, double &dConcentration, double *pdTmin = NULL, double *pdTmax = NULL );
	SurfValReturn_enum GetTminTmax( double dConcentration, double *pdTmin = NULL, double *pdTmax = NULL );
	SurfValReturn_enum GetPoint( double dConcentration, double dTemp, double &dRho, double &dNu, double &dCp, double &dVp );

	int GetFunctionNumber();

	void Write( OUTSTREAM outf );
	bool Read ( INPSTREAM inpf );
};
#endif // !defined(SURFFIT_H__INCLUDED_)
