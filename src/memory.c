#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <engine/logging.h>

#include <engine/memory.h>

memory *memory_new(usize max_size) {
  memory *m = malloc(sizeof(memory));
  m->data   = malloc(max_size);
  m->size   = max_size;
  m->pos    = 0;
  m->free   = max_size;

  memset(m->data, 0, max_size);

  return m;
}

/* Returns a pointer to the allocated data */
void *memory_allocate(memory* mem, usize size) {
  void* data = NULL;

  if (mem->pos + size <= mem->size) {
    data = (void*)((usize)mem->data + mem->pos);
    mem->pos  += size;
    mem->free -= size;
  } else {
    ERROR("Trying to allocate %lu in a %lu sized memory block", size, mem->size);
    ERROR("No more room!");
    exit(EXIT_FAILURE);
  }

  return data;
}

memory memory_init(void *data, usize size) {
  memory m = {0};
  m.data = data;
  m.size = size;
  m.free = 0;
  return m;
}

void memory_free(memory *mem, usize size) {
  if (size > mem->pos) {
    perror("Freeing too much memory!");
    exit(EXIT_FAILURE);
  } else {
    mem->pos  -= size;
    mem->free += size;
  }
}

void memory_clear(memory *mem) {
	mem->pos  = 0;
	mem->free = mem->size;
	/* Reset the memory? */
	memset(mem->data, 0, mem->size);
}
