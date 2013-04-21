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


#include "StdAfx.h"
#include "SceneObjectGroup.h"

HRESULT SceneObjectGroup::Initialize(__in_z const wchar_t *nodeName)
{
    return SetName(nodeName);
}

HRESULT SceneObjectGroup::Unregister()
{
    UINT cnt = m_Children.Length();
    for (UINT i = 0; i<cnt; i++)
    {
        m_Children[i]->Unregister();
    }

    return SceneObject::Unregister();
}

HRESULT SceneObjectGroup::Register(__in ISceneGraph *pSceneGraph)
{
    HRESULT hr = SceneObject::Register(pSceneGraph);
    if (hr == S_OK)
    {

        UINT cnt = m_Children.Length();
        for (UINT i = 0; i<cnt; i++)
        {
            hr = m_Children[i]->Register(pSceneGraph);
            if (FAILED(hr))
            {
                break;
            }

            m_Children[i]->Invalidate(SOCT_TRANSFORM);
        }
    }
    return hr;
}

HRESULT SceneObjectGroup::Invalidate(SceneObjectContentType soct)
{
    if (soct & SOCT_TRANSFORM)
    {
        if ((m_InvalidContent & SOCT_TRANSFORM) == 0)
        {
            // invalidating transform must invalidate transform for all children objects
            UINT cnt = m_Children.Length();
            for (UINT i = 0; i<cnt; i++)
            {
                m_Children[i]->Invalidate(SOCT_TRANSFORM);
            }
        }
    }
    return SceneObject::Invalidate(soct);
}

HRESULT SceneObjectGroup::Destroy()
{
    UINT cnt = m_Children.Length();
    for (UINT i = 0; i<cnt; i++)
    {
        m_Children[i]->Destroy();
    }

    return SceneObject::Destroy();
}

UINT SceneObjectGroup::GetObjectCount()
{
    return m_Children.Length();
}

HRESULT SceneObjectGroup::GetObject(__in UINT index, __deref_out ISceneObject **ppSceneObject)
{
    if (!ppSceneObject)
        return E_POINTER;
    if (index >= m_Children.Length())
        return E_INVALIDARG;

    return m_Children[index].CopyTo(ppSceneObject);
}

HRESULT SceneObjectGroup::AddObject(__in ISceneObject *pSceneObject)
{
    if (!pSceneObject)
        return E_INVALIDARG;
    
    HRESULT hr;
    if (m_Registered)
    {
        hr = pSceneObject->Register(m_SceneGraph);
        if (FAILED(hr))
            return hr;
    }

    return m_Children.Add(pSceneObject);
}

HRESULT SceneObjectGroup::RemoveObject(__in ISceneObject *pSceneObject)
{
    if (!pSceneObject)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    if (m_Children.Remove(SmartPtr<ISceneObject>(pSceneObject)))
    {
        if (m_Registered)
        {
            hr = pSceneObject->Unregister();
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CreateGroup(__in_z const wchar_t *nodeName, __deref_out ISceneObjectGroup **ppResult)
{
    return SceneObjectGroup::CreateInstance(ppResult, nodeName);
}