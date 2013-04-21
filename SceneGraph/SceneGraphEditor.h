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

class SceneGraphEditor : public ImplementSmartObject
    <
        SceneGraphEditor, 
        ISceneGraphEditor
    >
{
	SmartPtr<ISceneGraph> m_spSceneGraph;
    SmartPtr<ISceneObjectGroup> m_spContent;
    SmartPtr<ISceneObjectCamera> m_spCamera;
	Vector<SmartPtr<ISceneObjectGroup>> m_PendingGroups;
	UINT m_TotalGroupsCount;
    bool m_DrawGrid;

    Vector<VertexType> m_PointCloud;

	
	HRESULT AddToGroup(__in ISceneObject *pObject, __in UINT depth);

public:
    SceneGraphEditor();
    ~SceneGraphEditor();

    HRESULT Initialize(__in ISceneGraph *pSceneGraph, __in ISceneObjectCamera *pCamera, __in const bool &bDrawGrid);
    HRESULT Destroy();
	HRESULT ClearSceneGraph();
    HRESULT CreateLineSegment(__in const DirectX::XMFLOAT3& origin, __in const DirectX::XMFLOAT3& end, __in const DirectX::XMFLOAT4& color, __in UINT depth);
	HRESULT CreateVector(__in const DirectX::XMFLOAT3& origin, __in const DirectX::XMFLOAT3& direction, __in const FLOAT scale, __in const DirectX::XMFLOAT4& color, __in UINT depth);
	HRESULT CreateSceneGrid();
	HRESULT DrawAxes();

	HRESULT BeginGroup();
	HRESULT EndGroup();

	HRESULT CreateTextMarker(__in const WCHAR *pText, __in const DirectX::XMFLOAT3 &position, __in UINT depth);
    HRESULT AddPointsToPointCloud(__deref_in_ecount(size) VertexType *pPoints, __in UINT length);
    HRESULT EndPointCloud( __in UINT depth);

    HRESULT CreateTestTriangle();
    HRESULT AddImage(__in_z const WCHAR *pszFileName);
    HRESULT CreateDepthMapMeshObject(__deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength);
};