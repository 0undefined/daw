#ifndef THREAD_H
#define THREAD_H

#include <engine/core/types.h>

typedef u64 thread_t;
typedef void*(thread_fn(void*));

thread_t thread_spawn(thread_fn* routine, void* arg);
void thread_join(thread_t thread);

#endif
