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

#include <DirectXMath.h>

enum SceneObjectContentType
{
    SOCT_NONE = 0,
    SOCT_TRANSFORM = 1,
    SOCT_SHAPE = 2,
    SOCT_PROJECTION = 4,
    SOCT_MATERIAL = 8,
    SOCT_LIGHT_PARAMS = 0x10,
    SOCT_VIEWPORT = 0x20,
    SOCT_OBJECT_ADD = 0x10000,
    SOCT_OBJECT_REMOVE = 0x20000,
    SOCT_ALL = 0x7FFFFFFF
};

enum MeshStreamType
{
    MST_VERTEX_POSITIONS,
    MST_VERTEX_COLORS,
    MST_VERTEX_NORMALS,
    MST_VERTEX_TEXCOORD1,
    MST_VERTEX_TEXCOORD2,
    MST_VERTEX_TEXCOORD3,
    MST_VERTEX_TEXCOORD4,

    MST_TRIANGLES_EDGES,
    MST_TRIANGLES_NORMALS_SMOOTHING,
    MST_TRIANGLES_VERTEX_INDEXES,
    MST_TRIANGLES_TEXCOORD_INDEXES1,
    MST_TRIANGLES_TEXCOORD_INDEXES2,
    MST_TRIANGLES_TEXCOORD_INDEXES3,
    MST_TRIANGLES_TEXCOORD_INDEXES4,
};

enum PrimitiveTopologyType
{
    PTT_POINTLIST = 1,
	PTT_LINELIST = 2,
	PTT_TRIANGLELIST = 4,  
};

struct VertexType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

DECLAREINTERFACE(ISceneObject, IUnknown, "{A286958A-AFD6-483B-8DAF-B9A58071B3E5}")
{
    HRESULT SetLocalTransform(__in DirectX::CXMMATRIX transform);
    DirectX::CXMMATRIX GetLocalTransform() const;
    DirectX::CXMMATRIX GetWorldTransform() const;

    HRESULT Update(SceneObjectContentType contentType);

    SceneObjectContentType GetInvalidContentMask();
    HRESULT Invalidate(SceneObjectContentType contentType);
    HRESULT Validate();

    HRESULT Register(__in ISceneGraph *pSceneGraph);
    HRESULT Unregister();

    HRESULT SetName(__in_z const WCHAR* pName);
    const WCHAR* GetName() const;

    HRESULT GetOwner(__deref_out_opt ISceneObject **ppOwner);
    HRESULT SetOwner(__in_opt ISceneObject *pOwner);
    
    HRESULT Destroy();

    HRESULT GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface);
};

DECLAREINTERFACE(ISceneObjectGroup, IUnknown, "{378DF73B-5A01-4F36-A4C3-8866CED20BEE}")
{
    UINT GetObjectCount();
    HRESULT GetObject(__in UINT index, __deref_out ISceneObject **ppSceneObject);

    HRESULT AddObject(__in ISceneObject *pSceneObject);
    HRESULT RemoveObject(__in ISceneObject *pSceneObject);
};

enum ProjectionType
{
    Perspective,
    Orthographic,
}; 

DECLAREINTERFACE(ISceneObjectCamera, IUnknown, "{1EB684E2-81AD-43A2-A8FB-CE0B5B045268}")
{
    HRESULT GetProjectionMatrix(__deref_out DirectX::XMMATRIX *projection);
    HRESULT SetProjectionMatrix(__in DirectX::CXMMATRIX projection);
	HRESULT GetViewMatrix(__deref_out DirectX::XMMATRIX *viewMatrix);
	HRESULT Look(__in const FLOAT &yawDelta, __in const FLOAT &pitchDelta, __in const FLOAT &rollDelta, __in const FLOAT &scaleDelta);
	HRESULT Move(__in const FLOAT &forwardDelta, __in const FLOAT &strideDelta, __in const FLOAT &upDelta);
	HRESULT SetPositionAtUpFov(__in DirectX::CXMVECTOR pos, __in DirectX::CXMVECTOR at, __in DirectX::CXMVECTOR up, __in const FLOAT &fov);
	HRESULT SetPositionAndScale(DirectX::CXMVECTOR pos, __in const FLOAT &scale);
    
    HRESULT SetViewport(__in const FLOAT &viewportX, __in const FLOAT &viewportY, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight);
    HRESULT GetViewport(__out FLOAT &viewportX, __out FLOAT &viewportY, __out FLOAT &viewportWidth, __out FLOAT &viewportHeight);
    HRESULT PanZoomOrthographic(__in const FLOAT &viewportOffsetX, __in const FLOAT &viewportOffsetY, __in const FLOAT &scaleDelta);
    HRESULT ApplyViewportOffsetX();
    HRESULT ApplyViewportOffsetY();

    HRESULT SetScreenSize(__in const FLOAT &width, __in const FLOAT &height);
    HRESULT GetScreenSize(__out FLOAT &width, __out FLOAT &height);
    HRESULT CalibrateToSceneObject(__in ISceneObject *pSceneObject);
};


