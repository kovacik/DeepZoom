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
#include "SceneObjectImage.h"


SceneObjectImage::SceneObjectImage(void)
{
    m_pFilePath = NULL;
    m_IsCalibrated = TRUE;
}

SceneObjectImage::~SceneObjectImage(void)
{
    free(m_pFilePath);
}

HRESULT SceneObjectImage::Destroy()
{
    m_spImageLoader->Destroy();
    m_spImageLoader.Release();

    free(m_pFilePath);
    m_pFilePath = NULL;

    return SceneObject::Destroy();
}


HRESULT SceneObjectImage::Initialize(__in_z const WCHAR *nodeName, __in_z const WCHAR *filePath)
{
    HRESULT hr = SetName(nodeName);

    
    if (SUCCEEDED(hr))
    {
        hr = SetFilePath(filePath);

        if (SUCCEEDED(hr))
        {
            hr = CreateImageLoader(filePath, &m_spImageLoader);

            if (SUCCEEDED(hr))
            {
                hr = m_spImageLoader->Open();
            }
        }
    }

    return hr;
}

HRESULT SceneObjectImage::SetFilePath(__in_z const WCHAR *filePath)
{
	free(m_pFilePath);

    if (filePath == NULL)
    {
        m_pFilePath = NULL;
    }
    else
    {
        size_t l = wcslen(filePath) + 1;
        m_pFilePath = (WCHAR *)malloc(l * sizeof(WCHAR));
        if (!m_pFilePath)
            return E_OUTOFMEMORY;

        memcpy(m_pFilePath, filePath, l * sizeof(WCHAR));
    }

	return S_OK;
}

HRESULT SceneObjectImage::GetImageLoader(__deref_out IImageLoader **ppImageLoader)
{
    return m_spImageLoader.CopyTo(ppImageLoader);
}

HRESULT SceneObjectImage::Calibrate()
{
    m_IsCalibrated = FALSE;
    return S_OK;
}

BOOL SceneObjectImage::IsCalibrated()
{
    return m_IsCalibrated;
}

void SceneObjectImage::SetCalibrated()
{
    m_IsCalibrated = TRUE;
}

HRESULT CreateImage(__in_z const WCHAR *nodeName, __in_z const WCHAR *filePath, __deref_out ISceneObjectImage **ppResult)
{
    return SceneObjectImage::CreateInstance(ppResult, nodeName, filePath);
}