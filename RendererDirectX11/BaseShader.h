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

class BaseShader
	: public ImplementsInterfaces<IShader>
{
private:

	SmartPtr<ID3D11VertexShader> m_spVertexShader;
	SmartPtr<ID3D11PixelShader> m_spPixelShader;
	SmartPtr<ID3D11InputLayout> m_spLayout;

	SmartPtr<ID3D11Device> m_spDevice;
	SmartPtr<ID3D11DeviceContext> m_spDeviceContext;

	HRESULT InitializeVertexShaderAndInputLayout(__in ID3D11Device* pDevice, __in const BYTE* shaderBytecode, __in const UINT shaderBytesCount, __in const D3D11_INPUT_ELEMENT_DESC *inputLayout, __in UINT inputLayoutLength);
	HRESULT InitializePixelShader(__in ID3D11Device* pDevice, __in const BYTE* shaderBytecode, __in const UINT shaderBytesCount);	

protected:
	HRESULT InitializeInternal(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext, __in const BYTE* vertexShaderBytecode, __in const UINT vsByteCount, __in const BYTE* pixelShaderBytecode, __in const UINT psByteCount, __in const D3D11_INPUT_ELEMENT_DESC *inputLayout, __in const UINT inputLayoutLength);
	virtual HRESULT InitializeConstantBuffers(__in ID3D11Device* pDevice) = 0;

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

public:
	BaseShader();
	~BaseShader();

	void SetVertextAndPixelShader();
};