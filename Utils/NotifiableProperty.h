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

class BaseNotifiableProperty
{
public:
    SmartDelegateEvent<> OnPropertyChanged;
};

template<class ValueType> 
class NotifiableProperty : public BaseNotifiableProperty
{
private:
    ValueType m_Value;

public:
    NotifiableProperty(){};

    NotifiableProperty(const ValueType &v)
    {
        m_Value = v;
    }

    ValueType& Get()
    {
        return m_Value;
    }

    void Update()
    {
        OnPropertyChanged();
    }

    ValueType& Set( const ValueType &val )
    {
        ValueType oldValue = m_Value;

        if ( memcmp( &m_Value, &val, sizeof(ValueType) ) )
        {
            m_Value = val;
        }

        OnPropertyChanged();

        return m_Value;
    }

    template< class C >
    ValueType& operator=( const C &val ) 
    { 
        return Set( ( const ValueType )val ); 
    }

    template< class C >
    ValueType& operator=( const NotifiableProperty<C> &val ) 
    { 
        return Set( ( const ValueType )val.m_Value ); 
    }

    template< class C >
    ValueType& operator+=( const C &val ) 
    {
        return Set( m_Value + ( const ValueType )val ); 
    }

    template< class C >
    ValueType& operator-=( const C &val ) 
    {
        return Set( m_Value - ( const ValueType )val ); 
    }

    template< class C >
    ValueType& operator/=( const C &val ) 
    {
        return Set( m_Value / ( const ValueType )val ); 
    }

    template< class C >
    ValueType& operator*=( const C &val ) 
    {
        return Set( m_Value * ( const ValueType )val ); 
    }

    template< class C >
    ValueType& operator^=( const C &val ) 
    {
        return Set( m_Value ^ ( const ValueType )val ); 
    }

    template< class C >
    ValueType& operator|=( const C &val ) 
    {
        return Set( m_Value | ( const ValueType )val ); 
    }

    ValueType& operator++()
    {
        return (*this += 1);
    }

    ValueType operator--()
    {
        return (*this -= 1);
    }

    ValueType operator++( int )
    {
        ValueType val = m_Value;
        (*this += 1);
        return val;
    }

    ValueType operator--( int )
    {
        ValueType val = m_Value;
        (*this -= 1);
        return val;
    }

    template< class C >
    ValueType& operator&=( const C &val ) 
    {	
        return Set( m_Value & ( const ValueType )val ); 
    }

    operator ValueType&() 
    {
        return m_Value; 
    }

    ValueType* operator->() 
    {
        return &m_Value; 
    }
};