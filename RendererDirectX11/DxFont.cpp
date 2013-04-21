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
#include "DxFont.h"

DxFont::DxFont()
{
	m_TexWidth = 1024;
	m_TexHeight = 0;
	m_SpaceWidth = 0;
	m_CharHeight = 0;
}

DxFont::~DxFont()
{
}

HRESULT DxFont::Initialize(__in ID3D11Device *pDevice, __in_z const wchar_t *fontName, __in const FLOAT &fontSize, __in const WORD &fontStyle, __in const BOOL &antiAliased)
{
	if (!pDevice || !fontName)
	{
		return E_INVALIDARG;
	}
	
	m_spDevice = pDevice;

    ULONG_PTR token = 0;
    Gdiplus::GdiplusStartupInput  startupInput( 0, TRUE, TRUE );
    Gdiplus::GdiplusStartupOutput startupOutput;

    Gdiplus::GdiplusStartup( &token, &startupInput, &startupOutput );

    {
        Gdiplus::Font font( fontName, fontSize, fontStyle, Gdiplus::UnitPixel );
 
        Gdiplus::TextRenderingHint hint = antiAliased ? Gdiplus::TextRenderingHintAntiAlias : Gdiplus::TextRenderingHintSystemDefault;

        INT                    tempSize = static_cast<INT>( fontSize * 2 );
        Gdiplus::Bitmap        charBitmap( tempSize, tempSize, PixelFormat32bppARGB );
        Gdiplus::Graphics    charGraphics( &charBitmap );

        charGraphics.SetPageUnit( Gdiplus::UnitPixel );
        charGraphics.SetTextRenderingHint( hint );
        
        MeasureChars( font, charGraphics );

        Gdiplus::Bitmap        fontSheetBitmap( m_TexWidth, m_TexHeight, PixelFormat32bppARGB );
        Gdiplus::Graphics    fontSheetGraphics( &fontSheetBitmap );

        fontSheetGraphics.SetCompositingMode( Gdiplus::CompositingModeSourceCopy );
        fontSheetGraphics.Clear( Gdiplus::Color( 0, 0, 0, 0 ) );

        BuildFontSheetBitmap( font, charGraphics, charBitmap, fontSheetGraphics );

        if( !BuildFontSheetTexture( fontSheetBitmap ) )
        {
            Gdiplus::GdiplusShutdown( token );

            return false;
        }
    }

    Gdiplus::GdiplusShutdown( token );

    return true;
}

ID3D11ShaderResourceView* DxFont::GetFontSheetSRV( )
{
    return m_spFontSheetSRV;
}

const Fonts::Rect& DxFont::GetCharRect( WCHAR c )
{
    return m_CharRects[ c - m_StartChar ];
}

INT DxFont::GetSpaceWidth( )
{
    return m_SpaceWidth;
}

INT DxFont::GetCharHeight( )
{
    return m_CharHeight;
}

void DxFont::MeasureChars( Gdiplus::Font & font, Gdiplus::Graphics & charGraphics )
{
    WCHAR allChars[ m_NumChars + 1 ];

    for( WCHAR i = 0; i < m_NumChars; ++i )
    {
        allChars[i] = m_StartChar + i;
    }

    allChars[ m_NumChars ] = 0;

    Gdiplus::RectF sizeRect;
    charGraphics.MeasureString( allChars, m_NumChars, &font, Gdiplus::PointF( 0, 0 ), &sizeRect );
    m_CharHeight = static_cast<INT>( sizeRect.Height + 0.5f );

    INT numRows = static_cast<INT>( sizeRect.Width / m_TexWidth ) + 1;
    m_TexHeight   = static_cast<INT>( numRows * m_CharHeight ) + 1;

    WCHAR charString[ 2 ] = { ' ', 0 };
    charGraphics.MeasureString( charString, 1, &font, Gdiplus::PointF( 0, 0 ), &sizeRect );
    m_SpaceWidth = static_cast<INT>( sizeRect.Width + 0.5f );
}

