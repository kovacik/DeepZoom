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

class ImageLoaderWIC : public ImplementSmartObject
    <
        ImageLoaderWIC, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        IImageLoader
    >
{ 
    static const UINT CHUNK_HEIGHT = 16;
    
    // Has to be multiple of CHUNK_HEIGHT;
    static const UINT TILE_SIZE = CHUNK_HEIGHT * 64;


    struct LevelBuffer
    {
        BYTE* BasePtr;
        BYTE* CurrentPtr;
        UINT CurrentLine;
        UINT CbStride;
    
        UINT Width;
        UINT Height;
        UINT CbSize;
        UINT PageOf32BytesCount;

        LevelBuffer() : BasePtr(NULL), CurrentPtr(NULL), CurrentLine(0), CbStride(0), PageOf32BytesCount(0), Width(0), Height(0), CbSize(0) {};
        HRESULT Initialize(__in const UINT &width, __in const UINT &height, __in const UINT &cbPixelSize)
        {
            CurrentLine = 0;

            Height = height;
            Width = width;

            PageOf32BytesCount = (width * cbPixelSize + 31) / 32;
            CbStride = PageOf32BytesCount * 32;
            CbSize = CbStride * height;

            BasePtr = reinterpret_cast<BYTE*>(_aligned_malloc(CbSize, 16));
            if (!BasePtr)
            {
                return E_OUTOFMEMORY;
            }

            CurrentPtr = BasePtr;

            return S_OK;
        }

        ~LevelBuffer()
        {
            _aligned_free(BasePtr);
        }
    };

    // Structure just for internal storage of level metadata
    struct ImageLevelMetadata
    {
        UINT ImageWidth;
        UINT ImageHeight;
        UINT ColumnCount;
        UINT RowCount;
        ImageTileMetadata* Tiles;

        ImageLevelMetadata() : ImageWidth(0), ImageHeight(0), ColumnCount(0), RowCount(0), Tiles(NULL) {};
        
        ImageLevelMetadata(UINT width, UINT height, UINT columnCount, UINT rowCount) 
            : ImageWidth(width), ImageHeight(height), ColumnCount(columnCount), RowCount(rowCount), Tiles(NULL) {};
        
        ~ImageLevelMetadata()
        {
            delete[] Tiles;
        }

        HRESULT Initialize(__in const UINT &width, __in const UINT &height, __in const UINT &columnCount, __in const UINT &rowCount)
        {
            ImageWidth = width;
            ImageHeight = height;
            ColumnCount = columnCount;
            RowCount = rowCount;

            Tiles = new ImageTileMetadata[columnCount * rowCount];
            if (!Tiles)
            {
                return E_OUTOFMEMORY;
            }

            return S_OK;
        }
    };

    UINT m_ImageWidth;
    UINT m_ImageHeight;
    Vector<ImageLevelMetadata> m_Levels;

    String m_FilePath;
    String m_UltraZoomDirectory;
    String m_FileExtension;
 
    SmartPtr<IWICImagingFactory> m_spImagingFactory;

    HRESULT SaveBitmapToFile(__in const WCHAR* pFilePath, __in const GUID &containerFormat, __in const WICPixelFormatGUID *pPixelFormat, __in IWICBitmap *pBitmap);
    HRESULT Get32bppBGRFrameConverter(__in IWICBitmapDecoder *spDecoder,  __in const UINT &frame, __deref_out IWICFormatConverter **ppFormatConverter);
    
    HRESULT GetTilePath(__in const UINT &level, __in const UINT &column,__in const UINT &row, __in const UINT &size, __deref_out_ecount_z(size + 1) WCHAR *pTilePath);
    HRESULT GetLevelPath(__in const UINT &level, __in const UINT &size, __deref_out_ecount_z(length + 1) WCHAR *pLevelPath);

    HRESULT GenerateDeepZoomPyramid(__in IWICFormatConverter *pFormatConverter, __in const UINT &tileSize, __in const GUID &containerGuid, __in const WICPixelFormatGUID &pixelFormat);
    HRESULT SaveBitmapRectToFile(__in IWICBitmap *pBitmap, __in const GUID &containerFormat, __in const WICPixelFormatGUID *pPixelFormat, __in const WICRect &rect, __in const WCHAR *pTilePath);
    UINT    GetMaximumLevel(__in const UINT &width, __in const UINT &height, __in const UINT &minTileSize);

public:
    ImageLoaderWIC();
    ~ImageLoaderWIC();

    HRESULT Initialize(__in_z const WCHAR *pFilePath);

    HRESULT Open();
    HRESULT Destroy();

    UINT    GetLevelCount();
    HRESULT GetLevelSize(__in const UINT &level, __out UINT &width, __out UINT &height);
    HRESULT GetLevelRowColumnCount(__in const UINT &level, __out UINT &rowCount, __out UINT &columnCount);
    HRESULT GetLevelRowColumnMetadata(__in const UINT &level, __in const UINT &row, __in const UINT &column, __out const ImageTileMetadata** ppTileMetadata);
    HRESULT GetLevelRowColumnImage(__in const UINT &level, __in const UINT &row, __in const UINT &column, __out IDxImage** ppImage);
};