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
#include "SceneObjectPointCloud.h"

HRESULT SceneObjectPointCloud::Initialize(__in_z const wchar_t *nodeName, __deref_in_ecount(size) VertexType *pPoints, __in UINT length)
{
	HRESULT hr = CreateMesh(L"pointcloudmesh", &m_spMesh);

	if (SUCCEEDED(hr))
	{
        hr = InitializeMeshStreams(m_spMesh, pPoints, length);
		
		if (SUCCEEDED(hr))
		{
			hr = SetName(nodeName);
		}
	}

	return hr;
}

HRESULT SceneObjectPointCloud::InitializeMeshStreams(__in ISceneObjectMesh* pMesh, __deref_in_ecount(length) VertexType *pPoints, __in UINT length)
{
	if (!pMesh || !pPoints)
	{
		return E_INVALIDARG;
	}

	HRESULT hr;

	SmartPtr<IVertexStream> spVertexStream;
	hr = CreateVertexStream(length, pPoints, &spVertexStream);

	if (SUCCEEDED(hr))
	{
		hr = pMesh->SetMeshStream(MeshStreamType::MST_VERTEX_POSITIONS, spVertexStream.CastTo<IMeshStream>());

        pMesh->SetPrimitiveTopologyType(PrimitiveTopologyType::PTT_POINTLIST);
	}
   
	return hr;
}

HRESULT SceneObjectPointCloud::GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface)
{
	if (ppInterface == NULL)
	{
		return E_INVALIDARG;
	}

	if (IsEqualIID(extensionId, __uuidof(ISceneObjectMesh)))
	{
		return m_spMesh.CopyTo(ppInterface);
	}

    return SceneObject::GetInterface(extensionId, ppInterface);
}

HRESULT CreatePointCloud(__in_z const wchar_t *nodeName, __deref_in_ecount(size) VertexType *pPoints, __in UINT length, __deref_out ISceneObjectPointCloud **ppGrid)
{
    return SceneObjectPointCloud::CreateInstance(ppGrid, nodeName, pPoints, length);
}