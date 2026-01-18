#pragma once

#include <stdexcept>
#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
#else
#include <dlfcn.h>
#endif

template <typename T>
class DLLoader {
 public:
  DLLoader(const std::string& libraryPath, const std::string& symbolName) {
#ifdef _WIN32
    handle = LoadLibraryA(libraryPath.c_str());
    if (!handle) {
      throw std::runtime_error("LoadLibrary failed");
    }

    auto entry_point =
        reinterpret_cast<T* (*)()>(GetProcAddress(handle, symbolName.c_str()));

    if (!entry_point) {
      FreeLibrary(handle);
      throw std::runtime_error("GetProcAddress failed");
    }
#else
    handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle) {
      throw std::runtime_error(dlerror());
    }

    auto entry_point =
        reinterpret_cast<T* (*)()>(dlsym(handle, symbolName.c_str()));

    if (!entry_point) {
      dlclose(handle);
      throw std::runtime_error(dlerror());
    }
#endif

    instance = entry_point();
  }

  ~DLLoader() {
#ifdef _WIN32
    if (handle) FreeLibrary(handle);
#else
    if (handle) dlclose(handle);
#endif
  }

  T* getInstance() const { return instance; }

 private:
  T* instance = nullptr;

#ifdef _WIN32
  HMODULE handle = nullptr;
#else
  void* handle = nullptr;
#endif
};
