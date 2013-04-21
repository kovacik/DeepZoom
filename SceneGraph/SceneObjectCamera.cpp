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
#include "SceneObjectCamera.h"

HRESULT SceneObjectCamera::Initialize(__in_z const wchar_t *nodeName, __in const ProjectionType &projectionType)
{
    m_Scaling = XMMatrixScaling(1.0f, 1.33f, 1.0f);
    m_ProjectionType = projectionType;

    m_ScreenWidth = 0.0f;
    m_ScreenHeight = 0.0f;

    m_ViewportX = -1024.0f / 2.0f;
    m_ViewportY = -1024.0f / 2.0f;
    m_ViewportOffsetX = 0.0f;
    m_ViewportOffsetY = 0.0f;
    m_ViewportWidth = 1024.0f;
    m_ViewportHeight = 1024.0f;

    m_Scale = 0.0f;
    m_RollPitchYaw = XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 0.0f));

	// Projection initial settings
	m_Fov = 1.22f;
	m_AspectRation = 1.333333f;
    m_NearPlane = 1.0f;
    m_FarPlane = 1000.0f;

    // Set projection
    if (m_ProjectionType == ProjectionType::Perspective)
    {
        m_Position = XMLoadFloat3(&XMFLOAT3(0.0f, 7.0f, 0.0f));
	    m_Projection = XMMatrixPerspectiveFovLH(m_Fov, m_AspectRation, m_NearPlane, m_FarPlane); 
    }
    else
    {
        m_Position = XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, -1.0f));
        m_Projection = XMMatrixOrthographicOffCenterLH(0.0f, 1024.0f, 0.0f, 1024.0f, 1.0f, 1000.0f);
    }

    // View initial settings	
    m_Direction = XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 1.0f));
    m_UpVector = XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f));
    m_At = XMVectorAdd(m_Position, m_Direction);

	// Set view matrix
	HRESULT hr = SetLocalTransform(XMMatrixLookAtLH(m_Position, m_At, m_UpVector));
	
	if (SUCCEEDED(hr))
	{
        m_WorldTransform = m_LocalTransform;
		hr = SetName(nodeName);
	}
   
    return hr;
}

HRESULT SceneObjectCamera::GetProjectionMatrix(__deref_out XMMATRIX *projection)
{
   if (!projection)
        return E_POINTER;

   *projection = m_Projection;
   return S_OK;
}

HRESULT SceneObjectCamera::SetProjectionMatrix(__in CXMMATRIX projection)
{
    m_Projection = projection;
    return Invalidate(SOCT_PROJECTION);
}

HRESULT SceneObjectCamera::GetViewMatrix(__deref_out XMMATRIX *viewMatrix)
{
	*viewMatrix = m_WorldTransform;
	return S_OK;
}

HRESULT SceneObjectCamera::Look(__in const FLOAT &yawDelta, __in const FLOAT &pitchDelta, __in const FLOAT &rollDelta, __in const FLOAT &scaleDelta)
{
    FLOAT yaw = XMVectorGetY(m_RollPitchYaw) + yawDelta;
    if (yaw > XM_2PI)
    {
        yaw = yaw - XM_2PI;
    }

    FLOAT roll = XMVectorGetZ(m_RollPitchYaw) + rollDelta;
    if (roll > XM_2PI)
    {
        roll = roll - XM_2PI;
    }

    FLOAT pitch = XMVectorGetX(m_RollPitchYaw) + pitchDelta;
    if (pitch > XM_2PI)
    {
        pitch = pitch - XM_2PI;
    }

    m_RollPitchYaw = XMVectorSet(pitch, yaw, roll, 1.0f);
    
    m_Scale += scaleDelta;

	// Camera Rotation
    XMVECTOR quaternionRotation = XMQuaternionRotationRollPitchYawFromVector(m_RollPitchYaw);
    XMVECTOR direction = XMVector3Rotate(m_Direction, quaternionRotation);
    direction = XMVector3Normalize(direction);
    XMVECTOR upVector = XMVector3Rotate(m_UpVector, quaternionRotation);

	XMVECTOR position = XMVectorSubtract(m_Position, XMVectorScale(direction, m_Scale));
	XMVECTOR at = XMVectorAdd(position, direction);

    // Set ViewMatrix
	return SetLocalTransform(XMMatrixLookAtLH(position, at, upVector));
}

