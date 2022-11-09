/////////////////////////////////////////////
//                                         //
// Minimizing C++ Win32 App To System Tray //
//                                         //
// You found this at bobobobo's weblog,    //
// http://bobobobo.wordpress.com           //
//                                         //
// Creation date:  Mar 30/09               //
// Last modified:  Mar 30/09               //
//                                         //
/////////////////////////////////////////////

// GIVING CREDIT WHERE CREDIT IS DUE!!
// Thanks ubergeek!  http://www.gidforums.com/t-5815.html

//#pragma region include and define
#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <direct.h>
#include "resource.h"
#include "systray.h"
#include "loadpref.h"


#ifdef UNICODE
#define stringcopy wcscpy
#else
#define stringcopy strcpy
#endif

//#define ID_TRAY_APP_ICON                5000
//#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
//#define WM_TRAYICON ( WM_USER + 1 )
//#pragma endregion

//#pragma region constants and globals
UINT WM_TASKBARCREATED = 0 ;

HWND g_hwnd ;
HMENU g_menu ;

NOTIFYICONDATA g_notifyIconData ;
//#pragma endregion

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

//#pragma region helper funcs
// These next 2 functions are the STARS of this example.
// They perform the "minimization" function and "restore"
// functions for our window.  Notice how when you "minimize"
// the app, it doesn't really "minimize" at all.  Instead,
// you simply HIDE the window, so it doesn't display, and
// at the same time, stick in a little icon in the system tray,
// so the user can still access the application.
void ConvertWchar(wchar_t a1[], char a2[])
{
	int i;
	for (i = 0; a1[i] != NULL; i++)
		a2[i] = (char)a1[i];
	a2[i] = NULL;
}
void Minimize(HWND g_hwnd)
{
  // add the icon to the system tray
  Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);

  // ..and hide the main window
  ShowWindow(g_hwnd, SW_HIDE);
}

// Basically bring back the window (SHOW IT again)
// and remove the little icon in the system tray.
void Restore(HWND g_hwnd)
{
  // Remove the icon from the system tray
  Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);

  // ..and show the window
  ShowWindow(g_hwnd, SW_SHOW);
}

void Redraw(HWND g_hwnd)
{
	// add the icon to the system tray
	Shell_NotifyIcon(NIM_MODIFY, &g_notifyIconData);

	///ShowWindow(g_hwnd, SW_SHOWNA);
}

void UnloadIcon()
{
	// Free the resources
	DestroyIcon(g_notifyIconData.hIcon);
}

// Initialize the NOTIFYICONDATA structure.
// See MSDN docs http://msdn.microsoft.com/en-us/library/bb773352(VS.85).aspx
// for details on the NOTIFYICONDATA structure.
void InitNotifyIconData(HWND g_hwnd, const wchar_t minimize_icon[], const wchar_t tooltip_txt[])
{
  static char dir[2048];
  static bool first_time = true;


  if (first_time)
  {
	  _getcwd(dir,2048);
	  first_time = false;
  }
  else
  {
	_chdir(dir);
  }

  memset( &g_notifyIconData, 0, sizeof( NOTIFYICONDATA ) ) ;

  g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);

  /////
  // Tie the NOTIFYICONDATA struct to our
  // global HWND (that will have been initialized
  // before calling this function)
  g_notifyIconData.hWnd = g_hwnd;
  // Now GIVE the NOTIFYICON.. the thing that
  // will sit in the system tray, an ID.
  g_notifyIconData.uID = ID_TRAY_APP_ICON;
  // The COMBINATION of HWND and uID form
  // a UNIQUE identifier for EACH ITEM in the
  // system tray.  Windows knows which application
  // each icon in the system tray belongs to
  // by the HWND parameter.
  /////

  /////
  // Set up flags.
  g_notifyIconData.uFlags = NIF_ICON | // promise that the hIcon member WILL BE A VALID ICON!!
    NIF_MESSAGE | // when someone clicks on the system tray icon,
    // we want a WM_ type message to be sent to our WNDPROC
    NIF_TIP;      // we're gonna provide a tooltip as well, son.

  g_notifyIconData.uCallbackMessage = WM_TRAYICON; //this message must be handled in hwnd's window procedure. more info below.

  // Load da icon.  Be sure to include an icon "green_man.ico" .. get one
  // from the internet if you don't have an icon
  char temp[1024];
  ConvertWchar((wchar_t *)minimize_icon, temp);
  //LPCSTR(minimize_icon)
  
  g_notifyIconData.hIcon = (HICON)LoadImageA(NULL, temp, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);


  // set the tooltip text.  must be LESS THAN 64 chars
  
  stringcopy(g_notifyIconData.szTip, LPCTSTR(tooltip_txt));

}

void UpdateNotify(const char str[])
{
	//ConvertString(str,(wchar_t *)szTip)
	//ConvertString((char *)g_notifyIconData,(wchar_t *)szTip, (char *)str);
	///stringcopy(g_notifyIconData.szTip, (char *)str);
	Shell_NotifyIcon(NIM_MODIFY, &g_notifyIconData);
//	wcscpy()
}
