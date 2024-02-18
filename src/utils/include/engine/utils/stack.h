#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <engine/core/types.h>

typedef struct {
  isize head;             /* current number of elements */
  const usize elem_size;  /* size in bytes of each element */
  usize size;             /* current memory size used by the stack */
  const usize chunk_size; /* size of which the stack increases when running out
                             of mem */
  void* data;             /* memory buffer */
} Stack;

Stack stack_new_ex(const usize element_size, const usize size);

Stack stack_new(const usize element_size);

void stack_free(Stack* s);
void* stack_pop(Stack* s);
void stack_push(Stack* s, void* elem);
void* stack_peek(Stack* s);
isize stack_size(const Stack* s);
void stack_swap(Stack* s, Stack* t);

#ifdef __cplusplus
}
#endif
#endif
