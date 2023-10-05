//
// SurfFit.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "CurvFit.h"
#include "SurfFit.h"
#include "database.h"

//////////////////////////////////////////////////////////////////////////////
//
//					Class CSurfaceFitter
//
//////////////////////////////////////////////////////////////////////////////

// Constructor and destructor.

CSurfaceFitter::CSurfaceFitter(int nX, int nY, int nFunc, bool bYmin, bool bYmax)
	:m_nX(nX), m_nY(nY), m_nFunc(nFunc), m_bYmin(bYmin), m_bYmax(bYmax)
{
	ASSERT(m_nX>1 && m_nY>0 && m_nFunc>0);
	m_pLimitYCurve = NULL;
	m_pPointArray = NULL;

	// Allocate the X and Y vectors
	m_pX = new double [m_nX];
	m_pY = new double [m_nY];

	// Allocate CCurveFitter object for limit y curves
	if (bYmin && bYmax)
		m_pLimitYCurve = new CCurveFitter (2);
	else if (bYmin || bYmax)
		m_pLimitYCurve = new CCurveFitter (1);

	//Allocate the point array
	m_pPointArray = new SurfPt_struct* [m_nX];
	for (int i=0; i<m_nX; i++)
	{
		m_pPointArray[i] = new SurfPt_struct [m_nY];
		for (int j=0; j<m_nY; j++)
			m_pPointArray[i][j].f = NULL;
	};

	return;
}

CSurfaceFitter::~CSurfaceFitter()
{
	// Free the dynamically allocated memory
	delete[] m_pX;
	delete[] m_pY;
	delete m_pLimitYCurve;
	for (int i=0; i<m_nX ;i++)
	{
		for (int j=0; j<m_nY ;j++)
			if (m_pPointArray[i][j].f)
				delete[] m_pPointArray[i][j].f;
		delete[] m_pPointArray[i];
	};
	delete[] m_pPointArray;
}

///////////////////////////////////////////////////////////////////////////////
// Set the value of the ith component of the x vector
void CSurfaceFitter::SetX(int i, double x)
{
	ASSERT(i>=0 && i<m_nX);
	m_pX[i] = x;
}

///////////////////////////////////////////////////////////////////////////////
// Set the value of the ith component of the y vector
void CSurfaceFitter::SetY(int i, double y)
{
	ASSERT(i>=0 && i<m_nY);
	m_pY[i] = y;
}

///////////////////////////////////////////////////////////////////////////////
// Add a y_min and/or y_max value to m_LimitYCurve. yLim points to a vector of 
// 1 or 2 components depending on the existence of y_min and y_max. 
// The x values are increasingly ordered.	
void CSurfaceFitter::AddYminYmax(double x, double *yLim)
{
	m_pLimitYCurve->AddPoint(x,yLim);
}

///////////////////////////////////////////////////////////////////////////////
// Add a SurfPt_struct (fk(x,y)) to the array at position (i,j).
// fxy points to a vector which contains the values of the different functions.
// The size of this array should be equal to m_nFunc.
void CSurfaceFitter::AddPoint(int i, int j, double* fxy)
{
	ASSERT(i>=0 && i<m_nX && j>=0 && j<m_nY);
	if (!m_pPointArray[i][j].f)
		m_pPointArray[i][j].f = new double [m_nFunc];
	for (int k=0; k<m_nFunc; k++)
		m_pPointArray[i][j].f[k] = fxy[k];
}

