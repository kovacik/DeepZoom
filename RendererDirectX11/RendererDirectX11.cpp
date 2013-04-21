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
#include "RendererDirectX11.h"

using namespace DirectX;


RendererDirectX11::RendererDirectX11(void)
{
    m_DrawRequired = true;
}

RendererDirectX11::~RendererDirectX11(void)
{
	m_ObjectsToRender.Reset();
}

HRESULT RendererDirectX11::GetCamera(__deref_out ISceneObjectCamera **ppCamera)
{
    return m_spCamera.CopyTo(ppCamera);
}

HRESULT RendererDirectX11::SetCamera(__in ISceneObjectCamera *pCamera)
{
    if (!pCamera)
    {
        return E_INVALIDARG;
    }

    m_spCamera = pCamera;
    m_DrawRequired = true;

	HRESULT hr;
	SmartPtr<ISceneObjectGroup> spRoot;
    hr = m_spSceneGraph->GetSceneRoot(&spRoot); 
    if (SUCCEEDED(hr))
    {
        hr = m_spCamera->SetScreenSize((FLOAT)m_WindowWidth, (FLOAT)m_WindowHeight);
        if (SUCCEEDED(hr))
        {
		    hr = spRoot->AddObject(m_spCamera.CastTo<ISceneObject>());
        }
    }

    return hr;
}

void RendererDirectX11::DrawSceneContent()
{  
	if (m_ObjectsToRender.Size() == 0)
	{
		return;
	}

	//
	// Iteratively render all objects from hashtable
	// This can be further improved by adding tree structure into SceneGraph, to render only camera visible objects
	//
	for (HashTable<ISceneObject*, SmartPtr<IRenderableShape>>::Iterator objectToRenderItr = m_ObjectsToRender.GetIterator(); 
		 !objectToRenderItr.Eof(); 
		 objectToRenderItr++)
	{			
		UINT requiredPassesCount = objectToRenderItr->GetRequiredPasses();

		for (UINT i = 0; i < requiredPassesCount; ++i)
		{
			objectToRenderItr->Draw(i, this);
		}
	}
}

