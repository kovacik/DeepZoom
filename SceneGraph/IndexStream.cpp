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
#include "IndexStream.h"

HRESULT IndexStream::Initialize(__in const UINT length, __deref_in_ecount(length) const unsigned long* pInitialData)
{
	return MeshStream::Initialize(MeshStreamType::MST_TRIANGLES_VERTEX_INDEXES, sizeof(unsigned long), length, pInitialData);
}

HRESULT IndexStream::Lock(unsigned long **ppIndexBuffer)
{
	return MeshStream::Lock((void**)ppIndexBuffer);
}

HRESULT IndexStream::Unlock()
{
	return MeshStream::Unlock();
}

HRESULT CreateIndexStream(__in const UINT length, __deref_opt_in_ecount(length) const unsigned long* pInitialData, __deref_out IIndexStream **ppResult)
{
	return IndexStream::CreateInstance(ppResult, length, pInitialData);
}