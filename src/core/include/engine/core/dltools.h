#ifndef DLTOOLS_H
#define DLTOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/* Utility functions for handling runtime linked shared libraries */
bool dynamic_library_close(void* shared_library);
void* dynamic_library_open(const char* library_path);
void* dynamic_library_reload(void* shared_library, const char* library_path);

/* Returns the address of symbol in the provided shared_library handle.
 * NULL on error*/
void* dynamic_library_get_symbol(void* shared_library, const char* symbol);
char* dynamic_library_get_error(void);

#ifdef __cplusplus
}
#endif
#endif
