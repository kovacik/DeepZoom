//
// Copyright (C) 2013, Alojz Kovacik, http://kovacik.github.com
//
// This file is part of Deep Zoom.
//
// Deep Zoom is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Deep Zoom is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Deep Zoom. If not, see <http://www.gnu.org/licenses/>.
//



// DeepZoom.h : main header file for the DeepZoom application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CDeepZoomApp:
// See DeepZoom.cpp for the implementation of this class
//

class CDeepZoomApp : public CWinAppEx
{
    SmartPtr<ISceneGraph> m_SceneGraph2D;
    SmartPtr<ISceneObjectCamera> m_spCamera2D;
    SmartPtr<ISceneGraphEditor> m_spSceneGraph2DEditor;

    BOOL InitSceneGraph2D();
    BOOL InitCamera2D();

public:
	CDeepZoomApp();


	virtual BOOL InitInstance();
	virtual int ExitInstance();

    ISceneGraph* GetSceneGraph2D();
    ISceneObjectCamera* GetCamera2D();

// Implementation

public:
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();
    virtual int Run();

    afx_msg void OnAppAbout();
    afx_msg void OnImageOpen();
	DECLARE_MESSAGE_MAP()
};

extern CDeepZoomApp theApp;
