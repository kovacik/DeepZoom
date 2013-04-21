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
#include "ToolManager.h"

const int MAX_MOVE_DURING_CLICK = 5;
const ULONGLONG MAX_CLICK_DRAG_DURATION = 250;

HRESULT ToolManager::Initialize(__in IViewController *pView)
{
    if (!pView)
        return E_INVALIDARG;

    m_ClickState = MCS_DEFAULT;
    m_spView = pView;
    m_ClickButtons = 0;
    m_LastButtons = 0;

    return S_OK;
}

HRESULT ToolManager::HandleSingleEvent(UINT flags, const POINT &pt, UserEventType evt)
{
    UserInputEvent evnt;
    evnt.spView = m_spView;
    evnt.eventType = evt;
    evnt.buttons = flags;
    evnt.x = pt.x;
    evnt.y = pt.y;
    return ProcessEvent(evnt);
}

HRESULT ToolManager::HandleStateCancel(UINT flags, const POINT &pt, UserEventType event1, UserEventType event2)
{
    HRESULT hr;

    // cancel dblclick and start drag. Emit click and start of dragging
    UserInputEvent evnt;
    evnt.spView = m_spView;
    evnt.eventType = event1;
    evnt.buttons = m_ClickButtons;
    evnt.x = m_ClickPoint.x;
    evnt.y = m_ClickPoint.y;
    hr = ProcessEvent(evnt);

    if (event2 == UET_MOVE)
    {
        if ((m_ClickPoint.x == pt.x) && (m_ClickPoint.y == pt.y))
        {
            return hr;
        }
    }
    evnt.eventType = event2;
    evnt.buttons = flags;
    evnt.x = pt.x;
    evnt.y = pt.y;
    return ProcessEvent(evnt);
}

HRESULT ToolManager::HandleSingleButtonClick(UINT button, UINT flags, const POINT &pt)
{
    HRESULT hr;
    if (m_ClickState == MCS_DEFAULT)
    {
        m_ButtonGesture = button;
        m_ClickButtons = flags;
        m_ClickPoint = pt;
        m_ClickTime = GetTickCount64();
        m_ClickState = MCS_CLICK;
        hr = S_OK;
    }
    else if (m_ClickState == MCS_CLICK_RELEASE)
    {
        if (m_ButtonGesture != button)
        {
            // we clicked different button
            hr = HandleSingleEvent(m_ClickButtons, m_ClickPoint, UET_CLICK);
            m_ButtonGesture = button;
            m_ClickButtons = flags;
            m_ClickPoint = pt;
            m_ClickTime = GetTickCount64();
            m_ClickState = MCS_CLICK;
        }
        else
        {
            // clicked button is the one used for first click
            ULONGLONG tickCount = GetTickCount64() - m_ClickTime;
            int dstX = pt.x - m_ClickPoint.x;
            int dstY = pt.y - m_ClickPoint.y;
            int dst = dstX*dstX + dstY*dstY;
            if ((dst > MAX_MOVE_DURING_CLICK) || (tickCount > MAX_CLICK_DRAG_DURATION))
            {
                hr = HandleStateCancel(flags, pt, UET_CLICK, UET_DRAG);
            }
            else
            {
                hr = HandleSingleEvent(m_ClickButtons, m_ClickPoint, UET_DBLCLICK);
            }
            m_ClickState = MCS_DEFAULT;
        }
    }
    else
    {
        //clicking another button when clicked, cancel first event, start second one
        HandleSingleEvent(m_ClickButtons, m_ClickPoint, UET_MOVE);
        m_ButtonGesture = button;
        m_ClickPoint = pt;
        m_ClickTime = GetTickCount64();
        m_ClickState = MCS_CLICK;
        m_ClickButtons = flags;
        hr = S_OK;
    }
    return hr;
}

HRESULT ToolManager::HandleSingleButtonRelease(UINT button, UINT flags, const POINT &pt)
{
    if (m_ClickState == MCS_DEFAULT)
    {
        HandleSingleEvent(flags, pt, UET_MOVE);
    }
    else if (m_ClickState == MCS_CLICK)
    {
        ULONGLONG currentTickCount =  GetTickCount64();
        ULONGLONG tickCount = currentTickCount - m_ClickTime;
        int dstX = pt.x - m_ClickPoint.x;
        int dstY = pt.y - m_ClickPoint.y;
        int dst = dstX*dstX + dstY*dstY;
        if ((dst > MAX_MOVE_DURING_CLICK) || (tickCount > MAX_CLICK_DRAG_DURATION))
        {
            HandleStateCancel(flags, pt, UET_DRAG, UET_MOVE);
            m_ClickState = MCS_DEFAULT;
        }
        else
        {
            if (m_ButtonGesture != button)
            {
                // we released different button than we started with, emit click & move 
                HandleStateCancel(flags, pt, UET_CLICK, UET_MOVE);

                m_ClickState = MCS_DEFAULT;
            }
            else
            {
                m_ClickState = MCS_CLICK_RELEASE;
                m_ClickTime = currentTickCount;
            }
        }
    }
    else if (m_ClickState == MCS_CLICK_RELEASE)
    {
        //click->release->release --> this must be different button. Cancel click
        HandleStateCancel(flags, pt, UET_CLICK, UET_MOVE);
        m_ClickState = MCS_DEFAULT;
    }
    return S_OK;
}

