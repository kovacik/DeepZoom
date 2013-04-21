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

template <typename A1 = Details::unused_parameter, typename A2 = Details::unused_parameter>
class SmartDelegateEvent : public SmartDelegate<void, A1, A2>
{
    typedef SmartDelegate<void, A1, A2> DelegateType;
    Vector<DelegateType> m_Delegates;

public:
    ~SmartDelegateEvent()
    {
        Clear();
    }

    HRESULT Add(const DelegateType& dlgt)
    {
        return m_Delegates.Add(dlgt);
    }

    HRESULT Remove(const DelegateType& dlgt)
    {
        return m_Delegates.Remove(dlgt);
    }

    void operator()(A1 p1, A2 p2) const
    {
        for (UINT i = 0; i < m_Delegates.Length(); i++)
        {
            (m_Delegates[i])(p1, p2);
        }
    }

    void Clear()
    {
        m_Delegates.Clear();
    }
};

template <typename A1>
class SmartDelegateEvent<A1, Details::unused_parameter> : public SmartDelegate<void, A1>
{
    typedef SmartDelegate<void, A1> DelegateType;
    Vector<DelegateType> m_Delegates;

public:
    ~SmartDelegateEvent()
    {
        Clear();
    }

    HRESULT Add(const DelegateType& dlgt)
    {
        return m_Delegates.Add(dlgt);
    }

    HRESULT Remove(const DelegateType& dlgt)
    {
        return m_Delegates.Remove(dlgt);
    }

    void operator()(A1 p1) const
    {
        for (UINT i = 0; i < m_Delegates.Length(); i++)
        {
            (m_Delegates[i])(p1);
        }
    }

    void Clear()
    {
        m_Delegates.Clear();
    }
};

template <>
class SmartDelegateEvent<Details::unused_parameter, Details::unused_parameter> : public SmartDelegate<void>
{
    typedef SmartDelegate<void> DelegateType;
    Vector<DelegateType> m_Delegates;

public:
    ~SmartDelegateEvent()
    {
        Clear();
    }

    HRESULT Add(const DelegateType& dlgt)
    {
        return m_Delegates.Add(dlgt);
    }

    HRESULT Remove(const DelegateType& dlgt)
    {
        return m_Delegates.Remove(dlgt);
    }

    void operator()() const
    {
        for (UINT i = 0; i < m_Delegates.Length(); i++)
        {
            (m_Delegates[i])();
        }
    }

    void Clear()
    {
        m_Delegates.Clear();
    }
};
