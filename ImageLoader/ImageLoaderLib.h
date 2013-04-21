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

struct ImageTileMetadata
{
    UINT X;
    UINT Y;
    UINT Width;
    UINT Height;
    UINT Level;
    UINT Row;
    UINT Column;
    WCHAR TilePath[MAX_PATH];
    ImageTileMetadata(UINT x, UINT y, UINT width, UINT height) : X(x), Y(y), Width(width), Height(height){};
    ImageTileMetadata() {ImageTileMetadata(0, 0, 0, 0);};
};

DECLAREINTERFACE(IDxImage, IUnknown, "{620C3AC9-2C90-4C6A-B0D5-C39408B6B133}")
{
    void GetSize(__out UINT &width, __out UINT &height);
    UINT GetFormat();
    UINT GetRowPitch();
    const BYTE* GetPixelData();
    HRESULT Destroy();
};

DECLAREINTERFACE(IImageLoader, IUnknown, "{CBAAB0D2-7090-4E4C-B768-557F3DC0BDA2}")
{
    HRESULT Open();
    HRESULT Destroy();
    UINT GetLevelCount();
    HRESULT GetLevelSize(__in const UINT &level, __out UINT &width, __out UINT &height);
    HRESULT GetLevelRowColumnCount(__in const UINT &level, __out UINT &rowCount, __out UINT &columnCount);
    HRESULT GetLevelRowColumnMetadata(__in const UINT &level, __in const UINT &row, __in const UINT &column, __out const ImageTileMetadata** ppTileMetadata);
    HRESULT GetLevelRowColumnImage(__in const UINT &level, __in const UINT &row, __in const UINT &column, __out IDxImage** ppImage);
};

HRESULT CreateImageLoader(__in_z const WCHAR *pFilePath, __deref_out IImageLoader **ppResult);