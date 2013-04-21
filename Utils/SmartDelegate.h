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


template<typename R, typename A1 = Details::unused_parameter, typename A2 = Details::unused_parameter>
class SmartDelegate;


//-----------------------------------------------------------------------------
// Operator overloads
//-----------------------------------------------------------------------------
template<typename R, typename A1, typename A2>
bool operator== (const SmartDelegate<R, A1, A2> &smartDlg1, const SmartDelegate<R, A1, A2> &smartDlg2)
{
    return ((smartDlg1.m_ObjectInstance == smartDlg2.m_ObjectInstance) &&
            (smartDlg1.m_Func == smartDlg2.m_Func));
}

template<typename R, typename A1, typename A2>
bool operator!= (const SmartDelegate<R, A1, A2> &smartDlg1, const SmartDelegate<R, A1, A2> &smartDlg2)
{
    return !(smartDlg1 == smartDlg2);
}

template<typename R, typename A1>
bool operator== (const SmartDelegate<R, A1> &smartDlg1, const SmartDelegate<R, A1> &smartDlg2)
{
    return ((smartDlg1.m_ObjectInstance == smartDlg2.m_ObjectInstance) &&
            (smartDlg1.m_Func == smartDlg2.m_Func));
}

template<typename R, typename A1>
bool operator!= (const SmartDelegate<R, A1> &smartDlg1, const SmartDelegate<R, A1> &smartDlg2)
{
    return !(smartDlg1 == smartDlg2);
}

template<typename R>
bool operator== (const SmartDelegate<R> &smartDlg1, const SmartDelegate<R> &smartDlg2)
{
    return ((smartDlg1.m_ObjectInstance == smartDlg2.m_ObjectInstance) &&
            (smartDlg1.m_Func == smartDlg2.m_Func));
}

template<typename R>
bool operator!= (const SmartDelegate<R> &smartDlg1, const SmartDelegate<R> &smartDlg2)
{
    return !(smartDlg1 == smartDlg2);
}


//-----------------------------------------------------------------------------
// Smart delegate class - currently suported functions with at most 2 parameters
//-----------------------------------------------------------------------------
template<typename R, typename A1, typename A2>
class SmartDelegate
{
private:   
    typedef typename R (*FuncType)(void*, A1, A2);
    FuncType m_Func;
    SmartPtr<IUnknown> m_ObjectInstance;

public:
    SmartDelegate(IUnknown* objectInst, FuncType func) : m_Func(func) { m_ObjectInstance = (IUnknown*)objectInst; }
    SmartDelegate() : m_Func(NULL) {}
    SmartDelegate(IUnknown* objectInst) { m_ObjectInstance = (IUnknown*)objectInst; }

    R operator()(A1 a1, A2 a2) const
    {
        return (*m_Func)(m_ObjectInstance, a1, a2);
    }

    template<typename R, typename A1, typename A2>
    friend bool operator== (const SmartDelegate<R, A1, A2> &smartDlg1, const SmartDelegate<R, A1, A2> &smartDlg2);

    template<typename R, typename A1, typename A2>
    friend bool operator!= (const SmartDelegate<R, A1, A2> &smartDlg1, const SmartDelegate<R, A1, A2> &smartDlg2);

    SmartDelegate<R, A1, A2>& operator =(const SmartDelegate<R, A1, A2> &smartDlg)
    {
        if (this != &smartDlg)
        {
            m_ObjectInstance = smartDlg.m_ObjectInstance;
            m_Func = smartDlg.m_Func;
        }

        return *this;
    }
};

template<typename R, typename A1>
class SmartDelegate<R, A1, Details::unused_parameter>
{
private:   
    typedef typename R (*FuncType)(void*, A1);
    FuncType m_Func;
    SmartPtr<IUnknown> m_ObjectInstance;

public:
    SmartDelegate(IUnknown* objectInst, FuncType func) : m_Func(func) { m_ObjectInstance = (IUnknown*)objectInst; }
    SmartDelegate() : m_Func(NULL) {}
    SmartDelegate(IUnknown* objectInst) { m_ObjectInstance = (IUnknown*)objectInst; }

    R operator()(A1 a1) const
    {
        return (*m_Func)(m_ObjectInstance, a1);
    }

