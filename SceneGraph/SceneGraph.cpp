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
#include "SceneGraph.h"
#include "SceneObjectGroup.h"

HRESULT SceneGraph::Initialize()
{
    m_Timestamp = 0;
    m_ValidateInProgress = false;

    SmartPtr<SceneObjectGroup> spSceneRoot;
    HRESULT hr = m_Validation.Initialize();
    if (SUCCEEDED(hr))
    {
        hr = SceneObjectGroup::CreateInstance(&m_SceneRoot, L"root");
        if (SUCCEEDED(hr))
        {
            SmartPtr<ISceneObject> spRootObj;
            hr = m_SceneRoot.As(&spRootObj);
            if (SUCCEEDED(hr))
            {
                hr = spRootObj->Register(this);
            }
        }
    }
    return hr;
}

HRESULT SceneGraph::Destroy()
{
    SmartPtr<ISceneObject> spRootObj;
    HRESULT hr = m_SceneRoot.As(&spRootObj);
    if (SUCCEEDED(hr))
    {
        spRootObj->Unregister();
    }
    
    m_SceneRoot.Release();
    m_NewTopologyListeners.Clear();
    m_Listeners.Clear();
    return S_OK;
}

HRESULT SceneGraph::GetSceneRoot(__deref_out ISceneObjectGroup **ppSceneRoot)
{
    return m_SceneRoot.CopyTo(ppSceneRoot);
}

HRESULT SceneGraph::RegisterNotificationListener(__in ISceneChangesNotificationListener *pListener, SceneObjectContentType mask)
{
    if (pListener == NULL)
        return E_INVALIDARG;

    AutoCriticalSection acs(m_Validation);

    HRESULT hr;
    if (m_ValidateInProgress)
    {
        hr = E_FAIL;
    }
    else
    {
        NotificationListener nl;
        nl.listener = pListener;
        nl.notificationType = mask;
        hr = m_Listeners.Add(nl);
        if (SUCCEEDED(hr))
        {
            if (mask & SOCT_OBJECT_ADD)
            {
                hr = m_NewTopologyListeners.Add(pListener);
            }
        }
    }
    return hr;
}
    
HRESULT SceneGraph::UnregisterNotificationListener(__in ISceneChangesNotificationListener *pListener)
{
    if (pListener == NULL)
        return E_INVALIDARG;

    AutoCriticalSection acs(m_Validation);

    if (m_ValidateInProgress)
        return E_FAIL;

    UINT cnt = m_Listeners.Length();
    for (UINT i = 0; i<cnt; i++)
    {
        if (m_Listeners[i].listener == pListener)
        {
            m_Listeners.RemoveAt(i);
            m_NewTopologyListeners.Remove(SmartPtr<ISceneChangesNotificationListener>(pListener));
            return S_OK;
        }
    }
    return S_FALSE;
}

HRESULT SceneGraph::RegisterForValidate(__in ISceneObject *pObject)
{
    if (!pObject)
        return E_INVALIDARG;

    AutoCriticalSection acs(m_Validation);

    if (m_ValidateInProgress)
        return E_FAIL;

    return m_InvalidObjects.Add(pObject);
}

HRESULT SceneGraph::RecurentNotifyAddSceneContent(__in ISceneChangesNotificationListener *pListener, __in ISceneObjectGroup *pGroup)
{
    HRESULT hr = S_OK;
    UINT cnt = pGroup->GetObjectCount();
    for (UINT i = 0; i < cnt; i++)
    {
        SmartPtr<ISceneObject> spObj;
        hr = pGroup->GetObject(i, &spObj);
        if (FAILED(hr))
            break;

        (void)pListener->OnSceneObjectChanged(spObj, SOCT_OBJECT_ADD);

        SmartPtr<ISceneObjectGroup> spGroup;
        (void)spObj->QueryInterface(&spGroup);
        if (spGroup != NULL)
        {
            hr = RecurentNotifyAddSceneContent(pListener, spGroup); 
            if (FAILED(hr))
                break;
        }
    }
    return hr;
}

HRESULT SceneGraph::Validate()
{
    if (m_ValidateInProgress)
        return E_FAIL;

    AutoCriticalSection acs(m_Validation);
    m_ValidateInProgress = true;

    UINT cntTopology = m_NewTopologyListeners.Length();
    for (UINT i = 0; i<cntTopology;i++)
    {
        RecurentNotifyAddSceneContent(m_NewTopologyListeners[i], m_SceneRoot);
        m_NewTopologyListeners[i].Release();
    }
    m_NewTopologyListeners.Reset();

    UINT cntListeners = m_Listeners.Length();

    UINT cntObjects = m_InvalidObjects.Length();
    for (UINT i = 0; i<cntObjects;i++)
    {
        SmartPtr<ISceneObject> spObj = m_InvalidObjects[i];
        SceneObjectContentType invalidContentMask = spObj->GetInvalidContentMask();
        spObj->Validate();

        // Notify registered listeners
        for (UINT j = 0; j<cntListeners; j++)
        {
            if (invalidContentMask & m_Listeners[j].notificationType)
            {
                (void)m_Listeners[j].listener->OnSceneObjectChanged(spObj, invalidContentMask);
            }
        }

        m_InvalidObjects[i].Release();
    }

    m_Timestamp++;
    m_InvalidObjects.Reset();

    m_ValidateInProgress = false;
    return S_OK;
}

UINT SceneGraph::GetTimestamp()
{
    return m_Timestamp;
}


HRESULT CreateSceneGraph(__deref_out ISceneGraph **ppResult)
{
    return SceneGraph::CreateInstance(ppResult);
}