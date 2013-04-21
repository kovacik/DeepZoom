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

enum ExitCode
{
    Success = 0,
    InvalidLogin = 1,
    InvalidFilename = 2,
    UnknownError = 10
};

#define IF_FAILED_RETURN( hr ) \
    if( FAILED( hr ) ) \
    return hr;

#define IF_FAILED_BREAK( hr ) \
    if( FAILED( hr ) ) \
    break;

#define IF_FAILED_RETURN_INT( hr, retValue ) \
    if( FAILED( hr ) ) \
    return retValue;