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

#include "RenderableMesh.h"
#include "RenderableText.h"
#include "RenderableDepthMapMesh.h"
#include "RenderableImage.h"


//--------------------------------------------------------------------------------------
// Factory for renderable objects
//--------------------------------------------------------------------------------------
HRESULT CreateRenderableShape(__in ID3D11Device *pDevice, __in ID3D11DeviceContext* pDeviceContext, __in ISceneObject *pSceneObject, __in IDxFont *pFont, __deref_out IRenderableShape **ppResult)
{
    SmartPtr<ISceneObject> spSceneObject(pSceneObject);

    SmartPtr<ISceneObjectDepthMapMesh> spDepthMapMesh;
    HRESULT hr = spSceneObject->QueryInterface(&spDepthMapMesh);

    if (spDepthMapMesh != NULL)
    {
        return RenderableDepthMapMesh::CreateInstance(ppResult, pDevice, spSceneObject);
    }

    SmartPtr<ISceneObjectMesh> spMesh;
    hr = spSceneObject->QueryInterface(&spMesh);

    if (spMesh == NULL)
    {	
        hr = spSceneObject->GetInterface(__uuidof(ISceneObjectMesh), (IUnknown**)&spMesh);
    }

    if (spMesh != NULL)
    {
        hr = RenderableMesh::CreateInstance(ppResult, pDevice, spSceneObject);
    }

    if (spMesh == NULL)
    {
        SmartPtr<ISceneObjectText> spText;
        hr = spSceneObject->QueryInterface(&spText);

        if (SUCCEEDED(hr))
        {
            hr = RenderableText::CreateInstance(ppResult, pDevice, pDeviceContext, spSceneObject, pFont);
        }

        if (!spText)
        {
            SmartPtr<ISceneObjectImage> spImage;
            hr = spSceneObject->QueryInterface(&spImage);
            if (SUCCEEDED(hr))
            {
                hr = RenderableImage::CreateInstance(ppResult, pDevice, pDeviceContext, spSceneObject);
            }
        }
    }

    return hr;
}