HRESULT SceneObjectCamera::Move(__in const FLOAT &forwardDelta, __in const FLOAT &strideDelta, __in const FLOAT &upDelta)
{
    // Camera Rotation
	XMVECTOR quaternionRotation = XMQuaternionRotationRollPitchYawFromVector(m_RollPitchYaw);
	XMVECTOR quaternionRotationYaw = XMQuaternionRotationRollPitchYaw(0.0f, XMVectorGetY(m_RollPitchYaw), 0.0f);
	
	XMVECTOR directionYaw = XMVector3Rotate(m_Direction, quaternionRotationYaw);
    directionYaw = XMVector3Normalize(directionYaw);
	XMVECTOR direction = XMVector3Rotate(m_Direction, quaternionRotation);
    direction = XMVector3Normalize(direction);
	XMVECTOR upVector = XMVector3Rotate(m_UpVector, quaternionRotation);

	// Forward
	m_Position = XMVectorAdd(m_Position, XMVectorScale(directionYaw, forwardDelta));

    // Stride
    m_Position = XMVectorAdd(m_Position, XMVectorScale(XMVector3Normalize(XMVector3Cross(directionYaw, m_UpVector)), strideDelta));
    
	// Up
    m_Position = XMVectorAdd(m_Position, XMVectorScale(m_UpVector, upDelta));

	XMVECTOR position = XMVectorSubtract(m_Position, XMVectorScale(direction, m_Scale));
	XMVECTOR at = XMVectorAdd(position, direction);

    // Set ViewMatrix
	return SetLocalTransform(XMMatrixLookAtLH(position, at, upVector));
}

void SceneObjectCamera::ResetRotationAndScale()
{
	m_RollPitchYaw = XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_Scale = 0.0f;
}

HRESULT SceneObjectCamera::SetPositionAtUpFov(__in CXMVECTOR pos, __in CXMVECTOR at, __in CXMVECTOR up, __in const FLOAT &fov)
{
	ResetRotationAndScale();

	// Projection
	m_Fov = fov;	
	m_Projection = XMMatrixPerspectiveFovLH(m_Fov, m_AspectRation, m_NearPlane, m_FarPlane); 

	// View
	m_Position = pos;
	m_At = at;
	m_UpVector = up;
	m_Direction = XMVectorSubtract(m_At, m_Position);
	
	// Set ViewMatrix
	return SetLocalTransform(XMMatrixLookAtLH(m_Position, m_At, m_UpVector));
}

HRESULT SceneObjectCamera::SetPositionAndScale(__in CXMVECTOR pos, __in const FLOAT &scale)
{
	ResetRotationAndScale();

	m_Position = pos;
	m_Scale = scale;

	XMVECTOR position = XMVectorSubtract(m_Position, XMVectorScale(m_Direction, m_Scale));
	
	// Set ViewMatrix
	return SetLocalTransform(XMMatrixLookAtLH(position, m_At, m_UpVector));
}

HRESULT SceneObjectCamera::SetViewport(__in const FLOAT &viewportX, __in const FLOAT &viewportY, __in const FLOAT &viewportWidth, __in const FLOAT &viewportHeight)
{
    m_ViewportX = viewportX;
    m_ViewportY = viewportY;
    m_ViewportWidth = viewportWidth;
    m_ViewportHeight = viewportHeight;

    m_Projection = XMMatrixOrthographicOffCenterLH(m_ViewportX, m_ViewportX + m_ViewportWidth, m_ViewportY, m_ViewportY + m_ViewportHeight, 1.0f, 1000.0f);
    
    return Invalidate(SOCT_VIEWPORT);
}

