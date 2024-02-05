#include <stdlib.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
/* include winapi */
#else
#include <dlfcn.h>
#endif

#include <engine/dltools.h>
#include <engine/logging.h>

bool dynamic_library_close(void* shared_library) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
  return true;
#else
  return dlclose(shared_library) == 0;
#endif
}

void* dynamic_library_open(const char* library_path) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
  return NULL;
#else
  return dlopen(library_path, RTLD_NOW);
#endif
}

char* dynamic_library_get_error(void) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
  return "unsupported on windows";
#else
  return dlerror();
#endif
}

void* dynamic_library_reload(void* shared_library, const char* library_path) {
  void* library_address = NULL;
  if (!dynamic_library_close(shared_library)) {
    ERROR("Failed to close shared library: %s", dynamic_library_get_error());
    ERROR("Reloading dynamic library failed.");
    return library_address;
  }
  if ((library_address = dynamic_library_open(library_path)) == NULL) {
    ERROR("Failed to open shared library: %s", dynamic_library_get_error());
    ERROR("Reloading dynamic library %s failed.", library_path);
  }
  return library_address;
}

void* dynamic_library_get_symbol(void* restrict shared_library,
                                 const char* symbol) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
  return NULL;
#else
  return dlsym(shared_library, symbol);
#endif
}
