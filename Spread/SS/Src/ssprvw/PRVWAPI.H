/************
* PRVWAPI.H
************/

#ifndef PRVWAPI_H
#define PRVWAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#define PRVWM_BASE       (WM_USER + 1895)

#define PRVWM_OPEN       PRVWM_BASE
#define PRVWM_CLOSE      PRVWM_BASE + 1
#define PRVWM_GETINFO    PRVWM_BASE + 2
#define PRVWM_PRINTPAGE  PRVWM_BASE + 3
#define PRVWM_SETMARGINS PRVWM_BASE + 4

typedef struct prvw_open
   {
   BOOL fAllowMarginResize;
   } PRVW_OPEN, FAR *LPPRVW_OPEN;

typedef struct prvw_margins
   {
   float fMarginLeft;                  // Inches
   float fMarginTop;                   // Inches
   float fMarginRight;                 // Inches
   float fMarginBottom;                // Inches
   } PRVW_MARGINS, FAR *LPPRVW_MARGINS;

typedef struct prvw_info
   {
   HDC          hDCScreen;
   long         lPageBeg;
#ifdef PP40
   HDC          hDCScreen;
#endif
   long         lPageEnd;
   PRVW_MARGINS Margins;
   float        fPageWidth;
   float        fPageHeight;
   } PRVW_INFO, FAR *LPPRVW_INFO;

typedef struct prvw_page
   {
   HDC  hDC;
   RECT Rect;
   long lPage;
   } PRVW_PAGE, FAR *LPPRVW_PAGE;

#ifdef __cplusplus
}
#endif

#endif