HRESULT SceneObjectCamera::GetViewport(__out FLOAT &viewportX, __out FLOAT &viewportY, __out FLOAT &viewportWidth, __out FLOAT &viewportHeight)
{
    viewportX = m_ViewportX;
    viewportY = m_ViewportY;
    viewportWidth = m_ViewportWidth;
    viewportHeight = m_ViewportHeight;
    return S_OK;
}

HRESULT SceneObjectCamera::PanZoomOrthographic(__in const FLOAT &viewportOffsetX, __in const FLOAT &viewportOffsetY, __in const FLOAT &scaleDelta)
{
    FLOAT ratio = m_ViewportWidth / m_ScreenWidth;

    FLOAT delta = scaleDelta * ratio;

    m_ViewportWidth += delta;
    m_ViewportHeight += delta;
    m_ViewportOffsetX = ratio * viewportOffsetX;
    m_ViewportOffsetY = ratio * viewportOffsetY* (m_ScreenWidth / m_ScreenHeight);

    m_Projection = XMMatrixOrthographicOffCenterLH(m_ViewportX + m_ViewportOffsetX, m_ViewportX + m_ViewportOffsetX + m_ViewportWidth, m_ViewportY + m_ViewportOffsetY, m_ViewportY + m_ViewportOffsetY + m_ViewportHeight, 1.0f, 1000.0f); 
    return S_OK;
}

HRESULT SceneObjectCamera::ApplyViewportOffsetX()
{
    m_ViewportX += m_ViewportOffsetX;
    m_ViewportOffsetX = 0;
    return S_OK;
}

HRESULT SceneObjectCamera::ApplyViewportOffsetY()
{
    m_ViewportY += m_ViewportOffsetY;
    m_ViewportOffsetY = 0;
    return S_OK;
}

HRESULT SceneObjectCamera::SetScreenSize(__in const FLOAT &width, __in const FLOAT &height)
{
    m_ScreenWidth = width;
    m_ScreenHeight = height;

    if (m_ScreenWidth > 0 && m_ScreenHeight > 0)
    {
        m_Scaling = XMMatrixScaling(1.0f, m_ScreenWidth / m_ScreenHeight, 1.0);
    }

    return Invalidate(SceneObjectContentType::SOCT_TRANSFORM);
}
   
HRESULT SceneObjectCamera::GetScreenSize(__out FLOAT &width, __out FLOAT &height)
{
    width = m_ScreenWidth;
    height = m_ScreenHeight;

    return S_OK;
}

HRESULT SceneObjectCamera::CalibrateToSceneObject(__in ISceneObject *pSceneObject)
{
    SmartPtr<ISceneObject> spSceneObject(pSceneObject);

    SmartPtr<ICalibrable> spCalibrable;
    HRESULT hr = spSceneObject->QueryInterface(&spCalibrable);

    if (SUCCEEDED(hr))
    {
        spCalibrable->Calibrate();
    }

    return hr;
}

HRESULT SceneObjectCamera::Update(SceneObjectContentType soct)
{
    HRESULT hr = SceneObject::Update(soct); 

    // Scale so that aspect ratio remains the same
    if (SUCCEEDED(hr) && ((soct & SOCT_TRANSFORM) || (soct & SOCT_OBJECT_ADD)))
    {
        m_WorldTransform = XMMatrixMultiply(m_WorldTransform, m_Scaling);
    }
    
    return hr;
}

HRESULT CreateCamera(__in_z const wchar_t *nodeName, __in const ProjectionType &projectionType,__deref_out ISceneObjectCamera **ppResult)
{
    return SceneObjectCamera::CreateInstance(ppResult, nodeName, projectionType);
}