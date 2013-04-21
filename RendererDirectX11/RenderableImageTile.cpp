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
#include "RenderableImageTile.h"

using namespace DirectX;

RenderableImageTile::RenderableImageTile()
{
    m_IndexCount = 0;
    m_VertexCount = 0;
}

RenderableImageTile::~RenderableImageTile()
{
}

HRESULT RenderableImageTile::InitializeBuffers(__deref_in ISceneObjectCamera *pCamera, __in const XMMATRIX *worldMatrix)
{
    HRESULT hr = InitializeVertexBuffer(pCamera, worldMatrix);
    IF_FAILED_RETURN(hr);

    return InitializeIndexBuffer();
}

HRESULT RenderableImageTile::InitializeVertexBuffer(__deref_in ISceneObjectCamera *pCamera, __in const XMMATRIX *worldMatrix)
{
    _ASSERT((m_pTileMetadata && m_spDevice));
    SmartPtr<ISceneObjectCamera> spCamera(pCamera);

    FLOAT viewportX = 0, viewportY = 0, viewportWidth = 0, viewportHeight = 0, offsetX = 0.0f, offsetY = 0.0f;
    HRESULT hr = spCamera->GetViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    IF_FAILED_RETURN(hr);

    offsetX = 0.0f;//(viewportWidth / 2.0f);
    offsetY = 0.0f;//(viewportHeight / 2.0f);

    XMMATRIX projectionMatrix, viewMatrix;
    hr = spCamera->GetProjectionMatrix(&projectionMatrix);
    IF_FAILED_RETURN(hr);

    hr = spCamera->GetViewMatrix(&viewMatrix);
    IF_FAILED_RETURN(hr);


    UINT vertexCount = 4;
    Vector<TexturedVertexType> vertices;
    hr = vertices.SetSize(vertexCount);
    IF_FAILED_RETURN(hr);

    XMStoreFloat3(&m_Vertices[0], XMVector3Unproject(XMLoadFloat3(&XMFLOAT3(offsetX + (FLOAT)m_pTileMetadata->X, offsetY + (FLOAT)m_pTileMetadata->Y + m_pTileMetadata->Height, 0.0f)), 
                                                     viewportX, viewportY, viewportWidth, viewportHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, *worldMatrix));
    vertices[0].Position = m_Vertices[0];
    vertices[0].Texture = XMFLOAT2(0.0f, 1.0f);

    XMStoreFloat3(&m_Vertices[1], XMVector3Unproject(XMLoadFloat3(&XMFLOAT3(offsetX + (FLOAT)m_pTileMetadata->X, offsetY + (FLOAT)m_pTileMetadata->Y, 0.0f)), 
                                                     viewportX, viewportY, viewportWidth, viewportHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, *worldMatrix));
    vertices[1].Position = m_Vertices[1];
    vertices[1].Texture = XMFLOAT2(0.0f, 0.0f);

    XMStoreFloat3(&m_Vertices[2], XMVector3Unproject(XMLoadFloat3(&XMFLOAT3(offsetX + (FLOAT)m_pTileMetadata->X + m_pTileMetadata->Width, offsetY + (FLOAT)m_pTileMetadata->Y, 0.0f)), 
                                                     viewportX, viewportY, viewportWidth, viewportHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, *worldMatrix));
    vertices[2].Position = m_Vertices[2];
    vertices[2].Texture = XMFLOAT2(1.0f, 0.0f);

    XMStoreFloat3(&m_Vertices[3], XMVector3Unproject(XMLoadFloat3(&XMFLOAT3(offsetX + (FLOAT)m_pTileMetadata->X + m_pTileMetadata->Width, offsetY + (FLOAT)m_pTileMetadata->Y + m_pTileMetadata->Height, 0.0f)), 
                                                     viewportX, viewportY, viewportWidth, viewportHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, *worldMatrix));
    vertices[3].Position = m_Vertices[3];
    vertices[3].Texture = XMFLOAT2(1.0f, 1.0f);

    // Set up the description of the static vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(TexturedVertexType) * vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices.Ptr();
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Now create the vertex buffer
    m_spVertexBuffer = NULL;
    hr = m_spDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_spVertexBuffer);
    IF_FAILED_RETURN(hr);

    m_VertexCount = vertexCount;

    return hr;
}

