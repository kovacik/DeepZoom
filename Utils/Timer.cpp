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

#include "stdafx.h"
#include "Timer.h"

#define OUTPUT OutputW

void OutputW(LPCWSTR szFormat, ...)
{
    WCHAR szBuff[1024];
    va_list args;
    va_start(args, szFormat);
    vswprintf_s(szBuff, _countof(szBuff), szFormat, args);
    va_end(args);

    OutputDebugStringW(szBuff);
}

Timer::Timer()
{
    m_StartTicks.QuadPart = 0;
    m_Frequency.QuadPart = 0;    
}

BOOL Timer::Start(LPCWSTR szTimerName)
{
    BOOL ok = QueryPerformanceFrequency(&m_Frequency);
    if (ok)
    {
        m_StartTicks.QuadPart = 0;
        m_Text = szTimerName;

        ok = QueryPerformanceCounter(&m_StartTicks);
    }

    return ok;
}

BOOL Timer::Stop()
{
    LARGE_INTEGER endTicks;
    BOOL ok = QueryPerformanceCounter(&endTicks);
    if (ok)
    {
        LARGE_INTEGER ticksDifference;

        ticksDifference.QuadPart = endTicks.QuadPart - m_StartTicks.QuadPart;
        DOUBLE sec = (DOUBLE)ticksDifference.QuadPart / (DOUBLE)m_Frequency.QuadPart;

        OUTPUT(L"\n\n-------------------- ");
        OUTPUT(m_Text);
        OUTPUT(L" TIMER START -------------------------\n\n");             
        OUTPUT(L"     Duration: %f s (%f ms)\n", sec, 1000 * sec);
        OUTPUT(L"     Ticks   : %lld (frequency: %lld ticks/sec)\n\n", ticksDifference.QuadPart, m_Frequency.QuadPart);
        OUTPUT(L"-------------------- ");
        OUTPUT(m_Text);
        OUTPUT(L" TIMER END ---------------------------\n\n");
    }

    return ok;
}