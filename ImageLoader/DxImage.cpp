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
#include "DxImage.h"


DxImage::DxImage()
{
    m_Width = 0;
    m_Height = 0;
    m_Format = 0; // 0 == DXGI_FORMAT_UNKNOWN
    m_RowPitch = 0;
    m_SlicePitch = 0;
    m_PixelData = NULL;
}

DxImage::~DxImage()
{
    free(m_PixelData);
}

HRESULT DxImage::Destroy()
{
    m_Width = 0;
    m_Height = 0;
    m_Format = 0; // 0 == DXGI_FORMAT_UNKNOWN
    m_RowPitch = 0;
    m_SlicePitch = 0;

    free(m_PixelData);
    m_PixelData = NULL;

    return S_OK;
}

HRESULT DxImage::Initialize(__in const WICRect &rect, __in const UINT &dxFormat, __in const UINT &rowPitch, __deref_in IWICFormatConverter *pConverter)
{
    m_Width = rect.Width;
    m_Height = rect.Height;
    m_Format = dxFormat;
    m_RowPitch = rowPitch;

    UINT bufferSize = m_RowPitch * m_Height;
    
    // Allocate space for pixel data
    m_PixelData = reinterpret_cast<BYTE*>(malloc(bufferSize));
    if (!m_PixelData)
    {
        return E_OUTOFMEMORY;
    }

    return pConverter->CopyPixels(&rect, m_RowPitch, bufferSize, m_PixelData);
}

void DxImage::GetSize(__out UINT &width, __out UINT &height)
{
    width = m_Width;
    height = m_Height;
}

UINT DxImage::GetFormat()
{
    return m_Format;
}

UINT DxImage::GetRowPitch()
{
    return m_RowPitch;
}

const UINT8* DxImage::GetPixelData()
{
    return m_PixelData;
}