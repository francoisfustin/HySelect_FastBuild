// EnBitmap.h: interface for the CEnBitmap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENBITMAP_H__1FDE0A4E_8AB4_11D6_95AD_EFA89432A428__INCLUDED_)
#define AFX_ENBITMAP_H__1FDE0A4E_8AB4_11D6_95AD_EFA89432A428__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>


////////////////////////////////////////////////////////////////////////////////////
// helper struct. equates to COLORREF

#pragma pack(push)
#pragma pack(1)

inline int EB_round(double x)  // function to round a double.
{
	return int(x > 0.0 ? x + 0.5 : x - 0.5);
}

struct RGBX
{
public:
	RGBX() { btRed = btBlue = btGreen = btUnused = 0; }
	RGBX(BYTE red, BYTE green, BYTE blue) { btRed = red; btBlue = blue; btGreen = green; btUnused = 0; }
	RGBX(COLORREF color) { btRed = GetRValue(color); btBlue = GetBValue(color); btGreen = GetGValue(color); btUnused = 0; }

	BYTE btBlue;
	BYTE btGreen;
	BYTE btRed;

protected:
	BYTE btUnused;

public:
	inline BOOL Equals(const RGBX& rgb) { return (btRed == rgb.btRed && btGreen == rgb.btGreen && btBlue == rgb.btBlue); }
	inline BOOL IsGray() const { return (btRed == btGreen && btGreen == btBlue); }

	RGBX Gray() 
	{ 
		BYTE btGray = ((int)btBlue + (int)btGreen * 6 + (int)btRed * 3) / 10;
		return RGBX(btGray, btGray, btGray);
	}

};

#pragma pack(pop)
/////////////////////////////////////////////////////////////////////////////////////
// base class for image processing

class C32BitImageProcessor
{
public:
	C32BitImageProcessor(BOOL bEnableWeighting = FALSE);
	virtual ~C32BitImageProcessor();

	virtual CSize CalcDestSize(CSize sizeSrc);
	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

	inline RGBX CalcWeightedColor(RGBX* pPixels, CSize size, double dX, double dY);

protected:
	BOOL m_bWeightingEnabled;
};

typedef CArray<C32BitImageProcessor*, C32BitImageProcessor*> C32BIPArray;

//////////////////////////////////////////////////////////////////////////////////////
// derived image processors

class CImageRotator : public C32BitImageProcessor
{
public:
	inline int round(double x)  // function to round a double.
	{
		return int(x > 0.0 ? x + 0.5 : x - 0.5);
	}

	CImageRotator(int nDegrees, BOOL bEnableWeighting = TRUE);
	CImageRotator(double dRadians);
	virtual ~CImageRotator();

	virtual CSize CalcDestSize(CSize sizeSrc);
	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

protected:
	double m_dRadians;
};

class CImageShifter : public C32BitImageProcessor
{
public:
	inline int round(double x)  // function to round a double.
	{
		return int(x > 0.0 ? x + 0.5 : x - 0.5);
	}

	CImageShifter(CSize NewSize, CSize NewPos);
	virtual ~CImageShifter();

	virtual CSize CalcDestSize(CSize sizeSrc);
	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

protected:
	CSize m_NewSize;
	CSize m_Offset;
};
class CImageShearer : public C32BitImageProcessor
{
public:
	CImageShearer(int nHorz, int nVert, BOOL bEnableWeighting = TRUE);
	virtual ~CImageShearer();

	virtual CSize CalcDestSize(CSize sizeSrc);
	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

protected:
	int m_nHorz, m_nVert;
};

class CImageGrayer : public C32BitImageProcessor
{
public:
	CImageGrayer();
	virtual ~CImageGrayer();

	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);
};

class CImageBlurrer : public C32BitImageProcessor
{
public:
	CImageBlurrer(int nAmount = 5); // 1 - 10 (10 is very blurred)
	virtual ~CImageBlurrer();

	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

protected:
	int m_nAmount;
};

class CImageSharpener : public C32BitImageProcessor
{
public:
	CImageSharpener(int nAmount = 5); // 1 - 10 (10 is very sharp)
	virtual ~CImageSharpener();

	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

protected:
	int m_nAmount;
};

class CImageResizer : public C32BitImageProcessor
{
public:
	CImageResizer(double dFactor);
	virtual ~CImageResizer();