DECLAREINTERFACE(ICalibrable, IUnknown, "{69948A8C-0222-49DD-8790-F1FB0F1CFB95}")
{
    HRESULT Calibrate();
    BOOL IsCalibrated();
    void SetCalibrated();
};

DECLAREINTERFACE(IZoomable, IUnknown, "{443023F3-B545-4D42-919D-11D673640154}")
{
    
};

DECLAREINTERFACE(ISceneObjectGrid, IUnknown, "{125BA72C-1D1D-4D02-9864-497D30AA17DE}")
{
    HRESULT SetParameters(__in UINT resolution, __in FLOAT size);
};

DECLAREINTERFACE(ISceneObjectPointCloud, IUnknown, "{726888BE-25A1-491B-AFC8-3C9ACB5D8C49}")
{
};

DECLAREINTERFACE(ISceneObjectText, IUnknown, "{2A291473-8012-43C3-A47D-4DD143759B42}")
{
	HRESULT UpdateText(__in_z const wchar_t *newText);	
	const WCHAR* GetText();

	DirectX::XMFLOAT4 GetColor();
	HRESULT SetColor(__in const DirectX::XMFLOAT4 &color);

	const DirectX::XMFLOAT3& GetPosition();
	HRESULT SetPosition(__in const DirectX::XMFLOAT3 &newPosition);

    HRESULT Destroy();
};

DECLAREINTERFACE(IMeshStream, IUnknown, "{ACD47190-2717-43B3-B060-81DFF0F718C8}")
{
    MeshStreamType GetType();
    UINT GetLength();
    UINT GetItemSize();

    HRESULT Lock(void **ppBuffer);
    HRESULT Unlock();
};

DECLAREINTERFACE(ISceneObjectMesh, IUnknown, "{3F13FE23-6651-494B-9A8A-FDE3310B9D5E}")
{
    HRESULT GetMeshStream(__in MeshStreamType streamType, __deref_out_opt IMeshStream **ppStream);
    HRESULT SetMeshStream(__in MeshStreamType streamType, __in IMeshStream *pStream);
    HRESULT RemoveMeshStream(__in MeshStreamType streamType);
	PrimitiveTopologyType GetPrimitiveTopologyType();
	void SetPrimitiveTopologyType(PrimitiveTopologyType topologyType);
};

__interface IImageLoader;
DECLAREINTERFACE(ISceneObjectImage, IUnknown, "{F7F5E995-927E-4858-B0EB-7CBDAD9345F6}")
{
    HRESULT GetImageLoader(__deref_out IImageLoader **ppImageLoader);
    HRESULT Destroy();
};

DECLAREINTERFACE(ISceneObjectDepthMapMesh, IUnknown, "{767AD1C6-552E-47AE-979D-C019E6C6B1CF}")
{
    HRESULT GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface);
};

DECLAREINTERFACE(IVertexStream, IUnknown, "{6E7BFA51-0A3D-46D3-8B6D-EC8D5B662B8B}")
{
	HRESULT Lock(VertexType **ppVertexBuffer);
	HRESULT Unlock();
};

DECLAREINTERFACE(IIndexStream, IUnknown, "{3B99659B-99E1-412E-B487-A03BD6455044}")
{
	HRESULT Lock(unsigned long **ppIndexBuffer);
	HRESULT Unlock();
};

DECLAREINTERFACE(ISceneChangesNotificationListener, IUnknown, "{983369BB-65E3-44C9-A6F7-D1086F90C00E}")
{
    HRESULT OnSceneObjectChanged(__in ISceneObject *pSceneObject, SceneObjectContentType invalidContentMask);
};

