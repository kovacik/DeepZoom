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


Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 color;
	
    // Sample the texture pixel at this location.
    color = shaderTexture.Sample(SampleType, input.tex);
	
    // If the color is black on the texture then treat this pixel as transparent.
    if(color.r == 0.0f)
    {
        color.a = 0.0f;
    }
	
    // If the color is other than black on the texture then this is a pixel in the font so draw it using the font pixel color.
    else
    {
        color.a = 1.0f;
        color = color * input.color;
    }

    return color;
}