/////////////////////////////////////////////////////////////////////////////////
// Compute the value of each function. If the x or y value is out of the stored 
// range, the function is assumed to be the nearest available value. If both x
// y values are out of range the function aborts.
SurfValReturn_enum CSurfaceFitter::GetValue(double x, double y, SurfPt_struct* value)
{	
	int i,j;

	// Evaluate Ymin(x) and Ymax(x)
	double Ymin=0.0;
	double Ymax=0.0;
	if (m_bYmin)
		Ymin = m_pLimitYCurve->GetValue(x,0);
	if (m_bYmax)
		Ymax = m_pLimitYCurve->GetValue(x,1);

	// Check the x & y values and establish xyStatus
	SurfValReturn_enum xyStatus;
	if (y > m_pY[m_nY-1] || y < m_pY[0])
		xyStatus = SV_OUTOFYRANGE;
	else if (m_bYmax && y>Ymax)
		xyStatus = SV_ABOVEYMAX;
	else if (m_bYmin && y<Ymin)
		xyStatus = SV_BELOWYMIN;
	else
		xyStatus = SV_OK;

	if (x > m_pX[m_nX-1] || x < m_pX[0])
	{
		if (xyStatus == SV_OK)
			xyStatus = SV_OUTOFXRANGE;
		else
		{
			for (i=0; i<m_nFunc; i++)
				value->f[i] = 0.0;
			return SV_OUTOFXYRANGE;
		};
	};

	// Find the x segment
	double Zx;
	int xSegment = GetSegment(m_pX,m_nX,x,&Zx);
	// Find the y segment, usefull even if SV_ABOVEYMAX or SV_BELOWYMIN
	double Zy;
	int ySegment = GetSegment(m_pY,m_nY,y,&Zy);

	// Deal with SV_OUTOFXRANGE and SV_OUTOFYRANGE
	if (xyStatus == SV_OUTOFXRANGE)
	{
		int n = (xSegment == -1) ? 0 : xSegment-1;
		for (i=0; i<m_nFunc; i++)
			value->f[i] = m_pPointArray[n][ySegment].f[i]
				+Zy*(m_pPointArray[n][ySegment+1].f[i]-m_pPointArray[n][ySegment].f[i]);
		return xyStatus;
	};
	if (xyStatus == SV_OUTOFYRANGE)
	{
		int n = (ySegment == -1) ? 0 : ySegment-1;
		for (i=0; i<m_nFunc; i++)
			value->f[i] = m_pPointArray[xSegment][n].f[i]
				+Zx*(m_pPointArray[xSegment+1][n].f[i]-m_pPointArray[xSegment][n].f[i]);
		return xyStatus;
	};

	// Deal with SV_BELOWYMIN/SV_ABOVEYMAX 
	double yc = y;
	double xl = m_pLimitYCurve->GetPointX(xSegment);
	double xr = m_pLimitYCurve->GetPointX(xSegment+1);
	double yl,yr,ym;
	// Correct y coordinate if SV_BELOWYMIN
	if (xyStatus == SV_BELOWYMIN)
	{
		// Evaluate y_min for the x of the segment extremities
		yl = m_pLimitYCurve->GetPointF(xSegment,0);
		yr = m_pLimitYCurve->GetPointF(xSegment+1,0);

		// Get the linearly interpolated value of y_min corresp. to x
		ym = yl + (x-xl)*(yr-yl)/(xr-xl);
		// Take the largest of Ymin and ym as yc for the evaluation
		yc = max(Ymin,ym);

		// Re-evaluate ySegment
		ySegment = GetSegment(m_pY,m_nY,yc,&Zy);
	};
	// Correct y coordinate if SV_ABOVEYMAX
	if (xyStatus == SV_ABOVEYMAX)
	{
		// Evaluate y_max for the x of the segment extremities
		yl = m_pLimitYCurve->GetPointF(xSegment,1);
		yr = m_pLimitYCurve->GetPointF(xSegment+1,1);

		// Get the linearly interpolated value of y_max corresp. to x
		ym = yl + (x-xl)*(yr-yl)/(xr-xl);
		// Take the smallest of Ymax and ym as yc for the evaluation
		yc = min(Ymax,ym);

		// Re-evaluate ySegment
		ySegment = GetSegment(m_pY,m_nY,yc,&Zy);
	};

	// Compose the base cell for interpolation
	//		3----2
	//		|    |
	//		0----1
	int cellType = 4; int skip = -1;
	double xCell[4],yCell[4];
	SurfPt_struct *fCell = new SurfPt_struct [4];
	for (i=0; i<4; i++)
		fCell[i].f = new double [m_nFunc];
	xCell[0] = xCell[3] = m_pX[xSegment];
	xCell[1] = xCell[2] = m_pX[xSegment+1];
	yCell[0] = yCell[1] = m_pY[ySegment];
	yCell[2] = yCell[3] = m_pY[ySegment+1];
	for (i=0; i<m_nFunc; i++)
	{
		if (m_pPointArray[xSegment][ySegment].f)
			fCell[0].f[i] = m_pPointArray[xSegment][ySegment].f[i];
		if (m_pPointArray[xSegment+1][ySegment].f)
			fCell[1].f[i] = m_pPointArray[xSegment+1][ySegment].f[i];
		if (m_pPointArray[xSegment+1][ySegment+1].f)
			fCell[2].f[i] = m_pPointArray[xSegment+1][ySegment+1].f[i];
		if (m_pPointArray[xSegment][ySegment+1].f)
			fCell[3].f[i] = m_pPointArray[xSegment][ySegment+1].f[i];
	};

	// Deal with cell truncation from below
	if (m_bYmin)
	{
		// Evaluate y_min for the x of the segment extremities
		yl = m_pLimitYCurve->GetPointF(xSegment,0);
		yr = m_pLimitYCurve->GetPointF(xSegment+1,0);

		// If yl or yr belongs to ySegment and/or just equal to 1 extremity,
		// modify base cell
		if (yl >= m_pY[ySegment] && yl < m_pY[ySegment+1])
			yCell[0] = yl;
		if (yr >= m_pY[ySegment] && yr < m_pY[ySegment+1])
			yCell[1] = yr;
		if (yl == m_pY[ySegment+1])	// truncated cell is a triangle
		{
			cellType = 3;
			skip = 0;
		};
		if (yr == m_pY[ySegment+1])	// truncated cell is a triangle
		{
			cellType = 3;
			skip = 1;
		};
	};
	// Deal with cell truncation from above
	if (m_bYmax)
	{
		// Evaluate y_max for the x of the segment extremities
		yl = m_pLimitYCurve->GetPointF(xSegment,1);
		yr = m_pLimitYCurve->GetPointF(xSegment+1,1);

		// If yl or yr belongs to ySegment and/or just equal to 1 extremity,
		// modify base cell
		if (yl > m_pY[ySegment] && yl <= m_pY[ySegment+1])
			yCell[3] = yl;
		if (yr > m_pY[ySegment] && yr <= m_pY[ySegment+1])
			yCell[2] = yr;
		if (yl == m_pY[ySegment])	// truncated cell is a triangle
		{
			cellType = 3;
			skip = 3;
		};
		if (yr == m_pY[ySegment])	// truncated cell is a triangle
		{
			cellType = 3;
			skip = 2;
		};
	};
	// Achieve cell restructuration to triangle (0,1,2)
	if (skip != -1)
		for (i=skip; i<3; i++)
		{
			xCell[i] = xCell[i+1];
			yCell[i] = yCell[i+1];
			for (j=0; j<m_nFunc; j++)
				fCell[i].f[j] = fCell[i+1].f[j];
		};

	// Perform the interpolation
	if (cellType == 3)	// Triangle
	{
		double area2 = xCell[0]*yCell[1]+xCell[1]*yCell[2]+xCell[2]*yCell[0]
			-xCell[2]*yCell[1]-xCell[0]*yCell[2]-xCell[1]*yCell[0];
		double N0 = (yCell[1]-yCell[2])*x+(xCell[2]-xCell[1])*yc
			+(xCell[1]*yCell[2]-xCell[2]*yCell[1]);
		double N1 = (yCell[2]-yCell[0])*x+(xCell[0]-xCell[2])*yc
			+(xCell[2]*yCell[0]-xCell[0]*yCell[2]);
		double N2 = (yCell[0]-yCell[1])*x+(xCell[1]-xCell[0])*yc
			+(xCell[0]*yCell[1]-xCell[1]*yCell[0]);
		for (i=0; i<m_nFunc; i++)
			value->f[i] = (fCell[0].f[i]*N0+fCell[1].f[i]*N1+fCell[2].f[i]*N2)/area2;
	}
	else				// Quadrilateral
	{ 
		double xi = 2.0*Zx-1.0;
		double yd = yCell[0]+Zx*(yCell[1]-yCell[0]);
		double yu = yCell[3]+Zx*(yCell[2]-yCell[3]);
		double eta = 2.0*(yc-yd)/(yu-yd)-1.0;
		double N0 =	(1.0-xi)*(1.0-eta);
		double N1 =	(1.0+xi)*(1.0-eta);
		double N2 =	(1.0+xi)*(1.0+eta);
		double N3 =	(1.0-xi)*(1.0+eta);
		for (i=0; i<m_nFunc; i++)
			value->f[i] = (fCell[0].f[i]*N0+fCell[1].f[i]*N1
			+fCell[2].f[i]*N2+fCell[3].f[i]*N3)/4.0;
	};

	// Clean up
	for (i=0; i<4; i++)
		delete [] fCell[i].f;
	delete [] fCell;

	return xyStatus;
}