    template<typename R, typename A1>
    friend bool operator== (const SmartDelegate<R, A1> &smartDlg1, const SmartDelegate<R, A1> &smartDlg2);

    template<typename R, typename A1>
    friend bool operator!= (const SmartDelegate<R, A1> &smartDlg1, const SmartDelegate<R, A1> &smartDlg2);

    SmartDelegate<R, A1>& operator =(const SmartDelegate<R, A1> &smartDlg)
    {
        if (this != &smartDlg)
        {
            m_ObjectInstance = smartDlg.m_ObjectInstance;
            m_Func = smartDlg.m_Func;
        }

        return *this;
    }
};

template<typename R>
class SmartDelegate<R, Details::unused_parameter, Details::unused_parameter>
{
private:   
    typedef typename R (*FuncType)(void*);
    FuncType m_Func;
    SmartPtr<IUnknown> m_ObjectInstance;

public:
    SmartDelegate(IUnknown* objectInst, FuncType func) : m_Func(func) { m_ObjectInstance = (IUnknown*)objectInst; }
    SmartDelegate() : m_Func(NULL) {}
    SmartDelegate(IUnknown* objectInst) { m_ObjectInstance = (IUnknown*)objectInst; }

    R operator()() const
    {
        return (*m_Func)(m_ObjectInstance);
    }

    template<typename R>
    friend bool operator== (const SmartDelegate<R> &smartDlg1, const SmartDelegate<R> &smartDlg2);

    template<typename R>
    friend bool operator!= (const SmartDelegate<R> &smartDlg1, const SmartDelegate<R> &smartDlg2);

    SmartDelegate<R>& operator =(const SmartDelegate<R> &smartDlg)
    {
        if (this != &smartDlg)
        {
            m_ObjectInstance = smartDlg.m_ObjectInstance;
            m_Func = smartDlg.m_Func;
        }

        return *this;
    }
};

//-----------------------------------------------------------------------------
// Class member function binding helper
//-----------------------------------------------------------------------------
template
<
    class T, 
    typename R, 
    typename A1 = Details::unused_parameter, 
    typename A2 = Details::unused_parameter
>
struct FastDelegateHelper
{
public:
    template<R (T::*Func)(A1, A2)>
    static R Wrapper(void* o, A1 a1, A2 a2)
    {
        return (static_cast<T*>(o)->*Func)(a1, a2);
    }

    template<R (T::*Func)(A1)>
    static R Wrapper(void* o, A1 a1)
    {
        return (static_cast<T*>(o)->*Func)(a1);
    }

    template<R (T::*Func)()>
    static R Wrapper(void* o)
    {
        return (static_cast<T*>(o)->*Func)();
    }    

    template<R (T::*Func)(A1, A2)>
    inline static SmartDelegate<R, A1, A2> Bind(T* o)
    {
        return SmartDelegate<R, A1, A2>(o, &Wrapper<Func>);
    }

    template<R (T::*Func)(A1)>
    inline static SmartDelegate<R, A1> Bind(T* o)
    {
        return SmartDelegate<R, A1>(o, &Wrapper<Func>);
    }

    template<R (T::*Func)()>
    inline static SmartDelegate<R> Bind(T* o)
    {
        return SmartDelegate<R>(o, &Wrapper<Func>);
    }
};

template<class T, typename R, typename A1, typename A2>
FastDelegateHelper<T, R, A1, A2> DeduceMemberFuncDelegate(R (T::*)(A1, A2))
{
    return FastDelegateHelper<T, R, A1, A2>();
}

template<class T, typename R, typename A1>
FastDelegateHelper<T, R, A1> DeduceMemberFuncDelegate(R (T::*)(A1))
{
    return FastDelegateHelper<T, R, A1>();
}

template<class T, typename R>
FastDelegateHelper<T, R> DeduceMemberFuncDelegate(R (T::*)())
{
    return FastDelegateHelper<T, R>();
}

#define BIND_MEMBER_FUNCTION_SMART(memberFuncPtr, instancePtr) \
    (DeduceMemberFuncDelegate(memberFuncPtr).Bind<(memberFuncPtr)>(instancePtr))