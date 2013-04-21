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
#include "SceneObjectGrid.h"

HRESULT SceneObjectGrid::Initialize(__in_z const wchar_t *nodeName)
{
    m_Resolution = 50;
    m_Size = 50.0f;
	m_Color = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_ParametersChanged = true;
	
	HRESULT hr = CreateMesh(L"gridMesh", &m_spMesh);

	if (SUCCEEDED(hr))
	{
		hr = InitializeMeshStreams(m_spMesh, m_Resolution, m_Size, m_Color);
		
		if (SUCCEEDED(hr))
		{
			hr = SetName(nodeName);
		}
	}

	return hr;
}

HRESULT SceneObjectGrid::InitializeMeshStreams(__in ISceneObjectMesh* pMesh, const XMVECTOR xAxis, const XMVECTOR yAxis, const XMFLOAT3& origin, UINT xResolution,
											   UINT yResolution, XMFLOAT4 color)
{
	if (xResolution == 0 || yResolution == 0 || pMesh == NULL)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

    xResolution = max(1, xResolution);
    yResolution = max(1, yResolution);

    // Build grid geometry
    INT iLineCount = xResolution + yResolution + 2;
	VertexType* pLines = new VertexType[2 * iLineCount];

	if (!pLines)
	{
		hr = E_OUTOFMEMORY;
	}
	else
	{
		XMVECTOR vOrigin = XMLoadFloat3(&origin);

		for (UINT i = 0; i <= xResolution; i++)
		{
			FLOAT fPercent = (FLOAT)i / (FLOAT)xResolution;
			fPercent = (fPercent * 2.0f) - 1.0f;
			
            XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
			vScale = XMVectorAdd( vScale, vOrigin );

			XMStoreFloat3(&pLines[(i * 2)].position, XMVectorSubtract(vScale, yAxis));
            pLines[(i * 2)].color = color;

			XMStoreFloat3(&pLines[(i * 2) + 1].position, XMVectorAdd(vScale, yAxis));
            pLines[(i * 2) + 1].color = color;
		}

		UINT iStartIndex = (xResolution + 1) * 2;
		for (UINT i = 0; i <= yResolution; i++)
		{
			FLOAT fPercent = (FLOAT)i / (FLOAT)yResolution;
			fPercent = (fPercent * 2.0f) - 1.0f;
			
            XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
			vScale = XMVectorAdd(vScale, vOrigin);
			
            XMStoreFloat3(&pLines[(i * 2) + iStartIndex].position, XMVectorSubtract(vScale, xAxis));
            pLines[(i * 2) + iStartIndex].color = color;

			XMStoreFloat3(&pLines[(i * 2) + 1 + iStartIndex].position, XMVectorAdd(vScale, xAxis));
            pLines[(i * 2) + 1 + iStartIndex].color = color;
		}

		SmartPtr<IVertexStream> spVertexStream;
		hr = CreateVertexStream(2 * iLineCount, pLines, &spVertexStream);
		if (SUCCEEDED(hr))
		{
			hr = pMesh->SetMeshStream(MeshStreamType::MST_VERTEX_POSITIONS, spVertexStream.CastTo<IMeshStream>());		
		}

		pMesh->SetPrimitiveTopologyType(PrimitiveTopologyType::PTT_LINELIST);
	}

    delete[] pLines;

	return hr;
}

HRESULT SceneObjectGrid::InitializeMeshStreams(__in ISceneObjectMesh* pMesh, __in UINT resolution, __in FLOAT size, __in XMFLOAT4 color)
{
	if (resolution == 0 || size <= 0 || pMesh == NULL)
	{
		return E_INVALIDARG;
	}

	XMVECTOR xAxis = XMVectorScale(XMLoadFloat3(&XMFLOAT3(1.0f, 0.0f, 0.0f)), size);
	XMVECTOR yAxis = XMVectorScale(XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 1.0f)), size);
	XMFLOAT3 origin(0.0f, 0.0f, 0.0f);

	return InitializeMeshStreams(pMesh, xAxis, yAxis, origin, resolution, resolution, color);
}

HRESULT SceneObjectGrid::SetParameters(__in UINT resolution, __in float size)
{
    if (resolution == 0)
        return E_INVALIDARG;
    if (size > 100.0f)
        return E_INVALIDARG;

    m_Resolution = resolution;
    m_Size = size;
    m_ParametersChanged = true;    

    return Invalidate(SOCT_SHAPE);
}

HRESULT SceneObjectGrid::Update(SceneObjectContentType soct)
{
	if (soct == (SceneObjectContentType::SOCT_SHAPE | SceneObjectContentType::SOCT_OBJECT_ADD)
		&& m_ParametersChanged)
	{
		m_ParametersChanged = false;

		HRESULT hr;

		hr = InitializeMeshStreams(m_spMesh, m_Resolution, m_Size, m_Color);
		
		if (FAILED(hr))
		{
			return hr;
		}
	}

	return SceneObject::Update(soct);
}

HRESULT SceneObjectGrid::GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface)
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

HRESULT CreateGrid(__in_z const wchar_t *nodeName, __deref_out ISceneObjectGrid **ppGrid)
{
    return SceneObjectGrid::CreateInstance(ppGrid, nodeName);
}