	virtual CSize CalcDestSize(CSize sizeSrc);
	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

protected:
	double m_dFactor;

protected:
	BOOL Enlarge(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);
	BOOL Shrink(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);
};

class CImageNegator : public C32BitImageProcessor
{
public:
	CImageNegator();
	virtual ~CImageNegator();

	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);
};

class CImageFlipper : public C32BitImageProcessor
{
public:
	CImageFlipper(BOOL bHorz, BOOL bVert = 0); 
	virtual ~CImageFlipper();

	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

protected:
	BOOL m_bHorz, m_bVert;
};

class CColorReplacer : public C32BitImageProcessor
{
public:
	CColorReplacer(COLORREF crFrom, COLORREF crTo);
	virtual ~CColorReplacer();

	virtual BOOL ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest);

protected:
	COLORREF m_crFrom, m_crTo;
};



//////////////////////////////////////////////////////////////////////////////////////////

class  CEnBitmap : public CBitmap  
{
public:
	enum ePosition
	{
		epRight,
		epTop,
		epLeft,
		epBottom
	};
	CEnBitmap(COLORREF crBkgnd = RGB(255, 255, 255));
	virtual ~CEnBitmap();

	BOOL LoadImage(LPCTSTR szImagePath, COLORREF crBack = -1);
	BOOL LoadImage(UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst = NULL, COLORREF crBack = -1); 
	// Add an image to the current image;
	// Increase size if needed
	BOOL AppendImage(CEnBitmap* pBmp, ePosition Pos);
	BOOL CopyImage(HBITMAP hBitmap);
	BOOL CopyImage(CBitmap* pBitmap);

	BOOL ShiftImage(CSize NewSize, CSize Offset);
	BOOL RotateImage(int nDegrees, BOOL backgroundColor = -1, BOOL bEnableWeighting = FALSE); // rotates about centrepoint, +ve (clockwise) or -ve (anticlockwise) from 12 o'clock
	BOOL ShearImage(int nHorz, int nVert, BOOL bEnableWeighting = FALSE); // shears +ve (right, down) or -ve (left, up)
	BOOL GrayImage();
	BOOL BlurImage(int nAmount = 5);
	BOOL SharpenImage(int nAmount = 5);
	BOOL ResizeImage(CSize size /*Width, Height*/);
	BOOL ResizeImage(double dFactor);
	BOOL FlipImage(BOOL bHorz, BOOL bVert = 0);
	BOOL NegateImage();
	BOOL ReplaceColor(COLORREF crFrom, COLORREF crTo);

	BOOL ProcessImage(C32BitImageProcessor* pProcessor);
	BOOL ProcessImage(C32BIPArray& aProcessors); // ordered list of processors
	void DrawOnDC(CDC *pDC,CRect PicRect, bool bStretch=true, bool bTransparent=false);
	void DrawOnDC(CDC *pDC,CPoint ptPos, bool bTransparent=false);
	bool DrawOnDC32(CDC *pDC,CPoint ptPos, BYTE cstAlpha=255);

	// helpers
	static HBITMAP LoadImageFile( LPCTSTR szImagePath, COLORREF crBack = -1, int iWidth = 0, int iHeight = 0 );
	static HBITMAP LoadImageResource( UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst = NULL, COLORREF crBack = 0 );
	static BOOL GetResource( LPCTSTR lpName, LPCTSTR lpType, HMODULE hInst, void *pResource, int &nBufSize );
	static IPicture *LoadFromBuffer( BYTE *pBuff, int nSize );
	RGBX *GetDIBits32();
	BOOL PrepareBitmapInfo32( BITMAPINFO& bi, HBITMAP hBitmap = NULL );


	// PasteCEnBitmap : insert the newCEnBitmap in the current bitmap.
	// if cfBCK is complete is the "TRANSPARENT" color. ... the pixel in this color are not paste in the current image.
	// cx, cy are the coordonate of the pasted picture.
	void PasteCEnBitmap( int cx, int cy, CEnBitmap *newCEnBitmap, RGBX *crBCK = NULL );

	// Save the current bitmap to the lpFileName (BMP format).
	BOOL SaveAsBMP( LPCTSTR lpFileName );

	// Save the bitmap passed as argument to the lpFileName (BMP format).
	BOOL SaveAsBMP( HBITMAP hBitmap, LPCTSTR lpFileName );

