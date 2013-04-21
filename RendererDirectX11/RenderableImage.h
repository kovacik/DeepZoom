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

class RenderableImage : public ImplementSmartObject
    <
        RenderableImage, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        IRenderableShape
    >
{
private:
    static const FLOAT VIEWPORT_SIZE;

    Vector<Vector<SmartPtr<IRenderableTile>>> m_LevelTiles;
    SmartPtr<IImageLoader> m_spImageLoader;
	SmartPtr<ISceneObject> m_spSceneObject;

	SmartPtr<ID3D11Device> m_spDevice; 
    SmartPtr<ID3D11DeviceContext> m_spDeviceContext;

    HRESULT DetermineLevelFromCamera(__deref_in ISceneObjectCamera *pCamera, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight, __out UINT &closestLevel);
    HRESULT CalibrateCamera(__deref_in ISceneObjectCamera *pCamera);
    void GetLevelViewportSize(__in const INT &level, __out FLOAT &width, __out FLOAT &height);
    HRESULT GetLevelProjectionMatrix(__in const UINT &level, __out DirectX::XMMATRIX &projectionMatrix);
    HRESULT GenerateTiles();

public:
    RenderableImage();
    ~RenderableImage();

    HRESULT UpdateFromSceneObject(__in ISceneObject *pSceneObject);
    UINT    GetRequiredPasses();
    HRESULT Draw(UINT passIndex, __in IRendererHandler *pRenderer);

	HRESULT Initialize(__in ID3D11Device *pDevice, __in ID3D11DeviceContext* pDeviceContext, __in ISceneObject *pSceneObject);
    HRESULT GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface);
};