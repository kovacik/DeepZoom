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

class DxFont : public ImplementSmartObject
	<
		DxFont, 
		IDxFont
	>
{
private:

	void MeasureChars( Gdiplus::Font & font, Gdiplus::Graphics & charGraphics );
	void BuildFontSheetBitmap( Gdiplus::Font & font, Gdiplus::Graphics & charGraphics, Gdiplus::Bitmap & charBitmap, Gdiplus::Graphics & fontSheetGraphics );
	bool BuildFontSheetTexture( Gdiplus::Bitmap & fontSheetBitmap );
	INT	GetCharMinX( Gdiplus::Bitmap & charBitmap );
	INT	GetCharMaxX( Gdiplus::Bitmap & charBitmap );

	static const WCHAR m_StartChar = 33;
	static const WCHAR m_EndChar = 127;
	static const UINT m_NumChars = m_EndChar - m_StartChar;
	Fonts::Rect m_CharRects[m_NumChars];

	SmartPtr<ID3D11ShaderResourceView> m_spFontSheetSRV;
	SmartPtr<ID3D11Texture2D> m_spFontSheetTex;
    
	UINT m_TexWidth, m_TexHeight;
	INT m_SpaceWidth, m_CharHeight;

	SmartPtr<ID3D11Device> m_spDevice;

public:

	DxFont( );
	~DxFont( );

	HRESULT Initialize(__in ID3D11Device *pDevice, __in_z const wchar_t *fontName, __in const FLOAT &fontSize, __in const WORD &fontStyle, __in const BOOL &antiAliased);

	ID3D11ShaderResourceView* GetFontSheetSRV();
	const Fonts::Rect& GetCharRect(WCHAR c);
	INT GetSpaceWidth();
	INT GetCharHeight();
};