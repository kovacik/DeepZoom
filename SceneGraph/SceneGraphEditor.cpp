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


#include "stdafx.h"
#include "SceneGraphEditor.h"

using namespace DirectX;

SceneGraphEditor::SceneGraphEditor()
{
	m_TotalGroupsCount = 0;
}

SceneGraphEditor::~SceneGraphEditor()
{
}

HRESULT SceneGraphEditor::Initialize(__in ISceneGraph *pSceneGraph, __in ISceneObjectCamera *pCamera, const bool &bDrawGrid)
{
	if (!pSceneGraph || !pCamera)
		return E_INVALIDARG;

    HRESULT hr = CreateGroup(L"SceneGraphEditor", &m_spContent);
    IF_FAILED_RETURN(hr);

    SmartPtr<ISceneObjectGroup> spRoot;
    hr = pSceneGraph->GetSceneRoot(&spRoot);
    IF_FAILED_RETURN(hr);

	m_spSceneGraph = pSceneGraph;
    hr = spRoot->AddObject(m_spContent.CastTo<ISceneObject>());
    IF_FAILED_RETURN(hr);

    m_DrawGrid = bDrawGrid;
    m_spCamera = pCamera;

    if (bDrawGrid)
    {
        hr = CreateSceneGrid();
    }

    return hr;
}

HRESULT SceneGraphEditor::Destroy()
{
    for (UINT i = 0; i < m_PendingGroups.Length(); i++)
    {
        m_PendingGroups[i].Release();
        m_PendingGroups[i] = NULL;
    }

    m_PendingGroups.Clear();
    m_spContent.Release();
    m_spSceneGraph.Release();

    return S_OK;
}

HRESULT SceneGraphEditor::ClearSceneGraph()
{
	SmartPtr<ISceneObjectGroup> spRoot;
    HRESULT hr = m_spSceneGraph->GetSceneRoot(&spRoot);
    IF_FAILED_RETURN(hr);

    hr = spRoot->RemoveObject(m_spContent.CastTo<ISceneObject>());
    IF_FAILED_RETURN(hr);

    hr = m_spSceneGraph->Validate();
    IF_FAILED_RETURN(hr);

    hr = m_spContent.CastTo<ISceneObject>()->Destroy();
    IF_FAILED_RETURN(hr);

    m_spContent.Release();
    m_PointCloud.Clear();

    for (UINT i = 0; i < m_PendingGroups.Length(); i++)
    {
        m_PendingGroups[i].Release();
    }

    m_PendingGroups.Clear();

    return Initialize(m_spSceneGraph, m_spCamera, m_DrawGrid);
}

HRESULT SceneGraphEditor::CreateTestTriangle()
{
	SmartPtr<ISceneObjectMesh> spTriangle;
	HRESULT hr = CreateMesh(L"triangle", &spTriangle);

	if (SUCCEEDED(hr))
	{
		VertexType pVertices[3];
		pVertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f); // Bottom left
		pVertices[0].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		
		pVertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
		pVertices[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

		pVertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
		pVertices[2].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

		unsigned long pIndices[3];

		// Load the index array with data.
		pIndices[0] = 0;  // Bottom left.
		pIndices[1] = 1;  // Top middle.
		pIndices[2] = 2;  // Bottom right.

		SmartPtr<IVertexStream> spVertexStream;
		hr = CreateVertexStream(3, pVertices, &spVertexStream);
        IF_FAILED_RETURN(hr);

		SmartPtr<IIndexStream> spIndexStream;
		hr = CreateIndexStream(3, pIndices, &spIndexStream);
        IF_FAILED_RETURN(hr);

		hr = spTriangle->SetMeshStream(MeshStreamType::MST_VERTEX_POSITIONS, spVertexStream.CastTo<IMeshStream>());		
        IF_FAILED_RETURN(hr);

		hr = spTriangle->SetMeshStream(MeshStreamType::MST_TRIANGLES_VERTEX_INDEXES, spIndexStream.CastTo<IMeshStream>());	
        IF_FAILED_RETURN(hr);

		hr = m_spContent->AddObject(spTriangle.CastTo<ISceneObject>());
        IF_FAILED_RETURN(hr);

        hr = m_spSceneGraph->Validate();
	}

    return hr;
}

HRESULT SceneGraphEditor::CreateSceneGrid()
{
	SmartPtr<ISceneObjectGrid> spGrid;
	HRESULT hr = CreateGrid(L"grid", &spGrid);
    IF_FAILED_RETURN(hr);

	hr = m_spContent->AddObject(spGrid.CastTo<ISceneObject>());
    IF_FAILED_RETURN(hr);

    return m_spSceneGraph->Validate();
}