void RendererDirectX11::Draw(bool forceUpdate)
{
    if (!m_spCamera)
        return;
 
    if (m_DrawRequired || forceUpdate)
    {
        m_DrawRequired = false;

        m_spDeviceContext->OMSetRenderTargets( 1, &m_spRenderTargetView.p, m_spDepthStencilView );

        const float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
        m_spDeviceContext->ClearRenderTargetView( m_spRenderTargetView, clearColor );
        m_spDeviceContext->ClearDepthStencilView( m_spDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
    
        DrawSceneContent();

        m_spSwapChain->Present(0, 0);
    }
}

HRESULT RendererDirectX11::Initialize(HWND hwnd, __in const UINT &width, __in const UINT &height, __in ISceneGraph *pSceneGraph)
{
    D3D_FEATURE_LEVEL featureLevel;

    if (!pSceneGraph)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CreateDeviceResources(&featureLevel);

    if (FAILED(hr)) 
	{
		return hr;
	}

	hr = InitializeRenderStates();

	if (FAILED(hr)) 
	{
		return hr;
	}

	hr = CreateWindowSizeDependentResources(hwnd, width, height);

	if (FAILED(hr)) 
	{
		return hr;
	}

	hr = InitializeShaders();

	if (FAILED(hr)) 
	{
		return hr;
	}

	hr = CreateDxFont(m_spDevice, L"Times New Roman", 20.0f, Fonts::DF_STYLE_NORMAL, TRUE, &m_spArialFont);

	if (FAILED(hr)) 
	{
		return hr;
	}

    m_spSceneGraph = pSceneGraph;

    hr = m_spSceneGraph->RegisterNotificationListener(this, (SceneObjectContentType)(SOCT_SHAPE | SOCT_MATERIAL | SOCT_TRANSFORM | SOCT_OBJECT_ADD | SOCT_OBJECT_REMOVE));

    return hr;
}

HRESULT RendererDirectX11::GetCameraMatrices(__out XMMATRIX *viewMatrix, __out XMMATRIX *projectionMatrix)
{
    if (!viewMatrix || !projectionMatrix)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = m_spCamera->GetViewMatrix(viewMatrix);

    if (SUCCEEDED(hr))
    {				
        hr = m_spCamera->GetProjectionMatrix(projectionMatrix);
    }

    return hr;
}

HRESULT RendererDirectX11::InitializeShaders()
{
	HRESULT hr;

	hr = CreateColorShader(m_spDevice, m_spDeviceContext, &m_spColorShader);
    IF_FAILED_RETURN(hr);

	hr = CreateFontShader(m_spDevice, m_spDeviceContext, &m_spFontShader);
    IF_FAILED_RETURN(hr);

	hr = CreateDepthMapShader(m_spDevice, m_spDeviceContext, &m_spDepthMapShader);
    IF_FAILED_RETURN(hr);

    return CreateTextureShader(m_spDevice, m_spDeviceContext, &m_spTextureShader);
}

HRESULT RendererDirectX11::GetShaderInterface(const GUID &extensionId, __deref_opt_out IShader **ppShaderInterface)
{
	if (ppShaderInterface == NULL)
	{
		return E_INVALIDARG;
	}

	HRESULT hr;

	if (IsEqualIID(extensionId, __uuidof(IColorShader)))
	{
		SmartPtr<IShader> spShader = m_spColorShader.CastTo<IShader>();
		hr = spShader.CopyToOpt(ppShaderInterface);
	}
	else if (IsEqualIID(extensionId, __uuidof(IFontShader)))
	{
		SmartPtr<IShader> spShader = m_spFontShader.CastTo<IShader>();
		hr = spShader.CopyToOpt(ppShaderInterface);
	}
    else if (IsEqualIID(extensionId, __uuidof(IDepthMapShader)))
	{
        SmartPtr<IShader> spShader = m_spDepthMapShader.CastTo<IShader>();
		hr = spShader.CopyToOpt(ppShaderInterface);
	}
    else if (IsEqualIID(extensionId, __uuidof(ITextureShader)))
    {
        SmartPtr<IShader> spShader = m_spTextureShader.CastTo<IShader>();
		hr = spShader.CopyToOpt(ppShaderInterface);
    }
	else
	{
		hr = E_NOINTERFACE;
	}

    return hr;
}

HRESULT RendererDirectX11::CreateDeviceResources(D3D_FEATURE_LEVEL *pFeatureLevel)
{
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    // If the project is in a debug build, enable debugging via SDK Layers with this flag
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,

    };

    // Create the DX11 API device object, and get a corresponding context
    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, featureLevels, ARRAYSIZE( featureLevels ), D3D11_SDK_VERSION, &m_spDevice, pFeatureLevel, &m_spDeviceContext);
    if (FAILED(hr))
    {
#if defined(_DEBUG)
        // if this is a debug build then probably debugging support for DX is not enabled. Try GPU without debug
        hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE( featureLevels ), D3D11_SDK_VERSION, &m_spDevice, pFeatureLevel, &m_spDeviceContext);
        if (FAILED(hr))
#endif
        {
            // on failure,failback to warp
            hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_WARP, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE( featureLevels ), D3D11_SDK_VERSION, &m_spDevice, pFeatureLevel, &m_spDeviceContext);
        }
    }

    return hr;
}

HRESULT RendererDirectX11::Resize(RECT rect)
{
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    HRESULT hr;
    if ((width == m_WindowWidth) && (height == m_WindowHeight))
    {
        hr = S_FALSE;
    }
    else
    {
        m_DrawRequired = true;
        m_spRenderTargetView.Release();
        m_spDepthStencilView.Release();

        if (m_spSwapChain)
        {
            hr = m_spSwapChain->ResizeBuffers( 2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0 );
            IF_FAILED_RETURN(hr);
        }
        
        hr = CreateRenderTarget(width, height);
        if (SUCCEEDED(hr))
        {
            m_WindowWidth = width;
            m_WindowHeight = height;

            m_spCamera->SetScreenSize((FLOAT)width, (FLOAT)height);
        }
    }
    return hr;
}

