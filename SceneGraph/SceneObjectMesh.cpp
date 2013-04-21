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
#include "SceneObjectMesh.h"


SceneObjectMesh::SceneObjectMesh(void)
{
	m_PrimitiveTopologyType = PrimitiveTopologyType::PTT_TRIANGLELIST;
}

SceneObjectMesh::~SceneObjectMesh(void)
{

    m_Streams.Reset();
}

HRESULT SceneObjectMesh::Initialize(__in_z const wchar_t *nodeName)
{
    return SetName(nodeName);
}

HRESULT SceneObjectMesh::GetMeshStream(__in MeshStreamType streamType, __deref_out_opt IMeshStream **ppStream)
{	
	HashPair<MeshStreamType, SmartPtr<IMeshStream>>* hashPair = m_Streams[streamType];

	if (hashPair != NULL)
	{
		hashPair->value.CopyToOpt(ppStream);
		return S_OK;
	}

	return E_FAIL;
}

HRESULT SceneObjectMesh::SetMeshStream(__in MeshStreamType streamType, __in IMeshStream *pStream)
{
	HRESULT hr = S_OK;
	
	HashPair<MeshStreamType, SmartPtr<IMeshStream>>* hashPair = m_Streams[streamType];

	if (hashPair)
	{
		hashPair->value.Attach(pStream);
	}
	else
	{
		SmartPtr<IMeshStream> spMeshStream(pStream);
		hr = m_Streams.Insert(streamType, spMeshStream);
	}

	return hr;
}

HRESULT SceneObjectMesh::RemoveMeshStream(__in MeshStreamType streamType)
{
	HashPair<MeshStreamType, SmartPtr<IMeshStream>>* hashPair = m_Streams[streamType];

	if (hashPair)
	{
		hashPair->value.Release();
		hashPair->value = NULL;
		bool erasedSuccessfully = m_Streams.Erase(hashPair);
		if (!erasedSuccessfully)
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

PrimitiveTopologyType SceneObjectMesh::GetPrimitiveTopologyType()
{
	return m_PrimitiveTopologyType;
}

void SceneObjectMesh::SetPrimitiveTopologyType(PrimitiveTopologyType topologyType)
{
	m_PrimitiveTopologyType = topologyType;
}

HRESULT CreateMesh(__in_z const wchar_t *nodeName, __deref_out ISceneObjectMesh **ppResult)
{
	return SceneObjectMesh::CreateInstance(ppResult, nodeName);
}