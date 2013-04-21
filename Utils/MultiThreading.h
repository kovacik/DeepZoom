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


class CriticalSection
{
    bool m_Initialized;
    CRITICAL_SECTION m_Obj;

public:
    CriticalSection()
    {
        m_Initialized = false;
    }

    ~CriticalSection()
    {
        if (m_Initialized)
        {
            DeleteCriticalSection(&m_Obj);
        }
    }

    HRESULT Initialize()
    {       
        if (m_Initialized)
        {
            return E_UNEXPECTED;
        }

        BOOL succeeded = InitializeCriticalSectionEx(&m_Obj, 4000, 0);
        m_Initialized = !!succeeded;
        return succeeded ? S_OK : E_OUTOFMEMORY;
    }

    void Lock()
    {
        EnterCriticalSection(&m_Obj);
    }

    void UnLock()
    {
        LeaveCriticalSection(&m_Obj);
    }

};


class AutoCriticalSection
{
    CriticalSection *pObj;

public:

    AutoCriticalSection(CriticalSection &pCs)
    {
        pObj = &pCs;
        pObj->Lock();
    }

    ~AutoCriticalSection()
    {
        pObj->UnLock();
    }
};

template <class Base>
class AsyncOperation
{
    struct Empty
    {
    };

    template <class T1 = Empty, class T2 = Empty, class T3 = Empty, class T4 = Empty, class T5 = Empty, class T6 = Empty, class T7 = Empty>
    struct ThreadParamBlock
    {
        SmartPtr<Base> ptr;
        T1 t1;
        T2 t2;
        T3 t3;
        T4 t4;
        T5 t5;
        T6 t6;
        T7 t7;
    };


    HANDLE m_ThreadHandle;

    static DWORD WINAPI ThreadProc(_In_  LPVOID lpParameter)
    {
        ThreadParamBlock<> *tpb = (ThreadParamBlock<>*)lpParameter;
        tpb->ptr->Invoke();
        delete tpb;
        return 0;
    };

public:

    AsyncOperation()
    {
        m_ThreadHandle = NULL;
    }

    ~AsyncOperation()
    {
        if (m_ThreadHandle)
        {
            CloseHandle(m_ThreadHandle);
        }
    }

    HRESULT Start(__in Base *owner)
    {
        HRESULT hr;
        ThreadParamBlock<> *tpb = new ThreadParamBlock<>();
        if (tpb == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            tpb->ptr = owner;
            m_ThreadHandle = CreateThread(NULL, 8192, &AsyncOperation<Base>::ThreadProc, tpb, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
            if (m_ThreadHandle == NULL)
            {
                delete tpb;
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
            {
                hr = S_OK;
            }
        }
        
        return 0;
    }

    HRESULT WaitForFinish()
    {
        DWORD s = WaitForSingleObjectEx(m_ThreadHandle, INFINITE, FALSE);
        return s == WAIT_FAILED ? E_FAIL : S_OK;
    }
};