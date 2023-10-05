// EnBitmap.cpp: implementation of the CEnBitmap class (c) daniel godson 2002.
//
// credits: Peter Hendrix's CPicture implementation for the original IPicture code 
//          Yves Maurer's GDIRotate implementation for the idea of working directly on 32 bit representations of bitmaps 
//          Karl Lager's 'A Fast Algorithm for Rotating Bitmaps' 
// 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EnBitmap.h"

#include <afxstr.h>
#include <atlimage.h>

#include <afxpriv.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const int HIMETRIC_INCH	= 2540;

enum 
{
	FT_BMP,
	FT_ICO,
	FT_JPG,
	FT_GIF,

	FT_UNKNOWN
};

///////////////////////////////////////////////////////////////////////

C32BitImageProcessor::C32BitImageProcessor(BOOL bEnableWeighting) : m_bWeightingEnabled(bEnableWeighting)
{
}

C32BitImageProcessor::~C32BitImageProcessor()
{
}

CSize C32BitImageProcessor::CalcDestSize(CSize sizeSrc) 
{ 
	return sizeSrc; // default
}

BOOL C32BitImageProcessor::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{ 
	CopyMemory(pDestPixels, pSrcPixels, sizeDest.cx * 4 * sizeDest.cy); // default
	return TRUE;
}
// C32BitImageProcessor::CalcWeightedColor(...) is inlined in EnBitmap.h

/////////////////////////////////////////////////////////////////////
// C32BitImageProcessor derivations

CImageRotator::CImageRotator(int nDegrees, BOOL bEnableWeighting)
	: C32BitImageProcessor(bEnableWeighting)
{
	// normalize the angle
	while (nDegrees >= 360)
		nDegrees -= 360;

	while (nDegrees < 0)
		nDegrees += 360;

	ASSERT (nDegrees >= 0 && nDegrees < 360);

	m_dRadians = nDegrees * M_PI / 180;
}

CImageRotator::CImageRotator(double dRadians)
{
	// normalize the angle
	while (dRadians >= 2 * M_PI)
		dRadians -= 2 * M_PI;

	while (dRadians <= 0)
		dRadians += 2 * M_PI;

	ASSERT (dRadians >= 0 && dRadians < 2 * M_PI);

	m_dRadians = dRadians;
}

CImageRotator::~CImageRotator()
{
}

CSize CImageRotator::CalcDestSize(CSize sizeSrc)
{
	if (!m_dRadians || !sizeSrc.cx || !sizeSrc.cy)
		return sizeSrc;

	// calculate the four rotated corners
	double dCosA = cos(m_dRadians);
	double dSinA = sin(m_dRadians);

	CPoint ptTopLeft, ptTopRight, ptBottomLeft, ptBottomRight;

	ptTopLeft.x = round(-sizeSrc.cx * dCosA / 2 + sizeSrc.cy * dSinA / 2);
	ptTopLeft.y = round(sizeSrc.cy * dCosA / 2 - (-sizeSrc.cx) * dSinA / 2);

	ptTopRight.x = round(sizeSrc.cx * dCosA / 2 + sizeSrc.cy * dSinA / 2);
	ptTopRight.y = round(sizeSrc.cy * dCosA / 2 - sizeSrc.cx * dSinA / 2);

	ptBottomLeft.x = round(-sizeSrc.cx * dCosA / 2 + (-sizeSrc.cy) * dSinA / 2);
	ptBottomLeft.y = round(-sizeSrc.cy * dCosA / 2 - (-sizeSrc.cx) * dSinA / 2);

	ptBottomRight.x = round(sizeSrc.cx * dCosA / 2 + (-sizeSrc.cy) * dSinA / 2);
	ptBottomRight.y = round(-sizeSrc.cy * dCosA / 2 - sizeSrc.cx * dSinA / 2);

	// find the max absolute values in each direction
	int nMaxY = max(abs(ptTopLeft.y), max(abs(ptTopRight.y), max(abs(ptBottomLeft.y), abs(ptBottomRight.y))));
	int nMaxX = max(abs(ptTopLeft.x), max(abs(ptTopRight.x), max(abs(ptBottomLeft.x), abs(ptBottomRight.x))));
	
	return CSize((nMaxX + 1) * 2, (nMaxY + 1) * 2);
}

BOOL CImageRotator::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;

	if (!m_dRadians)
		bRes = C32BitImageProcessor::ProcessPixels(pSrcPixels, sizeSrc, pDestPixels, sizeDest);
	else
	{
		// note: we also need to translate the coords after rotating
		CSize sizeDestOffset(sizeDest.cx / 2 + sizeDest.cx % 2, sizeDest.cy / 2 + sizeDest.cy % 2);
		CSize sizeSrcOffset(sizeSrc.cx / 2 + sizeSrc.cx % 2, sizeSrc.cy / 2 + sizeSrc.cy % 2);

		CRect rSrc(0, 0, sizeSrc.cx - 1, sizeSrc.cy - 1);
		rSrc.OffsetRect(-sizeSrcOffset);

		// note: traversing the src bitmap leads to artifacts in the destination image
		// what we do is to traverse the destination bitmaps and compute the equivalent 
		// source color - credit for this observation goes to Yves Maurer (GDIRotate) 2002
		double dCosA = cos(m_dRadians);
		double dSinA = sin(m_dRadians);

		for (int nY = 0; nY < sizeDest.cy; nY++)
		{
			// calc y components of rotation
			double dCosYComponent = (nY - sizeDestOffset.cy) * dCosA;
			double dSinYComponent = (nY - sizeDestOffset.cy) * dSinA;

			double dSrcX = -sizeDestOffset.cx * dCosA + dSinYComponent;
			double dSrcY = dCosYComponent - (-sizeDestOffset.cx * dSinA);

			for (int nX = 0; nX < sizeDest.cx; nX++)
			{
				dSrcX += dCosA;
				dSrcY -= dSinA;

				CPoint ptSrc(round(dSrcX), round(dSrcY));
				int nPixel = (nY * sizeDest.cx + nX);

				if (rSrc.PtInRect(ptSrc))
				{
					if (!m_bWeightingEnabled)
					{
						ptSrc.Offset(sizeSrcOffset);
						RGBX* pRGBSrc = &pSrcPixels[ptSrc.y * sizeSrc.cx + ptSrc.x];
						
						pDestPixels[nPixel] = *pRGBSrc;
					}
					else
						pDestPixels[nPixel] = CalcWeightedColor(pSrcPixels, sizeSrc, 
															dSrcX + sizeSrcOffset.cx, dSrcY + sizeSrcOffset.cy);
				}
			}
		}
	}

	return bRes;
}


///////////////////////////////////////////////////
CImageShifter::CImageShifter(CSize NewSize, CSize Offset)
{
	m_NewSize = NewSize;
	m_Offset = Offset;
}

CImageShifter::~CImageShifter()
{
}

CSize CImageShifter::CalcDestSize(CSize sizeSrc)
{
	if (sizeSrc.cx > m_NewSize.cx || sizeSrc.cy > m_NewSize.cy)
		return sizeSrc;
	return m_NewSize;
}

BOOL CImageShifter::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	BOOL bRes = TRUE;
	CSize sizeDestOffset(m_Offset.cx, m_Offset.cy);
	CSize sizeSrcOffset(0, 0);

	CRect rSrc(0, 0, sizeSrc.cx , sizeSrc.cy );

	for (int nY = 0; nY < sizeDest.cy; nY++)
	{

		double dSrcX = -sizeDestOffset.cx;
		double dSrcY = nY - sizeDestOffset.cy;

		for (int nX = 0; nX < sizeDest.cx; nX++)
		{
			CPoint ptSrc(round(dSrcX), round(dSrcY));
			int nPixel = (nY * sizeDest.cx + nX);

			if (rSrc.PtInRect(ptSrc))
			{
				if (!m_bWeightingEnabled)
				{
					ptSrc.Offset(sizeSrcOffset);
					RGBX* pRGBSrc = &pSrcPixels[ptSrc.y * sizeSrc.cx + ptSrc.x];
					
					pDestPixels[nPixel] = *pRGBSrc;
				}
				else
					pDestPixels[nPixel] = CalcWeightedColor(pSrcPixels, sizeSrc, dSrcX + sizeSrcOffset.cx, dSrcY + sizeSrcOffset.cy);
			}
			dSrcX ++;
		}
	}
	return bRes;
}