void DxFont::BuildFontSheetBitmap( Gdiplus::Font & font, Gdiplus::Graphics & charGraphics, Gdiplus::Bitmap & charBitmap, Gdiplus::Graphics & fontSheetGraphics )
{
    WCHAR                charString[ 2 ] = { ' ', 0 };
    Gdiplus::SolidBrush whiteBrush( Gdiplus::Color( 255, 255, 255, 255 ) );
    UINT                fontSheetX = 0;
    UINT                fontSheetY = 0;

    for ( UINT i = 0; i < m_NumChars; ++i )
    {
        charString[ 0 ] = static_cast<WCHAR>( m_StartChar + i );
        charGraphics.Clear( Gdiplus::Color( 0, 0, 0, 0 ) );
        charGraphics.DrawString( charString, 1, &font, Gdiplus::PointF( 0.0f, 0.0f ), &whiteBrush );

        INT minX        = GetCharMinX( charBitmap );
        INT maxX        = GetCharMaxX( charBitmap );
        INT charWidth    = maxX - minX + 1;

        if( fontSheetX + charWidth >= m_TexWidth )
        {
            fontSheetX = 0;
            fontSheetY += static_cast<INT>( m_CharHeight ) + 1;
        }

        m_CharRects[ i ] = Fonts::Rect( (FLOAT)fontSheetX, (FLOAT)fontSheetY, (FLOAT)fontSheetX + (FLOAT)charWidth, (FLOAT)fontSheetY + (FLOAT)m_CharHeight );

		fontSheetGraphics.DrawImage( &charBitmap, fontSheetX, fontSheetY, minX, 0, charWidth, m_CharHeight, Gdiplus::UnitPixel );

        fontSheetX += charWidth + 1;
    }
}

bool DxFont::BuildFontSheetTexture( Gdiplus::Bitmap & fontSheetBitmap )
{
    Gdiplus::BitmapData bmData;

	if (fontSheetBitmap.LockBits( &Gdiplus::Rect( 0, 0, m_TexWidth, m_TexHeight ), Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmData ) != Gdiplus::Ok)
		return false;

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width                = m_TexWidth;
    texDesc.Height                = m_TexHeight;
    texDesc.MipLevels            = 1;
    texDesc.ArraySize            = 1;
    texDesc.Format                = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count    = 1;
    texDesc.SampleDesc.Quality    = 0;
	texDesc.Usage                = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags        = 0;
    texDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA data;        
    data.pSysMem            = bmData.Scan0;
    data.SysMemPitch        = m_TexWidth * 4;
    data.SysMemSlicePitch    = 0;

	HRESULT hr = m_spDevice->CreateTexture2D( &texDesc, &data, &m_spFontSheetTex );
    if( FAILED(hr) )
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                        = DXGI_FORMAT_B8G8R8A8_UNORM;
    srvDesc.ViewDimension                = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels            = 1;
    srvDesc.Texture2D.MostDetailedMip    = 0;

    if( FAILED( m_spDevice->CreateShaderResourceView( m_spFontSheetTex, &srvDesc, &m_spFontSheetSRV ) ) )
        return false;

	if (fontSheetBitmap.UnlockBits( &bmData ) != Gdiplus::Ok)
		return false;

    return true;
}

INT DxFont::GetCharMinX( Gdiplus::Bitmap & charBitmap )
{
    INT width  = charBitmap.GetWidth( );
    INT height = charBitmap.GetHeight( );
    
    for( INT x = 0; x < width; ++x )
    {
        for( INT y = 0; y < height; ++y )
        {
            Gdiplus::Color color;

            charBitmap.GetPixel( x, y, &color );
            if( color.GetAlpha( ) > 0 )
                    return x;
        }
    }

    return 0;
}

INT DxFont::GetCharMaxX( Gdiplus::Bitmap & charBitmap )
{
    INT width  = charBitmap.GetWidth( );
    INT height = charBitmap.GetHeight( );

    for( INT x = width - 1; x >= 0; --x )
    {
        for( INT y = 0; y < height; ++y )
        {
            Gdiplus::Color color;

            charBitmap.GetPixel( x, y, &color );
            if( color.GetAlpha( ) > 0 )
                    return x;
        }
    }

    return width - 1;
}

HRESULT CreateDxFont(__in ID3D11Device *pDevice, __in_z const wchar_t *fontName, __in const FLOAT &fontSize, __in const WORD &fontStyle, __in const BOOL &antiAliased, __deref_out IDxFont **ppResult)
{
	return DxFont::CreateInstance(ppResult, pDevice, fontName, fontSize, fontStyle, antiAliased);
}