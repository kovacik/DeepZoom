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

#include "SceneObject.h"

class SceneObjectGroup : public ImplementSmartObject
    <
        SceneObjectGroup, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        InheritImplementation<SceneObject>, 
        ISceneObjectGroup
    >
{
    Vector<SmartPtr<ISceneObject>> m_Children;
public:
    HRESULT Initialize(__in_z const wchar_t *nodeName);
    HRESULT Invalidate(SceneObjectContentType soct);
    HRESULT Destroy();

    HRESULT Register(__in ISceneGraph *pSceneGraph);
    HRESULT Unregister();

    UINT GetObjectCount();
    HRESULT GetObject(__in UINT index, __deref_out ISceneObject **ppSceneObject);

    HRESULT AddObject(__in ISceneObject *pSceneObject);
    HRESULT RemoveObject(__in ISceneObject *pSceneObject);
};

