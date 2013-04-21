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

BaseShader::BaseShader()
{
};

BaseShader::~BaseShader()
{
};

HRESULT BaseShader::InitializeVertexShaderAndInputLayout(__in ID3D11Device* pDevice, __in const BYTE* shaderBytecode, __in const UINT shaderBytesCount, __in const D3D11_INPUT_ELEMENT_DESC *inputLayout, __in UINT inputLayoutLength)
{
	HRESULT hr = pDevice->CreateVertexShader(shaderBytecode, shaderBytesCount, NULL, &m_spVertexShader);
	
	if(SUCCEEDED(hr))
	{
		//
		// Initialize input layout
		//

		// Get a count of the elements in the layout
		unsigned int numElements = inputLayoutLength / sizeof(D3D11_INPUT_ELEMENT_DESC);

		// Create the vertex input layout.
		hr = pDevice->CreateInputLayout(inputLayout, numElements, shaderBytecode, shaderBytesCount, &m_spLayout);
	}

	return hr;
}

HRESULT BaseShader::InitializePixelShader(__in ID3D11Device* pDevice, __in const BYTE* shaderBytecode, __in const UINT shaderBytesCount)
{
	return pDevice->CreatePixelShader(shaderBytecode, shaderBytesCount, NULL, &m_spPixelShader);
}

HRESULT BaseShader::InitializeInternal(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext, __in const BYTE* vertexShaderBytecode, __in const UINT vsByteCount, __in const BYTE* pixelShaderBytecode, __in const UINT psByteCount, __in const D3D11_INPUT_ELEMENT_DESC *inputLayout, __in const UINT inputLayoutLength)
{
	HRESULT hr = InitializeVertexShaderAndInputLayout(pDevice, vertexShaderBytecode, vsByteCount, inputLayout, inputLayoutLength);

	if(SUCCEEDED(hr))
	{
		hr = InitializePixelShader(pDevice, pixelShaderBytecode, psByteCount);

		if(SUCCEEDED(hr))
		{
			hr = InitializeConstantBuffers(pDevice);

			if (SUCCEEDED(hr))
			{
				m_spDevice = pDevice;
				m_spDeviceContext = pDeviceContext;
			}
		}
	}

    return hr;
}

void BaseShader::SetVertextAndPixelShader()
{
	// Set the vertex input layout
	m_spDeviceContext->IASetInputLayout(m_spLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_spDeviceContext->VSSetShader(m_spVertexShader, NULL, 0);
	m_spDeviceContext->PSSetShader(m_spPixelShader, NULL, 0);
}

ID3D11Device* BaseShader::GetDevice()
{
	return m_spDevice;
}

ID3D11DeviceContext* BaseShader::GetDeviceContext()
{
	return m_spDeviceContext;
}