HRESULT SceneGraphEditor::DrawAxes()
{
	SmartPtr<ISceneObjectMesh> spAxes;
	HRESULT hr = CreateMesh(L"axes", &spAxes);
    IF_FAILED_RETURN(hr);

	static const VertexType vertices[] =
	{
		{ XMFLOAT3( 0.0f, 0.0f, 0.0f ), XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f)},
		{ XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f)},
		{ XMFLOAT3( 0.0f, 0.0f, 0.0f ), XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f)},
		{ XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f)},
		{ XMFLOAT3( 0.0f, 0.0f, 0.0f ), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
		{ XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
	};

	SmartPtr<IVertexStream> spVertexStream;
	hr = CreateVertexStream(6, vertices, &spVertexStream);
    IF_FAILED_RETURN(hr);

	hr = spAxes->SetMeshStream(MeshStreamType::MST_VERTEX_POSITIONS, spVertexStream.CastTo<IMeshStream>());		
    IF_FAILED_RETURN(hr);

	spAxes->SetPrimitiveTopologyType(PrimitiveTopologyType::PTT_LINELIST);

	hr = m_spContent->AddObject(spAxes.CastTo<ISceneObject>());
    IF_FAILED_RETURN(hr);

    return m_spSceneGraph->Validate();
}

HRESULT SceneGraphEditor::CreateLineSegment(__in const XMFLOAT3& origin, __in const XMFLOAT3& end, __in const XMFLOAT4& color, __in UINT depth)
{
	SmartPtr<ISceneObjectMesh> spLine;
	HRESULT hr = CreateMesh(L"lineSegment", &spLine);
    IF_FAILED_RETURN(hr);

	VertexType vertices[2];
	vertices[0].position = origin;
	vertices[0].color = color;
	vertices[1].position = end;
	vertices[1].color = color;

	SmartPtr<IVertexStream> spVertexStream;
	hr = CreateVertexStream(2, vertices, &spVertexStream);
	IF_FAILED_RETURN(hr);

	hr = spLine->SetMeshStream(MeshStreamType::MST_VERTEX_POSITIONS, spVertexStream.CastTo<IMeshStream>());		
	IF_FAILED_RETURN(hr);

	spLine->SetPrimitiveTopologyType(PrimitiveTopologyType::PTT_LINELIST);

	return AddToGroup(spLine.CastTo<ISceneObject>(), depth);
}

HRESULT SceneGraphEditor::CreateVector(__in const XMFLOAT3& origin, __in const XMFLOAT3& direction, __in const FLOAT scale, __in const XMFLOAT4& color, __in UINT depth)
{
	SmartPtr<ISceneObjectMesh> spVector;
	HRESULT hr = CreateMesh(L"vector", &spVector);
    IF_FAILED_RETURN(hr);

	XMVECTOR vectorOrigin = XMLoadFloat3( &origin );
	XMVECTOR vectorDirection = XMLoadFloat3( &direction );

	XMVECTOR scaledDirection = XMVectorScale( vectorDirection, scale );
	XMVECTOR vectorEnd = XMVectorAdd( vectorOrigin, scaledDirection );

	VertexType verts[2];
	verts[0].position = origin;
	verts[0].color = color;
	XMStoreFloat3(&(verts[1].position), vectorEnd);
	verts[1].color = color;

	SmartPtr<IVertexStream> spVertexStream;
	hr = CreateVertexStream(2, verts, &spVertexStream);
	IF_FAILED_RETURN(hr);

	hr = spVector->SetMeshStream(MeshStreamType::MST_VERTEX_POSITIONS, spVertexStream.CastTo<IMeshStream>());		
	IF_FAILED_RETURN(hr);

	spVector->SetPrimitiveTopologyType(PrimitiveTopologyType::PTT_LINELIST);

	return AddToGroup(spVector.CastTo<ISceneObject>(), depth);
}

HRESULT SceneGraphEditor::BeginGroup()
{
	HRESULT hr = E_FAIL;

	WCHAR strGroupName[25];
	BOOL groupNameCreated = swprintf_s(strGroupName, 25, L"Group%d_%d", m_PendingGroups.Length(), m_TotalGroupsCount);
	if (groupNameCreated)
	{
		SmartPtr<ISceneObjectGroup> spGroup;
		hr = CreateGroup(L"Group" , &spGroup);
		
		if (SUCCEEDED(hr))
		{
			hr = m_PendingGroups.Add(spGroup);
			m_TotalGroupsCount++;
		}
	}

	return hr;
}

