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
#include "DepthMapShader.h"

#include "DepthMapVS.hvs"
#include "DepthMapPS.hps"

using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC DepthMapShader::m_InputLayout[] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

DepthMapShader::DepthMapShader()
{
};

DepthMapShader::~DepthMapShader()
{
};

HRESULT DepthMapShader::InitializeConstantBuffers(__in ID3D11Device* pDevice)
{
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class
	return pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_spMatrixBuffer);
}

HRESULT DepthMapShader::Initialize(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext)
{
	return InitializeInternal(pDevice, pDeviceContext, g_vsdepthmapshader, sizeof(g_vsdepthmapshader), g_psdepthmapshader, sizeof(g_psdepthmapshader), m_InputLayout, sizeof(m_InputLayout));
}

HRESULT DepthMapShader::SetShaderParameters(__in CXMMATRIX worldMatrix, __in CXMMATRIX viewMatrix, __in CXMMATRIX projectionMatrix)
{
	// Transpose the matrices to prepare them for the shader
	XMMATRIX worldMatrixTransposed = XMMatrixTranspose(worldMatrix);
	XMMATRIX viewMatrixTransposed = XMMatrixTranspose(viewMatrix);
	XMMATRIX projectionMatrixTransposed = XMMatrixTranspose(projectionMatrix);

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to
	HRESULT hr = GetDeviceContext()->Map(m_spMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if(SUCCEEDED(hr))
	{
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
	}

	return hr;
}

HRESULT CreateDepthMapShader(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext, __deref_out IDepthMapShader **ppResult)
{
    return DepthMapShader::CreateInstance(ppResult, pDevice, pDeviceContext);
}