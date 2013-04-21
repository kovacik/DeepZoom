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
#include "VertexStream.h"

HRESULT VertexStream::Initialize(__in const UINT length, __deref_in_ecount(length) const VertexType* pInitialData)
{
	return MeshStream::Initialize(MeshStreamType::MST_VERTEX_POSITIONS, sizeof(VertexType), length, pInitialData);
}

HRESULT VertexStream::Lock(VertexType **ppVertexBuffer)
{
	return MeshStream::Lock((void**)ppVertexBuffer);
}

HRESULT VertexStream::Unlock()
{
	return MeshStream::Unlock();
}

HRESULT CreateVertexStream(__in const UINT length, __deref_opt_in_ecount(length) const VertexType* pInitialData, __deref_out IVertexStream **ppResult)
{		
	return VertexStream::CreateInstance(ppResult, length, pInitialData);
}