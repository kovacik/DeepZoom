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

class SceneObjectCamera : public ImplementSmartObject
    <
        SceneObjectCamera, 
        ClassFlags<CF_ALIGNED_MEMORY>,
        InheritImplementation<SceneObject>, 
        ISceneObjectCamera
    >
{
    ProjectionType m_ProjectionType;

    XMMATRIX m_Projection;
    XMMATRIX m_Scaling;
    float m_NearPlane, m_FarPlane;

    FLOAT m_ViewportX;
    FLOAT m_ViewportY;

    FLOAT m_ViewportOffsetX;
    FLOAT m_ViewportOffsetY;

    FLOAT m_ViewportWidth;
    FLOAT m_ViewportHeight;

    FLOAT m_ScreenWidth;
    FLOAT m_ScreenHeight;


	XMVECTOR m_Position;
    XMVECTOR m_Direction;
    XMVECTOR m_UpVector;
	XMVECTOR m_At;
    
	XMVECTOR m_RollPitchYaw;
    FLOAT m_Scale;
    FLOAT m_ScaleOrthographic;

	FLOAT m_Fov;
	FLOAT m_AspectRation;

	void ResetRotationAndScale();

public:
    HRESULT Initialize(__in_z const wchar_t *nodeName, __in const ProjectionType &projectionType);

    HRESULT GetProjectionMatrix(__deref_out XMMATRIX *projection);
    HRESULT SetProjectionMatrix(__in CXMMATRIX projection);
	HRESULT GetViewMatrix(__deref_out XMMATRIX *viewMatrix);

    HRESULT Update(SceneObjectContentType soct);

	HRESULT Look(__in const FLOAT &yawDelta, __in const FLOAT &pitchDelta, __in const FLOAT &rollDelta, __in const FLOAT &scaleDelta);
	HRESULT Move(__in const FLOAT &forwardDelta, __in const FLOAT &strideDelta, __in const FLOAT &upDelta);
	HRESULT SetPositionAtUpFov(__in CXMVECTOR pos, __in CXMVECTOR at, __in CXMVECTOR up, __in const FLOAT &fov);
	HRESULT SetPositionAndScale(CXMVECTOR pos, __in const FLOAT &scale);
    HRESULT SetViewport(__in const FLOAT &viewportX, __in const FLOAT &viewportY, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight);
    HRESULT GetViewport(__out FLOAT &viewportX, __out FLOAT &viewportY, __out FLOAT &viewportWidth, __out FLOAT &viewportHeight);
    HRESULT PanZoomOrthographic(__in const FLOAT &viewportOffsetX, __in const FLOAT &viewportOffsetY, __in const FLOAT &scaleDelta);
    HRESULT ApplyViewportOffsetX();
    HRESULT ApplyViewportOffsetY();

    HRESULT SetScreenSize(__in const FLOAT &width, __in const FLOAT &height);
    HRESULT GetScreenSize(__out FLOAT &width, __out FLOAT &height);

    HRESULT CalibrateToSceneObject(__in ISceneObject *pSceneObject);
};