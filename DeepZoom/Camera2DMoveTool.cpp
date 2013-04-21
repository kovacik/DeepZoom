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
#include "Camera2DMoveTool.h"

HRESULT Camera2DMoveTool::Initialize()
{
	m_IsDragging = false;
	m_LastButtons = 0;

    return S_OK;
}

HRESULT Camera2DMoveTool::GetCameraFromViewController(__in IViewController *pView, __deref_out_opt ISceneObjectCamera **ppCamera)
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

HRESULT Camera2DMoveTool::GetViewWidthAndHeight(__in IViewController *pView, __out UINT &width, __out UINT &height)
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

HRESULT Camera2DMoveTool::ProcessEvent(UserInputEvent &evnt)
{
	HRESULT hr = S_OK;
	
	ShowCursor(true);

	// Cancel dragging
	if (evnt.buttons == 0)
	{
		m_IsDragging = false;

        SmartPtr<ISceneObjectCamera> spCamera;
		hr = GetCameraFromViewController(evnt.spView, &spCamera);
        IF_FAILED_RETURN(hr);

        hr = spCamera->ApplyViewportOffsetX();
        IF_FAILED_RETURN(hr);

        hr = spCamera->ApplyViewportOffsetY();
        IF_FAILED_RETURN(hr);
	}

	// Store screen midpoint or the last dragged position
	if (evnt.eventType == UserEventType::UET_DRAG)
	{
		m_IsDragging = true;

		m_ClickPoint.x = evnt.x;
		m_ClickPoint.y = evnt.y;

        SmartPtr<ISceneObjectCamera> spCamera;
		hr = GetCameraFromViewController(evnt.spView, &spCamera);
        IF_FAILED_RETURN(hr);

        hr = spCamera->ApplyViewportOffsetX();
        IF_FAILED_RETURN(hr);

        hr = spCamera->ApplyViewportOffsetY();
        IF_FAILED_RETURN(hr);
	}

	// Compute the difference based on the last dragged position
	if (evnt.eventType == UserEventType::UET_MOVE)
	{
		if (m_IsDragging)
		{
            SmartPtr<ISceneObjectCamera> spCamera;
			hr = GetCameraFromViewController(evnt.spView, &spCamera);

			// Check for imidiate changes in direction
			if (abs(((FLOAT)evnt.x - (FLOAT)m_ClickPoint.x ) / ((FLOAT)m_LastPoint.x - (FLOAT)m_ClickPoint.x)) < 1.0f)
			{
				m_ClickPoint.x = evnt.x;
                hr = spCamera->ApplyViewportOffsetX();
                IF_FAILED_RETURN(hr);
			}

			if (abs(((FLOAT)evnt.y - (FLOAT)m_ClickPoint.y ) / ((FLOAT)m_LastPoint.y - (FLOAT)m_ClickPoint.y)) < 1.0f)
			{
				m_ClickPoint.y = evnt.y;
                hr = spCamera->ApplyViewportOffsetY();
                IF_FAILED_RETURN(hr);
			}

			int xDelta = evnt.x - m_ClickPoint.x;
			int yDelta = evnt.y - m_ClickPoint.y;
			
			
			if (SUCCEEDED(hr))
			{	
				if (evnt.buttons & MK_RBUTTON )
				{
                    if (abs(yDelta) > abs(xDelta))
                    {
					    // Change rotation
					    hr = spCamera->Look(0.0f, 0.0f, (0.0001f * yDelta), 0.000f);
                        IF_FAILED_RETURN(hr);
                    }
                    else
                    {
                        // Change rotation
					    hr = spCamera->Look(0.0f, 0.0f, (0.0001f * -xDelta), 0.000f);
                        IF_FAILED_RETURN(hr);
                    }
				}
				if (evnt.buttons & MK_LBUTTON)
				{
					// Move forward and backward
                    hr = spCamera->PanZoomOrthographic((FLOAT)-xDelta, (FLOAT)yDelta, 0.0f);
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
			float camScaleDelta = -30.5f;
			if (evnt.z < 0)
				camScaleDelta = 30.5f;
		
			if (evnt.buttons & MK_LBUTTON)
			{
				// Move up and down
				hr = spCamera->PanZoomOrthographic(0.0f, 0.0f, -camScaleDelta);
			}
			else
			{
                FLOAT width, height; 
                hr = spCamera->GetScreenSize(width, height);
                IF_FAILED_RETURN(hr);

                // Move to place of zoom
                hr = spCamera->PanZoomOrthographic((FLOAT)evnt.x, height - (FLOAT)evnt.y, 0.0f);
                IF_FAILED_RETURN(hr);
                spCamera->ApplyViewportOffsetX();
                IF_FAILED_RETURN(hr);
                spCamera->ApplyViewportOffsetY();
                IF_FAILED_RETURN(hr);

                // Zoom
                hr = spCamera->PanZoomOrthographic(0.0f, 0.0f, camScaleDelta);
                IF_FAILED_RETURN(hr);

                // Move back
                hr = spCamera->PanZoomOrthographic(-(FLOAT)evnt.x, (FLOAT)evnt.y - height, 0.0f);
                IF_FAILED_RETURN(hr);
                spCamera->ApplyViewportOffsetX();
                IF_FAILED_RETURN(hr);
                spCamera->ApplyViewportOffsetY();
                IF_FAILED_RETURN(hr);
			}
		}
	}

	m_LastPoint.x = evnt.x;
	m_LastPoint.y = evnt.y;

	return hr;
}

HRESULT Camera2DMoveTool::Activate(UserInputEvent &evnt)
{
	UNREFERENCED_PARAMETER(evnt);
	return S_OK;
}

HRESULT Camera2DMoveTool::Deactivate(__in_opt IViewController *pView)
{
	UNREFERENCED_PARAMETER(pView);
	return S_OK;
}

HRESULT Camera2DMoveTool::GetToolPriority(UserInputEvent &evnt, UINT *pPriority)
{
	UNREFERENCED_PARAMETER(evnt);
	*pPriority = 0xFFFFFFFF;
    
    return S_OK;
}

HRESULT CreateCamera2DMoveTool(__deref_out ITool **ppTool)
{
	return Camera2DMoveTool::CreateInstance(ppTool);
}