HRESULT RenderableImageTile::InitializeIndexBuffer()
{
    _ASSERT((m_pTileMetadata && m_spDevice));

    UINT indexCount = 6;

    Vector<UINT> indices;
    HRESULT hr = indices.SetSize(indexCount);
    IF_FAILED_RETURN(hr);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 0;
    indices[4] = 2;
    indices[5] = 3;

	// Set up the description of the static index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
				
	// Give the subresource structure a pointer to the index data
	D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices.Ptr();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
				
	// Create the index buffer
    m_spIndexBuffer = NULL;
	hr = m_spDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_spIndexBuffer);
    IF_FAILED_RETURN(hr);

    m_IndexCount = indexCount;

    return hr;
}

HRESULT RenderableImageTile::GetProjectedWorldToScreenDiff(__deref_in ISceneObjectCamera *pCamera, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight, __out FLOAT &difference)
{
    SmartPtr<ISceneObjectCamera> spCamera(pCamera);
    XMMATRIX viewMatrix, projectionMatrix;

    HRESULT hr = spCamera->GetViewMatrix(&viewMatrix);
    IF_FAILED_RETURN(hr);

    hr = spCamera->GetProjectionMatrix(&projectionMatrix);
    IF_FAILED_RETURN(hr);

    XMMATRIX worldMatrix = XMMatrixIdentity();

    XMFLOAT3 projectedVertices[4];
    XMVector3ProjectStream(projectedVertices, sizeof(XMFLOAT3), m_Vertices, sizeof(XMFLOAT3), 4, 0.0f, 0.0f, 
                           viewportWidth, viewportHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, worldMatrix);

    XMVECTOR vec1 = XMVector3Project(XMLoadFloat3(&m_Vertices[0]), 0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, worldMatrix);
    XMVECTOR vec2 = XMVector3Project(XMLoadFloat3(&m_Vertices[1]), 0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, worldMatrix);

    XMVECTOR diff = XMVector3Length(XMVectorSubtract(XMLoadFloat3(&projectedVertices[0]), XMLoadFloat3(&projectedVertices[1])));

    difference = abs(XMVectorGetX(diff) - m_pTileMetadata->Height);

    return S_OK;
}


BOOL RenderableImageTile::IsVisible(__in CXMMATRIX viewMatrix, __in CXMMATRIX projectionMatrix, __in CXMMATRIX worldMatrix, 
                                    __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight)
{
    XMFLOAT3 projectedVertices[4];
    XMVector3ProjectStream(projectedVertices, sizeof(XMFLOAT3), m_Vertices, sizeof(XMFLOAT3), 4, 0, 0, 
                           viewportWidth, viewportHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, worldMatrix);

    XMVECTOR MinA = XMLoadFloat2(&XMFLOAT2(projectedVertices[1].x, projectedVertices[1].y));
    XMVECTOR MaxA = XMLoadFloat2(&XMFLOAT2(projectedVertices[3].x, projectedVertices[3].y));

    XMVECTOR MinB = XMLoadFloat2(&XMFLOAT2(0.0f, 0.0f));
    XMVECTOR MaxB = XMLoadFloat2(&XMFLOAT2(viewportWidth, viewportHeight));

    XMVECTOR Disjoint = XMVectorOrInt( XMVectorGreater( MinA, MaxB ), XMVectorGreater( MinB, MaxA ) );

    return !XMComparisonAnyTrue(XMVector3EqualIntR( Disjoint, XMVectorTrueInt()));
}