///////

CImageShearer::CImageShearer(int nHorz, int nVert, BOOL bEnableWeighting)
	: C32BitImageProcessor(bEnableWeighting), m_nHorz(nHorz), m_nVert(nVert)
{
}

CImageShearer::~CImageShearer()
{
}

CSize CImageShearer::CalcDestSize(CSize sizeSrc)
{
	return CSize(sizeSrc.cx + abs(m_nHorz), sizeSrc.cy + abs(m_nVert));
}

BOOL CImageShearer::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;

	if (!m_nHorz && !m_nVert)
		bRes = C32BitImageProcessor::ProcessPixels(pSrcPixels, sizeSrc, pDestPixels, sizeDest);
	else
	{
		// shears +ve (down) or -ve (up)
		for (int nX = 0; nX < sizeDest.cx; nX++)
		{
			double dYOffset = 0;

			// calc the offset to src Y coord
			if (m_nVert > 0)
				dYOffset = (double)m_nVert * nX / sizeDest.cx;
				
			else if (m_nVert < 0)
				dYOffset = (double)-m_nVert * (sizeDest.cx - nX) / sizeDest.cx;

			// shears +ve (right) or -ve (left)
			for (int nY = 0; nY < sizeDest.cy; nY++)
			{
				double dXOffset = 0;

				// calc the offset to src X coord
				if (m_nHorz < 0)
					dXOffset = (double)-m_nHorz * nY / sizeDest.cy;
				
				else if (m_nHorz > 0)
					dXOffset = (double)m_nHorz * (sizeDest.cy - nY) / sizeDest.cy;

				double dSrcX = nX - dXOffset;
				double dSrcY = nY - dYOffset;

				if ((int)dSrcX >= 0 && (int)dSrcX < sizeSrc.cx && (int)dSrcY >= 0 && (int)dSrcY < sizeSrc.cy)
				{
					if (!m_bWeightingEnabled)
					{
						RGBX* pRGBSrc = &pSrcPixels[(int)dSrcY * sizeSrc.cx + (int)dSrcX];
						pDestPixels[nY * sizeDest.cx + nX] = *pRGBSrc;
					}
					else
						pDestPixels[nY * sizeDest.cx + nX] = CalcWeightedColor(pSrcPixels, sizeSrc, dSrcX, dSrcY);
				}
			}
		}
	}

	return bRes;
}

///////

CImageGrayer::CImageGrayer()
{
}

CImageGrayer::~CImageGrayer()
{
}

BOOL CImageGrayer::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	ASSERT (sizeSrc == sizeDest);

	DWORD dwTick = GetTickCount();

	for (int nX = 0; nX < sizeSrc.cx; nX++)
	{
		for (int nY = 0; nY < sizeSrc.cy; nY++)
		{
			RGBX* pRGBSrc = &pSrcPixels[nY * sizeSrc.cx + nX];
			RGBX* pRGBDest = &pDestPixels[nY * sizeSrc.cx + nX];

			*pRGBDest = pRGBSrc->Gray();
		}
	}

	return TRUE;
}

///////

CImageBlurrer::CImageBlurrer(int nAmount)
{
	m_nAmount = max(0, nAmount);
	m_nAmount = min(m_nAmount, 10);
}

CImageBlurrer::~CImageBlurrer()
{
}

BOOL CImageBlurrer::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;

	if (m_nAmount == 0)
		bRes = C32BitImageProcessor::ProcessPixels(pSrcPixels, sizeSrc, pDestPixels, sizeDest);
	else
	{
		ASSERT (sizeSrc == sizeDest);

		char cMask[9] = { 1, 1, 1, 
						  1, 5 * (11 - (char)m_nAmount), 1, 
						  1, 1, 1 };

		for (int nX = 0; nX < sizeSrc.cx; nX++)
		{
			for (int nY = 0; nY < sizeSrc.cy; nY++)
			{
				int nRed = 0, nGreen = 0, nBlue = 0, nSubCount = 0, nDivisor = 0;

				for (int nSubX = nX - 1; nSubX <= nX + 1; nSubX++)
				{
					for (int nSubY = nY - 1; nSubY <= nY + 1; nSubY++)
					{
						if (nSubX >= 0 && nSubX < sizeSrc.cx && nSubY >= 0 && nSubY < sizeSrc.cy)
						{
							RGBX* pRGBSub = &pSrcPixels[nSubY * sizeSrc.cx + nSubX];

							nRed += pRGBSub->btRed * cMask[nSubCount];
							nGreen += pRGBSub->btGreen * cMask[nSubCount];
							nBlue += pRGBSub->btBlue * cMask[nSubCount];

							nDivisor += cMask[nSubCount];
						}

						nSubCount++;
					}
				}

				RGBX* pRGBDest = &pDestPixels[nY * sizeDest.cx + nX];

				pRGBDest->btRed = min(255, nRed / nDivisor);
				pRGBDest->btGreen = min(255, nGreen / nDivisor);
				pRGBDest->btBlue = min(255, nBlue / nDivisor);
			}
		}
	}

	return TRUE;
}

///////

CImageSharpener::CImageSharpener(int nAmount)
{
	m_nAmount = max(0, nAmount);
	m_nAmount = min(m_nAmount, 10);
}

CImageSharpener::~CImageSharpener()
{
}

BOOL CImageSharpener::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;

	if (m_nAmount == 0)
		bRes = C32BitImageProcessor::ProcessPixels(pSrcPixels, sizeSrc, pDestPixels, sizeDest);
	else
	{
		ASSERT (sizeSrc == sizeDest);

		double dMinMaxRatio = (double)1 / (1 + (10 - m_nAmount) * 5);

		double dMaxFactor = 1 / (4 * (1 + dMinMaxRatio));
		double dMinFactor = dMaxFactor * dMinMaxRatio;

		double dMask[9] = { -dMinFactor, -dMaxFactor, -dMinFactor, -dMaxFactor,  2, -dMaxFactor, -dMinFactor, -dMaxFactor, -dMinFactor };

		for (int nX = 0; nX < sizeSrc.cx; nX++)
		{
			for (int nY = 0; nY < sizeSrc.cy; nY++)
			{
				if (nX > 0 && nX < sizeSrc.cx - 1 && nY > 0 && nY < sizeSrc.cy - 1)
				{
					double dRed = 0, dGreen = 0, dBlue = 0, dDivisor = 0;
					int nSubCount = 0;
					
					for (int nSubX = nX - 1; nSubX <= nX + 1; nSubX++)
					{
						for (int nSubY = nY - 1; nSubY <= nY + 1; nSubY++)
						{
							RGBX* pRGBSub = &pSrcPixels[nSubY * sizeSrc.cx + nSubX];
							
							dRed += pRGBSub->btRed * dMask[nSubCount];
							dGreen += pRGBSub->btGreen * dMask[nSubCount];
							dBlue += pRGBSub->btBlue * dMask[nSubCount];
							
							nSubCount++;
						}
					}

					RGBX* pRGBDest = &pDestPixels[nY * sizeDest.cx + nX];
					
					dRed = min(255, dRed);
					dGreen = min(255, dGreen);
					dBlue = min(255, dBlue);
					dRed = max(0, dRed);
					dGreen = max(0, dGreen);
					dBlue = max(0, dBlue);

					pRGBDest->btRed = (int)dRed;
					pRGBDest->btGreen = (int)dGreen;
					pRGBDest->btBlue = (int)dBlue;
				}
				else
				{
					pDestPixels[nY * sizeDest.cx + nX] = pSrcPixels[nY * sizeSrc.cx + nX];
				}
			}
		}
	}

	return TRUE;
}

////////
CImageResizer::CImageResizer(double dFactor) : m_dFactor(dFactor)
{
	ASSERT (m_dFactor > 0);

	if (m_dFactor > 1)
		m_bWeightingEnabled = TRUE;
}

CImageResizer::~CImageResizer()
{
}

CSize CImageResizer::CalcDestSize(CSize sizeSrc)
{
	return CSize((int)(sizeSrc.cx * m_dFactor), (int)(sizeSrc.cy * m_dFactor));
}

