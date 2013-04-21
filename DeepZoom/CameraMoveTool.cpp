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


#include "StdAfx.h"
#include "CameraMoveTool.h"

HRESULT CameraMoveTool::Initialize()
{
	m_IsDragging = false;
	m_LastButtons = 0;

    return S_OK;
}

HRESULT CameraMoveTool::GetCameraFromViewController(__in IViewController *pView, __deref_out_opt ISceneObjectCamera **ppCamera)
{
	HRESULT hr;

	SmartPtr<IViewControllerScene> spViewSceneController;
	hr = pView->QueryInterface(&spViewSceneController);

	if (SUCCEEDED(hr))
	{
		SmartPtr<IRenderer> spRenderer;
		hr = spViewSceneController->GetRenderer(&spRenderer);
		if (SUCCEEDED(hr))
		{
			SmartPtr<ISceneObjectCamera> spCamera;
			hr = spRenderer->GetCamera(&spCamera);

			if (SUCCEEDED(hr))
			{
				hr = spCamera.CopyToOpt(ppCamera);
			}
		}
	}
	return hr;
}

HRESULT CameraMoveTool::GetViewWidthAndHeight(__in IViewController *pView, __out UINT &width, __out UINT &height)
{
	HRESULT hr;

	SmartPtr<IViewControllerScene> spViewSceneController;
	hr = pView->QueryInterface(&spViewSceneController);

	if (SUCCEEDED(hr))
	{
		SmartPtr<IRenderer> spRenderer;
		hr = spViewSceneController->GetRenderer(&spRenderer);
		if (SUCCEEDED(hr))
		{
			spRenderer->GetWindowWidthAndHeight(width, height);
		}
	}
	return hr;
}

HRESULT CameraMoveTool::ProcessEvent(UserInputEvent &evnt)
{
	HRESULT hr = S_OK;
	
	ShowCursor(true);

	// Cancel dragging
	if (evnt.buttons == 0)
	{
		m_IsDragging = false;
	}

	// Store screen midpoint or the last dragged position
	if (evnt.eventType == UserEventType::UET_DRAG)
	{
		m_IsDragging = true;

		m_ClickPoint.x = evnt.x;
		m_ClickPoint.y = evnt.y;
	}

	// Compute the difference based on the last dragged position
	if (evnt.eventType == UserEventType::UET_MOVE)
	{
		if (m_IsDragging)
		{
			// Check for imidiate changes in direction
			if (abs(((FLOAT)evnt.x - (FLOAT)m_ClickPoint.x ) / ((FLOAT)m_LastPoint.x - (FLOAT)m_ClickPoint.x)) < 1.0f)
			{
				m_ClickPoint.x = evnt.x;
			}

			if (abs(((FLOAT)evnt.y - (FLOAT)m_ClickPoint.y ) / ((FLOAT)m_LastPoint.y - (FLOAT)m_ClickPoint.y)) < 1.0f)
			{
				m_ClickPoint.y = evnt.y;
			}

			int xDelta = evnt.x - m_ClickPoint.x;
			int yDelta = evnt.y - m_ClickPoint.y;
	
			SmartPtr<ISceneObjectCamera> spCamera;
			hr = GetCameraFromViewController(evnt.spView, &spCamera);
			
			if (SUCCEEDED(hr))
			{	
				if (evnt.buttons & MK_LBUTTON )
				{
					// Change rotation
					hr = spCamera->Look((0.0001f * xDelta), (0.0001f * yDelta), 0.0f, 0.000f);
                    IF_FAILED_RETURN(hr);
				}
				if (evnt.buttons & MK_RBUTTON)
				{
					// Move forward and backward
					hr = spCamera->Move(0.001f * yDelta, 0.001f * xDelta, 0.0f);
                    IF_FAILED_RETURN(hr);
				}
			}
		}
	}

	if (evnt.eventType == UserEventType::UET_WHEEL)
	{
		SmartPtr<ISceneObjectCamera> spCamera;
		hr = GetCameraFromViewController(evnt.spView, &spCamera);
			
		if (SUCCEEDED(hr))
		{	
			float camScaleDelta = -0.5f;
			if (evnt.z < 0)
				camScaleDelta = 0.5f;
		
			if (evnt.buttons & MK_LBUTTON)
			{
				// Move up and down
				hr = spCamera->Move(0.0f, 0.0f, -camScaleDelta);
			}
			else
			{
				// Change scale
				hr = spCamera->Look(0.0f, 0.0f, 0.0f, camScaleDelta);
			}
		}
	}

	m_LastPoint.x = evnt.x;
	m_LastPoint.y = evnt.y;

	return hr;
}

HRESULT CameraMoveTool::Activate(UserInputEvent &evnt)
{
	UNREFERENCED_PARAMETER(evnt);
	return S_OK;
}

HRESULT CameraMoveTool::Deactivate(__in_opt IViewController *pView)
{
	UNREFERENCED_PARAMETER(pView);
	return S_OK;
}

HRESULT CameraMoveTool::GetToolPriority(UserInputEvent &evnt, UINT *pPriority)
{
	UNREFERENCED_PARAMETER(evnt);
	*pPriority = 0xFFFFFFFF;
    
    return S_OK;
}

HRESULT CreateCameraMoveTool(__deref_out ITool **ppTool)
{
	return CameraMoveTool::CreateInstance(ppTool);
}