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

struct TexturedVertexType
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 Texture;
	DirectX::XMFLOAT4 Color;
};

class RenderableImageTile : public ImplementSmartObject
    <
        RenderableImageTile, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        IRenderableTile
    >
{
    FLOAT m_WorldToScreenSizeRatio;

    SmartPtr<ID3D11Buffer> m_spVertexBuffer;
	SmartPtr<ID3D11Buffer> m_spIndexBuffer;
	int m_VertexCount, m_IndexCount;

    const ImageTileMetadata* m_pTileMetadata;
    DirectX::XMFLOAT3 m_Vertices[4];

    SmartPtr<ID3D11Texture2D> m_spTexture;
    SmartPtr<ID3D11ShaderResourceView> m_spTextureSRV;

    SmartPtr<IImageLoader> m_spImageLoader;
    SmartPtr<ID3D11Device> m_spDevice; 
    SmartPtr<ID3D11DeviceContext> m_spDeviceContext;

    HRESULT InitializeBuffers(__deref_in ISceneObjectCamera *pCamera, __in const DirectX::XMMATRIX *worldMatrix);
	HRESULT InitializeVertexBuffer(__deref_in ISceneObjectCamera *pCamera, __in const DirectX::XMMATRIX *worldMatrix);
	HRESULT InitializeIndexBuffer();
    HRESULT InitializeTexture();

public:
    RenderableImageTile();
    ~RenderableImageTile();

    BOOL    IsVisible(__in DirectX::CXMMATRIX viewMatrix, __in DirectX::CXMMATRIX projectionMatrix, __in DirectX::CXMMATRIX worldMatrix, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight);
    HRESULT LockTexture(__deref_out ID3D11ShaderResourceView** ppTextureSRV);
    void    UnlockTexture();
    void    ReleaseTexture();
    HRESULT GetBuffersAndPrimitiveTopology(__deref_out ID3D11Buffer **vertexBuffer, __out UINT &vertexCount, __deref_out ID3D11Buffer **indexBuffer, __out UINT &indexCount, __out D3D_PRIMITIVE_TOPOLOGY &primitiveTopology);
    HRESULT GetProjectedWorldToScreenDiff(__deref_in ISceneObjectCamera *pCamera, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight, __out FLOAT &difference);
    HRESULT GetVertex(__in const UINT &index, __out DirectX::XMFLOAT3 &vertex);

    HRESULT Initialize(__deref_in ID3D11Device *pDevice, __deref_in ID3D11DeviceContext* pDeviceContext, __deref_in IImageLoader *pImageLoader, __deref_in ISceneObjectCamera *pCamera, __in const DirectX::XMMATRIX *worldMatrix, __deref_in const ImageTileMetadata *pTileMetadata);
};