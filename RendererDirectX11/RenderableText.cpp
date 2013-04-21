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
#include "DxFont.h"
#include "RenderableText.h"

using namespace DirectX;

RenderableText::RenderableText()
{
    m_IndexCount = 0;
    m_VertexCount = 0;
}

RenderableText::~RenderableText()
{
}

UINT RenderableText::GetRequiredPasses()
{
    return 1;
}

HRESULT RenderableText::UpdateFromSceneObject(__in ISceneObject *pSceneObject)
{
    if (!pSceneObject)
    {
        return E_INVALIDARG;
    }

    SmartPtr<ISceneObjectText> spText;
    HRESULT hr = pSceneObject->QueryInterface(&spText);

    if (spText != NULL)
    {
        m_spSceneObjectText = spText;
        m_FontColor = spText->GetColor();
    }

    return hr;
}

HRESULT RenderableText::InitializeBuffers()
{
    HRESULT hr = InitializeIndexBuffer();

    if (SUCCEEDED(hr))
    {
        hr = InitializeDynamicVertexBuffer();
    }

    return hr;
}

HRESULT RenderableText::InitializeIndexBuffer()
{
    if (!m_spDevice)
    {
        return E_FAIL;
    }

    HRESULT hr;
    Vector<unsigned long> indices;

    for (unsigned long i = 0; i < MAX_RENDERABLE_CHARS_COUNT; ++i)
    {
        hr = indices.Add(i * 4);
        IF_FAILED_BREAK(hr);
        hr = indices.Add(i * 4 + 1);
        IF_FAILED_BREAK(hr);
        hr = indices.Add(i * 4 + 2);
        IF_FAILED_BREAK(hr);
        hr = indices.Add(i * 4);
        IF_FAILED_BREAK(hr);
        hr = indices.Add(i * 4 + 3);
        IF_FAILED_BREAK(hr);
        hr = indices.Add(i * 4 + 1);
        IF_FAILED_BREAK(hr);
    }

    if (SUCCEEDED(hr))
    {
        // Set up the description of the static index buffer
        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(unsigned long) * indices.Length();
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
        hr = m_spDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_spIndexBuffer);

        if (SUCCEEDED(hr))
        {
            m_IndexCount = indices.Length();
        }
    }

    return hr;
}

HRESULT RenderableText::InitializeDynamicVertexBuffer()
{
    if (!m_spDevice)
    {
        return E_FAIL;
    }

    UINT vertexCount = 4 * MAX_RENDERABLE_CHARS_COUNT;

    TexturedVertexType* pVertices = new TexturedVertexType[vertexCount];
	if(!pVertices)
	{
        return E_OUTOFMEMORY;
	}

    // Initialize vertex array to zeros at first.
	memset(pVertices, 0, (sizeof(TexturedVertexType) * vertexCount));

    // Set up the description of the dynamic vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(TexturedVertexType) * vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = pVertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Now create the vertex buffer
    HRESULT hr = m_spDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_spVertexBuffer);

    if (SUCCEEDED(hr))
    {
        m_VertexCount = vertexCount;
    }

    delete[] pVertices;

    return hr;
}

