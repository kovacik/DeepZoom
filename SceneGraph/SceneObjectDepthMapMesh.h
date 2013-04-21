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

class SceneObjectDepthMapMesh : public ImplementSmartObject
     <
        SceneObjectDepthMapMesh, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        InheritImplementation<SceneObject>,
        ISceneObjectDepthMapMesh
    >
{
	SmartPtr<ISceneObjectMesh> m_spMesh;

	HRESULT InitializeMeshStreams(__in ISceneObjectMesh* pMesh, __deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength);

public:
    HRESULT Initialize(__in_z const wchar_t *nodeName, __deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength);
    HRESULT GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface);
};

