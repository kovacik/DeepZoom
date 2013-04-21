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
#include "RenderableImage.h"

using namespace DirectX;

const FLOAT RenderableImage::VIEWPORT_SIZE = 1024.0f;

RenderableImage::RenderableImage()
{
}

RenderableImage::~RenderableImage()
{
}

UINT RenderableImage::GetRequiredPasses()
{
    return 1;
}

//-----------------------------------------------------------------------------
// Get viewport size for the level
//-----------------------------------------------------------------------------
void RenderableImage::GetLevelViewportSize(__in const INT &level, __out FLOAT &width, __out FLOAT &height)
{
    width = pow(2.0f, level) * VIEWPORT_SIZE;
    height = pow(2.0f, level) * VIEWPORT_SIZE;
}

//-----------------------------------------------------------------------------
// Get projection matrix for the level
//-----------------------------------------------------------------------------
HRESULT RenderableImage::GetLevelProjectionMatrix(__in const UINT &level, __out XMMATRIX &projectionMatrix)
{
    FLOAT viewportWidth, viewportHeight;
    GetLevelViewportSize(level, viewportWidth, viewportHeight);
    projectionMatrix = XMMatrixOrthographicOffCenterLH(-viewportWidth / 2.0f, viewportWidth / 2.0f, -viewportHeight / 2.0f, viewportHeight / 2.0f, 1.0f, 1000.0f);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Generate image tile renderables
//-----------------------------------------------------------------------------
HRESULT RenderableImage::GenerateTiles()
{
    // Create camera for making projections for different scales
    SmartPtr<ISceneObjectCamera> spCamera;
    HRESULT hr = CreateCamera(L"myCamera", ProjectionType::Orthographic, &spCamera);
    IF_FAILED_RETURN(hr);

    // Fix the viewport and change just projection matrix
    hr = spCamera->SetViewport(-VIEWPORT_SIZE / 2.0f, -VIEWPORT_SIZE / 2.0f, VIEWPORT_SIZE, VIEWPORT_SIZE);
    IF_FAILED_RETURN(hr);

    XMMATRIX worldMatrix = m_spSceneObject->GetWorldTransform();

    INT levelCount = m_spImageLoader->GetLevelCount();
    hr = m_LevelTiles.SetSize(levelCount);
    IF_FAILED_RETURN(hr);

    for (INT level = levelCount - 1; level >= 0; --level)
    {
        // Change scale and generate tiles for another level
        XMMATRIX projectionMatrix;
        hr = GetLevelProjectionMatrix(level, projectionMatrix);
        IF_FAILED_RETURN(hr);

        hr = spCamera->SetProjectionMatrix(projectionMatrix);
        IF_FAILED_RETURN(hr);

        UINT rowCount = 0, columnCount = 0;
        hr = m_spImageLoader->GetLevelRowColumnCount(level, rowCount, columnCount);
        IF_FAILED_RETURN(hr);
            
        for (UINT row = 0; row < rowCount; ++row)
        {
            for (UINT column = 0; column < columnCount; ++column)
            {
                const ImageTileMetadata* tileMetadata;
                m_spImageLoader->GetLevelRowColumnMetadata(level, row, column, &tileMetadata);
                SmartPtr<IRenderableTile> spTile;
                hr = RenderableImageTile::CreateInstance(&spTile, m_spDevice, m_spDeviceContext, m_spImageLoader, spCamera, &worldMatrix, tileMetadata);
                IF_FAILED_RETURN(hr);

                hr = m_LevelTiles[level].Add(spTile);
                IF_FAILED_RETURN(hr);
            }
        }
    }

    return hr;
}

HRESULT RenderableImage::UpdateFromSceneObject(__in ISceneObject *pSceneObject)
{
    if (!pSceneObject)
    {
        return E_INVALIDARG;
    }

    SmartPtr<ISceneObjectImage> spImage;
    HRESULT hr = pSceneObject->QueryInterface(&spImage);

    if (spImage != NULL)
    {    
        m_spSceneObject = pSceneObject;
        m_spImageLoader = NULL;
        
        hr = spImage->GetImageLoader(&m_spImageLoader);
        IF_FAILED_RETURN(hr);

        // Generate tiles only when not already generated
        if (m_LevelTiles.Length() == 0)
        {
            hr = GenerateTiles();
        }
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Determine suitable image level for actual camera
//-----------------------------------------------------------------------------
HRESULT RenderableImage::DetermineLevelFromCamera(__deref_in ISceneObjectCamera *pCamera, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight, __out UINT &closestLevel)
{  
    HRESULT hr = E_FAIL;

    FLOAT minDifference = 0.0f;
    UINT bestLevel = 0;

    for (UINT level = 0; level < m_LevelTiles.Length(); ++level)
    {
        FLOAT diff;
        hr = m_LevelTiles[level][0]->GetProjectedWorldToScreenDiff(pCamera, viewportWidth, viewportHeight, diff);
        IF_FAILED_RETURN(hr);
        
        if (diff < minDifference || 0 == level)
        {
            minDifference = diff;
            bestLevel = level;
        }
    }

    closestLevel = bestLevel;

    return hr;
}

//-----------------------------------------------------------------------------
// Center and zoom camera, so that the image is in the middle of the screen
//-----------------------------------------------------------------------------
HRESULT RenderableImage::CalibrateCamera(__deref_in ISceneObjectCamera *pCamera)
{  
    SmartPtr<ISceneObjectCamera> spCamera(pCamera);

    HRESULT hr;

    UINT bestLevel = 0;
    FLOAT screenWidth = 0, screenHeight = 0;
    hr = spCamera->GetScreenSize(screenWidth, screenHeight);
    IF_FAILED_RETURN(hr);

    // Find the most suitable level, level with image width similar to screen width
    // 
    UINT width = 0, height = 0;
    for (UINT level = 0; level < m_LevelTiles.Length(); ++level)
    {
        hr = m_spImageLoader->GetLevelSize(level, width, height);
        IF_FAILED_RETURN(hr);
        
        if ((width - screenWidth <= 0) && (height - screenHeight <= 0))
        {
            bestLevel = level;
            break;
        }
    }

    // Get edge vertices of this level
    //
    XMFLOAT3 rect[2];
    hr = m_LevelTiles[bestLevel][0]->GetVertex(0, rect[0]);
    IF_FAILED_RETURN(hr);

    hr = m_LevelTiles[bestLevel].GetLast()->GetVertex(2, rect[1]);
    IF_FAILED_RETURN(hr);

    // Get level projection matrix and actual view and world matrices
    //
    XMMATRIX projectionMatrix, viewMatrix, worldMatrix;

    hr = spCamera->GetViewMatrix(&viewMatrix);
    IF_FAILED_RETURN(hr);

    worldMatrix = m_spSceneObject->GetWorldTransform();

    hr = GetLevelProjectionMatrix(bestLevel, projectionMatrix);
    IF_FAILED_RETURN(hr);

    // Project edge vertices to level screen space
    //
    XMFLOAT3 projectedRect[2];
    XMVector3ProjectStream(projectedRect, sizeof(XMFLOAT3), rect, sizeof(XMFLOAT3), 2, 0.0f, 0.0f, 
        screenWidth, screenHeight, 0.0f, 1.0f, projectionMatrix, viewMatrix, worldMatrix);


    hr = m_spImageLoader->GetLevelSize(bestLevel, width, height);
    IF_FAILED_RETURN(hr);

    // Compute the viewport, so that the image width and projected image width is equal for current screen space
    //
    FLOAT viewportWidth = 0, viewportHeight = 0;
    GetLevelViewportSize(bestLevel, viewportWidth, viewportHeight);
    viewportWidth = (viewportWidth / (FLOAT)width) * abs(projectedRect[0].x - projectedRect[1].x);

    // Set computed viewport (zoom) and center the image
    //
    hr = spCamera->SetViewport(-(viewportWidth / 2.0f), -(viewportWidth / 2.0f), viewportWidth,viewportWidth);
    IF_FAILED_RETURN(hr);

    hr = spCamera->PanZoomOrthographic(width / 2.0f, -(height / 2.0f), 0.0f);
    IF_FAILED_RETURN(hr);

    hr = spCamera->ApplyViewportOffsetX();
    IF_FAILED_RETURN(hr);

    hr = spCamera->ApplyViewportOffsetY();

    return hr;
}

HRESULT RenderableImage::Draw(UINT passIndex, __in IRendererHandler *pRenderer)
{	
    UNREFERENCED_PARAMETER(passIndex);
    HRESULT hr;

    // Get suitable renderer
    SmartPtr<IRenderer> spRenderer;
    hr = pRenderer->QueryInterface(&spRenderer);
    IF_FAILED_RETURN(hr);
    
    // Calibrate
    //
    SmartPtr<ICalibrable> spCalibrable = m_spSceneObject.CastTo<ICalibrable>();
    if (spCalibrable && !spCalibrable->IsCalibrated())
    {
        XMMATRIX viewMatrix, projectionMatrix;
        SmartPtr<ISceneObjectCamera> spCamera;
        hr = spRenderer->GetCamera(&spCamera);
        IF_FAILED_RETURN(hr);

        hr = CalibrateCamera(spCamera);
        IF_FAILED_RETURN(hr);

        spCalibrable->SetCalibrated();
    }

    SmartPtr<IRendererHandler> spRendererHandler(pRenderer);

    XMMATRIX viewMatrix, projectionMatrix;
    hr = spRendererHandler->GetCameraMatrices(&viewMatrix, &projectionMatrix);
    IF_FAILED_RETURN(hr);

    // Get object world transform
	XMMATRIX worldMatrix = m_spSceneObject->GetWorldTransform();

    UINT screenWidth = 0, screenHeight = 0;
    spRenderer->GetWindowWidthAndHeight(screenWidth, screenHeight);

    SmartPtr<ISceneObjectCamera> spCamera;
    hr = spRenderer->GetCamera(&spCamera);
    IF_FAILED_RETURN(hr);

    UINT level = 0;
    hr = DetermineLevelFromCamera(spCamera, (FLOAT)screenWidth, (FLOAT)screenHeight, level);
    IF_FAILED_RETURN(hr);

    // Get suitable shader
	SmartPtr<IShader> spShader;
    hr = pRenderer->GetShaderInterface(__uuidof(ITextureShader), &spShader);
    IF_FAILED_RETURN(hr);
		

    for (UINT i = 0; i < m_LevelTiles[level].Length(); ++i)
    {
        if (m_LevelTiles[level][i]->IsVisible(viewMatrix, projectionMatrix, worldMatrix, (FLOAT)screenWidth, (FLOAT)screenHeight))
        {
            SmartPtr<ID3D11ShaderResourceView> spTexture;
            hr = m_LevelTiles[level][i]->LockTexture(&spTexture);
            IF_FAILED_RETURN(hr);

            // Fill shader constants with transformation matrices
            hr = spShader.CastTo<ITextureShader>()->SetShaderParametersAndTexture(worldMatrix, viewMatrix, projectionMatrix, spTexture);
            IF_FAILED_RETURN(hr);

            spShader->SetVertextAndPixelShader();

            D3D_PRIMITIVE_TOPOLOGY primitiveTopology;
            UINT vertexCount = 0, indexCount = 0;
            SmartPtr<ID3D11Buffer> spVertexBuffer, spIndexBuffer;
            hr = m_LevelTiles[level][i]->GetBuffersAndPrimitiveTopology(&spVertexBuffer, vertexCount, &spIndexBuffer, indexCount, primitiveTopology); 
            IF_FAILED_RETURN(hr);      

            ID3D11Buffer* vertexBuffer = spVertexBuffer;
            pRenderer->DrawIndexedPrimitive(&vertexBuffer, sizeof(TexturedVertexType), spIndexBuffer, DXGI_FORMAT_R32_UINT, 
                                            indexCount, primitiveTopology);
        }
        else
        {
            m_LevelTiles[level][i]->ReleaseTexture();
        }

    }

    return hr;
}

HRESULT RenderableImage::Initialize(__in ID3D11Device *pDevice, __in ID3D11DeviceContext* pDeviceContext, __in ISceneObject *pSceneObject)
{
    if (!pDevice || !pDeviceContext || !pSceneObject)
    {
        return E_INVALIDARG;
    }

    m_spDevice = pDevice;
    m_spDeviceContext = pDeviceContext;

    return UpdateFromSceneObject(pSceneObject);
}
