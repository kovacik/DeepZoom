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
#include "ImageLoaderWIC.h"

ImageLoaderWIC::ImageLoaderWIC()
{
    m_ImageHeight = 0;
    m_ImageWidth = 0;
}

ImageLoaderWIC::~ImageLoaderWIC()
{
}

HRESULT ImageLoaderWIC::Destroy()
{ 
    m_ImageHeight = 0;
    m_ImageWidth = 0;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Calculates the average of two rgb32 pixels
//-----------------------------------------------------------------------------
inline static UINT32 avg(UINT32 a, UINT32 b)
{
    return (((a^b) & 0xfefefefeUL) >> 1) + (a&b);
}

//-----------------------------------------------------------------------------
// Calculates the average of 4 rgb32 pixels
//-----------------------------------------------------------------------------
inline static UINT32 avg(const UINT32 a[2], const UINT32 b[2])
{
    return avg(avg(a[0], a[1]), avg(b[0], b[1]));
}

//-----------------------------------------------------------------------------
// Calculates the average of two rows of rgb32 pixels
//-----------------------------------------------------------------------------
inline static void Average2Rows(__in const BYTE* pSrcImgRow1, __in const BYTE* pSrcImgRow2, __out BYTE* pDstImgRow, __in const UINT &widthInPages)
{
    __m128i w1, w2, avgLeft, avgRight;

    const __m128i* pSrcRow1 = (__m128i*)pSrcImgRow1;
    const __m128i* pSrcRow2 = (__m128i*)pSrcImgRow2;
    __m128i* pDstRow = (__m128i*)pDstImgRow;

    for (UINT cnt = widthInPages; cnt > 0; --cnt)
    {
        w1 = pSrcRow1[0]; pSrcRow1++; // _mm_loadu_si128(
        w2 = pSrcRow2[0]; pSrcRow2++;

        avgLeft  = _mm_avg_epu8(w1, w2);
           
        w1 =  pSrcRow1[0]; pSrcRow1++;
        w2 =  pSrcRow2[0]; pSrcRow2++;

        avgRight = _mm_avg_epu8(w1, w2);

        __m128i sumLeft = _mm_unpacklo_epi32(avgLeft, avgRight);
        __m128i sumRight = _mm_unpackhi_epi32(avgLeft, avgRight);

        __m128i shuffle1 = _mm_unpacklo_epi32(sumLeft, sumRight);
        __m128i shuffle2 = _mm_unpackhi_epi32(sumLeft, sumRight);

        pDstRow[0] = _mm_avg_epu8(shuffle1, shuffle2); //_mm_storeu_si128(
        pDstRow++;
    }
}

HRESULT ImageLoaderWIC::Get32bppBGRFrameConverter(__in IWICBitmapDecoder *spDecoder,  __in const UINT &nFrame, __deref_out IWICFormatConverter **ppFormatConverter)
{ 
    UINT nCount = 0;
    
    // Get the number of frames in this image
    HRESULT hr = spDecoder->GetFrameCount(&nCount);
    IF_FAILED_RETURN(hr);

    // Validate the given frame index nFrame
    if (nFrame >= nCount)
    {
        nCount--;
    }
    else
    {
        nCount = nFrame;
    }
    
    SmartPtr<IWICBitmapFrameDecode> spFrame;
    hr = spDecoder->GetFrame(nCount, &spFrame);
    IF_FAILED_RETURN(hr);

    // Convert the format of the image frame to 32bppBGR
    SmartPtr<IWICFormatConverter> spFrameConverter;
    hr = m_spImagingFactory->CreateFormatConverter(&spFrameConverter);
    IF_FAILED_RETURN(hr);

    hr = spFrameConverter->Initialize(
        spFrame,
        GUID_WICPixelFormat32bppBGR,
        WICBitmapDitherTypeNone,
        NULL,
        0.f,
        WICBitmapPaletteTypeCustom
    );
    IF_FAILED_RETURN(hr);

    return spFrameConverter.CopyTo(ppFormatConverter);
}

//-----------------------------------------------------------------------------
// Opens a frame of an image given by path, reads metadata
// In case of a big file, generates a deep zoom structure
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::Open()
{
    UINT nFrame = 0;

    // Create a decoder for the given image file
    SmartPtr<IWICBitmapDecoder> spDecoder;
    HRESULT hr = m_spImagingFactory->CreateDecoderFromFilename(m_FilePath.GetBuffer(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &spDecoder);
    IF_FAILED_RETURN(hr);

    SmartPtr<IWICFormatConverter> spFormatConverter;
    hr = Get32bppBGRFrameConverter(spDecoder, nFrame, &spFormatConverter);
    IF_FAILED_RETURN(hr);

    WICPixelFormatGUID pixelFormat;
    hr = spFormatConverter->GetPixelFormat(&pixelFormat);
    IF_FAILED_RETURN(hr);

    GUID containerFormat;
    hr = spDecoder->GetContainerFormat(&containerFormat);
    IF_FAILED_RETURN(hr);

    hr = spFormatConverter->GetSize(&m_ImageWidth, &m_ImageHeight);
    IF_FAILED_RETURN(hr);

    DEBUG_TIMER_START(L"Deep zoom pyramid generation");
        hr = GenerateDeepZoomPyramid(spFormatConverter, TILE_SIZE, containerFormat, pixelFormat);
    DEBUG_TIMER_STOP;

    return hr;
}

//-----------------------------------------------------------------------------
// Saves the bitmap to given file path
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::SaveBitmapToFile(__in const WCHAR* pFilePath, __in const GUID &containerFormat, __in const WICPixelFormatGUID *pPixelFormat, __in IWICBitmap *pBitmap)
{
    SmartPtr<IWICStream> spStream;
    HRESULT hr = m_spImagingFactory->CreateStream(&spStream);
    IF_FAILED_RETURN(hr);

    hr = spStream->InitializeFromFilename(pFilePath, GENERIC_WRITE);
    IF_FAILED_RETURN(hr);

    SmartPtr<IWICBitmapEncoder> spEncoder;
    hr = m_spImagingFactory->CreateEncoder(containerFormat, NULL, &spEncoder);
    IF_FAILED_RETURN(hr);

    hr = spEncoder->Initialize(spStream, WICBitmapEncoderNoCache);
    IF_FAILED_RETURN(hr);

    SmartPtr<IWICBitmapFrameEncode> spFrameEncode;
    hr = spEncoder->CreateNewFrame(&spFrameEncode, NULL);
    IF_FAILED_RETURN(hr);

    hr = spFrameEncode->Initialize(NULL);
    IF_FAILED_RETURN(hr);

    // Get size
    UINT width, height;
    SmartPtr<IWICBitmap> spBitmap(pBitmap);
    hr = spBitmap->GetSize(&width, &height);
    IF_FAILED_RETURN(hr);

    // Set image parameters
    hr = spFrameEncode->SetSize(width, height);
    IF_FAILED_RETURN(hr);

    WICPixelFormatGUID pixelFormat = *pPixelFormat;
    hr = spFrameEncode->SetPixelFormat(&pixelFormat);
    IF_FAILED_RETURN(hr);

    // Write
    hr = spFrameEncode->WriteSource(spBitmap, NULL);
    IF_FAILED_RETURN(hr);

    hr = spFrameEncode->Commit();
    IF_FAILED_RETURN(hr);

    hr = spEncoder->Commit();
    IF_FAILED_RETURN(hr);

    return hr;
}

//-----------------------------------------------------------------------------
// Get level directory path
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::GetLevelPath(__in const UINT &level, __in const UINT &size, __deref_out_ecount_z(length + 1) WCHAR *pLevelPath)
{
    String levelPath;
    HRESULT hr = levelPath.Set(m_UltraZoomDirectory.GetBuffer());
    IF_FAILED_RETURN(hr);

    WCHAR strLevel[MAX_PATH];
    UINT error = _itow_s(level, strLevel, MAX_PATH, 10);
    if (error != 0)
    {
        return E_FAIL;
    }

    hr = levelPath.Concat(strLevel);
    IF_FAILED_RETURN(hr);

    error = wcscpy_s(pLevelPath, size, levelPath.GetBuffer());
    if (error != 0)
    {
        return E_FAIL;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Get tile file path from level, column and row
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::GetTilePath(__in const UINT &level, __in const UINT &column,__in const UINT &row, __in const UINT &size, __deref_out_ecount_z(length + 1) WCHAR *pTilePath)
{
    WCHAR levelPath[MAX_PATH] = L"";
    HRESULT hr = GetLevelPath(level, MAX_PATH, levelPath);
    IF_FAILED_RETURN(hr);

    String tilePath;
    hr = tilePath.Set(levelPath);
    IF_FAILED_RETURN(hr);

    WCHAR strColumn[MAX_PATH];
    UINT error = _itow_s(column, strColumn, MAX_PATH, 10);
    if (error != 0)
    {
        return E_FAIL;
    }

    WCHAR strRow[MAX_PATH];
    error = _itow_s(row, strRow, MAX_PATH, 10);
    if (error != 0)
    {
        return E_FAIL;
    }

    hr = tilePath.Concat(L"\\");
    IF_FAILED_RETURN(hr);

    hr = tilePath.Concat(strColumn);
    IF_FAILED_RETURN(hr);

    hr = tilePath.Concat(L"_");
    IF_FAILED_RETURN(hr);

    hr = tilePath.Concat(strRow);
    IF_FAILED_RETURN(hr);

    hr = tilePath.Concat(m_FileExtension.GetBuffer());
    IF_FAILED_RETURN(hr);

    error = wcscpy_s(pTilePath, size, tilePath.GetBuffer());
    if (error != 0)
    {
        return E_FAIL;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Get the maximum level
//-----------------------------------------------------------------------------
UINT ImageLoaderWIC::GetMaximumLevel(__in const UINT &width, __in const UINT &height, __in const UINT &minTileSize)
{
    _ASSERT(width != 0 && height != 0);
    _ASSERT(minTileSize > 1);

    UINT maxLength = max(width, height);
    /*UINT numTiles = maxLength / minTileSize;

    return (UINT)floor(log(numTiles) / log(2));*/

    UINT counter = 0;
    while (maxLength >= minTileSize)
    {
        maxLength >>= 1;
        ++counter;
    }

    return counter;
}

//-----------------------------------------------------------------------------
// Create directory
//-----------------------------------------------------------------------------
HRESULT CreateDir(__in const WCHAR *directoryPath)
{
    HRESULT hr = S_OK;
    if (!CreateDirectory(directoryPath, NULL))
    {
        HRESULT error = GetLastError();
        hr = (error != ERROR_ALREADY_EXISTS) ? error : hr;
    }

    return hr;
}

HRESULT ProcessChunk(__deref_in_bcount(size) const BYTE* pBuffer, __in const UINT &size)
{
    return S_OK;
}

//-----------------------------------------------------------------------------
// Generate deep zoom structure in image path
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::GenerateDeepZoomPyramid(__in IWICFormatConverter *pFormatConverter, __in const UINT &tileSize, __in const GUID &containerGuid, __in const WICPixelFormatGUID &pixelFormat)
{
    // Create directory for ultrazoom data structure, if it does not already exist
    HRESULT hr = CreateDir(m_UltraZoomDirectory.GetBuffer());
    IF_FAILED_RETURN(hr);


    UINT levelCount = GetMaximumLevel(m_ImageWidth, m_ImageHeight, tileSize);
    levelCount++;

    m_Levels.SetSize(levelCount);


    Vector<LevelBuffer> levelBuffers;
    levelBuffers.SetSize(levelCount);

    UINT width = m_ImageWidth;
    UINT height = m_ImageHeight;
 
    Vector<LevelBuffer> levelTileBuffers;
    hr = levelTileBuffers.SetSize(((UINT)width / tileSize) * levelCount);
    IF_FAILED_RETURN(hr);

    Vector<UINT> levelColumnCount;
    hr = levelColumnCount.SetSize(levelCount);
    IF_FAILED_RETURN(hr);

    Vector<UINT> levelRowCount;
    hr = levelRowCount.SetSize(levelCount);
    IF_FAILED_RETURN(hr);

    UINT lastColumn = 0;
    for (UINT level = 0; level < levelCount; level++)
    {
        if (level == 0)
        {
            hr = levelBuffers[level].Initialize(width, CHUNK_HEIGHT, 4);
            IF_FAILED_RETURN(hr);
        }
        else
        {
            hr = levelBuffers[level].Initialize(width, 2, 4);
            IF_FAILED_RETURN(hr);
        }

        WCHAR levelPath[MAX_PATH] = L"";
        hr = GetLevelPath(level, MAX_PATH, levelPath);
        IF_FAILED_RETURN(hr);

        // Create directory for each level
        //
        hr = CreateDir(levelPath);
        IF_FAILED_RETURN(hr);

        INT rowCount = height / tileSize;
        UINT lastTileHeight = height % tileSize;
        rowCount = (lastTileHeight == 0) ? rowCount : rowCount + 1;

        UINT columnCount = width / tileSize;
        UINT lastTileWidth = width % tileSize;
        columnCount = (lastTileWidth == 0) ? columnCount : columnCount + 1;
        levelColumnCount[level] = columnCount;

        // Initialize tile buffers
        //
        UINT stride = 0;
        for (UINT column = 0; column < columnCount; ++column)
        {
            UINT tileWidth = (column == columnCount - 1 && lastTileWidth != 0) ? lastTileWidth : tileSize;
            hr = levelTileBuffers[lastColumn].Initialize(tileWidth, tileSize, 4);
            IF_FAILED_RETURN(hr);

            stride += levelTileBuffers[lastColumn].CbStride;
            lastColumn++;
        }
        _ASSERT(stride == levelBuffers[level].CbStride);

        levelRowCount[level] = 0;

        // Initialize tile metadata for each level
        //
        hr = m_Levels[level].Initialize(width, height, columnCount, rowCount);
        IF_FAILED_RETURN(hr);

        width >>= 1;
        height >>= 1;
    }


    UINT chunksCount = m_ImageHeight / CHUNK_HEIGHT;
    UINT lastChunkHeight = m_ImageHeight % CHUNK_HEIGHT;
    lastChunkHeight = (lastChunkHeight == 0) ? CHUNK_HEIGHT : lastChunkHeight;
    WICRect chunkRect = {0, 0, m_ImageWidth, CHUNK_HEIGHT};

    BOOL isLastChunk = FALSE;
    for (UINT chunk = 0; chunk < chunksCount; ++chunk)
    {
        if (chunk == chunksCount - 1)
        {
            chunkRect.Height = lastChunkHeight;
            isLastChunk = TRUE;
        }

        _ASSERT((levelBuffers[0].CurrentLine == 0) && (levelBuffers[0].CurrentPtr == levelBuffers[0].BasePtr));

        hr = pFormatConverter->CopyPixels(&chunkRect, levelBuffers[0].CbStride, levelBuffers[0].CbSize, levelBuffers[0].CurrentPtr);
        IF_FAILED_RETURN(hr);

        for (INT line = 0; line < chunkRect.Height; ++line)
        {
            BOOL isLastRow = isLastChunk && line == chunkRect.Height - 1;

            UINT lastLevelColumn = 0;
            for (UINT level = 0; level < levelCount; ++level) 
            {
                // Copy pixels to tile buffers
                //
                UINT lastColumn = lastLevelColumn;
                for (UINT column = 0; column < levelColumnCount[level]; ++column)
                {
                    
                    UINT error = memcpy_s(levelTileBuffers[lastColumn].CurrentPtr,
                                          levelTileBuffers[lastColumn].CbStride,
                                          levelBuffers[level].CurrentPtr,
                                          levelTileBuffers[lastColumn].CbStride);
                    if (error != 0)
                    {
                        return E_FAIL;
                    }

                    levelTileBuffers[lastColumn].CurrentLine++;           
                    levelBuffers[level].CurrentPtr += levelTileBuffers[lastColumn].CbStride;                    
                    levelTileBuffers[lastColumn].CurrentPtr += levelTileBuffers[lastColumn].CbStride;
                    lastColumn++;    
                }
                levelBuffers[level].CurrentLine++;

                _ASSERT(levelBuffers[level].CurrentPtr - ((levelBuffers[level].CurrentLine) * levelBuffers[level].CbStride) == levelBuffers[level].BasePtr);

                BOOL isLineEven = (levelBuffers[level].CurrentLine % 2 == 0);
                
                // Do scaling each 2 rows
                //
                if (isLineEven)
                {
                    if (level < levelCount - 1)
                    {
                        BYTE* srcRow2 = levelBuffers[level].CurrentPtr - levelBuffers[level].CbStride; 
                        BYTE* srcRow1 = srcRow2 - levelBuffers[level].CbStride;
                        BYTE* destRow = levelBuffers[level + 1].CurrentPtr;
  
                        Average2Rows(srcRow1, srcRow2, destRow, levelBuffers[level].PageOf32BytesCount);
                    }

                    // We have buffers with just 2 lines, so after each scaling set pointer to zero position
                    //
                    if (level > 0)
                    {
                        levelBuffers[level].CurrentPtr = levelBuffers[level].BasePtr;
                        levelBuffers[level].CurrentLine = 0;
                    }
                }

                // Save images when buffers have enought data or in case of last row
                //
                lastColumn = lastLevelColumn;

                if (levelTileBuffers[lastColumn].CurrentLine == levelTileBuffers[lastColumn].Height || isLastRow)
                {
                    UINT row = levelRowCount[level];

                    for (UINT column = 0; column < levelColumnCount[level]; ++column)
                    {
                        _ASSERT((levelTileBuffers[lastColumn].CurrentLine * levelTileBuffers[lastColumn].CbStride) <= levelTileBuffers[lastColumn].CbSize);

                        SmartPtr<IWICBitmap> spBitmap;
                        hr = m_spImagingFactory->CreateBitmapFromMemory(levelTileBuffers[lastColumn].Width, levelTileBuffers[lastColumn].CurrentLine, 
                                                                        pixelFormat, levelTileBuffers[lastColumn].CbStride, 
                                                                        levelTileBuffers[lastColumn].CurrentLine * levelTileBuffers[lastColumn].CbStride,
                                                                        levelTileBuffers[lastColumn].BasePtr, &spBitmap);
                        IF_FAILED_RETURN(hr);
                  
                        // Fill the tile metadata
                        //
                        UINT index = m_Levels[level].ColumnCount * row + column;
                        m_Levels[level].Tiles[index].X = column * levelTileBuffers[0].Width;
                        m_Levels[level].Tiles[index].Y = row * levelTileBuffers[0].Height;
                        m_Levels[level].Tiles[index].Height = levelTileBuffers[lastColumn].CurrentLine;
                        m_Levels[level].Tiles[index].Width = levelTileBuffers[lastColumn].Width;
                        m_Levels[level].Tiles[index].Level = level;
                        m_Levels[level].Tiles[index].Row = row;
                        m_Levels[level].Tiles[index].Column = column;

                        // Get tile file path and save
                        //
                        hr = GetTilePath(level, column, levelRowCount[level], MAX_PATH, m_Levels[level].Tiles[index].TilePath); 
                        IF_FAILED_RETURN(hr);
                        hr = SaveBitmapToFile(m_Levels[level].Tiles[index].TilePath, containerGuid, &pixelFormat, spBitmap);
                        IF_FAILED_RETURN(hr);

                        // Set pointer of each buffer to zero position
                        //
                        levelTileBuffers[lastColumn].CurrentLine = 0;
                        levelTileBuffers[lastColumn].CurrentPtr = levelTileBuffers[lastColumn].BasePtr;
                        lastColumn++;
                    }

                    levelRowCount[level]++;
                    levelBuffers[level].CurrentLine = 0;
                    levelBuffers[level].CurrentPtr = levelBuffers[level].BasePtr;
                }

                lastLevelColumn += levelColumnCount[level];
                
                if (!isLineEven && !isLastRow)
                {
                    break;
                }
            }
        }

        levelBuffers[0].CurrentLine = 0;
        levelBuffers[0].CurrentPtr = levelBuffers[0].BasePtr;
        chunkRect.Y += CHUNK_HEIGHT;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Save bitmap rectangular segment to file
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::SaveBitmapRectToFile(__in IWICBitmap *pBitmap, __in const GUID &containerFormat, __in const WICPixelFormatGUID *pPixelFormat, __in const WICRect &rect, __in const WCHAR *pTilePath)
{
    SmartPtr<IWICBitmap> spBitmap(pBitmap);

    SmartPtr<IWICBitmapLock> bitmapLock;
    HRESULT hr = spBitmap->Lock(&rect, WICBitmapLockRead, &bitmapLock);
    IF_FAILED_RETURN(hr);

    UINT cbBufferSize = 0;
    BYTE *ptr = NULL;
    hr = bitmapLock->GetDataPointer(&cbBufferSize, &ptr);
    IF_FAILED_RETURN(hr);

    UINT stride = 0; 
    bitmapLock->GetStride(&stride);
    IF_FAILED_RETURN(hr);

    SmartPtr<IWICBitmap> spNewBitmap;
    hr = m_spImagingFactory->CreateBitmapFromMemory(rect.Width, rect.Height, *pPixelFormat, stride, cbBufferSize, ptr, &spNewBitmap);
    IF_FAILED_RETURN(hr);

    return SaveBitmapToFile(pTilePath, containerFormat, pPixelFormat, spNewBitmap);
}

//-----------------------------------------------------------------------------
// Get number of levels
//-----------------------------------------------------------------------------
UINT ImageLoaderWIC::GetLevelCount()
{
    return m_Levels.Length();
}

//-----------------------------------------------------------------------------
// Get size of the level
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::GetLevelSize(__in const UINT &level, __out UINT &width, __out UINT &height)
{
    if (level > m_Levels.Length() - 1)
    {
        return E_INVALIDARG;
    }

    width = m_Levels[level].ImageWidth;
    height = m_Levels[level].ImageHeight;
    
    return S_OK;
}

//-----------------------------------------------------------------------------
// Get level row and column count
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::GetLevelRowColumnCount(__in const UINT &level, __out UINT &rowCount, __out UINT &columnCount)
{
    if (level > m_Levels.Length() - 1)
    {
        return E_INVALIDARG;
    }

    rowCount = m_Levels[level].RowCount;
    columnCount = m_Levels[level].ColumnCount;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Get tile metadata
//-----------------------------------------------------------------------------
HRESULT ImageLoaderWIC::GetLevelRowColumnMetadata(__in const UINT &level, __in const UINT &row, __in const UINT &column, __out const ImageTileMetadata** ppTileMetadata)
{
    if ((level > m_Levels.Length() - 1) || (row > m_Levels[level].RowCount - 1) || (column > m_Levels[level].ColumnCount - 1))
    {
        return E_INVALIDARG;
    }

    UINT index = m_Levels[level].ColumnCount * row + column;
    *ppTileMetadata = &(m_Levels[level].Tiles[index]);

    return S_OK;
}


HRESULT ImageLoaderWIC::Initialize(__in_z const WCHAR *pFilePath)
{
    HRESULT hr = m_FilePath.Set(pFilePath);

    if (SUCCEEDED(hr))
    {
        WCHAR pDrive[5] = L"";
        WCHAR pDirectory[MAX_PATH] = L"";
        WCHAR pFileName[MAX_PATH] = L"";
        WCHAR pExtension[30] = L"";

        UINT error = _wsplitpath_s(m_FilePath.GetBuffer(), pDrive, 5, pDirectory, MAX_PATH, pFileName, MAX_PATH, pExtension, 30); 
        if (error != 0)
        {
            return E_FAIL;
        }

        hr = m_UltraZoomDirectory.Concat(pDrive);
        IF_FAILED_RETURN(hr);

        hr = m_UltraZoomDirectory.Concat(pDirectory);
        IF_FAILED_RETURN(hr);

        hr = m_UltraZoomDirectory.Concat(pFileName);
        IF_FAILED_RETURN(hr);

        hr = m_UltraZoomDirectory.Concat(L"_dzfiles\\");
        IF_FAILED_RETURN(hr);

        hr = m_FileExtension.Set(pExtension);
        IF_FAILED_RETURN(hr);

        hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&m_spImagingFactory);
    }

    return hr;
}

HRESULT ImageLoaderWIC::GetLevelRowColumnImage(__in const UINT &level, __in const UINT &row, __in const UINT &column, __out IDxImage** ppImage)
{
    if ((level > m_Levels.Length() - 1) || (row > m_Levels[level].RowCount - 1) || (column > m_Levels[level].ColumnCount - 1))
    {
        return E_INVALIDARG;
    }

    UINT index = m_Levels[level].ColumnCount * row + column;

    UINT nFrame = 0;

    // Create a decoder for the given image file
    SmartPtr<IWICBitmapDecoder> spDecoder;
    HRESULT hr = m_spImagingFactory->CreateDecoderFromFilename(m_Levels[level].Tiles[index].TilePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &spDecoder);
    IF_FAILED_RETURN(hr);

    SmartPtr<IWICFormatConverter> spFormatConverter;
    hr = Get32bppBGRFrameConverter(spDecoder, nFrame, &spFormatConverter);
    IF_FAILED_RETURN(hr);

    WICRect rect = {0, 0, m_Levels[level].Tiles[index].Width, m_Levels[level].Tiles[index].Height};
    UINT rowPitch = (m_Levels[level].Tiles[index].Width * 32 + 7) / 8;

    return DxImage::CreateInstance(ppImage, rect, 88, rowPitch, spFormatConverter); // 88 == DXGI_FORMAT_B8G8R8X8_UNORM
}

HRESULT CreateImageLoader(__in_z const WCHAR *pFilePath, __deref_out IImageLoader **ppResult)
{
    return ImageLoaderWIC::CreateInstance(ppResult, pFilePath);
}
