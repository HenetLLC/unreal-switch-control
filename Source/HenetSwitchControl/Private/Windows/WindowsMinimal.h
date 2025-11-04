// Copyright Henet LLC 2025
// Minimal Windows include to avoid polluting the global namespace

#pragma once

#if PLATFORM_WINDOWS && HENET_WINDOWS_SERIAL

// We need to disable the new min/max macros to avoid conflicts with UE
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Only include necessary headers
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

#endif // PLATFORM_WINDOWS

