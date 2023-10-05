/*
$Revision:   1.4  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITGEN/EDITGEN.H_V  $
 * 
 *    Rev 1.4   12 Jun 1990 14:11:12   Dirk
 * No change.
 * 
 *    Rev 1.3   29 May 1990  9:38:22   Randall
 * No change.
 * 
 *    Rev 1.2   29 May 1990  9:20:36   Sonny
 * 
 *    Rev 1.1   21 May 1990 13:28:36   Randall
 * No change.
 * 
 *    Rev 1.0   11 May 1990 16:16:32   Bruce
 * Initial revision.
 * 
 *    Rev 1.0   07 May 1990 10:19:56   Randall
 * Initial revision.
*/

typedef struct tagGenField
{
   HWND  hParent;          // Handle of the parent window
} GENFIELD;
typedef GENFIELD FAR *LPGENFIELD;

BOOL  GenCreateField      (HWND, LPARAM);
void  GenInitializeField  (HWND, LPARAM);
void  GenDestroyField     (HWND);
void  GenLimitText        (HWND hWnd, WPARAM wParam);
