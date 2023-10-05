/************
* SS_FONT.H
************/

#ifndef SS_FONT_H
#define SS_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************
* Declare function prototypes
******************************/

LPSS_FONT SS_RetrieveFont(LPSPREADSHEET lpSS, LPSS_FONT Font, LPSS_CELL Cell,
                          SS_COORD CellCol, SS_COORD CellRow);
SS_FONTID SS_RetrieveFontId(LPSPREADSHEET lpSS, LPSS_CELL Cell,
                            SS_COORD CellCol, SS_COORD CellRow);
SS_FONTID SSx_RetrieveFontId(LPSPREADSHEET lpSS, LPSS_COL lpCol,
                             LPSS_ROW lpRow, LPSS_CELL Cell,
                             SS_COORD CellCol, SS_COORD CellRow);
SS_FONTID SS_InitFont(LPSS_BOOK lpBook, HFONT hFont, BOOL fDeleteFont,
                      BOOL fCreateFont, LPBOOL lpfDeleteFontObject);
#if SS_V80
SS_FONTID SS_InitFontLogFont(LPSS_BOOK lpBook, LOGFONT *pLogFont);
#endif
void      SS_FontTableReCreate(LPSS_BOOK lpBook);
SS_FONTID SS_GetFontId(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
LPSS_FONT SS_GetFont(LPSS_BOOK lpBook, LPSS_FONT Font, SS_FONTID FontId);
BOOL      SS_SetFont(LPSS_BOOK lpBook, LPSS_FONT lpFont, SS_FONTID FontId);
BOOL      SS_SetRowMaxFont(LPSPREADSHEET lpSS, LPSS_ROW lpRow, SS_COORD Row, short FontIdNew);
void      SS_DestroyPrinterFonts(LPSPREADSHEET lpSS);
BOOL      SS_FontTableAlloc(void);
void      SS_FontTableDelete(void);
LPSS_FONT SS_FontTableLock(void);
void      SS_FontTableUnlock(void);

#ifdef __cplusplus
}
#endif

#endif
