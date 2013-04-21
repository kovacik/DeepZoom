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


#pragma once

class Camera2DMoveTool : 
	public ImplementSmartObject
	<
		Camera2DMoveTool, 
		ITool
	>
{
	POINT m_ClickPoint;
	POINT m_LastPoint;

	bool m_IsDragging;
	DWORD m_LastButtons;

	HRESULT GetCameraFromViewController(__in IViewController *pView, __deref_out_opt ISceneObjectCamera **ppCamera);
	HRESULT GetViewWidthAndHeight(__in IViewController *pView, __out UINT &width, __out UINT &height);
public:    

	HRESULT Initialize();
    HRESULT ProcessEvent(UserInputEvent &evnt);
    HRESULT Activate(UserInputEvent &evnt);
    HRESULT Deactivate(__in_opt IViewController *pView);

    HRESULT GetToolPriority(UserInputEvent &evnt, UINT *pPriority);
};