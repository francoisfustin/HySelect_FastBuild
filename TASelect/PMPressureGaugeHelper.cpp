#include "stdafx.h"
#include "Gdiplus.h"
#include "MainFrm.h"
#include "TASelect.h"
#include "SelectPM.h"
#include "PMPressureGaugeHelper.h"


HBITMAP CPMPressureGaugeHelper::GetPressureGauge( double dp0, double dpa, double dpe, double dPSV, CWnd *pWnd )
{
	// Remark: keep in mind that to draw, the origin (0, 0) is the top left corner.
	int iAreaWidth = 350;
	int iAreaHeight = 175;
	int iGaugeLeftMargin = 40;
	int iGaugeRightMargin = 40;
	int iGaugeTopMargin = 20;
	int iGaugeBottomMargin = 20;
	int iGaugeWidth = iAreaWidth - iGaugeLeftMargin - iGaugeRightMargin;
	int iGaugeHeight = iAreaHeight - iGaugeTopMargin - iGaugeBottomMargin;
	double dHorzPenWidth = 3.0;
	int iChordPenWidth = 3;

	m_GaugeVariables.m_dPI = 3.14159265359;
	m_GaugeVariables.m_dMagic = 15.0;

	// Defines all needed color.
	m_GaugeVariables.m_colorp0.SetValue( Gdiplus::Color::MakeARGB( 255, 74, 144, 226 ) );			// p0 (minimum pressure) - dark blue
	m_GaugeVariables.m_colorpa.SetValue( Gdiplus::Color::MakeARGB( 255, 8, 148, 161 ) );			// pa (initial pressure) - light blue
	m_GaugeVariables.m_colorpfill.SetValue( Gdiplus::Color::MakeARGB( 255, 71, 171, 108 ) );		// pfill (filling pressure) - green
	m_GaugeVariables.m_colorpe.SetValue( Gdiplus::Color::MakeARGB( 255, 245, 166, 35 ) );			// pe (final pressure) - orange
	m_GaugeVariables.m_colorpsv.SetValue( Gdiplus::Color::MakeARGB( 255, 208, 2, 27 ) );			// PSV - light red

	m_GaugeVariables.m_rectArc = Gdiplus::Rect( iGaugeLeftMargin, iGaugeTopMargin, iGaugeWidth, 2 * iGaugeHeight );
	m_GaugeVariables.m_dXCenter = m_GaugeVariables.m_rectArc.GetLeft() + m_GaugeVariables.m_rectArc.Width / 2.0;
	m_GaugeVariables.m_dYCenter = m_GaugeVariables.m_rectArc.GetTop() + m_GaugeVariables.m_rectArc.Height / 2.0 + dHorzPenWidth / 2.0;
	m_GaugeVariables.m_dRadius = m_GaugeVariables.m_rectArc.Width / 2.0;
	m_GaugeVariables.m_dArcWidth = 7.0;
	m_GaugeVariables.m_dTopHypothenuse = m_GaugeVariables.m_dXCenter - m_GaugeVariables.m_rectArc.GetLeft() + m_GaugeVariables.m_dArcWidth / 2.0;
	m_GaugeVariables.m_dBottomHypothenuse = m_GaugeVariables.m_dXCenter - m_GaugeVariables.m_rectArc.GetLeft() - m_GaugeVariables.m_dArcWidth / 2.0;
	m_GaugeVariables.m_rectArcTop = m_GaugeVariables.m_rectArc;
	m_GaugeVariables.m_rectArcTop.Inflate( (int)( m_GaugeVariables.m_dArcWidth / 2.0 ), (int)( m_GaugeVariables.m_dArcWidth / 2.0 ) );
	m_GaugeVariables.m_rectArcBottom = m_GaugeVariables.m_rectArc;
	m_GaugeVariables.m_rectArcBottom.Inflate( (int)( -m_GaugeVariables.m_dArcWidth / 2.0 ), (int)( -m_GaugeVariables.m_dArcWidth / 2.0 ) );

	Gdiplus::Bitmap clBitmap( iAreaWidth, iAreaHeight, PixelFormat32bppARGB );

	Gdiplus::Graphics g( &clBitmap );
	m_GaugeVariables.m_pclGraphics = &g;

	g.SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );

	Gdiplus::SolidBrush clBrush( Gdiplus::Color( 255, 255, 255, 255 ) );
	g.FillRectangle( &clBrush, 0, 0, iAreaWidth, iAreaHeight );

	// Draw horizontal line.
	Gdiplus::Color colorHorzPen( 255, 0, 0, 0 );
	Gdiplus::Pen clBlackPen( colorHorzPen, ( Gdiplus::REAL )dHorzPenWidth );
	g.DrawLine( &clBlackPen, iGaugeLeftMargin, iAreaHeight - iGaugeBottomMargin, iAreaWidth - iGaugeRightMargin, iAreaHeight - iGaugeBottomMargin );

	// Draw arc between 'p0' and 'pa'.
	double dpaAngleDeg = ( dpa - dp0 ) / ( dPSV - dp0 ) * 180.0;

	_GaugeDrawArc( 0.0, dpaAngleDeg, m_GaugeVariables.m_colorp0, m_GaugeVariables.m_colorpa );

	// Draw chord between center et 'pa'.
	// Remark: we draw until the top arc (m_GaugeVariables.m_dTopHypothenuse).
	double dpaAngleRad = m_GaugeVariables.m_dPI / 180.0 * dpaAngleDeg;
	double dpaY = sin( dpaAngleRad ) * m_GaugeVariables.m_dTopHypothenuse;
	int ipaY = iAreaHeight - iGaugeBottomMargin - (int)dpaY;
	double dpaX = cos( dpaAngleRad ) * m_GaugeVariables.m_dTopHypothenuse;
	int ipaX = iAreaWidth / 2 - (int)dpaX;

	Gdiplus::Pen clPenPa( m_GaugeVariables.m_colorpa, ( Gdiplus::REAL )iChordPenWidth );
	g.DrawLine( &clPenPa, ipaX, ipaY, iGaugeLeftMargin + iGaugeWidth / 2, iAreaHeight - iGaugeBottomMargin );

	double dpeAngleDeg = ( dpe - dp0 ) / ( dPSV - dp0 ) * 180.0;

	// Don't do it if dpe = dPSV otherwise we will get strange results.
	if( dpe < dPSV )
	{
		// Draw arc between 'pe' and 'psv'.
		_GaugeDrawArc( dpeAngleDeg, 180.0, m_GaugeVariables.m_colorpe, m_GaugeVariables.m_colorpsv );
	}

	// Draw chord between center et 'pe'.
	// Remark: we draw until the top arc (m_GaugeVariables.m_dTopHypothenuse).
	double dpeAngleRad = m_GaugeVariables.m_dPI / 180.0 * dpeAngleDeg;
	double dpeY = sin( dpeAngleRad ) * m_GaugeVariables.m_dTopHypothenuse;
	int ipeY = iAreaHeight - iGaugeBottomMargin - (int)dpeY;
	double dpeX = cos( dpeAngleRad ) * m_GaugeVariables.m_dTopHypothenuse;
	int ipeX = iAreaWidth / 2 - (int)dpeX;

	Gdiplus::Pen clPenPe( m_GaugeVariables.m_colorpe, ( Gdiplus::REAL )iChordPenWidth );
	g.DrawLine( &clPenPe, ipeX, ipeY, iGaugeLeftMargin + iGaugeWidth / 2, iAreaHeight - iGaugeBottomMargin );

	// Draw arrows with gradient fill.
	// Remark: We recompute all to be at the middle of the arc.
	dpaY = sin( dpaAngleRad ) * m_GaugeVariables.m_dRadius;
	ipaY = iAreaHeight - iGaugeBottomMargin - (int)dpaY;
	dpaX = cos( dpaAngleRad ) * m_GaugeVariables.m_dRadius;
	ipaX = iAreaWidth / 2 - (int)dpaX;

	dpeY = sin( dpeAngleRad ) * m_GaugeVariables.m_dRadius;
	ipeY = iAreaHeight - iGaugeBottomMargin - (int)dpeY;
	dpeX = cos( dpeAngleRad ) * m_GaugeVariables.m_dRadius;
	ipeX = iAreaWidth / 2 - (int)dpeX;

	_GaugeDrawArrowsAndArc( ipaX, ipaY, dpaAngleDeg, ipeX, ipeY, dpeAngleDeg );

	// Add text.
	LOGFONT rLogFont;
	ZeroMemory( &rLogFont, sizeof( LOGFONT ) );
	rLogFont.lfHeight = -MulDiv( 8, GetDeviceCaps( pWnd->GetDC()->GetSafeHdc(), LOGPIXELSY ), 72 );
	rLogFont.lfWeight = FW_BOLD;
	rLogFont.lfCharSet = DEFAULT_CHARSET;
	wcscpy_s( rLogFont.lfFaceName, _T("Arial 8") );
	Gdiplus::Font clFont( pWnd->GetDC()->GetSafeHdc(), &rLogFont );

	Gdiplus::RectF rectText( Gdiplus::PointF( 0.0, 0.0 ), Gdiplus::SizeF( ( Gdiplus::REAL )iAreaWidth, ( Gdiplus::REAL )iAreaHeight ) );
	Gdiplus::RectF rectTextSize;

	// p0.
	Gdiplus::SolidBrush blackBrush( Gdiplus::Color( 255, 0, 0, 0 ) );
	CString strLegend = _T("p0");
	g.MeasureString( strLegend, -1, &clFont, rectText, &rectTextSize );
	g.DrawString( (LPCTSTR)strLegend, -1, &clFont, Gdiplus::PointF( iGaugeLeftMargin - rectTextSize.Width - 12, iAreaHeight - iGaugeBottomMargin - rectTextSize.Height + 2 ), &blackBrush );

	// pa.
	double dpaLegendX = cos( dpaAngleRad ) * (double)( ( iGaugeWidth / 2.0 ) + m_GaugeVariables.m_dArcWidth );
	int ipaLegendX = iAreaWidth / 2 - (int)dpaLegendX;
	double dpaLegendY = sin( dpaAngleRad ) * (double)( ( iGaugeWidth / 2.0 ) + m_GaugeVariables.m_dArcWidth );
	int ipaLegendY = iAreaHeight - iGaugeBottomMargin - (int)dpaLegendY;

	strLegend = _T("pa");
	g.MeasureString( strLegend, -1, &clFont, rectText, &rectTextSize );
	g.DrawString( (LPCTSTR)strLegend, -1, &clFont, Gdiplus::PointF( ( Gdiplus::REAL )( ipaLegendX - rectTextSize.Width ), ( Gdiplus::REAL )( ipaLegendY - rectTextSize.Height ) ), &blackBrush );

	// Write PSV first. To verify if pe is not too close and adapt in consequence.
	int iPSVx = iAreaWidth - iGaugeRightMargin + 12;
	int iPSVy = iAreaHeight - iGaugeBottomMargin - (int)rectTextSize.Height;
	int iPSVh = iAreaHeight - iGaugeBottomMargin;
	strLegend = _T("psv");
	g.DrawString( (LPCTSTR)strLegend, -1, &clFont, Gdiplus::PointF( ( Gdiplus::REAL )iPSVx, ( Gdiplus::REAL )iPSVy ), &blackBrush );

	// pe.
	double dpeLegendX = cos( dpeAngleRad ) * (double)( ( iGaugeWidth / 2.0 ) + m_GaugeVariables.m_dArcWidth );
	int ipeLegendX = iAreaWidth / 2 - (int)dpeLegendX;
	double dpeLegendY = sin( dpeAngleRad ) * (double)( ( iGaugeWidth / 2.0 ) + m_GaugeVariables.m_dArcWidth );
	int ipeLegendY = iAreaHeight - iGaugeBottomMargin - (int)dpeLegendY;

	// Verify with pSV.
	ipeLegendY = min( ipeLegendY, iPSVy - 4 );
	strLegend = _T("pe");
	g.MeasureString( strLegend, -1, &clFont, rectText, &rectTextSize );

	if( dpeAngleDeg > 90.0 )
	{
		g.DrawString( (LPCTSTR)strLegend, -1, &clFont, Gdiplus::PointF( ( Gdiplus::REAL )ipeLegendX, ( Gdiplus::REAL )( ipeLegendY - rectTextSize.Height + 2 ) ), &blackBrush );
	}
	else
	{
		g.DrawString( (LPCTSTR)strLegend, -1, &clFont, Gdiplus::PointF( ( Gdiplus::REAL )( ipeLegendX - rectTextSize.Width ), ( Gdiplus::REAL )( ipeLegendY - rectTextSize.Height + 2 ) ), &blackBrush );
	}

	HBITMAP hBitmap = NULL;
	clBitmap.GetHBITMAP( Gdiplus::Color( 0, 0, 0 ), &hBitmap );

	return hBitmap;
}

