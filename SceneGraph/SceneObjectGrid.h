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

#include "SceneObjectMesh.h"

class SceneObjectGrid : public ImplementSmartObject
     <
        SceneObjectGrid, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        InheritImplementation<SceneObject>,
        ISceneObjectGrid
    >
{
    UINT m_Resolution;
    float m_Size;
    bool m_ParametersChanged;
	XMFLOAT4 m_Color;
	SmartPtr<ISceneObjectMesh> m_spMesh;

	HRESULT InitializeMeshStreams(__in ISceneObjectMesh* pMesh, __in UINT resolution, __in float size, __in XMFLOAT4 color);
	HRESULT InitializeMeshStreams(__in ISceneObjectMesh* pMesh, const XMVECTOR xAxis, const XMVECTOR yAxis, const XMFLOAT3& origin, UINT xResolution,
								  UINT yResolution, XMFLOAT4 color);

public:
    HRESULT Initialize(__in_z const wchar_t *nodeName);

    HRESULT SetParameters(__in UINT resolution, __in FLOAT size);

	HRESULT Update(SceneObjectContentType soct);

    HRESULT GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface);
};

