#define	NOMINMAX
#include	<windows.h>
#include	<toolbox.h>
#include    <limits.h>

static HWND	hCap = NULL;
static HWND	hFoc = NULL;
static WORD wHourGlassCounter = 0;

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³Turn on hour glass cursor and capture.												³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
void	DLLENTRY HourGlassOn()
{
   if (wHourGlassCounter >= USHRT_MAX)
        ;
   else
      if (!wHourGlassCounter++)     // the very first time
   	{
   		hCap = GetFocus();
/*
   		hFoc = GetFocus ();
			SetFocus(NULL);
*/
 			SetCapture(hCap); 
			SetCursor(LoadCursor(NULL,IDC_WAIT));	
      }
}

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³Turn off hour glass cursor and release capture.                          ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
void	DLLENTRY HourGlassOff()
{
   if (!wHourGlassCounter)
        ;
	else
      if (!--wHourGlassCounter)
  		{
/*
			SetFocus(hFoc);
*/
 			ReleaseCapture(); 
			SetCursor(LoadCursor(NULL,IDC_ARROW));	
   	}
}