HRESULT RenderableImageTile::GetVertex(__in const UINT &index, __out XMFLOAT3 &vertex)
{
    if (index > 4)
        return E_INVALIDARG;

    vertex = m_Vertices[index];

    return S_OK;
}

HRESULT RenderableImageTile::InitializeTexture()
{
    m_spTextureSRV.Release();
    m_spTexture.Release();

    // Load image data
    SmartPtr<IDxImage> spImage;
    HRESULT hr = m_spImageLoader->GetLevelRowColumnImage(m_pTileMetadata->Level, m_pTileMetadata->Row, m_pTileMetadata->Column, &spImage);

    D3D11_TEXTURE2D_DESC texDesc;
    spImage->GetSize(texDesc.Width, texDesc.Height);
    texDesc.MipLevels            = 1;
    texDesc.ArraySize            = 1;
    texDesc.Format               = (DXGI_FORMAT)spImage->GetFormat();
    texDesc.SampleDesc.Count     = 1;
    texDesc.SampleDesc.Quality   = 0;
	texDesc.Usage                = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags       = 0;
    texDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA data;        
    data.pSysMem                 = spImage->GetPixelData();
    data.SysMemPitch             = spImage->GetRowPitch();
    data.SysMemSlicePitch        = 0;

	hr = m_spDevice->CreateTexture2D( &texDesc, &data, &m_spTexture );
    IF_FAILED_RETURN(hr);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                       = (DXGI_FORMAT)spImage->GetFormat();
    srvDesc.ViewDimension                = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels          = 1;
    srvDesc.Texture2D.MostDetailedMip    = 0;

    return m_spDevice->CreateShaderResourceView( m_spTexture, &srvDesc, &m_spTextureSRV );
}

HRESULT RenderableImageTile::LockTexture(__deref_out ID3D11ShaderResourceView** ppTextureSRV)
{
    if (m_spTexture)
    {
        return m_spTextureSRV.CopyTo(ppTextureSRV);
    }

    HRESULT hr = InitializeTexture();
    
    if (SUCCEEDED(hr))
    {
        hr = m_spTextureSRV.CopyTo(ppTextureSRV);
    }

    return hr;
}

void RenderableImageTile::UnlockTexture()
{
    if (m_spTexture)
    {
        ReleaseTexture();    
    }
}

void RenderableImageTile::ReleaseTexture()
{
    m_spTexture.Release();
    m_spTexture = NULL;

    m_spTextureSRV.Release();
    m_spTextureSRV = NULL;
}

HRESULT RenderableImageTile::GetBuffersAndPrimitiveTopology(__deref_out ID3D11Buffer **vertexBuffer, __out UINT &vertexCount, __deref_out ID3D11Buffer **indexBuffer, __out UINT &indexCount, __out D3D_PRIMITIVE_TOPOLOGY &primitiveTopology)
{
    HRESULT hr = m_spVertexBuffer.CopyTo(vertexBuffer);
    if (SUCCEEDED(hr))
    {
        hr = m_spIndexBuffer.CopyTo(indexBuffer);

        if (SUCCEEDED(hr))
        {
            vertexCount = m_VertexCount;
            indexCount = m_IndexCount;
            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
    }

    return hr;
}

HRESULT RenderableImageTile::Initialize(__deref_in ID3D11Device *pDevice, __deref_in ID3D11DeviceContext* pDeviceContext, __deref_in IImageLoader *pImageLoader, __deref_in ISceneObjectCamera *pCamera, __in const XMMATRIX *worldMatrix, __deref_in const ImageTileMetadata *pTileMetadata)
{
    if (!pDevice || !pDeviceContext || !pImageLoader || !pCamera || !pTileMetadata)
    {
        return E_INVALIDARG;
    }

    m_spDevice = pDevice;
    m_spDeviceContext = pDeviceContext;
    m_spImageLoader = pImageLoader;
    m_pTileMetadata = pTileMetadata;
    

    return InitializeBuffers(pCamera, worldMatrix);
}