/////////////////////////////////////////////////////////////////////////////////
// Get the segment index of vector vec in which x falls. vecSize is the size
// of the zero-based vector vec. Return -1 if x<vec[0], return vecSize if 
// x>vec[vecSize-1]. On return, z contains the value (x-xLeft)/(xRight-xLeft).
//
int CSurfaceFitter::GetSegment(const double* vec, int vecSize, double x, 
							   double* z) const
{
	int iLeft = 0;
	int iRight = vecSize-1;
	int iMid;
	double xLeft = vec[iLeft];
	double xRight = vec[iRight];
	double xMid;

	// Check range
	if (x<xLeft)
		return iLeft-1;
	else if (x>xRight)
		return iRight+1;

	// Look for the stored points in between the x value is.
	// Dichotomic algorithm is used. iRight will contain the segment index.
	do
	{
		iMid = (iLeft+iRight)/2;
		xMid = vec[iMid];
		if(xMid>x)
		{
			iRight = iMid;
			xRight = xMid;
		}
		else
		{
			iLeft = iMid;
			xLeft = xMid;
		}
	}
	while((iRight-iLeft)>1);
	*z = (x-xLeft)/(xRight-xLeft);
	return iLeft;
} 

CAdditFitter::CCurvTemp::CCurvTemp()
{
	m_pCurveFnFctTemp = NULL;
	m_vecFn.resize(NUMBER_OF_FUNCTION);
}