	// Save the bitmap passed as argument and with its device context to the lpFileName (BMP format).
	BOOL SaveAsBMP( HDC hMemDC, HBITMAP hBitmap, LPCTSTR lpFileName );

	// Allow to save the current internal bitmap to a CStatic.
	// Remark: I do not found the solution to display a 32bits bmp image on a CStatic.
	//         This is why here we use the same code as the "SaveAsBMP( HBITMAP hBitmap, LPCTSTR lpFileName )" method to have
	//         the image to a good format.
	BOOL SaveIntoCStatic( CStatic *pclDestination );

	// save the current bitmap to the lpFileName (the extension determines de format of the file).
	void SaveImageToFile( LPCTSTR lpFileName );

	// add an horizontal text (lpString), with the color (crText) and the font (lFont) at the coordonate (x,y) 
	// in the current Bitmap.
	void DrawText( int x, int y, LPCTSTR lpString, COLORREF crText = 0, CFont *lFont = NULL );

	// add a text in the orientation nDegrees.
	void DrawText( int x, int y, int nDegrees, LPCTSTR lpString, COLORREF crText = 0, CFont *lFont = NULL );

	// draw a horizontal double arrow (<---->). length is the length of the arrow  (|<-------->|)
	//                                                                               --length--
	void DrawArrow( int x, int y, int length, double dDeg = 0 );	
	void DrawArrow( CPoint start, CPoint stop );

	void DrawRect( CRect rect, COLORREF fillColor );

	// draw a centered text above a double arrow in the direction nDegrees.
	void DrawDimension( LPCTSTR lpString, int x, int y, int length, int nDegrees = 0, COLORREF crText = 0, CFont *lFont = NULL );

	COLORREF GetPixel( int x, int y );
	CRect GetSizeImage();

	void SetBackgroundColor( COLORREF color);
	COLORREF GetBackgroundColor() { return m_crBkgnd; }

	inline int Round(double x)   // function to round a double.
	{
		return int(x > 0.0 ? x + 0.5 : x - 0.5);
	}

	static BOOL Fill(RGBX* pPixels, CSize size, COLORREF color);

protected:
	COLORREF m_crBkgnd;

protected:
	static HBITMAP ExtractBitmap( IPicture *pPicture, COLORREF crBack, int iWidth = 0, int iHeight = 0 );
	static int GetFileType( LPCTSTR szImagePath );
};

// inline weighting function
inline RGBX C32BitImageProcessor::CalcWeightedColor(RGBX* pPixels, CSize size, double dX, double dY)
{
	ASSERT (m_bWeightingEnabled);

	// interpolate between the current pixel and its pixel to the right and down
	int nX = (int)dX;
	int nY = (int)dY;

	if (dX < 0 || dY < 0)
		return pPixels[max(0, nY) * size.cx + max(0, nX)]; // closest

	RGBX* pRGB = &pPixels[nY * size.cx + nX]; // current

	double dXFraction = dX - nX;
	double dX1MinusFraction = 1 - dXFraction;
	
	double dYFraction = dY - nY;
	double dY1MinusFraction = 1 - dYFraction;
	
	RGBX* pRGBXP = &pPixels[nY * size.cx + min(nX + 1, size.cx - 1)]; // x + 1
	RGBX* pRGBYP = &pPixels[min(nY + 1, size.cy - 1) * size.cx + nX]; // y + 1
	
	int nRed = (int)((dX1MinusFraction * pRGB->btRed +
						dXFraction * pRGBXP->btRed +
						dY1MinusFraction * pRGB->btRed +
						dYFraction * pRGBYP->btRed) / 2);
	
	int nGreen = (int)((dX1MinusFraction * pRGB->btGreen +
						dXFraction * pRGBXP->btGreen +
						dY1MinusFraction * pRGB->btGreen +
						dYFraction * pRGBYP->btGreen) / 2);
	
	int nBlue = (int)((dX1MinusFraction * pRGB->btBlue +
						dXFraction * pRGBXP->btBlue +
						dY1MinusFraction * pRGB->btBlue +
						dYFraction * pRGBYP->btBlue) / 2);

	return RGBX(nRed, nGreen, nBlue);
}
#endif // !defined(AFX_ENBITMAP_H__1FDE0A4E_8AB4_11D6_95AD_EFA89432A428__INCLUDED_)
