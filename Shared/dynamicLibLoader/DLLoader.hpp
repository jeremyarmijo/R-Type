#ifndef SHARED_DYNAMICLIBLOADER_DLLOADER_HPP_
#define SHARED_DYNAMICLIBLOADER_DLLOADER_HPP_

#include <dlfcn.h>
#include <string>

template <typename T>
class DLLoader {
 public:
  DLLoader(const std::string &className, const std::string &symbole_name) {
    handle = dlopen(className.c_str(), RTLD_LAZY);
    if (!handle) {
      throw std::runtime_error(dlerror());
    }
    T *(*entry_point)() = (T * (*)()) dlsym(handle, symbole_name.c_str());
    if (!entry_point) {
      dlclose(handle);
      throw std::runtime_error(dlerror());
    }
    instance = entry_point();
  }
  ~DLLoader() {
    if (handle) {
      dlclose(handle);
    }
  }
  T *getInstance() { return instance; }

 private:
  T *instance = nullptr;
  void *handle = nullptr;
};

#endif  // SHARED_DYNAMICLIBLOADER_DLLOADER_HPP_