CAdditFitter::CCurvTemp::~CCurvTemp()
{
	if ( NULL != m_pCurveFnFctTemp )
	{
		delete 	m_pCurveFnFctTemp;
	}
}

CCurveFitter* CAdditFitter::CCurvTemp::CreateTempCurve()
{
	// Curve already exist !
	if (NULL != m_pCurveFnFctTemp)
		return m_pCurveFnFctTemp;
	m_pCurveFnFctTemp = new CCurveFitter(NUMBER_OF_FUNCTION);
	double dData[NUMBER_OF_FUNCTION];
	typeMapTempFn::iterator it;
	for (it=m_mapTemp.begin(); it !=m_mapTemp.end(); ++it)
	{
		double dTemp = it->first;
		// Explore functions std::vector<double>
		for (UINT ui=0; ui < it->second.size(); ui++)
		{
			if ( ui >= NUMBER_OF_FUNCTION )
			{
				ASSERT (0);
				break;
			}
			dData[ui] = it->second[ui];
		}
		m_pCurveFnFctTemp->AddPoint(dTemp,dData);
	}
	return m_pCurveFnFctTemp;
}

SurfValReturn_enum CAdditFitter::CCurvTemp::GetPointOnTempCurve( double &dTemp, std::vector<double> &vecFn )
{
	typeMapTempFn::iterator it;
	it = m_mapTemp.begin();

	// No point exist
	if ( it == m_mapTemp.end() )
	{
		ASSERT (0);
		return SV_UNKNOWERROR;
	}
	// Temperature is below minimum value
	if ( dTemp < it->first )
	{
		//ASSERT (0);
		dTemp = it->first;
		vecFn = it->second;
		return SV_BELOWYMIN;
	}

	// Temperature is above maximum value
	typeMapTempFn::reverse_iterator rit;
	rit = m_mapTemp.rbegin();
	if ( dTemp > rit->first )
	{
		//ASSERT (0);
		dTemp = rit->first;
		vecFn = rit->second;
		return SV_ABOVEYMAX;
	}

	// Curve doesn't exist create it
	if (NULL == CreateTempCurve())
	{
		ASSERT(0);
		return SV_UNKNOWERROR;
	}

	// Extract / interpolate  each function
	for (int iFunc=0; iFunc< NUMBER_OF_FUNCTION; iFunc++)
	{
		vecFn[iFunc] = m_pCurveFnFctTemp->GetValue(dTemp, iFunc);
	}

	return SV_OK;
}

