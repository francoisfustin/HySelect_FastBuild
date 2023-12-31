#define	NOMINMAX
#include	<windows.h>
#include	<toolbox.h>
#include    <limits.h>

static HWND	hCap = NULL;
static HWND	hFoc = NULL;
static WORD wHourGlassCounter = 0;

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
쿟urn on hour glass cursor and capture.												�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
쿟urn off hour glass cursor and release capture.                          �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
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
