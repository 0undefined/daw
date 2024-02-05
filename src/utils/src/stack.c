#include <engine/logging.h>
#include <engine/stack.h>
#include <stdlib.h>

Stack stack_new_ex(const usize element_size, const usize size) {
  Stack s = {
      .head = 0,
      .elem_size = element_size,
      .size = element_size * size,
      .chunk_size = element_size * size,
      .data = NULL,
  };

  s.data = (void*)calloc(element_size, size);
  return s;
}

Stack stack_new(const usize element_size) {
  return stack_new_ex(element_size, 512);
}

void stack_free(Stack* s) {
  if (s->data == NULL) return;
  free(s->data);
  s->data = NULL;
}

void* stack_pop(Stack* s) {
  if (s->head == 0) return NULL; /* Empty stack */
  return (u8*)s->data + (--(s->head) * s->elem_size);
}

void stack_push(Stack* s, void* elem) {
  if (elem == NULL) {
    WARN("%s received a nullptr", __func__);
    return;
  }
  if (s->head > 0 && s->head * s->elem_size >= s->size) {
    WARN("Allocating more stack memory");
    /* Reallocate more memory and update size */
    void* ptr = realloc(s->data, s->size + s->chunk_size);
    if (ptr == NULL) {
      ERROR("Failed to resize memory for stack");
      exit(EXIT_FAILURE);
    }
    s->data = ptr;
    // memset((void*)((u64)s->data + (s->size - s->elem_size)), 0,
    // s->chunk_size);
    s->size += s->chunk_size;
  }
  memcpy((u8*)s->data + s->head * s->elem_size, elem, s->elem_size);
  s->head++;
}

void* stack_peek(Stack* s) {
  if (s->head <= 0) return NULL; /* Empty stack */
  return (u8*)s->data + ((s->head - 1) * s->elem_size);
}

isize stack_size(const Stack* s) { return s->head; }

void stack_swap(Stack* s, Stack* t) {
  if (s->size > t->size) {
    t->data = realloc(t->data, s->size);
  } else if (t->size > s->size) {
    s->data = realloc(s->data, t->size);
  }
  void* tmp = malloc(s->size);
  if (tmp == NULL) {
    ERROR("Failed to allocate memory for stack swapping!");
    exit(EXIT_FAILURE);
  }
  isize shead = s->head;

  memcpy(tmp, s->data, s->size);
  memcpy(s->data, t->data, t->size);
  memcpy(t->data, tmp, s->size);

  s->head = t->head;
  t->head = shead;
  free(tmp);
}
