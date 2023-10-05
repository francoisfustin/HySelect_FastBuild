#pragma once


// CGdiplusDlg dialog
class CPMPressureGaugeHelper
{
public:
	CPMPressureGaugeHelper() {}
	virtual ~CPMPressureGaugeHelper() {}

	HBITMAP GetPressureGauge( double dp0, double dpa, double dpe, double dPSV, CWnd *pWnd );

// Private methods.
private:
	void _GaugeDrawArrowsAndArc( int iX1, int iY1, double dStartAngleDeg, int iX2, int iY2, double dEndAngleDeg );

	void _GaugeDrawArc( double dStartAngleDeg, double dEndAngleDeg, Gdiplus::Color colorStart, Gdiplus::Color colorEnd );

	void _GaugeDrawGradient( double dStartAngleDeg, double dEndAngleDeg, Gdiplus::Color colorStart, Gdiplus::Color colorEnd );

	double _ComputeAngle( double dX, double dY, double dXCenter, double dYCenter, double dHypothenuse );

	// Private variables.
private:
	HBITMAP m_hBitmap;

	typedef struct _GaugeVariable
	{
		Gdiplus::Graphics *m_pclGraphics;
		double m_dMagic;
		double m_dPI;
		Gdiplus::Color m_colorp0;
		Gdiplus::Color m_colorpa;
		Gdiplus::Color m_colorpfill;
		Gdiplus::Color m_colorpe;
		Gdiplus::Color m_colorpsv;
		double m_dXCenter;
		double m_dYCenter;
		double m_dRadius;
		double m_dTopHypothenuse;
		double m_dBottomHypothenuse;
		Gdiplus::Rect m_rectArc;
		Gdiplus::Rect m_rectArcTop;
		Gdiplus::Rect m_rectArcBottom;
		double m_dArcWidth;
	}GaugeVariable;

	GaugeVariable m_GaugeVariables;
};
