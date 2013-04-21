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

#define DEF_GUIDNAME(x) __uuidof(x)

#define DECLAREINTERFACE(iface, baseiface, iid) __interface __declspec(uuid(iid)) __declspec(novtable)  iface : baseiface 

enum ClassFlags_e
{
    CF_DEFAULT = 0,
    CF_ALIGNED_MEMORY = 1,
};

template <int flags>
class ClassFlags
{
    static const int ClassFlagsValue = flags;
};

template <typename T>
struct InheritImplementation : T
{
};

namespace std
{
    struct nothrow_t;
};

#pragma push_macro("new")
#pragma push_macro("delete")

#undef new
#undef delete


#define IMPLEMENT_NEW_DELETE() \
    void * operator new (size_t size) \
    { \
        return malloc(size); \
    } \
    void * operator new (size_t size, void *pBuffer) \
    { \
        UNREFERENCED_PARAMETER(size); \
        return pBuffer; \
    } \
    void * operator new (size_t size, const std::nothrow_t &) \
    { \
        return malloc(size); \
    } \
    void operator delete (void* ptr) \
    { \
        free(ptr); \
    } \
    void operator delete (void* ptr, void *buf) \
    { \
        ptr; buf; \
    } \
    void operator delete (void* ptr, const std::nothrow_t &) \
    { \
        free(ptr); \
    } \

#define IMPLEMENT_NEW_DELETE_ALIGNED() \
    void * operator new (size_t size) \
    { \
        return _aligned_malloc(size, 16); \
    } \
    void * operator new (size_t size, const std::nothrow_t &) \
    { \
        return _aligned_malloc(size, 16); \
    } \
    void operator delete (void* ptr) \
    { \
        _aligned_free(ptr); \
    } \
    void operator delete (void* ptr, const std::nothrow_t &) \
    { \
        _aligned_free(ptr); \
    } \

#pragma pop_macro("delete")
#pragma pop_macro("new")

namespace Details {

// smart object implementation (ref counting)
template <class T>
class __declspec(novtable) SmartObjectBase : public T
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, __deref_out_opt void **ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
};

template <class T>
class SmartObject : public SmartObjectBase<T>
{
    volatile LONG m_RefCount;
public:

    SmartObject()
    {
        m_RefCount = 0;
    }

    STDMETHOD(QueryInterface)(REFIID riid, __deref_out_opt void **ppvObject)
    {
        return InternalQueryInterface(riid, ppvObject);
    };

    STDMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement(&m_RefCount);
    }

    // we implement release in inherited class so we call the correct delete operator.
    STDMETHOD_(ULONG, Release)()
    {
        ULONG refCnt = InterlockedDecrement(&m_RefCount);
        if (!refCnt)
            delete this;

        return refCnt;
    }

    static HRESULT CreateInstance(__deref_out SmartObjectBase<T> **ppInstance)
    {
        SmartObjectBase<T> *pResult = new SmartObject<T>();
        if (pResult)
        {
            *ppInstance = pResult;
            return S_OK;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

private:

    IMPLEMENT_NEW_DELETE();

};

template <class T>
class SmartObjectAligned : public SmartObjectBase<T>
{
    volatile LONG m_RefCount;
public:

    SmartObjectAligned()
    {
        m_RefCount = 0;
    }

    STDMETHOD(QueryInterface)(REFIID riid, __deref_out_opt void **ppvObject)
    {
        return InternalQueryInterface(riid, ppvObject);
    };

    STDMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement(&m_RefCount);
    }

    STDMETHOD_(ULONG, Release)()
    {
        ULONG refCnt = InterlockedDecrement(&m_RefCount);
        if (!refCnt)
            delete this;

        return refCnt;
    }

    static HRESULT CreateInstance(__deref_out SmartObjectBase<T> **ppInstance)
    {
        SmartObjectBase<T> *pResult = new SmartObjectAligned<T>();
        if (pResult)
        {
            *ppInstance = pResult;
            return S_OK;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

private:
    IMPLEMENT_NEW_DELETE_ALIGNED()
};

template <class T>
class SmartObjectPtr
{
    SmartObjectBase<T>* p;

public:
    SmartObjectPtr()
    {
        p = NULL;
    }

    ~SmartObjectPtr()
    {
        delete p;
    }

    template <class Interface>
    HRESULT DetachTo(__deref_out Interface** ppResult)
    {
        if (!ppResult)
            return E_POINTER;

        p->AddRef();
        *ppResult = p;
        p = NULL;
        return S_OK;
    }

    SmartObjectBase<T>** operator&()
    {
        _ASSERT(p==NULL);
        return &p;
    }

    SmartObjectBase<T>* operator -> () const 
    {
        return p;
    }
};

template <typename flags, typename BaseClass> 
class ClassFlagsImpl;

template <typename BaseClass>
class ClassFlagsImpl<ClassFlags<CF_DEFAULT>, BaseClass>
{
public:

    typedef SmartObject<BaseClass> *SmartObjectImplClass;

    static HRESULT CreateInstance(__deref_out SmartObjectBase<BaseClass> **ppObj)
    {
        return SmartObject<BaseClass>::CreateInstance(ppObj);
    }
};

template <typename BaseClass>
class ClassFlagsImpl<ClassFlags<CF_ALIGNED_MEMORY>, BaseClass>
{
public:
    typedef SmartObjectAligned<BaseClass> *SmartObjectImplClass;

    static HRESULT CreateInstance(__deref_out SmartObjectBase<BaseClass> **ppObj)
    {
        return SmartObjectAligned<BaseClass>::CreateInstance(ppObj);
    }
};

class unused_parameter
{
};

template <typename I0>
struct __declspec(novtable) InterfaceTraits
{
    template <typename T>
    static bool TryCastTo(__in T* ptr, REFIID riid, __deref_out_opt void **ppv)
    {
        if (riid == __uuidof(I0))
        {
            if (ppv)
            {
                I0 *pRes = static_cast<I0*>(ptr);
                *ppv = pRes;
                pRes->AddRef();
            }
            return true;
        }
        return false;
    }
};

template <typename ComObj>
struct __declspec(novtable) InterfaceTraits<InheritImplementation<ComObj>>
{
    template <typename T>
    static bool TryCastTo(__in T* ptr, REFIID riid, __deref_out_opt void **ppv)
    {
        return ((ComObj*)(ptr))->InternalQueryInterface(riid, ppv) == S_OK;
    }
};

template<> 
struct __declspec(novtable) InterfaceTraits<unused_parameter>
{
    template <typename T>
    static bool TryCastTo(__in T* ptr, REFIID riid, __deref_out void **ppv)
    {
        UNREFERENCED_PARAMETER(ptr);
        UNREFERENCED_PARAMETER(riid);
        UNREFERENCED_PARAMETER(ppv);
        return false;
    }
};

template <typename Base, typename T1, typename T2, typename T3, typename T4, typename T5>
class ImplementsInterfacesImpl : public Base, public T1, public T2, public T3, public T4, public T5 {};

template <typename Base, typename T1, typename T2, typename T3, typename T4>
class ImplementsInterfacesImpl<Base, T1, T2, T3, T4, unused_parameter> : public Base, public T1, public T2, public T3, public T4 {};

template <typename Base, typename T1, typename T2, typename T3>
class ImplementsInterfacesImpl<Base, T1, T2, T3, unused_parameter, unused_parameter> : public Base, public T1, public T2, public T3 {};

template <typename Base, typename T1, typename T2>
class ImplementsInterfacesImpl<Base, T1, T2, unused_parameter, unused_parameter, unused_parameter> : public Base, public T1, public T2 {};

template <typename Base, typename T1>
class ImplementsInterfacesImpl<Base, T1, unused_parameter, unused_parameter, unused_parameter, unused_parameter> : public Base, public T1 {};

template <typename Base>
class ImplementsInterfacesImpl<Base, unused_parameter, unused_parameter, unused_parameter, unused_parameter, unused_parameter> : public Base {};

template 
<
    typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 
>
class ImplementsInterfacesHelper :
    public ImplementsInterfacesImpl< T1, T2, T3, T4, T5, T6 >
{
public:
    HRESULT InternalQueryInterface(REFIID riid, __deref_out_opt void **ppvObject)
    {
        ImplementsInterfacesImpl< T1, T2, T3, T4, T5, T6 > *pImpl = this;
        return (InterfaceTraits<T1>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T2>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T3>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T4>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T5>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T6>::TryCastTo(pImpl, riid, ppvObject)) ? S_OK : E_NOINTERFACE;
    }   
};

template 
<
    typename BaseClass, 
    typename Allocator, 
    typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 
>
class ImplementSmartObjectHelper :
    public ImplementsInterfacesImpl< T1, T2, T3, T4, T5, T6 >
{
public:
    typedef typename Allocator::SmartObjectImplClass SmartObjectImplClass;

    HRESULT InternalQueryInterface(REFIID riid, __deref_out_opt void **ppvObject)
    {
        ImplementsInterfacesImpl< T1, T2, T3, T4, T5, T6 > *pImpl = this;
        return (InterfaceTraits<T1>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T2>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T3>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T4>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T5>::TryCastTo(pImpl, riid, ppvObject) ||
                InterfaceTraits<T6>::TryCastTo(pImpl, riid, ppvObject) || 
                InterfaceTraits<IUnknown>::TryCastTo((IUnknown*)(T1*)(pImpl), riid, ppvObject)) ? S_OK : E_NOINTERFACE;
    }   

    template <class Result>
    static HRESULT CreateInstance(__deref_out Result **ppBase)
    {   
        HRESULT hr;
        SmartObjectPtr<BaseClass> spBase;
        hr = Allocator::CreateInstance(&spBase);
        if (SUCCEEDED(hr))
        {
            hr = spBase->Initialize();
            if (SUCCEEDED(hr))
            {
                hr = spBase.DetachTo(ppBase);
            }
        }
        return hr;
    };

    template <class Result, typename T1>
    static HRESULT CreateInstance(__deref_out Result **ppBase, T1 t1)
    {   
        HRESULT hr;
        SmartObjectPtr<BaseClass> spBase;
        hr = Allocator::CreateInstance(&spBase);
        if (SUCCEEDED(hr))
        {
            hr = spBase->Initialize(t1);
            if (SUCCEEDED(hr))
            {
                hr = spBase.DetachTo(ppBase);
            }
        }
        return hr;
    };

    template <class Result, typename T1, typename T2>
    static HRESULT CreateInstance(__deref_out Result **ppBase, T1 t1, T2 t2)
    {   
        HRESULT hr;
        SmartObjectPtr<BaseClass> spBase;
        hr = Allocator::CreateInstance(&spBase);
        if (SUCCEEDED(hr))
        {
            hr = spBase->Initialize(t1, t2);
            if (SUCCEEDED(hr))
            {
                hr = spBase.DetachTo(ppBase);
            }
        }
        return hr;
    };

    template <class Result, typename T1, typename T2, typename T3>
    static HRESULT CreateInstance(__deref_out Result **ppBase, T1 t1, T2 t2, T3 t3)
    {   
        HRESULT hr;
        SmartObjectPtr<BaseClass> spBase;
        hr = Allocator::CreateInstance(&spBase);
        if (SUCCEEDED(hr))
        {
            hr = spBase->Initialize(t1, t2, t3);
            if (SUCCEEDED(hr))
            {
                hr = spBase.DetachTo(ppBase);
            }
        }
        return hr;
    };

    template <class Result, typename T1, typename T2, typename T3, typename T4>
    static HRESULT CreateInstance(__deref_out Result **ppBase, T1 t1, T2 t2, T3 t3, T4 t4)
    {   
        HRESULT hr;
        SmartObjectPtr<BaseClass> spBase;
        hr = Allocator::CreateInstance(&spBase);
        if (SUCCEEDED(hr))
        {
            hr = spBase->Initialize(t1, t2, t3, t4);
            if (SUCCEEDED(hr))
            {
                hr = spBase.DetachTo(ppBase);
            }
        }
        return hr;
    };
    
    template <class Result, typename T1, typename T2, typename T3, typename T4, typename T5>
    static HRESULT CreateInstance(__deref_out Result **ppBase, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    {   
        HRESULT hr;
        SmartObjectPtr<BaseClass> spBase;
        hr = Allocator::CreateInstance(&spBase);
        if (SUCCEEDED(hr))
        {
            hr = spBase->Initialize(t1, t2, t3, t4, t5);
            if (SUCCEEDED(hr))
            {
                hr = spBase.DetachTo(ppBase);
            }
        }
        return hr;
    };

    template <class Result, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    static HRESULT CreateInstance(__deref_out Result **ppBase, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    {   
        HRESULT hr;
        SmartObjectPtr<BaseClass> spBase;
        hr = Allocator::CreateInstance(&spBase);
        if (SUCCEEDED(hr))
        {
            hr = spBase->Initialize(t1, t2, t3, t4, t5, t6);
            if (SUCCEEDED(hr))
            {
                hr = spBase.DetachTo(ppBase);
            }
        }
        return hr;
    };

    template <class Result, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    static HRESULT CreateInstance(__deref_out Result **ppBase, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
    {   
        HRESULT hr;
        SmartObjectPtr<BaseClass> spBase;
        hr = Allocator::CreateInstance(&spBase);
        if (SUCCEEDED(hr))
        {
            hr = spBase->Initialize(t1, t2, t3, t4, t5, t6, t7);
            if (SUCCEEDED(hr))
            {
                hr = spBase.DetachTo(ppBase);
            }
        }
        return hr;
    };
};

template<typename T>
__inline IUnknown *GetUnknown(__in T* p)
{
    __if_exists (T::SmartObjectImplClass)
    {
        return (IUnknown*)(T::SmartObjectImplClass*)(p);
    }
    __if_not_exists(T::SmartObjectImplClass)
    {
        return (IUnknown*)(T*)(p);
    }
};

};//namespace details

//default implementation
template 
< 
    typename BaseClass, 
    typename T1, 
    typename T2 = Details::unused_parameter, 
    typename T3 = Details::unused_parameter,
    typename T4 = Details::unused_parameter, 
    typename T5 = Details::unused_parameter, 
    typename T6 = Details::unused_parameter
>
class ImplementSmartObject 
    : public Details::ImplementSmartObjectHelper
    <
        BaseClass, 
        Details::ClassFlagsImpl< ClassFlags<CF_DEFAULT>, BaseClass >,
        T1, T2, T3, T4, T5, T6
    >

{
};


template 
<
    typename BaseClass, int flags,  typename T1, typename T2, typename T3, typename T4, typename T5
>
class ImplementSmartObject< BaseClass, ClassFlags<flags>, T1, T2, T3, T4, T5> 
    : public Details::ImplementSmartObjectHelper
    <
        BaseClass, 
        Details::ClassFlagsImpl<ClassFlags<flags>, BaseClass>,
        T1, T2, T3, T4, T5, Details::unused_parameter
    >
{
};

template 
<
    typename T1, 
    typename T2 = Details::unused_parameter, 
    typename T3 = Details::unused_parameter,
    typename T4 = Details::unused_parameter, 
    typename T5 = Details::unused_parameter, 
    typename T6 = Details::unused_parameter
>
class ImplementsInterfaces
    : public Details::ImplementsInterfacesHelper<T1, T2, T3, T4, T5, T6>
{
};