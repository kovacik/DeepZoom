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
#include "SceneObjectText.h"

SceneObjectText::SceneObjectText()
{
	m_Text = NULL;
}

SceneObjectText::~SceneObjectText()
{
    free(m_Text);
}

HRESULT SceneObjectText::Initialize(__in_z const wchar_t *nodeName, __in_z const wchar_t *text, __in const XMFLOAT4 &color, __in const XMFLOAT3 &position)
{
	HRESULT hr = UpdateText(text);
	
	if (SUCCEEDED(hr))
	{
		hr = SetName(nodeName);

		if (SUCCEEDED(hr))
		{
			hr = SetPosition(position);

			if (SUCCEEDED(hr))
			{
				hr = SetColor(color);
			}
		}
	}
	return hr;
}

HRESULT SceneObjectText::UpdateText(__in_z const wchar_t *newText)
{
	free(m_Text);
    if (newText == NULL)
    {
        m_Text  = NULL;
    }
    else
    {
        size_t l = wcslen(newText) + 1;
        m_Text = (WCHAR *)malloc(l * sizeof(WCHAR));
        if (!m_Text)
            return E_OUTOFMEMORY;

        memcpy(m_Text, newText, l * sizeof(WCHAR));
    }

	return S_OK;
}


const WCHAR* SceneObjectText::GetText()
{
	return m_Text;
}

const XMFLOAT3& SceneObjectText::GetPosition()
{
	return m_Position;
}

HRESULT SceneObjectText::SetPosition(__in const XMFLOAT3 &newPosition)
{
	m_Position = newPosition;
	return Invalidate(SOCT_SHAPE);
}

XMFLOAT4 SceneObjectText::GetColor()
{
	return m_Color;
}

HRESULT SceneObjectText::SetColor(__in const XMFLOAT4 &color)
{
	m_Color = color;
	return Invalidate(SOCT_SHAPE);
}

HRESULT SceneObjectText::Destroy()
{
    free(m_Text);
    m_Text = NULL;

    return SceneObject::Destroy();
}

HRESULT CreateText(__in_z const wchar_t *nodeName, __in_z const wchar_t *text, __in const XMFLOAT4 &color, __in const XMFLOAT3 &position, __deref_out ISceneObjectText **ppResult)
{
	return SceneObjectText::CreateInstance(ppResult, nodeName, text, color, position);
}