HRESULT SceneGraphEditor::EndGroup()
{
	if (m_PendingGroups.Length() == 0)
	{
		return NTE_BAD_LEN;
	}

	HRESULT hr;

	if (m_PendingGroups.Length() == 1)
	{
		// Add last group to content
		hr = m_spContent->AddObject(m_PendingGroups.GetLast().CastTo<ISceneObject>());
	}
	else
	{
		// Add group to the previous one
		hr = m_PendingGroups[m_PendingGroups.Length() - 1]->AddObject(m_PendingGroups.GetLast().CastTo<ISceneObject>());
	}

	if (SUCCEEDED(hr))
	{
		m_PendingGroups.GetLast().Release();
		m_PendingGroups.GetLast() = NULL;
		bool groupRemoved = m_PendingGroups.Remove(m_PendingGroups.GetLast());
		hr = groupRemoved ? hr : E_FAIL; 
	}

    return m_spSceneGraph->Validate();
}

HRESULT SceneGraphEditor::AddToGroup(__in ISceneObject *pObject, __in UINT depth)
{
	// Invalid depth
	if (m_PendingGroups.Length() != depth)
	{
		return E_INVALIDARG;
	}

	HRESULT hr;

	if (m_PendingGroups.Length() == 0)
	{
		hr = m_spContent->AddObject(pObject);    
	}
	else
	{
		SmartPtr<ISceneObjectGroup> spGroup = m_PendingGroups.GetLast();
		hr = spGroup->AddObject(pObject);
	}

    return m_spSceneGraph->Validate();
}

HRESULT SceneGraphEditor::CreateTextMarker(__in const WCHAR *pText, __in const XMFLOAT3 &position, __in UINT depth)
{
	static const XMFLOAT4 color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	SmartPtr<ISceneObjectText> spText;
	HRESULT hr = CreateText(L"TextMarker", pText, color, position, &spText);
    IF_FAILED_RETURN(hr);

	return AddToGroup(spText.CastTo<ISceneObject>(), depth);
}

HRESULT SceneGraphEditor::AddPointsToPointCloud(__deref_in_ecount(size) VertexType *pPoints, __in UINT length)
{
    if (!pPoints || length == 0)
    {
        return E_INVALIDARG;
    }

	return m_PointCloud.Add(pPoints, length);
}

HRESULT SceneGraphEditor::EndPointCloud( __in UINT depth)
{
    HRESULT hr = S_OK;

    if (m_PointCloud.Length() > 0)
    {
        SmartPtr<ISceneObjectPointCloud> spPointCloud;
        hr = CreatePointCloud(L"PointCloud", m_PointCloud.Ptr(), m_PointCloud.Length(), &spPointCloud);
        IF_FAILED_RETURN(hr);

        m_PointCloud.Clear();

		hr = AddToGroup(spPointCloud.CastTo<ISceneObject>(), depth);
    }

    return hr;
}

HRESULT SceneGraphEditor::AddImage(__in_z const WCHAR *pszFileName)
{
    SmartPtr<ISceneObjectImage> spImage;
    HRESULT hr = CreateImage(L"Image", pszFileName, &spImage);

	if (SUCCEEDED(hr))
	{
		hr = AddToGroup(spImage.CastTo<ISceneObject>(), 0);

        if (SUCCEEDED(hr))
        {
            hr = m_spCamera->CalibrateToSceneObject(spImage.CastTo<ISceneObject>());
        }
    }

    return hr;
}



HRESULT SceneGraphEditor::CreateDepthMapMeshObject(__deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength)
{
    SmartPtr<ISceneObjectDepthMapMesh> spDepthMapMesh;
    HRESULT hr = CreateDepthMapMesh(L"depthmapmeshobject", pPoints, verticesLength, pIndices, indicesLength , &spDepthMapMesh);
    IF_FAILED_RETURN(hr);

	hr = m_spContent->AddObject(spDepthMapMesh.CastTo<ISceneObject>());
    IF_FAILED_RETURN(hr);

    return m_spSceneGraph->Validate();
}

HRESULT CreateSceneGraphEditor(__in ISceneGraph *pSceneGraph, __in ISceneObjectCamera *pCamera, __in bool bDrawMesh, __deref_out ISceneGraphEditor **ppSceneGraphEditor)
{
    if (!pSceneGraph || !ppSceneGraphEditor || !pCamera)
        return E_INVALIDARG;

    if (!ppSceneGraphEditor)
        return E_POINTER;

    return SceneGraphEditor::CreateInstance(ppSceneGraphEditor, pSceneGraph, pCamera, bDrawMesh);
};