HRESULT RenderableText::Draw(UINT passIndex, __in IRendererHandler *pRenderer)
{	
    UNREFERENCED_PARAMETER(passIndex);

    XMMATRIX viewMatrix, projectionMatrix;
    HRESULT hr = pRenderer->GetCameraMatrices(&viewMatrix, &projectionMatrix);
    IF_FAILED_RETURN(hr);

    // Get suitable renderer
    SmartPtr<IRenderer> spRenderer;
    hr = pRenderer->QueryInterface(&spRenderer);
    IF_FAILED_RETURN(hr);

    UINT screenWidth = 0, screenHeight = 0;
    spRenderer->GetWindowWidthAndHeight(screenWidth, screenHeight);

    // Transform text position to screen coordinates
    XMFLOAT3 screenCoordinatesPosition;
    XMMATRIX worldMatrix = m_spSceneObjectText.CastTo<ISceneObject>()->GetWorldTransform();
    XMVECTOR textPosition = XMLoadFloat3(&m_spSceneObjectText->GetPosition());
    XMVECTOR transformedTextPosition = XMVector3Project(textPosition, 0, 0, (FLOAT)screenWidth, (FLOAT)screenHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, worldMatrix);  
    XMStoreFloat3(&screenCoordinatesPosition, transformedTextPosition);
    
    // Draw only if within view port
    if (screenCoordinatesPosition.x < 0.0f || screenCoordinatesPosition.x > screenWidth ||
        screenCoordinatesPosition.y < 0.0f || screenCoordinatesPosition.y > screenHeight ||
        screenCoordinatesPosition.z > 1.0f || screenCoordinatesPosition.z < 0.0f)
    {
        return hr;
    }

    // Update text vertices according to new screen coordinates
    hr = UpdateDynamicVertexBuffer(screenCoordinatesPosition, screenWidth, screenHeight);
    IF_FAILED_RETURN(hr);

    // Return if no text graphics created
    if (m_IndexCount == 0 || m_VertexCount == 0)
        return hr;

    // Get font shader wrapper
    SmartPtr<IShader> spShaderWrapper;
    hr = pRenderer->GetShaderInterface(__uuidof(IFontShader), &spShaderWrapper);
    IF_FAILED_RETURN(hr);

    // Get 2D projection matrices
    projectionMatrix = XMMatrixOrthographicLH((FLOAT)screenWidth, (FLOAT)screenHeight, 1.0f, 1000.0f);
	viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, -1.0f)), XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 0.0f)), XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f)));

    // Fill shader constants with transformation matrices
    hr = spShaderWrapper.CastTo<IFontShader>()->SetShaderParametersAndTexture(worldMatrix, viewMatrix, projectionMatrix, m_spFontSheet);
    IF_FAILED_RETURN(hr);

    spShaderWrapper->SetVertextAndPixelShader();

    pRenderer->TurnOnAlphaBlending();

    pRenderer->DrawIndexedPrimitive(&m_spVertexBuffer.p, sizeof(TexturedVertexType), m_spIndexBuffer, DXGI_FORMAT_R32_UINT, 
        m_IndexCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pRenderer->TurnOffAlphaBlending();		

    return hr;
}

