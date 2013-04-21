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

template <class Key, class Value>
struct HashPair
{
    Key key;
    Value value;
    HashPair *next;
};

template <class ValueType> class Hashable
{
public:
    static UINT GetHash( __in const ValueType v )
    {
        return (UINT)v;
    };

    static bool Compare(  __in const ValueType a,  __in const ValueType b )
    {
        return a == b;
    };
};

namespace Details {

template <class Key, class Value, UINT PageBitWidth, UINT OverloadLimit, class KeyHash> class HashTableImpl
{
public:
    typedef HashPair<Key, Value> HashPairType;
    typedef HashTableImpl<Key, Value, PageBitWidth, OverloadLimit, KeyHash> HashTableType;

    class Iterator
    {
    protected:
        HashTableType *m_Owner;
        HashPairType *m_CurrentKey;
        UINT     m_Index;

    public:
        Iterator(HashTableType *p)
        {
            m_Owner = p;
            Reset();
        };

        Iterator(const Iterator &i)
        {
            m_Owner = i.m_Owner;
            m_CurrentKey = i.m_CurrentKey;
            m_Index = i.m_Index;
        }

        Value operator -> ()
        {
            return m_CurrentKey->value;
        };

        HashPairType& Get()
        {
            return *m_CurrentKey;
        };

        bool Eof() const
        {
            return m_CurrentKey == NULL;
        };

        void Reset()
        {
            m_Index = 0;
            m_CurrentKey = NULL;
            while (m_Index < m_Owner->m_BucketsCount)
            {
                m_CurrentKey = m_Owner->m_Buckets[m_Index];
                m_Index++;
                if (m_CurrentKey)
                    break;
            };
        };

        void operator ++(int)
        {
            m_CurrentKey = m_CurrentKey->next;
            while (!m_CurrentKey) 
            {
                if (m_Index == m_Owner->m_BucketsCount)
                    return;

                m_CurrentKey = m_Owner->m_Buckets[m_Index];
                m_Index++;
            };
        };
    };
    
    __field_ecount(m_BucketsCount) HashPairType** m_Buckets;
    PagedVector<HashPairType, PageBitWidth> m_Keys;
    HashPairType* m_FreeKeys;
    UINT m_BucketsCount;
    UINT m_KeysCount;

    HashTableImpl()
    {
        m_FreeKeys = NULL;
        m_BucketsCount = 0;
        m_KeysCount = 0;
        m_Buckets = NULL;
    };

    ~HashTableImpl()
    {
        free(m_Buckets);
    };

    UINT Size() const
    {
        return m_KeysCount;
    };

    HRESULT Insert(const Key &k, Value d)
    {
        HRESULT hr;
        if (m_KeysCount >= m_BucketsCount * OverloadLimit) 
        {
            hr = Rehash(m_BucketsCount * 2);
            if (FAILED(hr))
                return hr;
        };

        //allocate new pair
        HashPairType *p;
        if (m_FreeKeys) 
        {
            p = m_FreeKeys;
            m_FreeKeys = m_FreeKeys->next;
        }
        else 
        {
            hr = m_Keys.GetNew(&p);
            if (FAILED(hr))
                return hr;
        };
        m_KeysCount++;

        UINT h = KeyHash::GetHash( k ) % m_BucketsCount;
        p->key = k;
        p->value = d;
        p->next = m_Buckets[h];
        m_Buckets[h] = p;
        return S_OK;
    };

    bool Erase(const Key& k)
    {
        if (!m_BucketsCount)
            return false;

        UINT h = KeyHash::GetHash( k) % m_BucketsCount;
        HashPairType *prev = NULL;
        for (HashPairType *i = m_Buckets[h]; i; i = i->next)
        {
            if ( KeyHash::Compare( k, i->key ) )
            {
                if (prev) 
                {
                    prev->next = i->next;
                }
                else
                {
                    m_Buckets[h] = i->next;
                }

                i->next = m_FreeKeys;
                m_FreeKeys = i;
                m_KeysCount--;
                return true;
            };
            prev = i;
        };
        return false;
    };

    bool Erase(HashPairType* k)
    {
        if (!m_BucketsCount)
            return false;

        UINT h = KeyHash::GetHash( k->key ) % m_BucketsCount;
        HashPairType *i = m_Buckets[h];
        HashPairType *prev = NULL;
        while (i) 
        {
            if (i == k) 
            {
                if (prev)
                {
                    prev->next = i->next;
                }
                else 
                {
                    m_Buckets[h] = i->next;
                }

                i->next = m_FreeKeys;
                m_FreeKeys = i;
                m_KeysCount--;
                return true;
            };
            prev = i;
            i = i->next;
        };
        return false;
    };

    __success(return == true) 
    bool Toggle(const Key &k, __out Value *pValue)
    {
        if (!m_BucketsCount)
            return false;

        UINT h = KeyHash::GetHash( k ) % m_BucketsCount;
        HashPairType *i = m_Buckets[h];
        HashPairType *prev = NULL;
        while (i) 
        {
            if ( KeyHash::Compare( i->key, k) )
            {
                *pValue = i->value;

                if (prev)
                {
                    prev->next = i->next;
                }
                else 
                {
                    m_Buckets[h] = i->next;
                }
                i->next = m_FreeKeys;
                m_FreeKeys = i;
                m_KeysCount--;
                return true;
            };
            prev = i;
            i = i->next;
        };
        return false;
    };

    Iterator GetIterator()
    {
        return Iterator(this);
    };

    bool Contains(const Key &k)
    {
        if (!m_BucketsCount)
            return false;

        UINT h = KeyHash::GetHash( k ) % m_BucketsCount;
        for (HashPairType *i = m_Buckets[h]; i; i = i->next)
        {
            if ( KeyHash::Compare( i->key, k ) )
            {
                return true;
            }
        };
        return false;
    };

    HRESULT EnsureSize(UINT requestedSize)
    {
        if (m_KeysSize < requestedSize)
        {
            UINT new_size = m_KeysSize;
            if (new_size == 0) 
                new_size = 4;

            while (new_size < requestedSize) 
            {
                new_size += new_size;
            };
            return Rehash(new_size);
        }
        else 
        {
            return S_OK;
        }
    };

    HashPairType* operator[] (const Key& k) const
    {
        if (!m_BucketsCount)
            return NULL;

        UINT h = KeyHash::GetHash( k ) % m_BucketsCount;
        for (HashPairType *i = m_Buckets[h]; i; i = i->next)
        {
            if ( KeyHash::Compare( i->key, k) )
            {
                return i;
            }
        };
        return NULL;
    };

    Key* FindKey(const Value& v) const
    {
        for (UINT h = 0; h < m_BucketsCount; h++)
        {
            for (HashPairType *i = m_Buckets[h];i; i = i->next)
            {
                if (i->value == v)
                {
                    return &i->key;
                }
            };
        }
        return NULL;
    };
    
    void Reset()
    {
        ZeroMemory(m_Buckets, sizeof(HashPairType*) * m_BucketsCount);
        m_KeysCount = 0;
        m_FreeKeys = NULL;
        m_Keys.Reset();
    };

    void Release()
    {
        free(m_Buckets);
        m_BucketsCount = 0;
        m_Buckets = NULL;
        m_KeysCount = 0;
        m_FreeKeys = NULL;
    };

protected:

    HRESULT Rehash(UINT newSize)
    {
        if (newSize == 0) 
            newSize = 4;

        HashPairType** newBuckets = (HashPairType**)malloc(sizeof(HashPairType*) * newSize);
        if (!newBuckets)
        {
            return E_OUTOFMEMORY;
        }

        ZeroMemory(newBuckets, sizeof(HashPairType*) * newSize);

        UINT i;
        for (i = 0; i<m_BucketsCount; i++)
        {
            HashPairType *j = m_Buckets[i]; 
            while (j)
            {
                HashPairType *n = j->next;
                UINT h = KeyHash::GetHash( j->key ) % newSize;

                j->next = newBuckets[h];
                newBuckets[h] = j;
                j = n;
            };
        };

        free(m_Buckets);
        m_Buckets = newBuckets;
        m_BucketsCount = newSize;
        return S_OK;
    };
};

};

template <class Key, class Value, UINT PageBitWidth = 10, UINT OverloadLimit = 4>
class HashTable;

template <class Key, class Value, UINT PageBitWidth, UINT OverloadLimit>
class HashTable<Key, Value, PageBitWidth, OverloadLimit> : public Details::HashTableImpl< Key, Value, PageBitWidth, OverloadLimit, Hashable<Key> >
{
};









