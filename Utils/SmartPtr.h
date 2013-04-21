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

template <class T>
class _HideAddrefReleaseSmartObject : public T
{
private:
    _HideAddrefReleaseSmartObject()
    {
    }

    STDMETHOD_(ULONG, AddRef)() = 0;

    STDMETHOD_(ULONG, Release)() = 0;
};

template <class T>
class SmartPtr
{
    
public:

    T *p;

    SmartPtr()
    {
        p = NULL;
    }

    SmartPtr(__in_opt T *pObj)
    {
        p = pObj;
        if (p)
        {
            Details::GetUnknown<T>(p)->AddRef();
        }
    }
    
    SmartPtr(__in const SmartPtr<T> &spObj)
    {
        p = spObj.p;
        if (p)
        {
            Details::GetUnknown<T>(p)->AddRef();
        }

    }

    ~SmartPtr()
    {
        Release();
    }
        
    bool operator!() const 
    {   
        return (p == NULL);
    }

    bool operator!=(__in_opt T* pT) const
    {
        return p != pT;
    }

    bool operator==(__in_opt T* pT) const
    {
        return p == pT;
    }

    T* operator = (__in const SmartPtr<T> &lp)
    {   
        _ASSERT(p != lp.p);

        if (lp.p != NULL)
        {
            Details::GetUnknown<T>(lp.p)->AddRef();
        }

        Release();
        p = lp.p;
        return lp.p;
    }

    T* operator=(__in_opt T* lp)
    {   
        if (lp != NULL)
            Details::GetUnknown<T>(lp)->AddRef();

        Release();
        p = lp;
        return p;       
    }

    operator T*() const 
    {
        return p;
    }

    _HideAddrefReleaseSmartObject<T>* operator->() const 
    {
        return static_cast< _HideAddrefReleaseSmartObject<T>* >(p);
    }
        
    T** operator&()
    {
        _ASSERT(p==NULL);
        return &p;
    }

    void Release()
    {
        T* pTemp = p;
        if (pTemp)
        {
            p = NULL;
            Details::GetUnknown<T>(pTemp)->Release();
        }
    }

    void Attach(__in_opt T* p2)
    {
        Release();
        p = p2;
    }

    T* Detach()
    {
        T* pt = p;
        p = NULL;
        return pt;
    }


    template <class Interface> Interface* CastTo() const
    {
        Interface *pResult = NULL;
        if (p)
        {
            (void)Details::GetUnknown<T>(p)->QueryInterface(DEF_GUIDNAME(Interface), (void**)&pResult);

            if (pResult)
            {
                pResult->Release();
            }
        }

        return pResult;
    }


    template <class Interface> HRESULT As(__deref_out Interface** ppT) const
    {
        if (ppT == NULL)
            return E_POINTER;

        T* temp = p;
        if (!temp)
        {
            return E_FAIL;
        };
        return Details::GetUnknown<T>(temp)->QueryInterface(DEF_GUIDNAME(Interface), (void**)ppT);
    }

    template <class Interface> HRESULT CopyTo(__deref_out Interface** ppT) const
    {
        if (ppT == NULL)
            return E_POINTER;
        
        Interface *temp = p;
        if (!temp)
        {
            return E_FAIL;
        };
        temp->AddRef();
        *ppT = temp;
        return S_OK;
    }
    
    template <class Interface>  HRESULT CopyToOpt(__deref_out_opt Interface** ppT) const
    {
        if (ppT == NULL)
            return E_POINTER;

        Interface *temp = p;
        if (temp)
        {
            temp->AddRef();
        }
        *ppT = temp;
        return S_OK;
    }

    template <class Interface> HRESULT DetachTo(__deref_out Interface** ppT)
    {
        if (ppT == NULL)
            return E_POINTER;
        if (!p)
            return E_FAIL;

        *ppT = p;
        p = NULL;
        return S_OK;
    } 
};


