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


template <class T> class Vector
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

    HRESULT GrowOne()
    {
        UINT newSize = m_Size + m_Size;
        if (newSize < 4) 
        {
            newSize = 4;
        }

        T* n = MemoryAllocator<T>::Allocate( newSize );
        if (n == NULL)
        {
            return E_OUTOFMEMORY;
        }
        for (UINT i = 0; i<m_Count;i++) 
        {
            n[i] = m_List[i];
        };
        MemoryAllocator<T>::Delete( m_List );
        m_List = n;
        m_Size = newSize;
        return S_OK;
    };

    HRESULT Grow(__in UINT requestedSize)
    {
        UINT newSize = m_Size + m_Size;
        if (newSize < 4) 
        {
            newSize = 4;
        }

        while (newSize < requestedSize)
        {
            newSize += newSize;
        }

        T* n = MemoryAllocator<T>::Allocate(newSize);
        if (n == NULL)
        {
            return E_OUTOFMEMORY;
        }
        for (UINT i = 0; i<m_Count;i++) 
        {
            n[i] = m_List[i];
        };

        MemoryAllocator<T>::Delete( m_List );
        m_List = n;
        m_Size = newSize;
        return S_OK;
    };

    UINT m_Size;
    UINT m_Count;
    __field_ecount(m_Size) T* m_List;

public:

    Vector()
    {
        m_Size = 0;
        m_Count = 0;
        m_List = NULL;
    };

    ~Vector()
    {
        Clear();
    };

    inline void Clear()
    {
        T *pBuffer = m_List;
        m_List = NULL;
        m_Count = 0;
        m_Size = 0;
        MemoryAllocator<T>::Delete(pBuffer);
    };

    inline void DetachTo( Vector<T> &target )
    {
        target.Clear();
        target.m_List = m_List;
        target.m_Count = m_Count;
        target.m_Size = m_Size;

        m_Count = 0;
        m_Size = 0;
        m_List = NULL;
    };

    inline void Delete()
    {
        UINT items = m_Count;

        T *pBuffer = m_List;
        m_List = NULL;
        m_Size = 0;
        m_Count = 0;

        for (UINT i = 0; i<items;i++)
        {
            delete pBuffer[i];
        };
        MemoryAllocator<T>::Delete(pBuffer);
    };

    inline HRESULT Add(__in ItemType v)
    {
        if (m_Size == m_Count) 
        {
            HRESULT hr = GrowOne();
            if (FAILED(hr))
                return hr;
        }
        m_List[m_Count++] = v;
        return S_OK;
    };
    
   
    inline HRESULT Add(__in_ecount(itemsCount) T *pV, __in UINT itemsCount)
    {
        if (m_Count + itemsCount > m_Size ) 
        {
            HRESULT hr = Grow(m_Count + itemsCount);
            if (FAILED(hr))
                return hr;
        }
        for (UINT i =0; i<itemsCount; i++)
        {
            m_List[m_Count++] = pV[i];
        }
        return S_OK;
    };
    
    inline HRESULT SetLength(__in UINT newLength)
    {
        if (newLength > m_Size)
            return E_INVALIDARG;

        m_Count = newLength;
        return S_OK;
    };

    inline HRESULT Allocate(__in UINT itemsCount)
    {
        HRESULT hr = S_OK;
        if (m_Count + itemsCount > m_Size ) 
        {
            hr = Grow(m_Count + itemsCount);
        }
        return hr;
    };


    inline T& PopLast() 
    {
        return m_List[--m_Count];
    };
    
    inline void RemoveLast() 
    {
        if (m_Count > 0)
            m_Count--;
    };
    
    inline T& GetLast() 
    {
        return m_List[m_Count - 1];
    };
        
    inline void RemoveAt(__in UINT i)
    {
        __analysis_assume(m_Count <= m_Size);
        if (m_Count > 0)
        {
            m_Count--;
            m_List[i] = m_List[m_Count];
        }
    };

    inline bool Remove(__in ItemType &v)
    {
        for (UINT i = 0; i<m_Count;i++) 
        {
            if (m_List[i] == v)
            {
                m_Count--;

                // Check to prevent reference adition, when T is SmartPtr
                if (i != m_Count)
                {
                    m_List[i] = m_List[m_Count];
                }

                m_List[m_Count] = NULL;
                return true;
            }
        };
        return false;
    }

    inline bool RemoveKeepOrdering(__in ItemType v)
    {
        for (UINT i = 0; i<m_Count;i++) 
        {
            if (m_List[i] == v)
            {
                m_Count--;
                for (;i < m_Count; i++)
                {
                    m_List[i] = m_List[i + 1];
                }
                m_List[m_Count] = NULL;
                return true;
            }
        };
        return false;
    }

    inline UINT GetSize() const
    {
        return m_Size;
    };

    inline HRESULT SetSize(__in UINT newSize)
    {
        if (newSize <= m_Size)
        {
            m_Count = newSize;
            return S_OK;
        }

        UINT newCount = newSize;
        if (newSize < 4) 
        {
            newSize = 4;
        }

        T* n = MemoryAllocator<T>::Allocate(newSize);
        if (n == NULL)
        {
            return E_OUTOFMEMORY;
        }
        for (UINT i = 0; i<m_Count;i++) 
        {
            n[i] = m_List[i];
        };
        MemoryAllocator<T>::Delete(m_List);
        m_List = n;
        m_Size = newSize;
        m_Count = newCount;
        return S_OK;
    };

    inline void Reset()
    {
        m_Count = 0;
    };

    inline UINT Length() const
    {
        return m_Count;
    };

    inline T* Ptr() const
    {
        return m_List;
    };

    inline T& operator [] (__in UINT index) const 
    {
        return m_List[index];
    };

};