void CPMPressureGaugeHelper::_GaugeDrawArrowsAndArc( int iX1, int iY1, double dStartAngleDeg, int iX2, int iY2, double dEndAngleDeg )
{
	double dMagicDemi = m_GaugeVariables.m_dMagic / 2.0;

	if( (int)m_GaugeVariables.m_dMagic < m_GaugeVariables.m_dArcWidth )
	{
		return;
	}

	double dArcPart = (double)m_GaugeVariables.m_dArcWidth / 3.0;

	CArray<CPoint, CPoint> arPoints;

	// These points define the drawing of an arrow.
	double ardPointX[7] = { -dMagicDemi, -dArcPart - dArcPart / 2.0, -dArcPart / 2.0, 0.0, dArcPart / 2.0, dArcPart / 2.0 + dArcPart, dMagicDemi };
	double ardPointY[7] = { m_GaugeVariables.m_dMagic, -ardPointX[1] + dMagicDemi, -ardPointX[2] + dMagicDemi, dMagicDemi, -ardPointX[2] + dMagicDemi, -ardPointX[1] + dMagicDemi, m_GaugeVariables.m_dMagic };

	double dStartAngleRad = dStartAngleDeg / 180.0 * m_GaugeVariables.m_dPI;
	double dEndAngleRad = dEndAngleDeg / 180.0 * m_GaugeVariables.m_dPI;

	// We do now a rotation of these two arrows in regards to the angles (pa and pe).
	for( int iLoopArrow = 0; iLoopArrow < 2; ++iLoopArrow )
	{
		double dAngle = ( 0 == iLoopArrow ) ? dStartAngleRad : -( m_GaugeVariables.m_dPI - dEndAngleRad );
		int iX = ( 0 == iLoopArrow ) ? iX1 : iX2;
		int iY = ( 0 == iLoopArrow ) ? iY1 : iY2;

		for( int iLoopPoint = 0; iLoopPoint < 7; ++iLoopPoint )
		{
			double dX = ardPointX[iLoopPoint];
			double dY = ardPointY[iLoopPoint];

			double dHyp = sqrt( dX * dX + dY * dY );
			double dAlphaRad = atan( abs( dX ) / abs( dY ) );

			if( dX < 0.0 )
			{
				dAlphaRad += ( m_GaugeVariables.m_dPI / 2.0 );
			}
			else
			{
				dAlphaRad = ( m_GaugeVariables.m_dPI / 2.0 ) - dAlphaRad;
			}

			if( dAngle != 0.0 )
			{
				double dNewAlphaRad = dAlphaRad - dAngle;
				dX = cos( dNewAlphaRad ) * dHyp;
				dY = sin( dNewAlphaRad ) * dHyp;
			}

			int iNextX = (int)( (double)iX + dX );
			int iNextY = (int)( (double)iY - dY );

			arPoints.Add( CPoint( iNextX, iNextY ) );
		}
	}

	// 1st: we draw a 'path' that is in fact the arrows linked by the two arcs.
	// The path will serve as a region that itself will serve as a clip.
	// Clipping allow to draw everywhere but the drawing will be only visible in this clipped region.
	Gdiplus::GraphicsPath clGraphicsPath;

	// Start of the 'pa' arrow.
	clGraphicsPath.AddLine( iX1, iY1, arPoints[0].x, arPoints[0].y );
	clGraphicsPath.AddLine( arPoints[0].x, arPoints[0].y, arPoints[1].x, arPoints[1].y );

	// Prepare top of the arc between 'pa' and 'pe'.

	// Define where to start the top of the arc on the 'pa' arrow.
	// Remark: "360.0" to convert in the Gdi+ coordinates.
	double dArcTopAngleStartDeg = 360.0 - _ComputeAngle( arPoints[1].x, arPoints[1].y, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, m_GaugeVariables.m_dTopHypothenuse );

	// Define where to end the top of the arc on the 'pe' arrow.
	double dArcTopAngleEndDeg = 360.0 - _ComputeAngle( arPoints[12].x, arPoints[12].y, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, m_GaugeVariables.m_dTopHypothenuse );

	// Draw top of the arc.
	clGraphicsPath.AddArc( m_GaugeVariables.m_rectArcTop, ( Gdiplus::REAL )dArcTopAngleStartDeg, ( Gdiplus::REAL )( dArcTopAngleEndDeg - dArcTopAngleStartDeg ) );

	// Draw the 'pe' arrow.
	clGraphicsPath.AddLine( arPoints[12].x, arPoints[12].y, arPoints[13].x, arPoints[13].y );
	clGraphicsPath.AddLine( arPoints[13].x, arPoints[13].y, iX2, iY2 );
	clGraphicsPath.AddLine( iX2, iY2, arPoints[7].x, arPoints[7].y );
	clGraphicsPath.AddLine( arPoints[7].x, arPoints[7].y, arPoints[8].x, arPoints[8].y );

	// Prepare bottom of the arc between 'pe' and 'pa'.

	// Define where to start the bottom of the arc on the 'pe' arrow.
	// Remark: "360.0" to convert in the Gdi+ coordinates.
	double dArcBottomAngleStartDeg = 360.0 - _ComputeAngle( arPoints[8].x, arPoints[8].y, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, m_GaugeVariables.m_dBottomHypothenuse );

	// Define where to end the bottom of the arc on the 'pa' arrow.
	double dArcBottomAngleEndDeg = 360.0 - _ComputeAngle( arPoints[5].x, arPoints[5].y, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, m_GaugeVariables.m_dBottomHypothenuse );

	// Draw bottom of the arc.
	clGraphicsPath.AddArc( m_GaugeVariables.m_rectArcBottom, ( Gdiplus::REAL )dArcBottomAngleStartDeg, ( Gdiplus::REAL )( dArcBottomAngleEndDeg - dArcBottomAngleStartDeg ) );

	// Finish the 'pa' arrow.
	clGraphicsPath.AddLine( arPoints[5].x, arPoints[5].y, arPoints[6].x, arPoints[6].y );
	clGraphicsPath.AddLine( arPoints[6].x, arPoints[6].y, iX1, iY1 );

	// Transform this path in a region.
	Gdiplus::Region clRgn( &clGraphicsPath );

	// Now transform this region in a clipping area.
	m_GaugeVariables.m_pclGraphics->SetClip( &clRgn, Gdiplus::CombineModeReplace );

	// Now prepare the filling of the clipped area with a gradient going from the color of 'pa'
	// to the color of 'pe'.
	for( int iLoopColor = 0; iLoopColor < 2; ++iLoopColor )
	{
		Gdiplus::Color colorStart = ( 0 == iLoopColor ) ? m_GaugeVariables.m_colorpa : m_GaugeVariables.m_colorpfill;
		Gdiplus::Color colorEnd = ( 0 == iLoopColor ) ? m_GaugeVariables.m_colorpfill : m_GaugeVariables.m_colorpe;

		double dStartAngle = ( 0 == iLoopColor ) ? dStartAngleDeg : dStartAngleDeg + ( dEndAngleDeg - dStartAngleDeg ) / 2.0;
		double dEndAngle = ( 0 == iLoopColor ) ? dStartAngleDeg + ( dEndAngleDeg - dStartAngleDeg ) / 2.0 : dEndAngleDeg;

		_GaugeDrawGradient( dStartAngle, dEndAngle, colorStart, colorEnd );
	}

	// Now draw white on border to apply antialiasing effect.
	Gdiplus::Pen clPenWhite( Gdiplus::Color( 155, 255, 255, 255 ) );

	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, iX1, iY1, arPoints[0].x, arPoints[0].y );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, arPoints[0].x, arPoints[0].y, arPoints[1].x, arPoints[1].y );
	m_GaugeVariables.m_pclGraphics->DrawArc( &clPenWhite, m_GaugeVariables.m_rectArcTop, ( Gdiplus::REAL )dArcTopAngleStartDeg, ( Gdiplus::REAL )( dArcTopAngleEndDeg - dArcTopAngleStartDeg ) );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, arPoints[12].x, arPoints[12].y, arPoints[13].x, arPoints[13].y );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, arPoints[13].x, arPoints[13].y, iX2, iY2 );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, iX2, iY2, arPoints[7].x, arPoints[7].y );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, arPoints[7].x, arPoints[7].y, arPoints[8].x, arPoints[8].y );
	m_GaugeVariables.m_pclGraphics->DrawArc( &clPenWhite, m_GaugeVariables.m_rectArcBottom, ( Gdiplus::REAL )dArcBottomAngleStartDeg, ( Gdiplus::REAL )( dArcBottomAngleEndDeg - dArcBottomAngleStartDeg ) );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, arPoints[5].x, arPoints[5].y, arPoints[6].x, arPoints[6].y );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, arPoints[6].x, arPoints[6].y, iX1, iY1 );

	m_GaugeVariables.m_pclGraphics->ResetClip();

	// Internal arrows and curves.
	clGraphicsPath.Reset();

	clGraphicsPath.AddLine( arPoints[3].x, arPoints[3].y, arPoints[2].x, arPoints[2].y );

	Gdiplus::Rect rectArcTop = m_GaugeVariables.m_rectArc;
	rectArcTop.Inflate( (int)( m_GaugeVariables.m_dArcWidth / 2.0 ), (int)( m_GaugeVariables.m_dArcWidth / 2.0 ) );
	rectArcTop.Inflate( (int)( -m_GaugeVariables.m_dArcWidth / 3.0 ), (int)( -m_GaugeVariables.m_dArcWidth / 3.0 ) );

	double dTopHypothenuse = pow( arPoints[2].x - m_GaugeVariables.m_dXCenter, 2 );
	dTopHypothenuse += pow( arPoints[2].y - m_GaugeVariables.m_dYCenter, 2 );
	dTopHypothenuse = sqrt( dTopHypothenuse );
	dArcTopAngleStartDeg = 360.0 - _ComputeAngle( arPoints[2].x, arPoints[2].y, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, dTopHypothenuse );
	dArcTopAngleEndDeg = 360.0 - _ComputeAngle( arPoints[11].x, arPoints[11].y, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, dTopHypothenuse );

	clGraphicsPath.AddArc( rectArcTop, ( Gdiplus::REAL )dArcTopAngleStartDeg, ( Gdiplus::REAL )( dArcTopAngleEndDeg - dArcTopAngleStartDeg ) );

	clGraphicsPath.AddLine( arPoints[11].x, arPoints[11].y, arPoints[10].x, arPoints[10].y );
	clGraphicsPath.AddLine( arPoints[10].x, arPoints[10].y, arPoints[9].x, arPoints[9].y );

	Gdiplus::Rect rectArcBottom = m_GaugeVariables.m_rectArc;
	rectArcBottom.Inflate( (int)( -m_GaugeVariables.m_dArcWidth / 2.0 ), (int)( -m_GaugeVariables.m_dArcWidth / 2.0 ) );
	rectArcBottom.Inflate( (int)( m_GaugeVariables.m_dArcWidth / 3.0 ), (int)( m_GaugeVariables.m_dArcWidth / 3.0 ) );

	double dBottomHypothenuse = pow( arPoints[9].x - m_GaugeVariables.m_dXCenter, 2 );
	dBottomHypothenuse += pow( arPoints[9].y - m_GaugeVariables.m_dYCenter, 2 );
	dBottomHypothenuse = sqrt( dBottomHypothenuse );
	dArcBottomAngleStartDeg = 360.0 - _ComputeAngle( arPoints[9].x, arPoints[9].y, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, dBottomHypothenuse );
	dArcBottomAngleEndDeg = 360.0 - _ComputeAngle( arPoints[4].x, arPoints[4].y, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, dBottomHypothenuse );

	clGraphicsPath.AddArc( rectArcBottom, ( Gdiplus::REAL )dArcBottomAngleStartDeg, ( Gdiplus::REAL )( dArcBottomAngleEndDeg - dArcBottomAngleStartDeg ) );

	clGraphicsPath.AddLine( arPoints[4].x, arPoints[4].y, arPoints[3].x, arPoints[3].y );
	clGraphicsPath.CloseFigure();

	Gdiplus::Region clRgn2( &clGraphicsPath );
	Gdiplus::SolidBrush clInternalBrush( Gdiplus::Color( 255, 255, 255, 255 ) );
	m_GaugeVariables.m_pclGraphics->FillRegion( &clInternalBrush, &clRgn2 );

	// Draw now border to apply antialiasing.
	Gdiplus::Pen clPen( Gdiplus::Color( 155, 255, 255, 255 ), 1 );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPen, arPoints[3].x, arPoints[3].y, arPoints[2].x, arPoints[2].y );
	m_GaugeVariables.m_pclGraphics->DrawArc( &clPen, rectArcTop, ( Gdiplus::REAL )dArcTopAngleStartDeg, ( Gdiplus::REAL )( dArcTopAngleEndDeg - dArcTopAngleStartDeg ) );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPen, arPoints[11].x, arPoints[11].y, arPoints[10].x, arPoints[10].y );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPen, arPoints[10].x, arPoints[10].y, arPoints[9].x, arPoints[9].y );
	m_GaugeVariables.m_pclGraphics->DrawArc( &clPen, rectArcBottom, ( Gdiplus::REAL )dArcBottomAngleStartDeg, ( Gdiplus::REAL )( dArcBottomAngleEndDeg - dArcBottomAngleStartDeg ) );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPen, arPoints[4].x, arPoints[4].y, arPoints[3].x, arPoints[3].y );
}