void CAdditFitter::CCurvTemp::AddPoint( double dTemp, std::vector<double> *pVecFn )
{
	m_mapTemp[dTemp] = *pVecFn;
	// If curve exist delete it, obsolete
	if ( NULL != m_pCurveFnFctTemp )
	{
		delete 	m_pCurveFnFctTemp;
	}
}


void CAdditFitter::CCurvTemp::AddPoint( double dTemp, double dRho, double dNu, double dCp, double dVaporPressure )
{
	m_vecFn[efnRho] = dRho;
	m_vecFn[efnNu] = dNu;
	m_vecFn[efnCp] = dCp;
	m_vecFn[efnVp] = dVaporPressure;

	AddPoint(dTemp, &m_vecFn);
}

double CAdditFitter::CCurvTemp::GetTmin()
{
	typeMapTempFn::iterator it = m_mapTemp.begin();

	if( m_mapTemp.end() == it )
	{
		return DBL_MIN;
	}

	return it->first;
}

double CAdditFitter::CCurvTemp::GetTmax()
{
	typeMapTempFn::reverse_iterator rit = m_mapTemp.rbegin();

	if( m_mapTemp.rend() == rit )
	{
		return DBL_MAX;
	}

	return rit->first;
}

int CAdditFitter::CCurvTemp::GetFunctionNumber()
{
	int iNumber = 0;

	if( NULL != m_pCurveFnFctTemp )
	{
		iNumber = m_pCurveFnFctTemp->GetFuncNumber();
	}

	return iNumber;
}

#define CCURVTEMP_VERSION		1
void CAdditFitter::CCurvTemp::Write( OUTSTREAM outf )
{
	// Version
	BYTE Version = CCURVTEMP_VERSION;
	outf.write( (char*)&Version, sizeof( Version ) );

	// Map size and function size
	UINT uiSize = m_mapTemp.size();
	outf.write( (char*)&uiSize, sizeof( uiSize ) );
	uiSize = NUMBER_OF_FUNCTION;
	outf.write( (char*)&uiSize, sizeof( uiSize ) );
	// Values
	
	for (typeMapTempFn::iterator it = m_mapTemp.begin(); m_mapTemp.end() != it; ++it)
	{
		outf.write( (char*)&it->first, sizeof( double ) );
		for ( int i=0; i<NUMBER_OF_FUNCTION; i++ )
		{
			outf.write( (char*)&it->second[i], sizeof( double ) );
		}
	}
}

bool CAdditFitter::CCurvTemp::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version;
	inpf.read( (char*)&Version, sizeof( Version ) );
	if( Version < 1 || Version > CCURVTEMP_VERSION )
		return false;
	// Map size and function size
	UINT uiMapSize = 0;
	inpf.read( (char*)&uiMapSize, sizeof( uiMapSize ) );

	UINT uiVecSize = 0;
	inpf.read( (char*)&uiVecSize, sizeof( uiVecSize ) );
	m_vecFn.resize(uiVecSize);

	// Values
	for (; 0 != uiMapSize ; --uiMapSize)
	{
		double dTemp = ReadDouble( inpf );
		for (UINT i=0; i < uiVecSize; i++)
		{
			double d = ReadDouble( inpf );
			m_vecFn[i] = d;
		}
		AddPoint(dTemp, &m_vecFn);
	}
	return true;
}

