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

class DxImage : public ImplementSmartObject
    <
        DxImage, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        IDxImage
    >
{

    UINT        m_Width;
    UINT        m_Height;
    UINT        m_Format;
    UINT        m_RowPitch;
    UINT        m_SlicePitch;
    BYTE*       m_PixelData;
 
public:
    DxImage();
    ~DxImage();

    HRESULT Initialize(__in const WICRect &rect, __in const UINT &dxFormat, __in const UINT &rowPitch, __deref_in IWICFormatConverter *pConverter);

    void GetSize(__out UINT &width, __out UINT &height);
    UINT GetFormat();
    UINT GetRowPitch();
    const UINT8* GetPixelData(); 

    HRESULT Destroy();
};