HRESULT RendererDirectX11::CreateWindowSizeDependentResources(__deref_in HWND hwnd, __in const UINT &targetWidth, __in const UINT &targetHeight)
{ 
    HRESULT hr = S_OK;
    UINT width = 0, height = 0;

    // If windows exists, get it's with and height and create swap chain
    if (hwnd)
    {
        RECT rect;
        GetClientRect(hwnd, &rect);

        width = rect.right - rect.left;
        height = rect.bottom - rect.top;

        // If the swap chain already exists, resize it
        // Create a descriptor for the swap chain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // This is the most common swapchain format
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;                               // Use two buffers to enable flip effect
        
        swapChainDesc.SampleDesc.Count = 1;                          // Do not use multi-sampling
        swapChainDesc.SampleDesc.Quality = 0;

        swapChainDesc.Windowed = true;
        swapChainDesc.OutputWindow = hwnd;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; 
        swapChainDesc.Flags = 0;
        
        SmartPtr<IDXGIDevice> spDxgiDevice;
        hr = m_spDevice.As(&spDxgiDevice);
        if (SUCCEEDED(hr))
        {
            SmartPtr<IDXGIAdapter> spDxgiAdapter;
            hr = spDxgiDevice->GetAdapter( &spDxgiAdapter );
       
            if (SUCCEEDED(hr))
            {
                SmartPtr<IDXGIFactory> spDxgiFactory;
                hr = spDxgiAdapter->GetParent(__uuidof( IDXGIFactory ), (void**)&spDxgiFactory);
            
                if (SUCCEEDED(hr))
                {
                    hr = spDxgiFactory->CreateSwapChain(m_spDevice, &swapChainDesc, &m_spSwapChain);
                }
            }
        }
    }
    // Otherwise get width and height from parameters
    else
    {
        width = targetWidth;
        height = targetHeight;
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateRenderTarget(width, height);
        if (SUCCEEDED(hr))
        {
            m_WindowWidth = width;
            m_WindowHeight = height;
        }
    }
    return hr;
}

void RendererDirectX11::GetWindowWidthAndHeight(__out UINT &width, __out UINT &height)
{
	width = m_WindowWidth;
	height = m_WindowHeight;
}

HRESULT RendererDirectX11::CreateRenderTarget(__in const UINT &width, __in const UINT &height)
{
    HRESULT hr;

    // Obtain the backbuffer for this window which will be the final 3D rendertarget
    SmartPtr<ID3D11Texture2D> pBackBuffer;

    if (m_spSwapChain)
    {
        hr = m_spSwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer);
    }
    else
    {
        m_spBackBuffer = NULL;

        CD3D11_TEXTURE2D_DESC bufferDesc(
                DXGI_FORMAT_R32G32B32A32_FLOAT, 
                width,
                height,
                1,
                1,
                D3D11_BIND_RENDER_TARGET
            );

        m_spBackBuffer = NULL;
        hr = m_spDevice->CreateTexture2D(&bufferDesc, NULL, &m_spBackBuffer);
        
        if (SUCCEEDED(hr))
        {
            pBackBuffer = m_spBackBuffer;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_spRenderTargetView = NULL;
        hr = m_spDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_spRenderTargetView);
        
        if (SUCCEEDED(hr))
        {
            D3D11_TEXTURE2D_DESC backBufferDesc;
            pBackBuffer->GetDesc( &backBufferDesc );
            
            CD3D11_TEXTURE2D_DESC depthStencilDesc(
                DXGI_FORMAT_D24_UNORM_S8_UINT, 
                backBufferDesc.Width,
                backBufferDesc.Height,
                1,
                1,
                D3D11_BIND_DEPTH_STENCIL
            );

            m_spDepthStencil = NULL;
            hr = m_spDevice->CreateTexture2D(&depthStencilDesc, NULL, &m_spDepthStencil);

            if (SUCCEEDED(hr)) 
            {
                m_spDepthStencilView = NULL;
                CD3D11_DEPTH_STENCIL_VIEW_DESC desc(D3D11_DSV_DIMENSION_TEXTURE2D);
                hr = m_spDevice->CreateDepthStencilView(m_spDepthStencil, &desc, &m_spDepthStencilView);
                
                if (SUCCEEDED(hr))
                {
                    CD3D11_VIEWPORT viewPort(0.0f,0.0f, float(backBufferDesc.Width), float(backBufferDesc.Height));
                    m_spDeviceContext->RSSetViewports(1, &viewPort);
                }
            }
        }
    }
    return hr;
}

