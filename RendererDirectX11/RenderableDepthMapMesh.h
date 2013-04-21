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

class RenderableDepthMapMesh : public ImplementSmartObject
    <
        RenderableDepthMapMesh, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        IRenderableShape
    >
{
private:
	SmartPtr<ID3D11Buffer> m_spVertexBuffer;
	SmartPtr<ID3D11Buffer> m_spIndexBuffer;
	SmartPtr<ISceneObject> m_spSceneObject;
	D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;

	int m_VertexCount, m_IndexCount;

	SmartPtr<ID3D11Device> m_spDevice;

	HRESULT InitializeBuffers(__in ISceneObjectMesh *pMesh);
	HRESULT InitializeVertexBuffer( __in IMeshStream *pStream);
	HRESULT InitializeIndexBuffer(__in IMeshStream *pStream);

protected:
	HRESULT GetCameraMatricesFromRenderer(__in IRendererHandler *pRenderer, __out DirectX::XMMATRIX *viewMatrix, __out DirectX::XMMATRIX *projectionMatrix);

public:
    RenderableDepthMapMesh();
    ~RenderableDepthMapMesh();

    HRESULT UpdateFromSceneObject(__in ISceneObject *pSceneObject);
    UINT GetRequiredPasses();
    HRESULT Draw(UINT passIndex, __in IRendererHandler *pRenderer);

	HRESULT Initialize(__in ID3D11Device *pDevice, __in ISceneObject *pSceneObject);
    HRESULT GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface);
};