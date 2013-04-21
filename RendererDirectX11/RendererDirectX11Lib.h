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

#include <d3d11.h>
#include "FontsLib.h"

DECLAREINTERFACE(IShader, IUnknown, "{CDADE052-E49B-4F2C-98CA-CBA0161790AB}")
{
	void SetVertextAndPixelShader();
};

DECLAREINTERFACE(IFontShader, IUnknown, "{08B8752B-B66C-4B1C-BD69-6FB87190F5B9}")
{
	HRESULT SetShaderParametersAndTexture(__in DirectX::CXMMATRIX worldMatrix, __in DirectX::CXMMATRIX viewMatrix, __in DirectX::CXMMATRIX projectionMatrix, __in ID3D11ShaderResourceView* texture);
};

DECLAREINTERFACE(IColorShader, IUnknown, "{B5E3BA19-0860-4CB0-A90F-0D5CA7CF556C}")
{
	HRESULT SetShaderParameters(__in DirectX::CXMMATRIX worldMatrix, __in DirectX::CXMMATRIX viewMatrix, __in DirectX::CXMMATRIX projectionMatrix);
};

DECLAREINTERFACE(ITextureShader, IUnknown, "{3E9AA2BB-C3FC-4392-8223-5D799562A952}")
{
	HRESULT SetShaderParametersAndTexture(__in DirectX::CXMMATRIX worldMatrix, __in DirectX::CXMMATRIX viewMatrix, __in DirectX::CXMMATRIX projectionMatrix, __in ID3D11ShaderResourceView* texture);
};

DECLAREINTERFACE(IRendererHandler, IUnknown, "{52362515-5910-42AB-80B5-23BCDF84915D}")
{
    HRESULT GetShaderInterface(const GUID &extensionId, __deref_opt_out IShader **ppShaderInterface);

    HRESULT GetCameraMatrices(__out DirectX::XMMATRIX *viewMatrix, __out DirectX::XMMATRIX *projectionMatrix);

	void TurnOffAlphaBlending();
	void TurnOnAlphaBlending();

	void TurnZBufferOn();
	void TurnZBufferOff();

	void DrawIndexedPrimitive(__in ID3D11Buffer** ppVertexBuffer, UINT stride, __in ID3D11Buffer* pIndexBuffer, DXGI_FORMAT indexFormat, UINT indexCount, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);
	void DrawPrimitive(__in ID3D11Buffer** ppVertexBuffer, UINT stride, UINT vertexCount, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);
};

DECLAREINTERFACE(IDepthMapShader, IUnknown, "{9A49AC67-A92C-4B3C-AAE1-B97EC15D6103}")
{
	HRESULT SetShaderParameters(__in DirectX::CXMMATRIX worldMatrix, __in DirectX::CXMMATRIX viewMatrix, __in DirectX::CXMMATRIX projectionMatrix);
};

DECLAREINTERFACE(IRenderableShape, IUnknown, "{F8282BDE-CA4D-468D-BD58-FAA8E30E8F91}")
{
    HRESULT UpdateFromSceneObject(__in ISceneObject *pSceneObject);

    UINT GetRequiredPasses();

    HRESULT Draw(UINT passIndex, __in IRendererHandler *pRenderer);
};

DECLAREINTERFACE(IRenderableTile, IUnknown, "{A2FA7C7B-AA8F-44AD-B368-6B84B977946C}")
{
    BOOL IsVisible(__in DirectX::CXMMATRIX viewMatrix, __in DirectX::CXMMATRIX projectionMatrix, __in DirectX::CXMMATRIX worldMatrix, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight);
    HRESULT LockTexture(__deref_out ID3D11ShaderResourceView** ppTextureSRV);
    void UnlockTexture();
    void ReleaseTexture();
    HRESULT GetBuffersAndPrimitiveTopology(__deref_out ID3D11Buffer **vertexBuffer, __out UINT &vertexCount, __deref_out ID3D11Buffer **indexBuffer, __out UINT &indexCount, __out D3D_PRIMITIVE_TOPOLOGY &primitiveTopology);
    HRESULT GetProjectedWorldToScreenDiff(__deref_in ISceneObjectCamera *pCamera, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight, __out FLOAT &difference);
    HRESULT GetVertex(__in const UINT &index, __out DirectX::XMFLOAT3 &vertex);
};


HRESULT CreateRendererDirectX11(HWND hwnd, __in ISceneGraph *pSceneGraph, __deref_out IRenderer **ppResult);

HRESULT CreateRendererDirectX11(__in const UINT &width, __in const UINT &height, __in ISceneGraph *pSceneGraph, __deref_out IRenderer **ppResult);

HRESULT CreateColorShader(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext, __deref_out IColorShader **ppResult);

HRESULT CreateFontShader(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext, __deref_out IFontShader **ppResult);

HRESULT CreateDepthMapShader(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext, __deref_out IDepthMapShader **ppResult);

HRESULT CreateTextureShader(__in ID3D11Device* pDevice, __in ID3D11DeviceContext* pDeviceContext, __deref_out ITextureShader **ppResult);


HRESULT CreateDxFont(__in ID3D11Device *pDevice, __in_z const wchar_t *fontName, __in const FLOAT &fontSize, __in const WORD &fontStyle, __in const BOOL &antiAliased, __deref_out IDxFont **ppResult);

HRESULT CreateRenderableShape(__in ID3D11Device *pDevice, __in ID3D11DeviceContext* pDeviceContext, __in ISceneObject *pSceneObject, __in IDxFont *pFont, __deref_out IRenderableShape **ppResult);