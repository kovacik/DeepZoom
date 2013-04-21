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

class RenderableText : public ImplementSmartObject
    <
        RenderableText, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        IRenderableShape
    >
{
private:

    struct TexturedVertexType
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Texture;
		DirectX::XMFLOAT4 Color;
	};

    static const UINT MAX_RENDERABLE_CHARS_COUNT = 512;

	SmartPtr<ISceneObjectText> m_spSceneObjectText;
	DirectX::XMFLOAT4 m_FontColor;

	UINT m_TexWidth;
	UINT m_TexHeight;

	SmartPtr<ID3D11ShaderResourceView> m_spFontSheet;
	SmartPtr<IDxFont> m_spFont;

	SmartPtr<ID3D11Buffer> m_spVertexBuffer;
	SmartPtr<ID3D11Buffer> m_spIndexBuffer;
	int m_VertexCount, m_IndexCount;

	SmartPtr<ID3D11Device> m_spDevice; 
    SmartPtr<ID3D11DeviceContext> m_spDeviceContext;

	HRESULT InitializeIndexBuffer();
	HRESULT InitializeDynamicVertexBuffer();
    HRESULT UpdateDynamicVertexBuffer(__in const DirectX::XMFLOAT3 &position, __in const UINT &screenWidth, __in const UINT &screenHeight);

	HRESULT InitializeBuffers();
	HRESULT BuildTexturedQuad(__in const Fonts::Rect &sourceRect, __in const Fonts::Rect &destinationRect, __in DirectX::XMFLOAT4 &color, 
							  __in const UINT &textureWidth, __in const UINT &textureHeight,  
							  __deref_out_ecount(4) TexturedVertexType *pQuad);

public:
    RenderableText();
    ~RenderableText();

    HRESULT UpdateFromSceneObject(__in ISceneObject *pSceneObject);
    UINT    GetRequiredPasses();
    HRESULT Draw(UINT passIndex, __in IRendererHandler *pRenderer);

	HRESULT Initialize(__in ID3D11Device *pDevice, __in ID3D11DeviceContext* pDeviceContext, __in ISceneObject *pSceneObject, __in IDxFont *pFont);
    HRESULT GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface);
};