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

enum MouseClickState
{
    MCS_DEFAULT, 
    MCS_CLICK,
    MCS_CLICK_RELEASE,
};

class ToolManager
    : public ImplementSmartObject<ToolManager, IToolManager>
{
    // mouse event resolver
    MouseClickState m_ClickState;
    ULONGLONG m_ClickTime;
    POINT m_ClickPoint;
    DWORD m_ClickButtons;
    DWORD m_ButtonGesture;

    POINT m_LastPoint;
    DWORD m_LastButtons;
        
    SmartPtr<ITool> m_spActiveTool;
    SmartPtr<IToolSource> m_spToolSource;
    Vector<SmartPtr<IToolSource>> m_ToolSources;
    SmartPtr<IViewController> m_spView;
    
    HRESULT ProcessEvent(UserInputEvent &evnt);
    HRESULT SetToolSource(IToolSource *pToolSource);

    HRESULT PickToolSource(UserInputEvent &evnt);

    HRESULT HandleSingleEvent(UINT flags, const POINT &pt, UserEventType evt);
    HRESULT HandleStateCancel(UINT flags, const POINT &pt, UserEventType event1, UserEventType event2);

    HRESULT HandleSingleButtonClick(UINT button, UINT flags, const POINT &pt);
    HRESULT HandleSingleButtonRelease(UINT button, UINT flags, const POINT &pt);
    HRESULT HandleMultiButtonGesture(UINT flags, const POINT &pt);
    HRESULT HandleMoveEvent(UINT flags, const POINT &pt);

public:
    
    HRESULT ProcessMouseEvent(UINT flags, const POINT &pt);
    HRESULT ProcessMouseEvent(UINT flags, const POINT &pt, short zDelta);
    
    HRESULT Initialize(__in IViewController *pView);

    HRESULT AddToolSource(IToolSource *pToolSource);
    bool RemoveToolSource(IToolSource *pToolSource);

    HRESULT OnIdle();

    HRESULT Destroy();
};