HRESULT RendererDirectX11::InitializeRenderStates()
{
	HRESULT hr;

	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	hr = m_spDevice->CreateDepthStencilState(&depthStencilDesc, &m_spDepthStencilState);
	if(FAILED(hr))
	{
		return hr;
	}

	D3D11_RASTERIZER_DESC rasterDesc;

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	hr = m_spDevice->CreateRasterizerState(&rasterDesc, &m_spRasterState);
	if(FAILED(hr))
	{
		return hr;
	}

	// Now set the rasterizer state.
	m_spDeviceContext->RSSetState(m_spRasterState);

	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
	// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the state using the device.
	hr = m_spDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &m_spDepthDisabledStencilState);
	if(FAILED(hr))
	{
		return hr;
	}

	D3D11_BLEND_DESC blendStateDescription;
	// Clear the blend state description.
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	hr = m_spDevice->CreateBlendState(&blendStateDescription, &m_spAlphaEnableBlendingState);
	if(FAILED(hr))
	{
		return hr;
	}

	// Modify the description to create an alpha disabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

	// Create the blend state using the description.
	hr = m_spDevice->CreateBlendState(&blendStateDescription, &m_spAlphaDisableBlendingState);

	return hr;
}

void RendererDirectX11::TurnOnAlphaBlending()
{
	FLOAT blendFactor[4];

	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	
	// Turn on the alpha blending
	m_spDeviceContext->OMSetBlendState(m_spAlphaEnableBlendingState, blendFactor, 0xffffffff);
}

void RendererDirectX11::TurnOffAlphaBlending()
{
	FLOAT blendFactor[4];

	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	
	// Turn off the alpha blending
	m_spDeviceContext->OMSetBlendState(m_spAlphaDisableBlendingState, blendFactor, 0xffffffff);
}

void RendererDirectX11::TurnZBufferOn()
{
	m_spDeviceContext->OMSetDepthStencilState(m_spDepthStencilState, 1);
}

void RendererDirectX11::TurnZBufferOff()
{
	m_spDeviceContext->OMSetDepthStencilState(m_spDepthDisabledStencilState, 1);
}

HRESULT RendererDirectX11::OnSceneObjectChanged(__in ISceneObject *pSceneObject, SceneObjectContentType invalidContentMask)
{
    m_DrawRequired = true;
	
	return UpdateRenderableFromSceneObject(pSceneObject, invalidContentMask);	
}

HRESULT RendererDirectX11::UpdateRenderableFromSceneObject(__in ISceneObject *pSceneObject, SceneObjectContentType invalidContentMask)
{
	HRESULT hr = S_OK;

	if (invalidContentMask & SOCT_OBJECT_ADD)
	{
		HashPair<ISceneObject*, SmartPtr<IRenderableShape>>* pSceneObjectVP = m_ObjectsToRender[pSceneObject];
		if (pSceneObjectVP == NULL)
		{
			SmartPtr<IRenderableShape> spNewRenderableShape;        
            hr = CreateRenderableShape(m_spDevice, m_spDeviceContext, pSceneObject, m_spArialFont, &spNewRenderableShape);
			if (spNewRenderableShape != NULL)
			{
				hr = m_ObjectsToRender.Insert(pSceneObject, spNewRenderableShape);
			}
		}
	}
	else if (invalidContentMask & SOCT_SHAPE)
	{
		HashPair<ISceneObject*, SmartPtr<IRenderableShape>>* pSceneObjectVP = m_ObjectsToRender[pSceneObject];
		if (pSceneObjectVP != NULL)
		{
			hr = pSceneObjectVP->value->UpdateFromSceneObject(pSceneObject);
		}
	}
	else if (invalidContentMask & SOCT_OBJECT_REMOVE)
	{
		HashPair<ISceneObject*, SmartPtr<IRenderableShape>>* pSceneObjectVP = m_ObjectsToRender[pSceneObject];
		if (pSceneObjectVP != NULL)
		{
			pSceneObjectVP->value.Release();
			pSceneObjectVP->value = NULL;
			bool erasedSucessfully = m_ObjectsToRender.Erase(pSceneObject);
			if (!erasedSucessfully)
			{
				hr = E_FAIL;
			}
		}
	}

	return hr;
}

