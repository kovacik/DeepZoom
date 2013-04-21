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
#include "BackgroundToolSource.h"

HRESULT BackgroundToolSource::GetToolSourcePriority(UserInputEvent &evnt, __out UINT *pPriority)
{
    if (evnt.eventType != UET_MOVE)
    {
        *pPriority = 0;
    }
    else
    {
        *pPriority = 0xFFFFFFFF;
    }
    return S_OK;
}

HRESULT CreateBackgroundToolSource(__in HWND hWnd, __deref_out IToolSource **ppBackgroundToolSource)
{
    return BackgroundToolSource::CreateInstance(ppBackgroundToolSource, hWnd);
}