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

namespace Details
{

    template <class T, bool useDefaultAllocation>
    class MemoryAllocatorImpl;


    template <class T>
    class MemoryAllocatorImpl<T, true>
    {
    
    public:
        static T* Allocate(UINT size)
        {
            return new T[size];
        };

        static void Delete(T* buffer)
        {
            delete[] buffer;
        }
    };

    template <class T>
    class MemoryAllocatorImpl<T, false>
    {
        struct AllocationHelper
        {
            T v;

#pragma push_macro("new")
#pragma push_macro("delete")

#undef new
#undef delete

            void * operator new (size_t size)
            {
                return _aligned_malloc(size, __alignof(T));
            }
            void * operator new (size_t size, const std::nothrow_t &)
            {
                return _aligned_malloc(size, __alignof(T));
            }
            void * operator new[] (size_t size)
            {
                return _aligned_malloc(size, __alignof(T));
            }
            void * operator new[] (size_t size, const std::nothrow_t &)
            {
                return _aligned_malloc(size, __alignof(T));
            }
            void operator delete (void* ptr)
            {
                _aligned_free(ptr);
            }
            void operator delete (void* ptr, const std::nothrow_t &)
            {
                _aligned_free(ptr);
            }
            void operator delete[] (void* ptr)
            {
                _aligned_free(ptr);
            }
            void operator delete[] (void* ptr, const std::nothrow_t &)
            {
                _aligned_free(ptr);
            }

#pragma pop_macro("delete")
#pragma pop_macro("new")
        };

    public:
        static T* Allocate(UINT size)
        {
            return (T*)(new AllocationHelper[size]);
        };

        static void Delete(T* buffer)
        {
            delete[] (AllocationHelper*)(buffer);
        }
    };
};

template <class T>
class MemoryAllocator : public Details::MemoryAllocatorImpl<T, __alignof(T) <= 2*sizeof(size_t) >
{
};
