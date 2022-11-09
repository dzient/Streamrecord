#include "stdafx.h"
#include <afx.h>
#include <afxwin.h>

#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON ( WM_USER + 1 )

#ifndef _SYSTRAY_H
#define _SYSTRAY_H

void Minimize(HWND g_hwnd);
void Restore(HWND g_hwnd);
void Redraw(HWND g_hwnd);
void UnloadIcon();
void InitNotifyIconData(HWND g_hwnd, const wchar_t minimize_icon[], const wchar_t tooltip_txt[]);
void UpdateNotify(const char str[]);

#endif




