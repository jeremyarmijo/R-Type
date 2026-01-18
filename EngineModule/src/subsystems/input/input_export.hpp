#pragma once

#ifdef _WIN32
    #ifdef INPUT_EXPORTS
        #define INPUT_API __declspec(dllexport)
    #else
        #define INPUT_API __declspec(dllimport)
    #endif
#else
    #define INPUT_API
#endif
