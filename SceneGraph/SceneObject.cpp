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
#include "SceneObject.h"

SceneObject::SceneObject()
{
    m_Registered = false;
    m_ObjectName = NULL;
    m_InvalidContent = SOCT_NONE;
	m_LocalTransform = XMMatrixIdentity();
	m_WorldTransform = m_LocalTransform;
}

SceneObject::~SceneObject()
{
    free(m_ObjectName);
}

CXMMATRIX SceneObject::GetLocalTransform() const
{
    return m_LocalTransform;
}

HRESULT SceneObject::SetLocalTransform(__in CXMMATRIX transform)
{
    m_LocalTransform = transform;
    return Invalidate(SOCT_TRANSFORM);
}

CXMMATRIX SceneObject::GetWorldTransform() const
{
    return m_WorldTransform;
}

HRESULT SceneObject::Update(SceneObjectContentType soct)
{
    HRESULT hr;
    if (soct & SOCT_TRANSFORM)
    {
        if (m_spOwner!= NULL)
        {
            hr = m_spOwner->Validate();
            m_WorldTransform = XMMatrixMultiply(m_LocalTransform, m_spOwner->GetWorldTransform() );
        }
        else
        {
            hr = S_OK;
            m_WorldTransform = m_LocalTransform;
        }
    }
    else
    {
        hr = S_OK;
    }
    return hr;
}

HRESULT SceneObject::Validate()
{
    HRESULT hr;
    if (m_InvalidContent != SOCT_NONE)
    {
        SceneObjectContentType ct = m_InvalidContent;
        m_InvalidContent = SOCT_NONE;
        hr = Update(ct);
    }
    else
    {
        hr = S_FALSE;
    }
    return S_OK;
}

SceneObjectContentType SceneObject::GetInvalidContentMask()
{
    return m_InvalidContent;
}

HRESULT SceneObject::InvalidateInternal(SceneObjectContentType soct)
{
    HRESULT hr = S_OK;
    if (m_InvalidContent == SOCT_NONE)
    {
        hr = m_SceneGraph->RegisterForValidate(this);
    }
    m_InvalidContent = (SceneObjectContentType)(m_InvalidContent | soct);
    return hr;
}

HRESULT SceneObject::Invalidate(SceneObjectContentType soct)
{
    HRESULT hr = S_OK;
    if (m_SceneGraph)
    {
        hr = InvalidateInternal(soct);
    }
    return hr;
}

HRESULT SceneObject::Unregister()
{
    if (!m_Registered)
        return S_FALSE;

    m_Registered = false;
    HRESULT hr = Invalidate(SOCT_OBJECT_REMOVE);
    m_SceneGraph.Release();
    return hr;
}

HRESULT SceneObject::Register(__in ISceneGraph *pSceneGraph)
{
    if (pSceneGraph == NULL)
        return E_INVALIDARG;

    if (m_Registered)
        return S_FALSE;

    m_Registered = true;
    m_SceneGraph = pSceneGraph;
    return InvalidateInternal(SOCT_OBJECT_ADD);
}

HRESULT SceneObject::SetName(__in_z const WCHAR* pName)
{
    free(m_ObjectName);
    if (pName == NULL)
    {
        m_ObjectName  = NULL;
    }
    else
    {
        size_t l = wcslen(pName) + 1;
        m_ObjectName = (WCHAR *)malloc(l * sizeof(WCHAR));
        if (!m_ObjectName)
            return E_OUTOFMEMORY;

        memcpy(m_ObjectName, pName, l * sizeof(WCHAR));
    }
    return S_OK;
}

const WCHAR* SceneObject::GetName() const
{
    return m_ObjectName;
}

HRESULT SceneObject::GetOwner(__deref_out_opt ISceneObject **ppOwner)
{
    return m_spOwner.CopyToOpt(ppOwner);
}

HRESULT SceneObject::SetOwner(__in_opt ISceneObject *pOwner)
{
    if (m_spOwner && pOwner)
    {
        return E_UNEXPECTED;
    }

    m_spOwner = pOwner;
    return Invalidate(SOCT_TRANSFORM);
}

HRESULT SceneObject::Destroy()
{
    m_SceneGraph.Release();
    m_spOwner.Release();
    free(m_ObjectName);
    m_ObjectName = NULL;
    return S_OK;
}

HRESULT SceneObject::GetInterface(const GUID &extensionId, __deref_out IUnknown **ppInterface)
{
    UNREFERENCED_PARAMETER(extensionId);
    UNREFERENCED_PARAMETER(ppInterface);
    return E_NOINTERFACE;
}