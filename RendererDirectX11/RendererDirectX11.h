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

class RendererDirectX11 
    : public ImplementSmartObject<RendererDirectX11, IRenderer, IRendererHandler, ISceneChangesNotificationListener>
{
    UINT m_WindowWidth;
    UINT m_WindowHeight;
    bool m_DrawRequired;

    HashTable<ISceneObject*, SmartPtr<IRenderableShape>> m_ObjectsToRender;
    SmartPtr<ISceneGraph> m_spSceneGraph;
    
    SmartPtr<ISceneObjectCamera> m_spCamera;

	// Shaders
	SmartPtr<IColorShader> m_spColorShader;
	SmartPtr<IFontShader> m_spFontShader;
    SmartPtr<IDepthMapShader> m_spDepthMapShader;
    SmartPtr<ITextureShader> m_spTextureShader;

	// Fonts
	SmartPtr<IDxFont> m_spArialFont;

    SmartPtr<ID3D11RenderTargetView> m_spRenderTargetView;
    SmartPtr<ID3D11DepthStencilView> m_spDepthStencilView;
    SmartPtr<IDXGISwapChain> m_spSwapChain;
    SmartPtr<ID3D11Texture2D> m_spBackBuffer;
    SmartPtr<ID3D11Texture2D> m_spDepthStencil;

	// Render states
	SmartPtr<ID3D11DepthStencilState> m_spDepthStencilState;
	SmartPtr<ID3D11DepthStencilState> m_spDepthDisabledStencilState;
	SmartPtr<ID3D11RasterizerState> m_spRasterState;
	SmartPtr<ID3D11BlendState> m_spAlphaEnableBlendingState;
	SmartPtr<ID3D11BlendState> m_spAlphaDisableBlendingState;

    SmartPtr<ID3D11Device> m_spDevice;
    SmartPtr<ID3D11DeviceContext> m_spDeviceContext;

    // Initializers
	HRESULT Initialize2DRenderingCamera();
	HRESULT InitializeShaders();
    HRESULT CreateDeviceResources(D3D_FEATURE_LEVEL *pFeatureLevel);
    HRESULT CreateWindowSizeDependentResources(__in HWND hwnd, __in const UINT &targetWidth = 0, __in const UINT &targetHeight = 0);
    HRESULT CreateRenderTarget(__in const UINT &width, __in const UINT &height);
	HRESULT InitializeRenderStates();
  
	HRESULT UpdateRenderableFromSceneObject(__in ISceneObject *pSceneObject, SceneObjectContentType invalidContentMask);
    void DrawSceneContent();
	
public:
    RendererDirectX11(void);
    ~RendererDirectX11(void);

    HRESULT Initialize(HWND hwnd, __in const UINT &width, __in const UINT &height, __in ISceneGraph *pSceneGraph);
    HRESULT Resize(RECT rect);
    void Draw(bool forceUpdate);

    HRESULT GetCamera(__deref_out ISceneObjectCamera **ppCamera);
    HRESULT SetCamera(__in ISceneObjectCamera *pCamera);

    HRESULT GetCameraMatrices(__out DirectX::XMMATRIX *viewMatrix, __out DirectX::XMMATRIX *projectionMatrix);
	HRESULT Get2DRenderingMatrices(__out DirectX::XMMATRIX *viewMatrix, __out DirectX::XMMATRIX *projectionMatrix);

    HRESULT OnSceneObjectChanged(__in ISceneObject *pSceneObject, SceneObjectContentType invalidContentMask);

	void DrawIndexedPrimitive(__in ID3D11Buffer** ppVertexBuffer, UINT stride, __in ID3D11Buffer* pIndexBuffer, DXGI_FORMAT indexFormat, UINT indexCount, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);
	void DrawPrimitive(__in ID3D11Buffer** ppVertexBuffer, UINT stride, UINT vertexCount, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);

	void GetWindowWidthAndHeight(__out UINT &width, __out UINT &height);
	HRESULT GetShaderInterface(const GUID &extensionId, __deref_opt_out IShader **ppShaderInterface);

	void TurnOffAlphaBlending();
	void TurnOnAlphaBlending();

	void TurnZBufferOn();
	void TurnZBufferOff();

    HRESULT TakeDepthRenderTargetSnapshot(__in const WCHAR *pszFilePath);
};

