#define	NOMINMAX
#include	<windows.h>
#include	<toolbox.h>
#include    <limits.h>

static HWND	hCap = NULL;
static HWND	hFoc = NULL;
static WORD wHourGlassCounter = 0;

/*
��������������������������������������������������������������������������Ŀ
�Turn on hour glass cursor and capture.												�
����������������������������������������������������������������������������
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
�������������������������������������������������������������������������Ŀ
�Turn off hour glass cursor and release capture.                          �
���������������������������������������������������������������������������
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