void RendererDirectX11::DrawIndexedPrimitive(__in ID3D11Buffer** ppVertexBuffer, UINT stride, __in ID3D11Buffer* pIndexBuffer, DXGI_FORMAT indexFormat, UINT indexCount, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	unsigned int offset = 0;
	m_spDeviceContext->IASetVertexBuffers(0, 1, ppVertexBuffer, &stride, &offset);
	m_spDeviceContext->IASetIndexBuffer(pIndexBuffer, indexFormat, 0);
	m_spDeviceContext->IASetPrimitiveTopology(primitiveTopology);

	m_spDeviceContext->DrawIndexed(indexCount, 0, 0);
}

void RendererDirectX11::DrawPrimitive(__in ID3D11Buffer** ppVertexBuffer, UINT stride, UINT vertexCount, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	unsigned int offset = 0;
	m_spDeviceContext->IASetVertexBuffers(0, 1, ppVertexBuffer, &stride, &offset);
	m_spDeviceContext->IASetPrimitiveTopology(primitiveTopology);

	m_spDeviceContext->Draw(vertexCount, 0);
}

HRESULT RendererDirectX11::TakeDepthRenderTargetSnapshot(__in const WCHAR *pszFilePath)
{
    if (!m_spCamera)
    {
        return E_FAIL;
    }

    // Clear render target and stencil buffer
    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_spDeviceContext->ClearRenderTargetView( m_spRenderTargetView, clearColor );
    m_spDeviceContext->ClearDepthStencilView( m_spDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    // Draw content
    m_spDeviceContext->OMSetRenderTargets( 1, &m_spRenderTargetView.p, m_spDepthStencilView );
    DrawSceneContent();

    // Create new texture with CPU access and copy stencil buffer to it
    D3D11_TEXTURE2D_DESC description =
    {
        m_WindowWidth, m_WindowHeight, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
        { 1, 0 }, // DXGI_SAMPLE_DESC
        D3D11_USAGE_STAGING,
        0, D3D11_CPU_ACCESS_READ, 0
    };

    SmartPtr<ID3D11Texture2D> spNewTexture;
    HRESULT hr = m_spDevice->CreateTexture2D(&description, NULL, &spNewTexture); 
    IF_FAILED_RETURN(hr);
    m_spDeviceContext->CopyResource(spNewTexture, m_spBackBuffer);

    //
    // Lock the texture and take the snapshot
    //
    D3D11_MAPPED_SUBRESOURCE mappedRes;
    hr = m_spDeviceContext->Map( spNewTexture, 0, D3D11_MAP_READ, 0, &mappedRes );
    IF_FAILED_RETURN(hr);

    FILE *pLogfile; 
    INT error = _wfopen_s(&pLogfile, pszFilePath, L"wb");
    if (error != 0)
    {
        return E_FAIL;
    }

    FLOAT* pDepthBuffer = (FLOAT*)mappedRes.pData;

    for (UINT y = 0; y < m_WindowHeight; ++y)
    {
        for (UINT x = 0;  x < m_WindowWidth; ++x)
        {
            FLOAT depth = pDepthBuffer[0];
            pDepthBuffer += 4;

            fwrite(&depth, sizeof(FLOAT), 1, pLogfile);
        }
    }

    error = fclose(pLogfile);
    if (error != 0)
    {
        return E_FAIL;
    }

    m_spDeviceContext->Unmap(spNewTexture, 0);

    return hr;
}

HRESULT CreateRendererDirectX11(HWND hwnd, __in ISceneGraph *pSceneGraph, __deref_out IRenderer **ppResult)
{
    return RendererDirectX11::CreateInstance(ppResult, hwnd, 0, 0, pSceneGraph);
}

HRESULT CreateRendererDirectX11(__in const UINT &width, __in const UINT &height, __in ISceneGraph *pSceneGraph, __deref_out IRenderer **ppResult)
{
    return RendererDirectX11::CreateInstance(ppResult, (HWND)NULL, width, height, pSceneGraph);
}