BOOL CImageResizer::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;

	if (m_dFactor <= 0)
		return FALSE;

	if (m_dFactor == 1)
		bRes = C32BitImageProcessor::ProcessPixels(pSrcPixels, sizeSrc, pDestPixels, sizeDest);

	else if (m_dFactor > 1)
		bRes = Enlarge(pSrcPixels, sizeSrc, pDestPixels, sizeDest);

	else
		bRes = Shrink(pSrcPixels, sizeSrc, pDestPixels, sizeDest);

	return TRUE;
}

BOOL CImageResizer::Enlarge(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	ASSERT (m_dFactor > 1);

	if (m_dFactor <= 1)
		return FALSE;

	double dFactor = 1 / m_dFactor;
	double dXSrc = 0;

	for (int nX = 0; nX < sizeDest.cx; nX++)
	{
		double dYSrc = 0;

		for (int nY = 0; nY < sizeDest.cy; nY++)
		{
			pDestPixels[nY * sizeDest.cx + nX] = CalcWeightedColor(pSrcPixels, sizeSrc, dXSrc, dYSrc);

			dYSrc += dFactor; // next dest pixel in source coords
		}

		dXSrc += dFactor; // next dest pixel in source coords
	}

	return TRUE;
}

BOOL CImageResizer::Shrink(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	ASSERT (m_dFactor < 1 && m_dFactor > 0);

	if (m_dFactor >= 1 || m_dFactor <= 0)
		return FALSE;

	double dFactor = 1 / m_dFactor;
	double dXEnd = -dFactor / 2;
	int nXStart, nXEnd = -1;

	for (int nX = 0; nX < sizeDest.cx; nX++)
	{
		int nYStart, nYEnd = -1;
		double dYEnd = -dFactor / 2;

		nXStart = nXEnd + 1;
		dXEnd += dFactor;
		nXEnd = min(sizeSrc.cx - 1, (int)dXEnd + 1);

		if (nXStart > nXEnd)
			continue;

		for (int nY = 0; nY < sizeDest.cy; nY++)
		{
			nYStart = nYEnd + 1;
			dYEnd += dFactor;
			nYEnd = min(sizeSrc.cy - 1, (int)dYEnd + 1);

			if (nYStart > nYEnd)
				continue;

			int nCount = 0, nRed = 0, nGreen = 0, nBlue = 0;

			// average the pixels over the range
			for (int nXSub = nXStart; nXSub <= nXEnd; nXSub++)
			{
				for (int nYSub = nYStart; nYSub <= nYEnd; nYSub++)
				{
					RGBX* pRGBSrc = &pSrcPixels[nYSub * sizeSrc.cx + nXSub];

					nRed += pRGBSrc->btRed;
					nGreen += pRGBSrc->btGreen;
					nBlue += pRGBSrc->btBlue;
					nCount++;
				}
			}

			RGBX* pRGBDest = &pDestPixels[nY * sizeDest.cx + nX];

			pRGBDest->btRed = nRed / nCount;
			pRGBDest->btGreen = nGreen / nCount;
			pRGBDest->btBlue = nBlue / nCount;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////

CImageNegator::CImageNegator()
{
}

CImageNegator::~CImageNegator()
{
}

BOOL CImageNegator::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	ASSERT (sizeSrc == sizeDest);

	DWORD dwTick = GetTickCount();

	for (int nX = 0; nX < sizeSrc.cx; nX++)
	{
		for (int nY = 0; nY < sizeSrc.cy; nY++)
		{
			RGBX* pRGBSrc = &pSrcPixels[nY * sizeSrc.cx + nX];
			RGBX* pRGBDest = &pDestPixels[nY * sizeSrc.cx + nX];

			pRGBDest->btRed = 255 - pRGBSrc->btRed;
			pRGBDest->btGreen = 255 - pRGBSrc->btGreen;
			pRGBDest->btBlue = 255 - pRGBSrc->btBlue;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////

CImageFlipper::CImageFlipper(BOOL bHorz, BOOL bVert) : m_bHorz(bHorz), m_bVert(bVert)
{
}

CImageFlipper::~CImageFlipper()
{
}

BOOL CImageFlipper::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	ASSERT (sizeSrc == sizeDest);

	DWORD dwTick = GetTickCount();

	for (int nX = 0; nX < sizeSrc.cx; nX++)
	{
		int nDestX = m_bHorz ? sizeDest.cx - nX - 1 : nX;

		for (int nY = 0; nY < sizeSrc.cy; nY++)
		{
			RGBX* pRGBSrc = &pSrcPixels[nY * sizeSrc.cx + nX];

			int nDestY = m_bVert ? sizeDest.cy - nY - 1 : nY;
			RGBX* pRGBDest = &pDestPixels[nDestY * sizeDest.cx + nDestX];

			*pRGBDest = *pRGBSrc;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////

CColorReplacer::CColorReplacer(COLORREF crFrom, COLORREF crTo) : m_crFrom(crFrom), m_crTo(crTo)
{
}

CColorReplacer::~CColorReplacer()
{
}

BOOL CColorReplacer::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;

	if (m_crFrom == m_crTo)
		bRes = C32BitImageProcessor::ProcessPixels(pSrcPixels, sizeSrc, pDestPixels, sizeDest);
	else
	{
		RGBX rgbFrom(m_crFrom), rgbTo(m_crTo);

		for (int nX = 0; nX < sizeSrc.cx; nX++)
		{
			for (int nY = 0; nY < sizeSrc.cy; nY++)
			{
				RGBX* pRGBSrc = &pSrcPixels[nY * sizeSrc.cx + nX];
				RGBX* pRGBDest = &pDestPixels[nY * sizeDest.cx + nX];

				if (pRGBSrc->Equals(rgbFrom))
					*pRGBDest = rgbTo;
				else
					*pRGBDest = *pRGBSrc;
			}
		}
	}
	
	return bRes;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEnBitmap::CEnBitmap(COLORREF crBkgnd) : m_crBkgnd(crBkgnd)
{

}

CEnBitmap::~CEnBitmap()
{

}

BOOL CEnBitmap::LoadImage(UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack)
{
	ASSERT(m_hObject == NULL);      // only attach once, detach on destroy

	if (m_hObject != NULL)
		return FALSE;

	return Attach(LoadImageResource(uIDRes, szResourceType, hInst, crBack == -1 ? m_crBkgnd : crBack));
}

BOOL CEnBitmap::LoadImage(LPCTSTR szImagePath, COLORREF crBack)
{
	ASSERT(m_hObject == NULL);      // only attach once, detach on destroy

	if (m_hObject != NULL)
		return FALSE;

	return Attach(LoadImageFile(szImagePath, crBack == -1 ? m_crBkgnd : crBack));
}

HBITMAP CEnBitmap::LoadImageFile(LPCTSTR szImagePath, COLORREF crBack, int iWidth, int iHeight)
{
	int nType = GetFileType(szImagePath);

	switch (nType)
	{
		case FT_BMP:
			// the reason for this is that i suspect it is more efficient to load
			// bmps this way since it avoids creating device contexts etc that the 
			// IPicture methods requires. that method however is still valuable
			// since it handles other image types and transparency
			return (HBITMAP)::LoadImage(NULL, szImagePath, IMAGE_BITMAP, iWidth, iHeight, LR_LOADFROMFILE);

		case FT_UNKNOWN:
			return NULL;

		default: // all the rest
		{
			USES_CONVERSION;
			IPicture* pPicture = NULL;
			
			HBITMAP hbm = NULL;
			HRESULT hr = OleLoadPicturePath(T2OLE((LPTSTR)szImagePath), NULL, 0, crBack, IID_IPicture, (LPVOID*)&pPicture);
					
			if (pPicture)
			{
				hbm = ExtractBitmap(pPicture, crBack, iWidth, iHeight );
				pPicture->Release();
			}

			return hbm;
		}
	}

	return NULL; // can't get here
}

HBITMAP CEnBitmap::LoadImageResource(UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack)
{
	BYTE* pBuff = NULL;
	int nSize = 0;
	HBITMAP hbm = NULL;

	// first call is to get buffer size
	if (GetResource(MAKEINTRESOURCE(uIDRes), szResourceType, hInst, 0, nSize))
	{
		if (nSize > 0)
		{
			pBuff = new BYTE[nSize];
			
			// this loads it
			if (GetResource(MAKEINTRESOURCE(uIDRes), szResourceType, hInst, pBuff, nSize))
			{
				IPicture* pPicture = LoadFromBuffer(pBuff, nSize);

				if (pPicture)
				{
					hbm = ExtractBitmap(pPicture, crBack);
					pPicture->Release();
				}
			}
			
			delete [] pBuff;
		}
	}

	return hbm;
}

IPicture* CEnBitmap::LoadFromBuffer(BYTE* pBuff, int nSize)
{
	bool bResult = false;

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, nSize);
	void* pData = GlobalLock(hGlobal);
	memcpy(pData, pBuff, nSize);
	GlobalUnlock(hGlobal);

	IStream* pStream = NULL;
	IPicture* pPicture = NULL;

	if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK)
	{
		HRESULT hr = OleLoadPicture(pStream, nSize, FALSE, IID_IPicture, (LPVOID *)&pPicture);
		pStream->Release();
	}

	return pPicture; // caller releases
}

BOOL CEnBitmap::GetResource(LPCTSTR lpName, LPCTSTR lpType, HMODULE hInst, void* pResource, int& nBufSize)
{ 
	HRSRC		hResInfo;
	HANDLE		hRes;
	LPSTR		lpRes	= NULL; 
	int			nLen	= 0;
	bool		bResult	= FALSE;

	// Find the resource
	hResInfo = FindResource(hInst, lpName, lpType);

	if (hResInfo == NULL) 
		return false;

	// Load the resource
	hRes = LoadResource(hInst, hResInfo);

	if (hRes == NULL) 
		return false;

	// Lock the resource
	lpRes = (char*)LockResource(hRes);

	if (lpRes != NULL)
	{ 
		if (pResource == NULL)
		{
			nBufSize = SizeofResource(hInst, hResInfo);
			bResult = true;
		}
		else
		{
			if (nBufSize >= (int)SizeofResource(hInst, hResInfo))
			{
				memcpy(pResource, lpRes, nBufSize);
				bResult = true;
			}
		} 

		UnlockResource(hRes);  
	}

	// Free the resource
	FreeResource(hRes);

	return bResult;
}

HBITMAP CEnBitmap::ExtractBitmap(IPicture* pPicture, COLORREF crBack, int iWidth, int iHeight)
{
	ASSERT(pPicture);

	if (!pPicture)
		return NULL;

	CBitmap bmMem;
	CDC dcMem;
	CDC* pDC = CWnd::GetDesktopWindow()->GetDC();

	if (dcMem.CreateCompatibleDC(pDC))
	{
		long hmWidth = 0;
		long hmHeight = 0;
		int nWidth = 0;
		int nHeight = 0;

		pPicture->get_Width( &hmWidth );
		pPicture->get_Height( &hmHeight );
		// HYS-1090: iWidth and iHeight are taking into account
		if( 0 == iWidth )
		{
			nWidth = MulDiv( hmWidth, pDC->GetDeviceCaps( LOGPIXELSX ), HIMETRIC_INCH );
		}
		else
		{
			nWidth = iWidth;
		}

		if( 0 == iHeight )
		{
			nHeight = MulDiv( hmHeight, pDC->GetDeviceCaps( LOGPIXELSY ), HIMETRIC_INCH );
		}
		else
		{
			nHeight = iHeight;
		}

		if (bmMem.CreateCompatibleBitmap(pDC, nWidth, nHeight))
		{
			CBitmap* pOldBM = dcMem.SelectObject(&bmMem);

			if (crBack != -1)
				dcMem.FillSolidRect(0, 0, nWidth, nHeight, crBack);
			
			HRESULT hr = pPicture->Render(dcMem, 0, 0, nWidth, nHeight, 0, hmHeight, hmWidth, -hmHeight, NULL);
			dcMem.SelectObject(pOldBM);
		}
	}

	CWnd::GetDesktopWindow()->ReleaseDC(pDC);

	return (HBITMAP)bmMem.Detach();
}

int CEnBitmap::GetFileType(LPCTSTR szImagePath)
{
	CString sPath(szImagePath);
	sPath.MakeUpper();

	if (sPath.Find(_T(".BMP")) > 0)
		return FT_BMP;

	else if (sPath.Find(_T(".ICO")) > 0)
		return FT_ICO;

	else if (sPath.Find(_T(".JPG")) > 0 || sPath.Find(_T(".JPEG")) > 0)
		return FT_JPG;

	else if (sPath.Find(_T(".GIF")) > 0)
		return FT_GIF;

	// else
	return FT_UNKNOWN;
}

BOOL CEnBitmap::ShiftImage(CSize NewSize, CSize Offset)
{
	return ProcessImage(&CImageShifter(NewSize, Offset));
}


BOOL CEnBitmap::RotateImage(int nDegrees, BOOL backgroundColor, BOOL bEnableWeighting)
{
	if(backgroundColor != -1)
	{
		m_crBkgnd = backgroundColor;
	}

	return ProcessImage(&CImageRotator(nDegrees, bEnableWeighting));
}

BOOL CEnBitmap::ShearImage(int nHorz, int nVert, BOOL bEnableWeighting)
{
	return ProcessImage(&CImageShearer(nHorz, nVert, bEnableWeighting));
}

BOOL CEnBitmap::GrayImage()
{
	return ProcessImage(&CImageGrayer());
}

BOOL CEnBitmap::BlurImage(int nAmount)
{
	return ProcessImage(&CImageBlurrer(nAmount));
}

BOOL CEnBitmap::SharpenImage(int nAmount)
{
	return ProcessImage(&CImageSharpener(nAmount));
}

BOOL CEnBitmap::ResizeImage(CSize size)
{
	BITMAP BM;
	if (!GetBitmap(&BM))
		return FALSE;
	CSize sizeSrc(BM.bmWidth, BM.bmHeight);
	double dWidthFactor = ((double)size.cx) /((double) sizeSrc.cx);
	double dHeightFactor = ((double) size.cy) /((double) sizeSrc.cy);
	double dFactor = min(dWidthFactor,dHeightFactor);
	if (dFactor <= 0) return false;
	return (ResizeImage(dFactor));
}

BOOL CEnBitmap::ResizeImage(double dFactor)
{
	return ProcessImage(&CImageResizer(dFactor));
}

BOOL CEnBitmap::FlipImage(BOOL bHorz, BOOL bVert)
{
	return ProcessImage(&CImageFlipper(bHorz, bVert));
}

BOOL CEnBitmap::NegateImage()
{
	return ProcessImage(&CImageNegator());
}

BOOL CEnBitmap::ReplaceColor(COLORREF crFrom, COLORREF crTo)
{
	return ProcessImage(&CColorReplacer(crFrom, crTo));
}

BOOL CEnBitmap::ProcessImage(C32BitImageProcessor* pProcessor)
{
	C32BIPArray aProcessors;

	aProcessors.Add(pProcessor);

	return ProcessImage(aProcessors);
}

BOOL CEnBitmap::ProcessImage(C32BIPArray& aProcessors)
{
	ASSERT (GetSafeHandle());

	if (!GetSafeHandle())
		return FALSE;

	if (!aProcessors.GetSize())
		return TRUE;

	int nProcessor, nCount = (int) aProcessors.GetSize();

	// retrieve src and final dest sizes
	BITMAP BM;

	if (!GetBitmap(&BM))
		return FALSE;

	CSize sizeSrc(BM.bmWidth, BM.bmHeight);
	CSize sizeDest(sizeSrc), sizeMax(sizeSrc);

	for (nProcessor = 0; nProcessor < nCount; nProcessor++)
	{
		sizeDest = aProcessors[nProcessor]->CalcDestSize(sizeDest);
		sizeMax = CSize(max(sizeMax.cx, sizeDest.cx), max(sizeMax.cy, sizeDest.cy));
	}

	// prepare src and dest bits
	RGBX* pSrcPixels = GetDIBits32();

	if (!pSrcPixels)
		return FALSE;

	RGBX* pDestPixels = new RGBX[sizeMax.cx * sizeMax.cy];

	if (!pDestPixels)
		return FALSE;

	Fill(pDestPixels, sizeMax, m_crBkgnd);

	BOOL bRes = TRUE;
	sizeDest = sizeSrc;

	// do the processing
	for (nProcessor = 0; bRes && nProcessor < nCount; nProcessor++)
	{
		// if its the second processor or later then we need to copy
		// the previous dest bits back into source.
		// we also need to check that sizeSrc is big enough
		if (nProcessor > 0)
		{
			if (sizeSrc.cx < sizeDest.cx || sizeSrc.cy < sizeDest.cy)
			{
				delete [] pSrcPixels;
				pSrcPixels = new RGBX[sizeDest.cx * sizeDest.cy];
			}

			CopyMemory(pSrcPixels, pDestPixels, sizeDest.cx * 4 * sizeDest.cy); // default
			Fill(pDestPixels, sizeDest, m_crBkgnd);
		}

		sizeSrc = sizeDest;
		sizeDest = aProcessors[nProcessor]->CalcDestSize(sizeSrc);
		
		bRes = aProcessors[nProcessor]->ProcessPixels(pSrcPixels, sizeSrc, pDestPixels, sizeDest);
	}

	// update the bitmap
	if (bRes)
	{
		// set the bits
		HDC hdc = GetDC(NULL);
		HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc, sizeDest.cx, sizeDest.cy);

		if (hbmSrc)
		{
			BITMAPINFO bi;

			if (PrepareBitmapInfo32(bi, hbmSrc))
			{
				if (SetDIBits(hdc, hbmSrc, 0, sizeDest.cy, pDestPixels, &bi, DIB_RGB_COLORS))
				{
					// delete the bitmap and attach new
					DeleteObject();
					bRes = Attach(hbmSrc);
				}
			}

			::ReleaseDC(NULL, hdc);

			if (!bRes)
				::DeleteObject(hbmSrc);
		}
	}

	delete [] pSrcPixels;
	delete [] pDestPixels;

	return bRes;
}

RGBX* CEnBitmap::GetDIBits32()
{
	BITMAPINFO bi;

	int nHeight = PrepareBitmapInfo32(bi);
	
	if (!nHeight)
		return FALSE;

	BYTE* pBits = (BYTE*)new BYTE[bi.bmiHeader.biSizeImage];
	HDC hdc = GetDC(NULL);

	if (!GetDIBits(hdc, (HBITMAP)GetSafeHandle(), 0, nHeight, pBits, &bi, DIB_RGB_COLORS))
	{
		delete[] pBits;
		pBits = NULL;
	}

	::ReleaseDC(NULL, hdc);

	return (RGBX*)pBits;
}

// Retrieve the pixel color
COLORREF CEnBitmap::GetPixel( int x, int y)
{
	CDC dc;
	CClientDC clientDC(0);
	dc.CreateCompatibleDC( &clientDC);

	dc.SelectObject( this);

	return dc.GetPixel(x,y);
}

CRect CEnBitmap::GetSizeImage()
{
	BITMAP BM;
	GetObject(sizeof(BM), &BM);	// Retrieve size of Bitmap
	
	CRect rect;
	rect.left = rect.top = 0;
	rect.bottom = BM.bmHeight;
	rect.right = BM.bmWidth;
	
	return rect;
}
void CEnBitmap::SetBackgroundColor( COLORREF color)
{
	m_crBkgnd = color;
}

BOOL CEnBitmap::PrepareBitmapInfo32(BITMAPINFO& bi, HBITMAP hBitmap)
{
	if (!hBitmap)
		hBitmap = (HBITMAP)GetSafeHandle();

	BITMAP BM;

	if (!::GetObject(hBitmap, sizeof(BM), &BM))
		return FALSE;

	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = BM.bmWidth;
	bi.bmiHeader.biHeight = -BM.bmHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32; // 32 bit
	bi.bmiHeader.biCompression = BI_RGB; // 32 bit
	bi.bmiHeader.biSizeImage = BM.bmWidth * 4 * BM.bmHeight; // 32 bit
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	return BM.bmHeight;
}
BOOL CEnBitmap::AppendImage(CEnBitmap* pBmp, ePosition Pos)
{
	BITMAPINFO bi;
	PrepareBitmapInfo32(bi);
	CSize CurSize(bi.bmiHeader.biWidth,-bi.bmiHeader.biHeight);
	PrepareBitmapInfo32(bi,(HBITMAP) pBmp->GetSafeHandle());
	CSize BmpSize(bi.bmiHeader.biWidth,-bi.bmiHeader.biHeight);
	
	CSize NewSize = CurSize;
	CSize Offset(0,0);

	
	CSize BmpPosInCurImg(0,0); 
	// Create space and move current image to his new position
	switch (Pos)
	{
		case epBottom:
		case epTop:
		{
			NewSize.cx = max(CurSize.cx, BmpSize.cx);	
			NewSize.cy = CurSize.cy+BmpSize.cy;	
			Offset.cx = (NewSize.cx-CurSize.cx)/2;
			Offset.cy = BmpSize.cy;
			
			BmpPosInCurImg.cy = 0;
			BmpPosInCurImg.cx = (NewSize.cx-BmpSize.cx)/2;
			if (Pos == epBottom)
			{
				Offset.cy *= -1;
				BmpPosInCurImg.cy = CurSize.cy;
			}
		}	
		break;
		case epRight:
		case epLeft:
		{
			NewSize.cx = CurSize.cx+BmpSize.cx;	
			NewSize.cy = max(CurSize.cy, BmpSize.cy);	

			Offset.cy = (NewSize.cy-CurSize.cy)/2;
			Offset.cx = BmpSize.cx;

			BmpPosInCurImg.cx = 0;
			BmpPosInCurImg.cy = (NewSize.cy-BmpSize.cy)/2;

			if (Pos == epRight)
			{
				Offset.cx *= -1;
				BmpPosInCurImg.cx = CurSize.cx;
			}
		}
		break;
		default: ASSERT(0);
	}


	ShiftImage(NewSize,Offset);
	PasteCEnBitmap(BmpPosInCurImg.cx,BmpPosInCurImg.cy,pBmp);

	return TRUE;
}

BOOL CEnBitmap::CopyImage(HBITMAP hBitmap)
{
	ASSERT (hBitmap);
	
	if (!hBitmap)
		return FALSE;
	
	BITMAPINFO bi;
	int nHeight = PrepareBitmapInfo32(bi, hBitmap);

	if (!nHeight)
		return FALSE;

	BYTE* pBits = (BYTE*)new BYTE[bi.bmiHeader.biSizeImage];
	HDC hdc = GetDC(NULL);
	BOOL bRes = FALSE;

	if (GetDIBits(hdc, hBitmap, 0, nHeight, pBits, &bi, DIB_RGB_COLORS))
	{
		int nWidth = bi.bmiHeader.biSizeImage / (nHeight * 4);

		HBITMAP hbmDest = ::CreateCompatibleBitmap(hdc, nWidth, nHeight);

		if (hbmDest)
		{
			if (SetDIBits(hdc, hbmDest, 0, nHeight, pBits, &bi, DIB_RGB_COLORS))
			{
				DeleteObject();
				bRes = Attach(hbmDest);
			}
		}
	}

	::ReleaseDC(NULL, hdc);
	delete [] pBits;

	return bRes;
}

BOOL CEnBitmap::CopyImage(CBitmap* pBitmap)
{
	if (!pBitmap)
		return FALSE;

	return CopyImage((HBITMAP)pBitmap->GetSafeHandle());
}

BOOL CEnBitmap::Fill(RGBX* pPixels, CSize size, COLORREF color)
{
	if (!pPixels)
		return FALSE;

	if (color == -1 || color == RGB(255, 255, 255))
		FillMemory(pPixels, size.cx * 4 * size.cy, 255); // white

	else if (color == 0)
		FillMemory(pPixels, size.cx * 4 * size.cy, 0); // black

	else
	{
		// fill the first line with the color
		RGBX* pLine = &pPixels[0];
		int nSize = 1;

		pLine[0] = RGBX(color);

		while (1)
		{
			if (nSize > size.cx)
				break;

			// else
			int nAmount = min(size.cx - nSize, nSize) * 4;

			CopyMemory(&pLine[nSize], pLine, nAmount);
			nSize *= 2;
		}

		// use that line to fill the rest of the block
		int nRow = 1;

		while (1)
		{
			if (nRow > size.cy)
				break;

			// else
			int nAmount = min(size.cy - nRow, nRow) * size.cx * 4;

			CopyMemory(&pPixels[nRow * size.cx], pPixels, nAmount);
			nRow *= 2;
		}
	}

	return TRUE;
}

void CEnBitmap::DrawOnDC(CDC *pDC,CRect PicRect, bool bStretch/*=true*/, bool bTransparent/*=false*/)
{
	COLORREF cBck = GetPixel(0,0);
	CBitmap BigBmp;
	CDC MemDcbb;
	CDC MemDc;
	MemDc.CreateCompatibleDC( pDC );
	CBitmap *pOldBmp = MemDc.SelectObject(this);
	if (!pOldBmp) return;

	BITMAP BM;
	GetObject(sizeof(BM), &BM);			// Retrieve size of Bitmap
	if (!bStretch)
	{
		PicRect.right = PicRect.left+BM.bmWidth;
		PicRect.bottom = PicRect.top+BM.bmHeight;
	}
	//	StretchBlt is not suppoted by all device
	//  Create a big Bitmap in memory, stretch the bitmap in this memory space 
	//	and  use BitBlt to print this new Big bitmap

	BigBmp.CreateCompatibleBitmap( pDC,PicRect.Width(), PicRect.Height() );
	MemDcbb.CreateCompatibleDC( pDC );
	MemDcbb.SelectObject( &BigBmp );
	MemDcbb.StretchBlt( 0, 0, PicRect.Width(), PicRect.Height(), &MemDc, 0, 0, BM.bmWidth, BM.bmHeight, SRCCOPY );

	if(bTransparent)
	{
		pDC->TransparentBlt(PicRect.left,PicRect.top,PicRect.Width(),PicRect.Height(),&MemDcbb,0,0,PicRect.Width(),PicRect.Height(),cBck);
	}
	else
	{
		if (pDC->GetDeviceCaps(RASTERCAPS) &RC_STRETCHBLT) 	
			pDC->StretchBlt(PicRect.left,PicRect.top,PicRect.Width(),PicRect.Height(),&MemDcbb,0,0,PicRect.Width(),PicRect.Height(), SRCCOPY);
		else if (pDC->GetDeviceCaps(RASTERCAPS) & RC_BITBLT) 	
			pDC->BitBlt(PicRect.left,PicRect.top,PicRect.Width(),PicRect.Height(),&MemDcbb,0,0,SRCCOPY);
	}
}

bool CEnBitmap::DrawOnDC32(CDC *pDC,CPoint ptPos, BYTE cstAlpha /*=255*/)
{
	CDC MemDc;
	MemDc.CreateCompatibleDC(pDC);
	CBitmap *pOldBmp = MemDc.SelectObject(this);
	if (!pOldBmp) 
		return false;
	BITMAP BM;
	GetObject(sizeof(BM), &BM);			// Retrieve size of Bitmap
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = cstAlpha;
	bf.AlphaFormat = AC_SRC_ALPHA;
	
	return (TRUE == pDC->AlphaBlend(ptPos.x,ptPos.y,BM.bmWidth,BM.bmHeight,&MemDc,0,0,BM.bmWidth,BM.bmHeight,bf)?true:false);
}

void CEnBitmap::DrawOnDC(CDC *pDC,CPoint ptPos, bool bTransparent/*=false*/)
{ 
	COLORREF cBck = GetPixel(0,0);
	CBitmap BigBmp;
	CDC MemDcbb,MemDc;
	MemDc.CreateCompatibleDC(NULL);
	CBitmap *pOldBmp = MemDc.SelectObject(this);
	if (!pOldBmp) return;

	BITMAP BM;
	GetObject(sizeof(BM), &BM);			// Retrieve size of Bitmap
	if(bTransparent)
		pDC->TransparentBlt(ptPos.x,ptPos.y,BM.bmWidth,BM.bmHeight,&MemDc,0,0,BM.bmWidth,BM.bmHeight,cBck);
	else
	{
		if (pDC->GetDeviceCaps(RASTERCAPS) & RC_STRETCHBLT) 	
			pDC->StretchBlt(ptPos.x,ptPos.y,BM.bmWidth,BM.bmHeight,&MemDc,0,0,BM.bmWidth,BM.bmHeight,SRCCOPY);
		else if (pDC->GetDeviceCaps(RASTERCAPS) & RC_BITBLT) 	
			pDC->BitBlt(ptPos.x,ptPos.y,BM.bmWidth,BM.bmHeight,&MemDc,0,0,SRCCOPY);
	}
}

void CEnBitmap::PasteCEnBitmap(int cx,int cy,CEnBitmap* newCEnBitmap,RGBX *crBCK)
{
	int T;
	if (cx<0 || cy<0)
	{
		ASSERT(0);
		return;
	}
	//initialize variables
	BITMAP BM;
	GetObject(sizeof(BM), &BM); // Retrieve size of Bitmap
	
	BITMAP BMSRC;
	newCEnBitmap->GetObject(sizeof(BMSRC), &BMSRC); // Retrieve size of Bitmap
	
	T= BM.bmWidth*cy+cx ;

	BITMAPINFOHEADER bih;
	
	// Initialize the bitmapinfoheader
	bih.biSize               = sizeof(BITMAPINFOHEADER);
	bih.biWidth              = BM.bmWidth ;
	bih.biHeight             = BM.bmHeight ;
	bih.biPlanes             = 1;
	bih.biBitCount           = BM.bmPlanes * BM.bmBitsPixel;
	bih.biSizeImage          = 0;
	bih.biXPelsPerMeter      = 0;
	bih.biYPelsPerMeter      = 0;
	bih.biClrUsed            = 0;
	bih.biClrImportant       = 0;

	// set the bits
	HDC hdc = GetDC(NULL);
	HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);

	BOOL bRes = TRUE;
    
	CEnBitmap* newSrcCEnBitmap = new CEnBitmap();
	newSrcCEnBitmap->CopyImage(newCEnBitmap);
    
	RGBX* pSrcPixels = newSrcCEnBitmap->GetDIBits32();
	RGBX* pDestPixels = GetDIBits32();


	for (int stepY=0;stepY<BMSRC.bmHeight ;stepY++) // for each line
	{
		if ((cy+stepY)<0)
		{
			stepY=-cy;
		}
		if ((cy+stepY)<BM.bmHeight) // if the height of the current image is less than the height of standard square.
		{
			for (int stepX=0;stepX<BMSRC.bmWidth ;stepX++) // for each columns
			{		
				if ((cx+stepX)<BM.bmWidth) // if the width of the current image is less than the width of standard square.
				{
					int dst=0, src=0;	
					if ((cx+stepX)<0)
					{
						stepX=-cx;
					}

					dst = T + stepY * BM.bmWidth +  stepX; 
					src = stepY * BMSRC.bmWidth + stepX; // The position of the current value is the number of line  calculated * the width + the number of columns calculated.
					if (crBCK)
					{
						if ((pSrcPixels[src].btGreen!=crBCK->btGreen)||(pSrcPixels[src].btRed!=crBCK->btRed)||(pSrcPixels[src].btBlue!=crBCK->btBlue))
						{
							pDestPixels[dst] = pSrcPixels[src];
						}						
					}
					else
					{
						pDestPixels[dst] = pSrcPixels[src];
					}			
				}
				else
					stepX=BMSRC.bmWidth;
			}
		}
		else
			stepY=BMSRC.bmHeight;
	}

	// attach the bitmap.
	if (hbmSrc)
	{
		BITMAPINFO bi;

		if (PrepareBitmapInfo32(bi, hbmSrc))
		{
			if (SetDIBits(hdc, hbmSrc, 0, bih.biHeight, pDestPixels, &bi, DIB_RGB_COLORS))
			{
				// delete the bitmap and attach new
				DeleteObject();
				bRes = Attach(hbmSrc);
			}
		}
		// delete the temporary elements.
		::ReleaseDC(NULL, hdc);

		if (!bRes)
			::DeleteObject(hbmSrc);
	}

	//free(pSrcPixels);
	delete pSrcPixels;
	newSrcCEnBitmap->DeleteObject();
	delete newSrcCEnBitmap;
	delete pDestPixels;
//	free(pDestPixels);
}

BOOL CEnBitmap::SaveAsBMP(LPCTSTR lpFileName)
{
	return SaveAsBMP( (HBITMAP)GetSafeHandle(), lpFileName );

	/*
	HBITMAP hBitmap;
	hBitmap = (HBITMAP) GetSafeHandle();
    BOOL bResult = FALSE;

    PICTDESC stPictDesc;
    stPictDesc.cbSizeofstruct = sizeof(PICTDESC);
    stPictDesc.picType = PICTYPE_BITMAP;
    stPictDesc.bmp.hbitmap = hBitmap;
    stPictDesc.bmp.hpal = NULL;

    LPPICTURE pPicture;
    HRESULT hr = OleCreatePictureIndirect( &stPictDesc, IID_IPicture, FALSE, reinterpret_cast<void**>(&pPicture) );
    if ( SUCCEEDED(hr) )
   {
      LPSTREAM pStream;
      hr = CreateStreamOnHGlobal( NULL, TRUE, &pStream );
      if ( SUCCEEDED(hr) )
      {
         long lBytesStreamed = 0;
         hr = pPicture->SaveAsFile( pStream, TRUE, &lBytesStreamed );
         if ( SUCCEEDED(hr) )
         {
            HANDLE hFile = CreateFile( lpFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
            if ( hFile )
            {
               HGLOBAL hMem = NULL;
               GetHGlobalFromStream( pStream, &hMem );
               LPVOID lpData = GlobalLock( hMem );

               DWORD dwBytesWritten;
               bResult = WriteFile( hFile, lpData, lBytesStreamed, &dwBytesWritten, NULL );
               bResult &= ( dwBytesWritten == (DWORD)lBytesStreamed );

               // clean up
               GlobalUnlock(hMem);
               CloseHandle(hFile);
            }
         }
         // clean up         
         pStream->Release();
      }
      // clean up      
      pPicture->Release();
   }

    return bResult;
	*/
}

BOOL CEnBitmap::SaveAsBMP( HBITMAP hBitmap, LPCTSTR lpFileName)
{
    BOOL bResult = FALSE;

    PICTDESC stPictDesc;
    stPictDesc.cbSizeofstruct = sizeof(PICTDESC);
    stPictDesc.picType = PICTYPE_BITMAP;
    stPictDesc.bmp.hbitmap = hBitmap;
    stPictDesc.bmp.hpal = NULL;

    LPPICTURE pPicture;
    HRESULT hr = OleCreatePictureIndirect( &stPictDesc, IID_IPicture, FALSE, reinterpret_cast<void**>(&pPicture) );

    if( SUCCEEDED( hr ) )
	{
		LPSTREAM pStream;
		hr = CreateStreamOnHGlobal( NULL, TRUE, &pStream );

		if( SUCCEEDED( hr ) )
		{
			long lBytesStreamed = 0;
			hr = pPicture->SaveAsFile( pStream, TRUE, &lBytesStreamed );

			if( SUCCEEDED( hr ) )
			{
				HANDLE hFile = CreateFile( lpFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
				
				if( hFile )
				{
				   HGLOBAL hMem = NULL;
				   GetHGlobalFromStream( pStream, &hMem );
				   LPVOID lpData = GlobalLock( hMem );

				   DWORD dwBytesWritten;
				   bResult = WriteFile( hFile, lpData, lBytesStreamed, &dwBytesWritten, NULL );
				   bResult &= ( dwBytesWritten == (DWORD)lBytesStreamed );

				   // Clean up.
				   GlobalUnlock( hMem );
				   CloseHandle( hFile );
				}
			}

			 // Clean up.
			 pStream->Release();
		}
      
		// Clean up.
		pPicture->Release();
	}

    return bResult;   
}

BOOL CEnBitmap::SaveAsBMP( HDC hMemDC, HBITMAP hBitmap, LPCTSTR lpFileName )
{
	int wBitCount = 32;

	BITMAP Bitmap;
	BITMAPFILEHEADER bmfHdr;
	BITMAPINFOHEADER bi;
 
	::GetObject( hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap );
 
	bi.biSize = sizeof( BITMAPINFOHEADER );
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
 
	DWORD dwBmBitsSize = ( ( Bitmap.bmWidth * wBitCount + 31 ) / 32) * 4 * Bitmap.bmHeight; 
 
	HANDLE hDib = GlobalAlloc( GHND, dwBmBitsSize + sizeof( BITMAPINFOHEADER ) );
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock( hDib );
	*lpbi = bi;
 
	GetDIBits( hMemDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof( BITMAPINFOHEADER ), (BITMAPINFO *)lpbi, DIB_RGB_COLORS );
 
	HANDLE fh = CreateFile( lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	ASSERT(fh != INVALID_HANDLE_VALUE);
 
	DWORD dwDIBSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + dwBmBitsSize;
 
	bmfHdr.bfType = 0x4D42;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
 
	DWORD dwWritten;
 
	WriteFile( fh, (LPSTR)&bmfHdr, sizeof( BITMAPFILEHEADER ), &dwWritten, NULL );
	WriteFile( fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL );
	GlobalUnlock( hDib );
	GlobalFree( hDib );
	CloseHandle( fh );

	return TRUE;
}

void CEnBitmap::DrawText(int x,int y, LPCTSTR lpString, COLORREF crText, CFont *lFont)
{
	if (NULL == lpString)
		return;
	if (*lpString == '\0')
		return;
	CDC dc; 
	dc.CreateCompatibleDC(NULL); 
	if (lFont)
		CFont *pOldFont= dc.SelectObject(lFont);
	
	if (crText)
	{
		dc.SetTextColor(crText);
	}
	dc.SetBkMode(TRANSPARENT);	

	dc.SelectObject(this); 
	dc.TextOut(x,y,lpString); 
}

BOOL CEnBitmap::SaveIntoCStatic( CStatic *pclDestination )
{
	if( NULL == pclDestination )
	{
		return FALSE;
	}

	 BOOL bResult = FALSE;

    PICTDESC stPictDesc;
    stPictDesc.cbSizeofstruct = sizeof( PICTDESC );
    stPictDesc.picType = PICTYPE_BITMAP;
    stPictDesc.bmp.hbitmap = (HBITMAP)GetSafeHandle();
    stPictDesc.bmp.hpal = NULL;

    LPPICTURE pPicture;
    HRESULT hr = OleCreatePictureIndirect( &stPictDesc, IID_IPicture, FALSE, reinterpret_cast<void **>( &pPicture ) );

    if( SUCCEEDED( hr ) )
	{
		LPSTREAM pStream;
		hr = CreateStreamOnHGlobal( NULL, TRUE, &pStream );

		if( SUCCEEDED( hr ) )
		{
			long lBytesStreamed = 0;
			hr = pPicture->SaveAsFile( pStream, TRUE, &lBytesStreamed );

			if( SUCCEEDED( hr ) )
			{
				CImage image;
				image.Load( pStream );
				pclDestination->SetBitmap( image.Detach() );
			}

			 // Clean up.
			 pStream->Release();
		}
      
		// Clean up.
		pPicture->Release();
	}

    return bResult;   
}

void CEnBitmap::SaveImageToFile( LPCTSTR lpFileName )
{
	// CBitmap to file format : bmp, png, gif, jpg
	// the extension defines the format of the file.
	CImage Image; 
	Image.Attach(*this, Image.DIBOR_DEFAULT); 
	Image.Save(lpFileName);
	Image.Detach();
}



	//Translation

	//      W°         x
	//    --------------->
	//    |...      |
	// H° |         |   begin of text coordonate   (0,0)
	//    |         |
	//    |_________|
	//    |
	//    |
	//    |
	// y  |
	//    V

	// £ = 0-->90
	//                  x
	//   --------------->
	//   |    /\)£|  
	//   |   / .\ |        Begin of text coordonate    (H° * sin(£) , 0)
	//   |  /   .\|
	//   | /    / |
	//   |/    /  |
	//   |\   /   |
	//   | \ /    |
	// y |__V_____|
	//	 V

	//90-->180
	//       W'       x
	//   --------------->
	//   |    /\  |  
	//   |   /  \ |         Begin of text coordonate    (W' , -H° * cos (£) )
	//   |  /   .\|_____
	//   | /   ./ |
	//   |/   ./  £
	//   |\   /   |
	//   | \ /    |
	// y |__V_____|
	//	 V

	// £ = 180-->270
	//                  x
	//   --------------->
	//   |    /\)£|  
	//   |   /  \ |        Begin of text coordonate    (-W° * cos(£) , H')
	//   |  /    \|
	// H'| /    / |
	//   |/    /  |
	//   |\.  /   |
	//   | \./    |
	// y |__V_____|
	//	 V


	// 270 --> 360
	//       W'       x
	//   --------------->
	//   |    /\  |  
	//   |   /  \ |         Begin of text coordonate    (0 , -W° * sin(£) )
	//   |  /.   \|
	//   | /.   / |
	//   |/.   /  |
	//   |\   /   |
	//   | \ /    |
	// y |__V_____|
	//	 V

	// now, we have the coordonate of the text, we can translate the begin of the text to the coordonate x,y...
	// x2 = x - Tx
	// y2 = y - Ty
void CEnBitmap::DrawText(int x,int y, int nDegrees, LPCTSTR lpString, COLORREF crText, CFont *lFont)
{
	if (NULL == lpString)
		return;
	if (*lpString == '\0')
		return;

	int nDegrees0to360 = nDegrees%360;  // modulo 360
	if (nDegrees0to360<0) 
		nDegrees0to360 +=360;    // transform negatif number to positif

	double nRadians = nDegrees0to360*M_PI/180; // convert to radians.
							
	// create a temporary Bitmap.
	CEnBitmap *EnBmpTMP= new CEnBitmap();

	// calculation of the text extent.
	CDC dc; 
	dc.CreateCompatibleDC(NULL); 
	if (lFont)
		CFont *pOldFont= dc.SelectObject(lFont);
	if (crText)
		dc.SetTextColor(crText);
	dc.SetBkMode(TRANSPARENT);	

	CSize cs = dc.GetTextExtent(lpString,((CString) lpString).GetLength());

	EnBmpTMP->CreateBitmap(cs.cx+5, cs.cy,1,32,NULL); // +5 to avoid to lose some pixels to the end of the picture.

	EnBmpTMP->NegateImage();
	EnBmpTMP->DrawText(0,0,lpString,crText,lFont);
	EnBmpTMP->RotateImage(nDegrees);

	BITMAP BM2;
	EnBmpTMP->GetObject(sizeof(BM2), &BM2); // Retrieve size of the temporary Bitmap.

	// translation  see comment above (just before void CEnBitmap::DrawText(int x,int y, int nDegrees, LPCTSTR lpString, COLORREF crText, CFont *lFont))
	int x2,y2;
	if (nDegrees0to360<=90)  //rotation : 0° --> 90°
	{
		x2 = Round(x - cs.cy * sin(nRadians));   
		y2 = y;
	}else if (nDegrees0to360<=180) //rotation : 90° --> 180°
	{
		x2 = Round(x - BM2.bmWidth ); 
		y2 = Round(y + cs.cy * cos(nRadians));
	}else if (nDegrees<=270) //rotation : 180° --> 270°
	{
		x2 = Round(x + (cs.cx+5) * cos(nRadians));  // cs.cx+5: width of orignal picture.
		y2 = Round(y - BM2.bmHeight );
	}else   //rotation : 270° --> 360°
	{
		x2 = x; 
		y2 = Round(y + (cs.cx+5) * sin(nRadians)); // cs.cx+5: width of orignal picture.
	}
	y2 = max(0,y2);
	x2 = max(0,x2);
	RGBX *pRGBX = new RGBX(255,255,255);
	PasteCEnBitmap(x2,y2,EnBmpTMP,pRGBX);
	delete (pRGBX);
	delete EnBmpTMP;
}

void CEnBitmap::DrawArrow(CPoint start, CPoint stop)
{
	CPoint d = stop-start;
	double length = sqrt(pow((double)d.x,2)+pow((double)d.y,2));
	double alpha = atan2((double)d.y,(double)d.x)/M_PI*180;
	DrawArrow(start.x, start.y, Round(length), alpha);		
}

void CEnBitmap::DrawArrow(int x, int y, int length, double dDeg)
{
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	dc.SelectObject(this);

	while(dDeg>360)	dDeg -= 360;
	while(dDeg<0)	dDeg += 360;

	double dRad = dDeg*M_PI/180;

	// Draw line
	double dX = x + length*cos(dRad);
	double dY = y + length*sin(dRad);
	dc.MoveTo(x,y);
	dc.LineTo(Round(dX),Round(dY));

	// DrawArrows
	const double dArrowWidth = 20;
	const double dArrowHalfHeight = 10;
	double radius = sqrt(pow(dArrowWidth,2)+pow(dArrowHalfHeight,2));
	double alpha1 = atan2(-dArrowHalfHeight,dArrowWidth)+dRad;
	double alpha2 = atan2(dArrowHalfHeight,dArrowWidth)+dRad;
	for (int i=0; i<2; i++)
	{
		CRgn rgnA;
		CPoint ptVertex[4];
		ptVertex[0].x = x;
		ptVertex[0].y = y;
		ptVertex[1].x = x+Round(radius*cos(alpha1));
		ptVertex[1].y = y+Round(radius*sin(alpha1));
		ptVertex[2].x = x+Round(radius*cos(alpha2));
		ptVertex[2].y = y+Round(radius*sin(alpha2));
		ptVertex[3].x = x;
		ptVertex[3].y = y;

		VERIFY(rgnA.CreatePolygonRgn( ptVertex, 4, ALTERNATE));
		CBrush brA;
		VERIFY(brA.CreateSolidBrush( RGB(0, 0, 0) ));  
		VERIFY(dc.FillRgn( &rgnA, &brA));					
		
		// Return the arrow, and go to the end of line
		x = Round(dX);
		y = Round(dY);
		alpha1+=M_PI;
		alpha2+=M_PI;
	}
}

void CEnBitmap::DrawRect( CRect rect, COLORREF fillColor )
{
	// calculation of the text extent.
	CDC dc; 
	dc.CreateCompatibleDC( NULL );
	dc.SelectObject( this );
	dc.FillSolidRect( rect, fillColor );
}

void CEnBitmap::DrawDimension(LPCTSTR lpString, int x,int y,int length,  int nDegrees, COLORREF crText,CFont *lFont)
{
	int nDegrees0to360 = nDegrees%360;  // modulo 360
	if (nDegrees0to360<0) 
		nDegrees0to360 +=360;    // transform negatif number to positif

	double nRadians = nDegrees0to360*M_PI/180; // convert to radians.

	// create a temporary Bitmap.
	CEnBitmap *EnBmpTMP= new CEnBitmap();  

	// calculation of the text extent.
	CDC dc; 
	dc.CreateCompatibleDC(NULL); 

	if (lFont)
		CFont *pOldFont= dc.SelectObject(lFont);
	if (crText)
		dc.SetTextColor(crText);
	dc.SetBkMode(TRANSPARENT);	

	CSize cs = dc.GetTextExtent(lpString,((CString) lpString).GetLength());

	EnBmpTMP->CreateBitmap(length+5, cs.cy +20,1,32,NULL); // translation  see comment above (just before void CEnBitmap::DrawText(int x,int y, int nDegrees, LPCTSTR lpString, COLORREF crText, CFont *lFont))
											// cs.cy = height of the text . 
											// + 7  : to draw the double arrow.
	EnBmpTMP->NegateImage();
    EnBmpTMP->DrawArrow(0,cs.cy + 2,length);
	EnBmpTMP->DrawText(length/2-cs.cx/2,0,lpString,crText,lFont);  // set text above the arrow (center)
	EnBmpTMP->RotateImage(nDegrees);

	BITMAP BM2;
	EnBmpTMP->GetObject(sizeof(BM2), &BM2); // Retrieve size of Bitmap

	// translation
	int x2,y2;
	if (nDegrees0to360<=90) //rotation : 0° --> 90°
	{
		x2 = Round(x - (cs.cy + 7) * sin(nRadians));     // cs.cy + 7 : height of orignal picture.
		y2 = y;
	}else if (nDegrees0to360<=180) //rotation : 90° --> 180°
	{
		x2 = Round(x - BM2.bmWidth ); 
		y2 = Round(y + (cs.cy + 7) * cos(nRadians));   // cs.cy + 7 : height of orignal picture.
	}else if (nDegrees<=270) //rotation : 180° --> 270° 
	{
		x2 = Round(x + (length+5) * cos(nRadians));   // length+5: width of orignal picture.
		y2 = Round(y - BM2.bmHeight );
	}else  //rotation : 270° --> 360°
	{
		x2 = x; 
		y2 = Round(y + (length+5) * sin(nRadians));  // length+5: width of orignal picture.
	}
	RGBX *pRGBX = new RGBX(255,255,255);
	PasteCEnBitmap(x2,y2,EnBmpTMP,pRGBX);
	delete (pRGBX);
	delete EnBmpTMP;
}
