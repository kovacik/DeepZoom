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

#include "SceneObject.h"

class SceneObjectText : public ImplementSmartObject
    <
        SceneObjectText, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        InheritImplementation<SceneObject>, 
        ISceneObjectText
    >
{
    WCHAR* m_Text;
	XMFLOAT3 m_Position;
	XMFLOAT4 m_Color;

public:
	SceneObjectText();
    ~SceneObjectText();

    HRESULT Initialize(__in_z const wchar_t *nodeName, __in_z const wchar_t *text, __in const XMFLOAT4 &color, __in const XMFLOAT3 &position);

	HRESULT UpdateText(__in_z const wchar_t *newText);	
	const WCHAR* SceneObjectText::GetText();

	XMFLOAT4 GetColor();
	HRESULT SetColor(__in const XMFLOAT4 &color);

	const XMFLOAT3& SceneObjectText::GetPosition();
	HRESULT SetPosition(__in const XMFLOAT3 &newPosition);

    HRESULT Destroy();
};