DECLAREINTERFACE(ISceneGraph, IUnknown, "{0E38F64D-0198-4BE4-8DC6-D0F363303083}")
{
    HRESULT GetSceneRoot(__deref_out ISceneObjectGroup **ppSceneRoot);

    HRESULT RegisterForValidate(__in ISceneObject *pObject);

    HRESULT RegisterNotificationListener(__in ISceneChangesNotificationListener *pListener, SceneObjectContentType mask);
    
    HRESULT UnregisterNotificationListener(__in ISceneChangesNotificationListener *pListener);
    
    HRESULT Validate();

    HRESULT Destroy();

    UINT GetTimestamp();
};

DECLAREINTERFACE(ISceneGraphEditor, IUnknown, "{19FAC57E-8574-493E-B692-5DCBB9EDEEB4}")
{
    HRESULT Destroy();

	HRESULT ClearSceneGraph();

	HRESULT CreateLineSegment(__in const DirectX::XMFLOAT3& origin, __in const DirectX::XMFLOAT3& end, __in const DirectX::XMFLOAT4& color, __in UINT depth);

	HRESULT CreateVector(__in const DirectX::XMFLOAT3& origin, __in const DirectX::XMFLOAT3& direction, __in const FLOAT scale, __in const DirectX::XMFLOAT4& color, __in UINT depth);

	HRESULT BeginGroup();

	HRESULT EndGroup();

	HRESULT CreateSceneGrid();

	HRESULT CreateTextMarker(__in const WCHAR *pText, __in const DirectX::XMFLOAT3 &position, __in UINT depth);

    HRESULT AddPointsToPointCloud(__deref_in_ecount(size) VertexType *pPoints, __in UINT length);

    HRESULT EndPointCloud( __in UINT depth);

    HRESULT AddImage(__in_z const WCHAR *pszFileName);

    HRESULT CreateTestTriangle();

    HRESULT CreateDepthMapMeshObject(__deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength);
};

HRESULT CreateSceneGraph(__deref_out ISceneGraph **ppResult); 

HRESULT CreateCamera(__in_z const wchar_t *nodeName, __in const ProjectionType &projectionType, __deref_out ISceneObjectCamera **ppResult);

HRESULT CreateGrid(__in_z const wchar_t *nodeName, __deref_out ISceneObjectGrid **ppResult);

HRESULT CreateGroup(__in_z const wchar_t *nodeName, __deref_out ISceneObjectGroup **ppResult);

HRESULT CreateMesh(__in_z const wchar_t *nodeName, __deref_out ISceneObjectMesh **ppResult);

HRESULT CreateMeshStream(__in const MeshStreamType meshType, __in const UINT itemSize, __in const UINT length, __deref_opt_in_bcount(itemSize * length) const void* pInitialData, __deref_out IMeshStream **ppResult);

HRESULT CreateVertexStream(__in const UINT length, __deref_opt_in_ecount(length) const VertexType* pInitialData, __deref_out IVertexStream **ppResult);

HRESULT CreateIndexStream(__in const UINT length, __deref_opt_in_ecount(length) const unsigned long* pInitialData, __deref_out IIndexStream **ppResult);

HRESULT CreateSceneGraphEditor(__in ISceneGraph *pSceneGraph, __in ISceneObjectCamera *pCamera, __in bool bDrawMesh, __deref_out ISceneGraphEditor **ppSceneGraphEditor);

HRESULT CreateText(__in_z const wchar_t *nodeName, __in_z const wchar_t *text, __in const DirectX::XMFLOAT4 &color, __in const DirectX::XMFLOAT3 &position, __deref_out ISceneObjectText **ppResult);

HRESULT CreatePointCloud(__in_z const wchar_t *nodeName, __deref_in_ecount(length) VertexType *pPoints, __in UINT length, __deref_out ISceneObjectPointCloud **ppGrid);

HRESULT CreateDepthMapMesh(__in_z const wchar_t *nodeName, __deref_in_ecount(verticesLength) VertexType *pPoints, __in const UINT &verticesLength, __deref_in_ecount(indicesLength) unsigned long *pIndices, __in const UINT &indicesLength, __deref_out ISceneObjectDepthMapMesh **ppGrid);

HRESULT CreateImage(__in_z const WCHAR *nodeName, __in_z const WCHAR *filePath, __deref_out ISceneObjectImage **ppResult);