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

namespace Fonts
{

enum DxFontType
{
	DF_STYLE_NORMAL      = 0,
	DF_STYLE_BOLD        = 1,
	DF_STYLE_ITALIC      = 2,
	DF_STYLE_BOLD_ITALIC = 3,
	DF_STYLE_UNDERLINE   = 4,
	DF_STYLE_STRIKEOUT   = 8
};

struct Rect
{
	FLOAT    left;
	FLOAT    top;
	FLOAT    right;
	FLOAT    bottom;

	Rect()
	{}

	explicit Rect(
		FLOAT Left,
		FLOAT Top,
		FLOAT Right,
		FLOAT Bottom)
	{
		left = Left;
		top = Top;
		right = Right;
		bottom = Bottom;
	}

	operator const Rect&() const { return *this; }

};

};

DECLAREINTERFACE(IDxFont, IUnknown, "{C2A778D8-94EE-4D3B-BB98-331C9324C1C9}")
{
	ID3D11ShaderResourceView* GetFontSheetSRV();

    const Fonts::Rect& GetCharRect(WCHAR c);

    INT GetSpaceWidth();

    INT GetCharHeight();
};