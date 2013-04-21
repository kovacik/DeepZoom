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

class MeshStream : 
    public ImplementSmartObject<MeshStream, IMeshStream>
{
	void *m_pBuffer;
    MeshStreamType m_MeshStreamType;
	CriticalSection m_BufferAccess;
	UINT m_ItemSize;
	UINT m_Length;
	bool m_IsLocked;
	bool m_IsDeleted;

	HRESULT DeleteBuffer();
	HRESULT InitializeBuffer(__in const UINT itemSize, __in const UINT length, __in const size_t alignment);

public:
    MeshStream();
    ~MeshStream();

    MeshStreamType GetType();
    UINT GetLength();
    UINT GetItemSize();

	HRESULT Initialize(__in const MeshStreamType meshType, __in const UINT itemSize, __in const UINT length, __deref_opt_in_bcount(itemSize * length) const void* pInitialData);

	HRESULT Lock(void **ppBuffer);
    HRESULT Unlock();
};