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
#include "String.h"


HRESULT StringBuffer::Initialize(__deref_in const WCHAR* pszString, __in const UINT &length)
{
    return SetBuffer(pszString, length);
}

HRESULT StringBuffer::SetBuffer(__deref_in const WCHAR* pszString, __in_z const UINT &length)
{
    size_t size = (pszString == NULL) ? 0 : (length + 1) * sizeof(WCHAR);

    WCHAR* temp = (WCHAR*)realloc(m_pszString, size);
        
    if (size == 0)
    {
        // Memory has been deallocated, set pointer to NULL
        m_pszString = NULL;
        return S_OK;
    }
    else if (temp)
    {
        m_pszString = temp;
        memcpy(m_pszString, pszString, size);
        return S_OK;
    }

    // If temp is NULL and size > 0, memory was not allocated
    return E_OUTOFMEMORY;
}

HRESULT StringBuffer::Concat(__deref_in const WCHAR* pszString, __in_z const UINT &length)
{
    if (!m_pszString)
    {
        return SetBuffer(pszString, length);
    }
        
    if (length > 0)
    {
        size_t actualLen = wcslen(m_pszString);
        size_t newSize = (actualLen + length + 1) * sizeof(WCHAR);

        WCHAR* temp = (WCHAR*)realloc(m_pszString, newSize);

        if (!temp)
        {
            return E_OUTOFMEMORY;
        }

        m_pszString = temp;
        memcpy(m_pszString + actualLen, pszString, (length + 1) * sizeof(WCHAR));
    }

    return S_OK;
}


static HRESULT CreateStringBuffer(__deref_in const WCHAR* pszString, __in const UINT &length, __deref_out IStringBuffer** ppStringBuffer)
{
    return StringBuffer::CreateInstance(ppStringBuffer, pszString, length);
}

HRESULT String::Set(__in_z const WCHAR* pszString)
{
    size_t len = wcslen(pszString);

    if (m_spStringBuffer)
    {
        return m_spStringBuffer->SetBuffer(pszString, (UINT)len);
    }

    return CreateStringBuffer(pszString, (UINT)len, &m_spStringBuffer);
}

HRESULT String::Concat(__in_z const WCHAR* pszString)
{
    size_t len = wcslen(pszString);

    if (m_spStringBuffer)
    {
        return m_spStringBuffer->Concat(pszString, (UINT)len);
    }

    return CreateStringBuffer(pszString, (UINT)len, &m_spStringBuffer);
}

bool String::operator == ( __in const String &str ) const
{
    return (*this) == str.GetBuffer();
}

bool String::operator == (__in_z const WCHAR* pszString) const
{
    if (m_spStringBuffer == NULL)
    {
        return (pszString == NULL);
    }

    const WCHAR *pszStr = GetBuffer();
    if (pszStr == NULL)
    {
        return (pszString == NULL);
    }

    return !wcscmp(pszStr, pszString);
}