CAdditFitter::~CAdditFitter()
{
	typeMapConcentration::iterator it;
	// delete existing curves
	for ( it = m_mapConcentration.begin(); it != m_mapConcentration.end(); ++it )
	{
		delete (it->second);
	}
	if ( NULL != pCurvTminTmax )
	{
		delete pCurvTminTmax;
		pCurvTminTmax = NULL;
	}
}

CAdditFitter::CAdditFitter()
{
	m_vecFn.resize(NUMBER_OF_FUNCTION);
	m_vecFnBlw.resize(NUMBER_OF_FUNCTION,1);
	m_vecFnAbv.resize(NUMBER_OF_FUNCTION,1);
	pCurvTminTmax = NULL;
}

bool CAdditFitter::CreateCurveTminTmax()
{
	// Delete previous curve if exist
	if ( NULL != pCurvTminTmax )
	{
		delete pCurvTminTmax;
		pCurvTminTmax = NULL;
	}
	if ( GetNbrOfConcentrationCurves() <=1 )
		return false;
	pCurvTminTmax = new CCurveFitter(2);

	typeMapConcentration::iterator it = m_mapConcentration.begin();
	for (; m_mapConcentration.end() != it; ++it)
	{
		double d[2];
		d[0] = it->second->GetTmin();
		d[1] = it->second->GetTmax();
		pCurvTminTmax->AddPoint( it->first, d );
	}
	return true;
}

SurfValReturn_enum CAdditFitter::GetTminTmax( double dConcentration, double *pdTmin /*= NULL*/, double *pdTmax /*= NULL*/ )
{
	if ( NULL == pCurvTminTmax )
	{
		return SV_UNKNOWERROR;
	}
	if ( dConcentration < pCurvTminTmax->GetMinX() || dConcentration > pCurvTminTmax->GetMaxX() )
		return SV_OUTOFXRANGE;
	if (NULL != pdTmin)
	{
		*pdTmin = pCurvTminTmax->GetValue(dConcentration,0);
	}
	if (NULL != pdTmax)
	{
		*pdTmax = pCurvTminTmax->GetValue(dConcentration,1);
	}
	return SV_OK;
}

void CAdditFitter::AddPoint( double dConcentration, double dTemp, double dRho, double dNu, double dCp, double dVaporPressure )
{
	CCurvTemp *CpCurve;
	typeMapConcentration::iterator it = m_mapConcentration.find(dConcentration);

	if( m_mapConcentration.end() == it )
	{
		//Not found Add it
		CpCurve = new CCurvTemp();
		m_mapConcentration[dConcentration] = CpCurve;
		it = m_mapConcentration.find(dConcentration);
	}

	CpCurve = it->second;
	CpCurve->AddPoint(dTemp, dRho, dNu, dCp, dVaporPressure);
}

