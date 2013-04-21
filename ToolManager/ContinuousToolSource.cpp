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
#include "ContinuousToolSource.h"

HRESULT ContinuousToolSource::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
    return S_OK;
}


HRESULT ContinuousToolSource::AddTool(__in ITool *pTool)
{
    if (!pTool)
        return E_INVALIDARG;

    return m_Tools.Add(pTool);
}

bool ContinuousToolSource::RemoveTool(__in ITool *pTool)
{
    return m_Tools.Remove(SmartPtr<ITool>(pTool));
}

HRESULT ContinuousToolSource::SelectTool(UserInputEvent &evnt, __deref_out_opt ITool **ppTool)
{
    UINT cnt = m_Tools.Length();
    UINT maxPriority = 0;
    UINT maxToolIndex = cnt;
    HRESULT hr;
    for (UINT i = 0; i < cnt; i++)
    {
        UINT priority;
        hr = m_Tools[i]->GetToolPriority(evnt, &priority);
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
        hr = m_Tools[maxToolIndex].CopyTo(ppTool);
    }
    else 
    {
        hr = S_FALSE;
    }
    return hr;
}

HRESULT ContinuousToolSource::OnToolChanged(ITool *pOldTool, ITool *pNewTool)
{
    if (pOldTool && !pNewTool)
    {
        //loosing focus, we shopuld show cursor and stop tracking
        SetCursorPos(m_CapturePoint.x, m_CapturePoint.y);
        ReleaseCapture();
        ShowCursor(true);
    }
    else if (!pOldTool && pNewTool)
    {
        //we gained focus, hide cursor & start tracking
        GetCursorPos(&m_CapturePoint);
        SetCapture(m_hWnd);
        ShowCursor(false);
    }
    return S_OK;
}