void CPMPressureGaugeHelper::_GaugeDrawArc( double dStartAngleDeg, double dEndAngleDeg, Gdiplus::Color colorStart, Gdiplus::Color colorEnd )
{
	double dArcWidthDemi = m_GaugeVariables.m_dArcWidth / 2.0;

	// Determine the 4 points of the arc to draw a path.
	double dStartAngleRad = 2.0 * m_GaugeVariables.m_dPI / 360.0 * dStartAngleDeg;

	double dY = sin( dStartAngleRad ) * ( m_GaugeVariables.m_dRadius - dArcWidthDemi );
	double dY1 = m_GaugeVariables.m_dYCenter - dY;
	double dX = cos( dStartAngleRad ) * ( m_GaugeVariables.m_dRadius - dArcWidthDemi );
	double dX1 = m_GaugeVariables.m_dXCenter - dX;

	dY = sin( dStartAngleRad ) * ( m_GaugeVariables.m_dRadius + dArcWidthDemi );
	double dY2 = m_GaugeVariables.m_dYCenter - dY;
	dX = cos( dStartAngleRad ) * ( m_GaugeVariables.m_dRadius + dArcWidthDemi );
	double dX2 = m_GaugeVariables.m_dXCenter - dX;

	double dEndAngleRad = 2.0 * m_GaugeVariables.m_dPI / 360.0 * dEndAngleDeg;

	dY = sin( dEndAngleRad ) * ( m_GaugeVariables.m_dRadius + dArcWidthDemi );
	double dY3 = m_GaugeVariables.m_dYCenter - dY;
	dX = cos( dEndAngleRad ) * ( m_GaugeVariables.m_dRadius + dArcWidthDemi );
	double dX3 = m_GaugeVariables.m_dXCenter - dX;

	dY = sin( dEndAngleRad ) * ( m_GaugeVariables.m_dRadius - dArcWidthDemi );
	double dY4 = m_GaugeVariables.m_dYCenter - dY;
	dX = cos( dEndAngleRad ) * ( m_GaugeVariables.m_dRadius - dArcWidthDemi );
	double dX4 = m_GaugeVariables.m_dXCenter - dX;

	// Define where to start the top of the arc on the 'pa' arrow.
	// Remark: "360 -" to transform in the Gdi+ coordinates.
	double dArcTopAngleStartDeg = 360.0 - _ComputeAngle( dX2, dY2, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, m_GaugeVariables.m_dTopHypothenuse );
	double dArcTopAngleEndDeg = 360.0 - _ComputeAngle( dX3, dY3, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, m_GaugeVariables.m_dTopHypothenuse );

	double dArcBottomAngleStartDeg = 360.0 - _ComputeAngle( dX4, dY4, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, m_GaugeVariables.m_dBottomHypothenuse );
	double dArcBottomAngleEndDeg = 360.0 - _ComputeAngle( dX1, dY1, m_GaugeVariables.m_dXCenter, m_GaugeVariables.m_dYCenter, m_GaugeVariables.m_dBottomHypothenuse );

	Gdiplus::GraphicsPath clGraphicsPath;

	clGraphicsPath.AddLine( ( Gdiplus::REAL )dX1, ( Gdiplus::REAL )dY1, ( Gdiplus::REAL )dX2, ( Gdiplus::REAL )dY2 );

	// Top arc.
	clGraphicsPath.AddArc( m_GaugeVariables.m_rectArcTop, ( Gdiplus::REAL )dArcTopAngleStartDeg, ( Gdiplus::REAL )( dArcTopAngleEndDeg - dArcTopAngleStartDeg ) );

	clGraphicsPath.AddLine( ( Gdiplus::REAL )dX3, ( Gdiplus::REAL )dY3, ( Gdiplus::REAL )dX4, ( Gdiplus::REAL )dY4 );

	// Bottom arc.
	clGraphicsPath.AddArc( m_GaugeVariables.m_rectArcBottom, ( Gdiplus::REAL )dArcBottomAngleStartDeg, ( Gdiplus::REAL )( dArcBottomAngleEndDeg - dArcBottomAngleStartDeg ) );

	// Transform this path in a region.
	Gdiplus::Region clRgn( &clGraphicsPath );

	// Now transform this region in a clipping area.
	m_GaugeVariables.m_pclGraphics->SetClip( &clRgn, Gdiplus::CombineModeReplace );

	_GaugeDrawGradient( dStartAngleDeg, dEndAngleDeg, colorStart, colorEnd );

	m_GaugeVariables.m_pclGraphics->ResetClip();

	// Now draw white on border to apply antialiasing effect.
	Gdiplus::Pen clPenWhite( Gdiplus::Color( 155, 255, 255, 255 ) );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, ( Gdiplus::REAL )dX1, ( Gdiplus::REAL )dY1, ( Gdiplus::REAL )dX2, ( Gdiplus::REAL )dY2 );
	m_GaugeVariables.m_pclGraphics->DrawArc( &clPenWhite, m_GaugeVariables.m_rectArcTop, ( Gdiplus::REAL )dArcTopAngleStartDeg, ( Gdiplus::REAL )( dArcTopAngleEndDeg - dArcTopAngleStartDeg ) );
	m_GaugeVariables.m_pclGraphics->DrawLine( &clPenWhite, ( Gdiplus::REAL )dX3, ( Gdiplus::REAL )dY3, ( Gdiplus::REAL )dX4, ( Gdiplus::REAL )dY4 );
	m_GaugeVariables.m_pclGraphics->DrawArc( &clPenWhite, m_GaugeVariables.m_rectArcBottom, ( Gdiplus::REAL )dArcBottomAngleStartDeg, ( Gdiplus::REAL )( dArcBottomAngleEndDeg - dArcBottomAngleStartDeg ) );
}

