/*************
* VIEWPICT.H
*************/

#ifndef VIEWPICT_H
#define VIEWPICT_H

#ifdef __cplusplus
extern "C" {
#endif

#define VPM_PAINT (WM_USER + 100)

/**********************
* ViewPict Structures
**********************/

typedef struct viewpict_paint
   {
   BOOL fUseDib;
   RECT Rect;
   RECT RectOrig;
   } VIEWPICT_PAINT, FAR *LPVIEWPICT_PAINT;

typedef struct viewpict_picture
   {
   TCHAR            PictName[100 + 1];
   BITMAPINFOHEADER bi;
   HBITMAP          hBitMap;
   HBITMAP          hBitMap2;
   HANDLE           hPal;
   BOOL             fNoDeletePal;
   long             PictStyle;
   } VIEWPICT_PICTURE, FAR *LPVIEWPICT_PICTURE;

typedef struct viewpict_frame
   {
   BOOL  ShowFrameOnce;
   BOOL  FrameDeleted;
   short PictureIndex;
   short TimeDelay;
   short x;
   short y;
   } VIEWPICT_FRAME, FAR *LPVIEWPICT_FRAME;

typedef struct viewpict
   {
   VIEWPICT_PICTURE Picture;
   GLOBALHANDLE     hGlobalAnimatePictures;
   GLOBALHANDLE     hGlobalAnimateFrames;
   COLORREF         BkColor;
   BOOL             AnimationFrozen;
   BOOL             fNoErase;
   short            AnimationCurFrame;
   short            AnimationTimeDelay;
   short            AnimationCurTimeDelay;
   short            AnimationPictureCnt;
   short            AnimationFrameCnt;
   short            xPointer;
   short            yPointer;
   short            cxClient;
   short            cyClient;
   short            OldcxClient;
   short            OldcyClient;
   short            ScrollIncX;
   short            ScrollIncY;
   short            ScrollWidth;
   short            ScrollHeight;
   short            PictureWidth;
   short            PictureHeight;
   short            CurLine;           /* Top line displayed */
   short            CurCol;            /* Left column displayed */
   long             PictStyle;
   } VIEWPICT, FAR *LPVIEWPICT;

/**********************
* Function Prototypes
**********************/

extern LPVIEWPICT ViewPictGetPtr(HWND hWnd, GLOBALHANDLE FAR *hGlobal);
extern short      ViewGifLoadImage(HWND hWnd, LPVIEWPICT_PICTURE Picture,
                                   LPCTSTR szGIFFile, LPBYTE GifResData,
                                   long GifResDataSize);
extern short      ViewPcxLoadImage(HWND hWnd, LPVIEWPICT_PICTURE Picture,
                                   LPCTSTR FileName, LPBYTE PcxResourceData,
                                   long PcxResourceDataSize);

#ifdef __cplusplus
}
#endif

#endif
