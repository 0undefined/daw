#ifndef MEMORY_H
#define MEMORY_H

#include <engine/core/types.h>

typedef struct memory {
  void* data;
  usize size;
  usize pos;
  usize free;
} memory;

memory* memory_new(usize max_size);

/* Returns a pointer to the allocated data */
void* memory_allocate(memory* mem, usize size);

memory memory_init(void* data, usize size);

void memory_free(memory* mem, usize size);

void memory_clear(memory* mem);

#endif
