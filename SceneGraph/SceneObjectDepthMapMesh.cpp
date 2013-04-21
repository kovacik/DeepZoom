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
#include "SceneObjectDepthMapMesh.h"

HRESULT SceneObjectDepthMapMesh::Initialize(__in_z const wchar_t *nodeName, __deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength)
{
    if (!pPoints || !pIndices || verticesLength == 0 || indicesLength == 0)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = CreateMesh(L"depthmapmesh", &m_spMesh);
    IF_FAILED_RETURN(hr);

    hr = InitializeMeshStreams(m_spMesh, pPoints, verticesLength, pIndices, indicesLength);
	IF_FAILED_RETURN(hr);	

	return SetName(nodeName);
}

HRESULT SceneObjectDepthMapMesh::InitializeMeshStreams(__in ISceneObjectMesh* pMesh, __deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength)
{
	SmartPtr<IVertexStream> spVertexStream;
    HRESULT hr = CreateVertexStream(verticesLength, pPoints, &spVertexStream);
    IF_FAILED_RETURN(hr);

    SmartPtr<IIndexStream> spIndexStream;
    hr = CreateIndexStream(verticesLength, pIndices, &spIndexStream);
    IF_FAILED_RETURN(hr);

    hr = pMesh->SetMeshStream(MeshStreamType::MST_VERTEX_POSITIONS, spVertexStream.CastTo<IMeshStream>());
    IF_FAILED_RETURN(hr);

    hr = pMesh->SetMeshStream(MeshStreamType::MST_TRIANGLES_VERTEX_INDEXES, spIndexStream.CastTo<IMeshStream>());
    IF_FAILED_RETURN(hr);

    pMesh->SetPrimitiveTopologyType(PrimitiveTopologyType::PTT_TRIANGLELIST);
   
	return hr;
}

HRESULT SceneObjectDepthMapMesh::GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface)
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

HRESULT CreateDepthMapMesh(__in_z const wchar_t *nodeName, __deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength, __deref_out ISceneObjectDepthMapMesh **ppGrid)
{
    return SceneObjectDepthMapMesh::CreateInstance(ppGrid, nodeName, pPoints, verticesLength, pIndices, indicesLength);
}