void CPMPressureGaugeHelper::_GaugeDrawGradient( double dStartAngleDeg, double dEndAngleDeg, Gdiplus::Color colorStart, Gdiplus::Color colorEnd )
{
	// This method allows to draw a gradient in the arc.
	// Before calling this method, a clipped region has been created.
	// We draw lines from the center of the arc to one position on the arc.
	// For each line, we change the color and we change the destination to the arc.
	// We create a kind of fan but only the arc region will be visible because this region is clipped.

	// Line version.
	double dR = (double)m_GaugeVariables.m_rectArc.Width / 2.0;
	double dC = dR + m_GaugeVariables.m_dMagic / 2.0;

	int iStep = 200;

	double dStartRedColor = colorStart.GetRed();
	double dEndRedColor = colorEnd.GetRed();
	double dRedColorStep = ( dEndRedColor - dStartRedColor ) / (double)iStep;
	double dCurrentRedColor = dStartRedColor;

	double dStartGreenColor = colorStart.GetGreen();
	double dEndGreenColor = colorEnd.GetGreen();
	double dGreenColorStep = ( dEndGreenColor - dStartGreenColor ) / (double)iStep;
	double dCurrentGreenColor = dStartGreenColor;

	double dStartBlueColor = colorStart.GetBlue();
	double dEndBlueColor = colorEnd.GetBlue();
	double dBlueColorStep = ( dEndBlueColor - dStartBlueColor ) / (double)iStep;
	double dCurrentBlueColor = dStartBlueColor;

	Gdiplus::Color currentColor( Gdiplus::Color::MakeARGB( 255, (BYTE)dStartRedColor, (BYTE)dStartGreenColor, (BYTE)dStartBlueColor ) );

	double dStartAngleRad = ( 180.0 - dStartAngleDeg ) / 180.0 * m_GaugeVariables.m_dPI;
	double dEndAngleRad = ( 180.0 - dEndAngleDeg ) / 180.0 * m_GaugeVariables.m_dPI;

	double dAngleStepRad = abs( dEndAngleRad - dStartAngleRad ) / (double)iStep;
	double dCurrentAngleRad = dStartAngleRad;

	for( int iLoop = 0; iLoop < iStep; ++iLoop )
	{
		Gdiplus::Pen clPen( currentColor, 3 );

		double dX = m_GaugeVariables.m_dXCenter + ( dC * cos( dCurrentAngleRad ) );
		double dY = m_GaugeVariables.m_dYCenter - ( dC * sin( dCurrentAngleRad ) );

		m_GaugeVariables.m_pclGraphics->DrawLine( &clPen, (int)m_GaugeVariables.m_dXCenter, (int)m_GaugeVariables.m_dYCenter, (int)dX, (int)dY );

		dCurrentRedColor += dRedColorStep;
		dCurrentGreenColor += dGreenColorStep;
		dCurrentBlueColor += dBlueColorStep;
		currentColor = Gdiplus::Color( Gdiplus::Color::MakeARGB( 255, (BYTE)dCurrentRedColor, (BYTE)dCurrentGreenColor, (BYTE)dCurrentBlueColor ) );

		dCurrentAngleRad -= dAngleStepRad;
	}
}

double CPMPressureGaugeHelper::_ComputeAngle( double dX, double dY, double dXCenter, double dYCenter, double dHypothenuse )
{
	return acos( ( dX - dXCenter ) / dHypothenuse ) / 3.14159265359 * 180.0;
}
