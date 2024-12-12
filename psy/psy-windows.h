
#pragma once

#include "psy-config.h"

#if defined _WINDOWS_
    #error "windows.h included before psy-windows.h"
#endif

#if defined(HAVE_WINDOWS_H)

    // Exclude a lot of rarely used windows header
    #define WIN32_LEAN_AND_MEAN

    // Create stricter Handles e.g. functions take either HWND or HDC,
    // Handle (pun intended) errors at compile instead of runtime
    #define STRICT

    // avoid overriding std::min, std::max
    #define NOMINMAX

    // include windows without everything
    #include <windows.h>

#endif