SurfValReturn_enum CAdditFitter::GetPoint( double dConcentration, double dTemp, double &dRho, double &dNu, double &dCp, double &dVp )
{
	typeMapConcentration::iterator it;
	it = m_mapConcentration.begin();

	// No point exist
	if ( it == m_mapConcentration.end() )
	{
		ASSERT (0);
		return SV_UNKNOWERROR;
	}

	// Concentration is below minimum value
	if ( dConcentration < it->first )
	{
		ASSERT (0);
		dConcentration = it->first;
		return SV_OUTOFXRANGE;
	}

	// Concentration is above maximum value
	typeMapConcentration::reverse_iterator rit;
	rit = m_mapConcentration.rbegin();
	if ( dConcentration > rit->first )
	{
		ASSERT (0);
		dConcentration = rit->first;
		return SV_OUTOFXRANGE;
	}

	// If we are on an existing curve
	it = m_mapConcentration.find(dConcentration);
	if ( it != m_mapConcentration.end() )
	{
		SurfValReturn_enum err = it->second->GetPointOnTempCurve( dTemp, m_vecFn );
		dRho = m_vecFn[efnRho];
		dNu = m_vecFn[efnNu];
		dCp	= m_vecFn[efnCp];
		dVp	= m_vecFn[efnVp];
		return err;
	}
	else
	{
		// We have to interpolate between two curves
		// Identify below and above curve
		typeMapConcentration::iterator itBlw = m_mapConcentration.end();
		typeMapConcentration::iterator itAbv = m_mapConcentration.end();
		for (it = m_mapConcentration.begin(); it != m_mapConcentration.end(); ++it)
		{
			if ( it->first < dConcentration )
			{
				itBlw = it;
			}
			else
			{
				itAbv = it;
				break;
			}
		}
		if (m_mapConcentration.end() != itBlw && m_mapConcentration.end() != itAbv)
		{
			double dt = dTemp;
			SurfValReturn_enum errBlw = itBlw->second->GetPointOnTempCurve( dt, m_vecFnBlw );
			dt = dTemp;
			SurfValReturn_enum errAbv = itAbv->second->GetPointOnTempCurve( dt, m_vecFnAbv );
			dTemp = dt;
			if (SV_OK != errAbv && SV_OK != errBlw)
				return errAbv;
			double dfactor = ( dConcentration - itBlw->first ) / ( itAbv->first - itBlw->first );
			if (dfactor < 0  || dfactor > 1)
			{
				ASSERT(0);
				return SV_UNKNOWERROR;
			}
			for (int i = 0; i< NUMBER_OF_FUNCTION; i++)
			{
				m_vecFn[i] = m_vecFnBlw[i] + dfactor * (m_vecFnAbv[i] - m_vecFnBlw[i]);
			}
			dRho = m_vecFn[efnRho];
			dNu = m_vecFn[efnNu];
			dCp	= m_vecFn[efnCp];
			dVp = m_vecFn[efnVp];
			return SV_OK;
		}
		else
		{
			ASSERT (0);
			return SV_UNKNOWERROR;
		}
	}
}

int CAdditFitter::GetFunctionNumber()
{
	if( 0 == (int)m_mapConcentration.size() )
	{
		return 0;
	}

	return m_mapConcentration.begin()->second->GetFunctionNumber();
}

int CAdditFitter::GetNbrOfConcentrationCurves()
{
	return m_mapConcentration.size();
}

bool CAdditFitter::GetPointX( UINT iPointX, double &dConcentration, double *pdTmin, double *pdTmax )
{
	if ( iPointX >= m_mapConcentration.size() )
		return false;
	typeMapConcentration::iterator it = m_mapConcentration.begin();

	for (UINT i = 0; i < iPointX; i++ )
		++it;

	if ( it == m_mapConcentration.end() )
		return false;

	dConcentration = it->first;
	CCurvTemp *CpCurve = it->second;

	bool fRet = true;
	if (NULL != pdTmin)
	{
		*pdTmin = CpCurve->GetTmin();
		if ( DBL_MIN == *pdTmin )
			fRet = false;
	}
	if (NULL != pdTmax)
	{
		*pdTmax = CpCurve->GetTmax();
		if ( DBL_MAX != *pdTmax )
		{
			fRet = false;
		}
	}
	return fRet;
}

#define CADDITFITTER_VERSION		1
void CAdditFitter::Write( OUTSTREAM outf )
{
	// Version
	BYTE Version = CADDITFITTER_VERSION;
	outf.write( (char*)&Version, sizeof( Version ) );

	// Map size 
	UINT uiSize = m_mapConcentration.size();
	outf.write( (char*)&uiSize, sizeof( uiSize ) );

	// For each concentration
	for (typeMapConcentration::iterator it = m_mapConcentration.begin(); m_mapConcentration.end() != it; ++it)
	{
		outf.write( (char*)&it->first, sizeof( double ) );
		it->second->Write(outf);
	}
}

bool CAdditFitter::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version;
	inpf.read( (char*)&Version, sizeof( Version ) );
	if( Version < 1 || Version > CADDITFITTER_VERSION )
		return false;
	// Map size and function size
	UINT uiMapSize = 0;
	inpf.read( (char*)&uiMapSize, sizeof( uiMapSize ) );

	// Values
	for (; 0 != uiMapSize ; --uiMapSize)
	{
		double dConc = ReadDouble( inpf );
		CCurvTemp *pCurvTemp = new CCurvTemp();
		if (true == pCurvTemp->Read( inpf ) )
			m_mapConcentration[dConc] = pCurvTemp;	
	}
	return true;
}
