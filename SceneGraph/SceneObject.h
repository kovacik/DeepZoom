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

using namespace DirectX; 

class SceneObject : 
    public ImplementsInterfaces<ISceneObject>
{
protected:

    XMMATRIX m_LocalTransform;
    XMMATRIX m_WorldTransform;
    SmartPtr<ISceneObject> m_spOwner;
    SmartPtr<ISceneGraph> m_SceneGraph;

    WCHAR *m_ObjectName;
    bool m_Registered;
    SceneObjectContentType m_InvalidContent;

    HRESULT InvalidateInternal(SceneObjectContentType soct);

public:
    SceneObject();
    ~SceneObject();

    HRESULT SetLocalTransform(__in CXMMATRIX transform);

    CXMMATRIX GetLocalTransform() const;
    CXMMATRIX GetWorldTransform() const;

    HRESULT Update(SceneObjectContentType soct);
    HRESULT Validate();
    SceneObjectContentType GetInvalidContentMask();
    HRESULT Invalidate(SceneObjectContentType soct);

    HRESULT Register(__in ISceneGraph *pSceneGraph);
    HRESULT Unregister();

    HRESULT SetName(__in_z const WCHAR* pName);
    const WCHAR* GetName() const;

    HRESULT GetOwner(__deref_out_opt ISceneObject **ppOwner);
    HRESULT SetOwner(__in_opt ISceneObject *pOwner);
    
    HRESULT Destroy();

    HRESULT GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface);
};
