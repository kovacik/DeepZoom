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

class SceneGraph : public ImplementSmartObject
    <
        SceneGraph, 
        ClassFlags<CF_DEFAULT>,
        ISceneGraph
    >
{
    struct NotificationListener
    {
        SceneObjectContentType notificationType;
        SmartPtr<ISceneChangesNotificationListener> listener;
    };

    CriticalSection m_Validation;

    SmartPtr<ISceneObjectGroup> m_SceneRoot;
    Vector<SmartPtr<ISceneObject>> m_InvalidObjects;
    Vector<NotificationListener> m_Listeners;
    Vector<SmartPtr<ISceneChangesNotificationListener>> m_NewTopologyListeners;
    UINT m_Timestamp;

    bool m_ValidateInProgress;

    HRESULT RecurentNotifyAddSceneContent(__in ISceneChangesNotificationListener *pListener, __in ISceneObjectGroup *pGroup);
public:
    HRESULT Initialize();
    HRESULT Destroy();

    HRESULT GetSceneRoot(__deref_out ISceneObjectGroup **ppSceneRoot);

    HRESULT RegisterForValidate(__in ISceneObject *pObject);
    HRESULT Validate();

    HRESULT RegisterNotificationListener(__in ISceneChangesNotificationListener *pListener, SceneObjectContentType mask);
    
    HRESULT UnregisterNotificationListener(__in ISceneChangesNotificationListener *pListener);


    UINT GetTimestamp();
};

