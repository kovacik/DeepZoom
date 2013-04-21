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
#include "MeshStream.h"

const size_t DEFAULT_ALIGNMENT = 16;

MeshStream::MeshStream()
{
	m_pBuffer = NULL;
    m_ItemSize = 0;
	m_Length = 0;
	m_IsLocked = false;
	m_IsDeleted = false;
}

MeshStream::~MeshStream()
{
	if (m_IsLocked)
	{
		m_BufferAccess.UnLock();
	}

	DeleteBuffer();
}

MeshStreamType MeshStream::GetType()
{
	return m_MeshStreamType;
}

UINT MeshStream::GetLength()
{
	return m_Length;
}

UINT MeshStream::GetItemSize()
{
	return m_ItemSize;
}

HRESULT MeshStream::Lock(void **ppBuffer)
{
	if (m_Length == 0 || m_ItemSize == 0)
	{
		return NTE_BAD_LEN;
	}

	m_BufferAccess.Lock();
	m_IsLocked = true;

	*ppBuffer = m_pBuffer;

	return S_OK;
}

HRESULT MeshStream::Unlock()
{
	if (m_IsLocked)
	{
		m_BufferAccess.UnLock();
		m_IsLocked = false;

		return S_OK;
	}
	
	return E_FAIL;
}

HRESULT MeshStream::InitializeBuffer(__in const UINT itemSize, __in const UINT length, __in const size_t alignment)
{
	if ((length == 0) || (itemSize == 0) || (alignment == 0))
		return E_INVALIDARG;

	m_pBuffer = _aligned_malloc(itemSize * length, alignment);
	if (!m_pBuffer)
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

HRESULT MeshStream::DeleteBuffer()
{
	if (!m_IsDeleted)
	{
		_aligned_free(m_pBuffer);
		m_pBuffer = NULL;
		m_IsDeleted = true;
	}

	m_ItemSize = 0;
	m_Length = 0;

	return S_OK;
}

HRESULT MeshStream::Initialize(__in const MeshStreamType meshType, __in const UINT itemSize, __in const UINT length, __deref_opt_in_bcount(itemSize * length) const void* pInitialData)
{
	HRESULT hr;

	hr = m_BufferAccess.Initialize();

	if (SUCCEEDED(hr))
	{
		hr = InitializeBuffer(itemSize, length, DEFAULT_ALIGNMENT);

		if (SUCCEEDED(hr))
		{
			if (pInitialData)
			{
				size_t size = itemSize * length;
				memcpy(m_pBuffer, pInitialData, size);
			}

			m_ItemSize = itemSize;
			m_Length = length;
			m_MeshStreamType = meshType;
		}
	}

	return hr;
}

HRESULT CreateMeshStream(__in const MeshStreamType meshType, __in const UINT itemSize, __in const UINT length, __deref_opt_in_bcount(itemSize * length) const void* pInitialData, __deref_out IMeshStream **ppResult)
{
	return MeshStream::CreateInstance(ppResult, meshType, itemSize, length, pInitialData);
}