HRESULT ToolManager::HandleMultiButtonGesture(UINT flags, const POINT &pt)
{
    if (m_ClickState == MCS_DEFAULT)
    {
        HandleSingleEvent(flags, pt, UET_MOVE);
    }
    else if (m_ClickState == MCS_CLICK)
    {
        HandleStateCancel(flags, pt, UET_DRAG, UET_MOVE);
        m_ClickState = MCS_DEFAULT;
    }
    else if (m_ClickState == MCS_CLICK_RELEASE)
    {
        HandleStateCancel(flags, pt, UET_CLICK, UET_MOVE);
        m_ClickState = MCS_DEFAULT;
    }
    return S_OK;
}

HRESULT ToolManager::HandleMoveEvent(UINT flags, const POINT &pt)
{
    HRESULT hr;
    // update state for motion & time 
    if (m_ClickState == MCS_DEFAULT)
    {
        hr = HandleSingleEvent(flags, pt, UET_MOVE);
    }
    else if (m_ClickState == MCS_CLICK)
    {
        //verify motion & time
        ULONGLONG tickCount = GetTickCount64() - m_ClickTime;

        int dstX = pt.x - m_ClickPoint.x;
        int dstY = pt.y - m_ClickPoint.y;
        int dst = dstX*dstX + dstY*dstY;
        if ((dst > MAX_MOVE_DURING_CLICK) || (tickCount > MAX_CLICK_DRAG_DURATION))
        {
            hr = HandleStateCancel(flags, pt, UET_DRAG, UET_MOVE);
            m_ClickState = MCS_DEFAULT;
        }
        else
        {
            //stay in click release wait mode
            hr = S_OK;
        }
    }
    else if (m_ClickState == MCS_CLICK_RELEASE)
    {
        //wait for doubleclick
        ULONGLONG tickCount = GetTickCount64() - m_ClickTime;

        int dstX = pt.x - m_ClickPoint.x;
        int dstY = pt.y - m_ClickPoint.y;
        int dst = dstX*dstX + dstY*dstY;
        if ((dst > MAX_MOVE_DURING_CLICK) || (tickCount > MAX_CLICK_DRAG_DURATION))
        {
            hr = HandleStateCancel(flags, pt, UET_CLICK, UET_MOVE);
            m_ClickState = MCS_DEFAULT;
        }
        else
        {
            //still waiting for release
            hr = S_OK;
        }
    }
    else
    {
        hr = S_OK;
    }
    return hr;
}

HRESULT ToolManager::ProcessMouseEvent(UINT flags, const POINT &pt)
{
    HRESULT hr;

    UINT changes = (m_LastButtons ^ flags) & 7;
    if (changes != 0)
    {
        UINT buttonsClicked = ((~m_LastButtons) & flags) & 7;
        UINT buttonsReleased = ((~flags) & m_LastButtons) & 7;
        UINT clickedPow2 = GetNextPow2(buttonsClicked);
        UINT releasedPow2 = GetNextPow2(buttonsReleased);

        if ((releasedPow2 == 0) && (clickedPow2 == buttonsClicked))
        {
            hr = HandleSingleButtonClick(buttonsClicked, flags, pt);
        }
        else if ((clickedPow2 == 0) && (releasedPow2 == buttonsReleased))
        {
            hr = HandleSingleButtonRelease(buttonsReleased, flags, pt);
        }
        else
        {
            //just do cancel
            hr = HandleMultiButtonGesture(flags, pt);
        }
    }
    else
    {
        
        hr = HandleMoveEvent(flags, pt);
    }

    m_LastButtons = flags;
    m_LastPoint = pt;
    return hr;
}

HRESULT ToolManager::ProcessMouseEvent(UINT flags, const POINT &pt, short zDelta)
{
    HRESULT hr;
    if (m_ClickState == MCS_CLICK)
    {
        hr = HandleSingleEvent(m_ClickButtons, m_ClickPoint, UET_MOVE);
        m_ClickState = MCS_DEFAULT;
    }
    else if (m_ClickState == MCS_CLICK_RELEASE)
    {
        hr = HandleSingleEvent(m_ClickButtons, m_ClickPoint, UET_CLICK);
        m_ClickState = MCS_DEFAULT;
    }
    else
    {
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        UserInputEvent evnt;
        evnt.spView = m_spView;
        evnt.eventType = UET_WHEEL;
        evnt.buttons = flags;
        evnt.x = pt.x;
        evnt.y = pt.y;
        evnt.z = zDelta;
        hr = ProcessEvent(evnt);
    }

    m_LastButtons = flags;
    m_LastPoint = pt;
    return hr;
}

