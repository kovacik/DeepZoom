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

class ContinuousToolSource : 
    public ImplementsInterfaces<IToolSource>
{
    Vector<SmartPtr<ITool>> m_Tools;

    HWND m_hWnd;
    POINT m_CapturePoint;
    
public:    
    HRESULT Initialize(HWND hWnd);
    
    HRESULT AddTool(__in ITool *pTool);
    bool RemoveTool(__in ITool *pTool);

    HRESULT SelectTool(UserInputEvent &evnt, __deref_out_opt ITool **ppTool);

    HRESULT OnToolChanged(__in_opt ITool *pOldTool, __in_opt ITool *pNewTool);

};