HRESULT RenderableText::UpdateDynamicVertexBuffer(__in const XMFLOAT3 &position, __in const UINT &screenWidth, __in const UINT &screenHeight)
{
    HRESULT hr;

    // Get screen coordinates
    FLOAT posX = (FLOAT)(((screenWidth / 2) * -1.0f) + position.x);
    FLOAT posY = (FLOAT)(((screenHeight / 2)) - position.y);

    // Get text length
    const WCHAR* pText = m_spSceneObjectText->GetText();
    UINT textLength = lstrlenW(pText);

    Vector<TexturedVertexType> vertices;
    UINT indexCount = 0;
    for (UINT i = 0; i < textLength; ++i)
    {
        WCHAR character = pText[ i ];

        if( character == ' ' ) 
        {
            posX += m_spFont->GetSpaceWidth( );
        }
        else if( character == '\n' )        
        {
            posX  = position.x;
            posY += m_spFont->GetCharHeight( );
        }
        else
        {
            Fonts::Rect charRect = m_spFont->GetCharRect(character);

            FLOAT width = charRect.right - charRect.left;
            FLOAT height = charRect.bottom - charRect.top;

            Fonts::Rect destRect = Fonts::Rect(posX, posY, posX + width, posY - height);


            if (abs(posX) > (screenWidth / 2) || abs(posY) > (screenHeight / 2))
            {
                continue;
            }

            TexturedVertexType pQuad[4];
                
            hr = BuildTexturedQuad(charRect, destRect, m_FontColor, m_TexWidth, m_TexHeight, pQuad); 
            IF_FAILED_RETURN(hr);

            pQuad[0].Position.z = position.z;
            pQuad[1].Position.z = position.z;
            pQuad[2].Position.z = position.z;
            pQuad[3].Position.z = position.z;


            hr = vertices.Add(pQuad, 4);
            IF_FAILED_RETURN(hr);

            indexCount += 6;

            posX += width + 1;
        }
    }

    // Lock the vertex buffer so it can be written to
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = m_spDeviceContext->Map(m_spVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    IF_FAILED_RETURN(hr);

    // Get a pointer to the data in the vertex buffer
	TexturedVertexType* verticesPtr = (TexturedVertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer
    memcpy(verticesPtr, (void*)vertices.Ptr(), (sizeof(TexturedVertexType) * vertices.Length()));
    
    // Unlock the vertex buffer
    m_spDeviceContext->Unmap(m_spVertexBuffer, 0);

    m_VertexCount = vertices.Length();
    m_IndexCount = indexCount;

    return hr;
}

HRESULT RenderableText::BuildTexturedQuad(__in const Fonts::Rect &sourceRect, __in const Fonts::Rect &destinationRect, __in XMFLOAT4 &color, 
                                          __in const UINT &textureWidth, __in const UINT &textureHeight,  
                                          __deref_out_ecount(4) TexturedVertexType *pQuad)
{
    if (!pQuad)
    {
        return E_INVALIDARG;
    }

    pQuad[0].Position = XMFLOAT3(destinationRect.left, destinationRect.top, 0.0f); 
    pQuad[1].Position = XMFLOAT3(destinationRect.right,  destinationRect.bottom, 0.0f);
    pQuad[2].Position = XMFLOAT3(destinationRect.left, destinationRect.bottom, 0.0f);
    pQuad[3].Position = XMFLOAT3(destinationRect.right, destinationRect.top, 0.0f);

    pQuad[0].Texture = XMFLOAT2((FLOAT)sourceRect.left / textureWidth, (FLOAT)sourceRect.top / textureHeight); 
    pQuad[1].Texture = XMFLOAT2((FLOAT)sourceRect.right / textureWidth, (FLOAT)sourceRect.bottom / textureHeight); 
    pQuad[2].Texture = XMFLOAT2((FLOAT)sourceRect.left / textureWidth, (FLOAT)sourceRect.bottom / textureHeight); 
    pQuad[3].Texture = XMFLOAT2((FLOAT)sourceRect.right / textureWidth, (FLOAT)sourceRect.top / textureHeight); 

    pQuad[0].Color = color;
    pQuad[1].Color = color;
    pQuad[2].Color = color;
    pQuad[3].Color = color;

    FLOAT tx = 0.5f * (pQuad[0].Position.x + pQuad[3].Position.x);
    FLOAT ty = 0.5f * (pQuad[0].Position.y + pQuad[1].Position.y);

    XMVECTOR scaling        = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
    XMVECTOR origin         = XMVectorSet(tx, ty, 0.0f, 0.0f);
    XMVECTOR translation    = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMMATRIX T              = XMMatrixAffineTransformation2D(scaling, origin, 0.0f, translation);

    for (int i = 0; i < 4; ++i)
    {
        XMVECTOR p = XMLoadFloat3(&pQuad[i].Position);
        p = XMVector3TransformCoord(p, T);
        XMStoreFloat3(&pQuad[i].Position, p);
    }

    return S_OK;
}

HRESULT RenderableText::Initialize(__in ID3D11Device *pDevice, __in ID3D11DeviceContext* pDeviceContext, __in ISceneObject *pSceneObject, __in IDxFont *pFont)
{
    if (!pDevice || !pDeviceContext || !pSceneObject || !pFont)
    {
        return E_INVALIDARG;
    }

    m_spDevice = pDevice;
    m_spDeviceContext = pDeviceContext;
    m_spFont = pFont;
    m_spFontSheet = m_spFont->GetFontSheetSRV();

    //
    // Get font sheet width and height
    //
    SmartPtr<ID3D11Resource> spResource;
    m_spFontSheet->GetResource(&spResource);

    SmartPtr<ID3D11Texture2D> spTexture;
    HRESULT hr = spResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&spTexture);

    if (SUCCEEDED(hr))
    {
        D3D11_TEXTURE2D_DESC texDesc;
        spTexture->GetDesc( &texDesc );

        m_TexWidth  = texDesc.Width; 
        m_TexHeight = texDesc.Height;

        hr = InitializeBuffers();

        if (SUCCEEDED(hr))
        {
            hr = UpdateFromSceneObject(pSceneObject);
        }
    }

    return hr;
}
