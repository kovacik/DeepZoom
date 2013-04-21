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

__interface IRenderer;

__interface ISceneObject;
__interface ISceneObjectCamera;
__interface ISceneObjectGroup;
__interface ISceneGraph;

enum UserEventType
{
    UET_MOVE = 0,
    UET_DRAG = 1,
    UET_CLICK = 2,
    UET_DBLCLICK = 3,
    UET_WHEEL = 4,
};


// view controller for all views
DECLAREINTERFACE(IViewController, IUnknown, "{D4E08933-25D2-4F4B-BE47-EAEABF20702E}")
{

};

//view controller for renderable scene
DECLAREINTERFACE(IViewControllerScene, IViewController, "{FD3CD767-3F55-4F38-833E-45AD53B9602B}")
{
    HRESULT SetRenderer(__in IRenderer *pRenderer);
    HRESULT GetRenderer(__deref_out_opt IRenderer **ppRenderer);
};


struct UserInputEvent
{
    SmartPtr<IViewController> spView;
    UserEventType eventType;
    UINT buttons;
    INT x,y,z;
};

DECLAREINTERFACE(IRenderer, IUnknown, "{FF92CD62-867C-4502-80D0-30EC6C101E1D}")
{
    HRESULT GetCamera(__deref_out ISceneObjectCamera **ppCamera);
    HRESULT SetCamera(__in ISceneObjectCamera *pCamera);

    void Draw(bool forceRefresh);
    HRESULT Resize(RECT rect);

    void GetWindowWidthAndHeight(__out UINT &width, __out UINT &height);
};

DECLAREINTERFACE(ITool, IUnknown, "{77156B7B-A728-4B4C-B34E-3E676DF97BF9}")
{
    HRESULT ProcessEvent(UserInputEvent &evnt);
    HRESULT Activate(UserInputEvent &evnt);
    HRESULT Deactivate(__in_opt IViewController *pView);

    HRESULT GetToolPriority(UserInputEvent &evnt, UINT *pPriority);
};

DECLAREINTERFACE(IToolSource, IUnknown, "{624C936E-3BC1-4F57-91D1-27DAB606D529}")
{
	HRESULT AddTool(ITool *pTool);
    bool RemoveTool(ITool *pTool);
    HRESULT SelectTool(UserInputEvent &evnt, ITool **ppTool);
    HRESULT GetToolSourcePriority(UserInputEvent &evnt, UINT *pPriority);
    HRESULT OnToolChanged(ITool *pOldTool, ITool *pNewTool);
};

DECLAREINTERFACE(IToolManager, IUnknown, "{9549F57B-00A6-408C-A10B-82214DC91756}")
{
    HRESULT ProcessMouseEvent(UINT flags, const POINT &pt);
    HRESULT ProcessMouseEvent(UINT flags, const POINT &pt, short zDelta);
    
    HRESULT AddToolSource(__in IToolSource *pToolSource);
    bool RemoveToolSource(__in IToolSource *pToolSource);

    HRESULT OnIdle();

    HRESULT Destroy();
};