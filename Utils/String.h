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

DECLAREINTERFACE(IStringBuffer, IUnknown, "{10579B92-9AD4-4CE9-B467-DFA34C74C453}")
{
    __out_z const WCHAR* GetBuffer() const;
    HRESULT SetBuffer(__deref_in const WCHAR* pszString, __in const UINT &size);
    HRESULT Concat(__deref_in const WCHAR* pszString, __in_z const UINT &length);
};

class StringBuffer : public ImplementSmartObject
    <
        StringBuffer,
        IStringBuffer
    >
{
    WCHAR* m_pszString;

public:
    StringBuffer() : m_pszString(NULL)
    {};

    ~StringBuffer()
    {
        free(m_pszString);
    }

    __out_z const WCHAR* GetBuffer() const
    {
        return m_pszString;
    }

    HRESULT Initialize(__deref_in const WCHAR* pszString, __in const UINT &length);
    HRESULT SetBuffer(__deref_in const WCHAR* pszString, __in_z const UINT &length);
    HRESULT Concat(__deref_in const WCHAR* pszString, __in_z const UINT &length);  
};

class String 
{
    friend class StringBuffer;

private:
    SmartPtr<IStringBuffer> m_spStringBuffer;

public:
    __out_z const WCHAR* GetBuffer() const
    {
        return (m_spStringBuffer == NULL) ? NULL : m_spStringBuffer->GetBuffer();
    }

    HRESULT Set(__in_z const WCHAR* pszString);
    HRESULT Concat(__in_z const WCHAR* pszString);

    bool operator == ( __in const String &str ) const;
    bool operator == (__in_z const WCHAR* pszString) const;
};

