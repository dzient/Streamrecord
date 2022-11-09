//--------------------------------------------------------------
// David Zientara
// 10-26-2022
// Streamrecord.h
// Header file for Streamrecord.cpp
// ---------------------------------------------------------
// Streamrecord.h : main header file for the PROJECT_NAME application
//
#include <afxwin.h>
#pragma once


#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CStreamrecordApp:
// See Streamrecord.cpp for the implementation of this class
//

class CStreamrecordApp : public CWinApp
{
public:
	CStreamrecordApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CStreamrecordApp theApp;
