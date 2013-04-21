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
#include "BaseShader.h"
#include "TextureShader.h"

#include "TextureVS.hvs"
#include "TexturePS.hps"

using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC TextureShader::m_InputLayout[] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

TextureShader::TextureShader()
{
};

TextureShader::~TextureShader()
{
};

HRESULT TextureShader::InitializeConstantBuffers(__in ID3D11Device* pDevice)
{
	HRESULT hr;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class
	hr = pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_spMatrixBuffer);

	if (FAILED(hr))
	{
		return hr;
	}

	// Create a texture sampler state description
	D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state
	hr = pDevice->CreateSamplerState(&samplerDesc, &m_spSampleState);
	
	return hr;
}

HRESULT TextureShader::Initialize(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext)
{
	return InitializeInternal(pDevice, pDeviceContext, g_vstextureshader, sizeof(g_vstextureshader), g_pstextureshader, sizeof(g_pstextureshader), m_InputLayout, sizeof(m_InputLayout));
}

HRESULT TextureShader::SetShaderParametersAndTexture(__in CXMMATRIX worldMatrix, __in CXMMATRIX viewMatrix, __in CXMMATRIX projectionMatrix, __in ID3D11ShaderResourceView* texture)
{
	// Transpose the matrices to prepare them for the shader
	XMMATRIX worldMatrixTransposed = XMMatrixTranspose(worldMatrix);
	XMMATRIX viewMatrixTransposed = XMMatrixTranspose(viewMatrix);
	XMMATRIX projectionMatrixTransposed = XMMatrixTranspose(projectionMatrix);

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to
	HRESULT hr = GetDeviceContext()->Map(m_spMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if(FAILED(hr))
	{
		return hr;
	}

	MatrixBufferType* dataPtr;

	// Get a pointer to the data in the constant buffer
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer
	dataPtr->mWorld = worldMatrixTransposed;
	dataPtr->mView = viewMatrixTransposed;
	dataPtr->mProjection = projectionMatrixTransposed;

	// Unlock the constant buffer
	GetDeviceContext()->Unmap(m_spMatrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader
	unsigned int bufferNumber = 0;

	// Set the constant buffer in the vertex shader with the updated values
	ID3D11Buffer* pMatrixBuffer = m_spMatrixBuffer;
	GetDeviceContext()->VSSetConstantBuffers(bufferNumber, 1, &pMatrixBuffer);

	// Set shader texture resource in the pixel shader
	GetDeviceContext()->PSSetShaderResources(0, 1, &texture);
	
	// Set the sampler state in the pixel shader
	ID3D11SamplerState*	pSampleState = m_spSampleState;
	GetDeviceContext()->PSSetSamplers(0, 1, &pSampleState);

	return hr;
}

HRESULT CreateTextureShader(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext, __deref_out ITextureShader **ppResult)
{
    return TextureShader::CreateInstance(ppResult, pDevice, pDeviceContext);
}