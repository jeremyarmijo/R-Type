#pragma once

#ifdef _WIN32
    #ifdef NETWORK_EXPORTS
        #define NETWORK_API __declspec(dllexport)
    #else
        #define NETWORK_API __declspec(dllimport)
    #endif
#else
    #define NETWORK_API
#endif
