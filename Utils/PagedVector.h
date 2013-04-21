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

template <class T, int PageBitWidth> class PagedVector
{
    template <class T, bool isAlignedOrBig>
    class TypeResolver;

    template <class T>
    class TypeResolver<T, true>
    {
    public:
        typedef const T& ItemType;
    };

    template <class T>
    class TypeResolver<T, false>
    {
    public:
        typedef T ItemType;
    };

    typedef typename TypeResolver<T, (sizeof(T) >= 2 * sizeof(void*)) || (__alignof(T) >= 2*sizeof(void*)) >::ItemType ItemType;


    static const UINT PageSize = (1 << PageBitWidth);
    static const UINT PageMask = PageSize - 1;

    HRESULT GrowOne()
    {
        UINT pagesCount = m_Size >> PageBitWidth;
        T** n = (T**)malloc((pagesCount + 1) * sizeof(T*));
        if (n == NULL)
        {
            return E_OUTOFMEMORY;
        }
        
        T* lastPage = MemoryAllocator<T>::Allocate( PageSize );
        if (lastPage == NULL)
        {
            free(n);
            return E_OUTOFMEMORY;
        }

        for (UINT i = 0; i<pagesCount;i++) 
        {
            n[i] = m_Pages[i];
        };
        
        n[pagesCount] = lastPage;

        free(m_Pages);
        m_Pages = n;
        m_Size += PageSize;
        return S_OK;
    };

    UINT m_Count;
    UINT m_Size;
    __field_ecount(m_Size >> PageBitWidth) T** m_Pages;
public:

    PagedVector()
    {
        m_Count = 0;
        m_Size = 0;
        m_Pages = NULL;
    };

    ~PagedVector()
    {
        Clear();
    };

    inline void Clear()
    {
        UINT pagesCount = m_Size >> PageBitWidth;

        //first detach, then delete
        T** pages = m_Pages;
        m_Pages = NULL;
        m_Size = 0;
        m_Count = 0;

        for (UINT i = pagesCount; i-->0;)
        {
            MemoryAllocator<T>::Delete( pages[i] );
        };
        free(pages);
    };
        
    template <class V>
    inline HRESULT GetNew(__deref_out V* pV)
    {
        if (m_Size == m_Count) 
        {
            HRESULT hr = GrowOne();
            if (FAILED(hr))
                return hr;
        }
        UINT itemIndex = m_Count++;
        *pV = &(m_Pages[itemIndex >> PageBitWidth][itemIndex & PageMask]);
        return S_OK;
    };

    inline HRESULT Add(__in ItemType v)
    {
        if (m_Size == m_Count) 
        {
            HRESULT hr = GrowOne();
            if (FAILED(hr))
                return hr;
        }
        UINT itemIndex = m_Count++;
        m_Pages[itemIndex >> PageBitWidth][itemIndex & PageMask] = v;
        return S_OK;
    };


    inline T& PopLast() 
    {
        UINT itemIndex = --m_Count;
        return m_Pages[itemIndex >> PageBitWidth][itemIndex & PageMask];
    };
    
    inline void RemoveLast() 
    {
        if (m_Count > 0)
            m_Count--;
    };
   
    inline T& GetLast() 
    {
        UINT itemIndex = m_Count - 1;
        return m_Pages[itemIndex >> PageBitWidth][itemIndex & PageMask];
    };
        
    inline void RemoveAt(__in UINT i)
    {
        __analysis_assume(m_Count <= m_Size);
        if (m_Count > 0)
        {
            UINT itemIndex = --m_Count;
            m_Pages[i >> PageBitWidth][i & PageMask] = m_Pages[itemIndex >> PageBitWidth][itemIndex & PageMask];
        }
    };

    inline UINT GetSize() const
    {
        return m_Size;
    };

    inline void Reset()
    {
        m_Count = 0;
    };

    inline UINT Length() const
    {
        return m_Count;
    };

    inline T& operator [] (__in UINT index) const 
    {
        return m_Pages[index >> PageBitWidth][index & PageMask];
    };
};