HRESULT ToolManager::OnIdle()
{
    HRESULT hr;
    if (m_ClickState == MCS_DEFAULT)
    {
        hr = S_OK;
    }
    if (m_ClickState == MCS_CLICK)
    {
        ULONGLONG tickCount = GetTickCount64() - m_ClickTime;

        if (tickCount > MAX_CLICK_DRAG_DURATION)
        {
            hr = HandleStateCancel(m_LastButtons, m_LastPoint, UET_DRAG, UET_MOVE);
            m_ClickState = MCS_DEFAULT;
        }
        else
        {
            // wait for timeout
            hr = S_FALSE;
        }
    }
    else if (m_ClickState == MCS_CLICK_RELEASE)
    {
        ULONGLONG tickCount = GetTickCount64() - m_ClickTime;

        if (tickCount > MAX_CLICK_DRAG_DURATION)
        {
            hr = HandleStateCancel(m_LastButtons, m_LastPoint, UET_CLICK, UET_MOVE);
            m_ClickState = MCS_DEFAULT;
        }
        else
        {
            // wait for timeout
            hr = S_FALSE;
        }
    }
    else
    {
        hr = S_OK;
    }
    return hr;
}

HRESULT ToolManager::AddToolSource(IToolSource *pProvider)
{
    return m_ToolSources.Add( pProvider );
}

bool ToolManager::RemoveToolSource(IToolSource *pProvider)
{
    return m_ToolSources.Remove( SmartPtr<IToolSource>(pProvider) );
}

HRESULT ToolManager::PickToolSource(UserInputEvent &evnt)
{
    UINT cnt = m_ToolSources.Length();
    UINT maxPriority = 0;
    UINT maxToolIndex = cnt;
    HRESULT hr;
    for (UINT i = 0; i < cnt; i++)
    {
        UINT priority;
        hr = m_ToolSources[i]->GetToolSourcePriority(evnt, &priority);
        if (SUCCEEDED(hr))
        {
            if (priority > maxPriority)
            {
                maxPriority = priority;
                maxToolIndex = i;
            }
        }
    }

    if (maxToolIndex < cnt)
    {
        hr = SetToolSource( m_ToolSources[maxToolIndex] );
    }
    else
    {
        hr = S_FALSE;
    }
    
    return hr;
}

HRESULT ToolManager::ProcessEvent(UserInputEvent &evnt)
{
    _RPT4(_CRT_WARN, "Process event %d, buttons %d, pos [%d, %d]\n", evnt.eventType, evnt.buttons, evnt.x, evnt.y);

    HRESULT hr = S_OK;
    SmartPtr<ITool> spNextTool;

    if (!m_spActiveTool)
    {
        // no tool active within tool group, pick new tool source
        PickToolSource(evnt);
    }

    if (m_spToolSource)
    {
        // check tool switch 
        hr = m_spToolSource->SelectTool(evnt, &spNextTool);
    }
    if (SUCCEEDED(hr))
    {
        if (spNextTool != m_spActiveTool)
        {
            if (m_spActiveTool)
            {
                m_spActiveTool->Deactivate(m_spView);
            }
            if (spNextTool)
            {
                hr = spNextTool->Activate(evnt);
                if (FAILED(hr))
                {
                    spNextTool.Release();
                }
            }
            if (m_spToolSource)
            {
                m_spToolSource->OnToolChanged(m_spActiveTool, spNextTool);
            }
            m_spActiveTool = spNextTool;
        }
    }

    if (m_spActiveTool != NULL)
    {
        hr = m_spActiveTool->ProcessEvent(evnt);
        if (FAILED(hr))
        {
            m_spActiveTool->Deactivate(m_spView);

            if (m_spToolSource)
            {
                m_spToolSource->OnToolChanged(m_spActiveTool, NULL);
            }
            m_spActiveTool.Release();
        }
    }

    return hr;
}

HRESULT ToolManager::SetToolSource(IToolSource *pToolSource)
{
    if (m_spToolSource && m_spActiveTool)
    {
        m_spActiveTool->Deactivate(m_spView);
        m_spToolSource->OnToolChanged(m_spActiveTool, NULL);
        m_spActiveTool.Release();
    }

    m_spToolSource = pToolSource;

    return S_OK;
}

HRESULT ToolManager::Destroy()
{
    m_spView.Release();
    return S_OK;
}


HRESULT CreateToolManager(__in IViewController *pView, __deref_out IToolManager **ppToolManager)
{
    return ToolManager::CreateInstance(ppToolManager, pView);
}