/* Copyright © 2021 Upqwerk
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * */

/* Stack implementation
 * Author: Upqwerk
 */
#ifndef STACK_H
#define STACK_H

#include "types.h"

typedef struct {
  isize       head;       /* current number of elements */
  const usize elem_size;  /* size in bytes of each element */
  usize       size;       /* current memory size used by the stack */
  const usize chunk_size; /* size of which the stack increases when running out of mem */
  void        *data;       /* memory buffer */
} Stack;

Stack stack_new_ex(const usize element_size, const usize size);

Stack stack_new(const usize element_size);

void stack_free(Stack *s);
void *stack_pop(Stack *s);
void stack_push(Stack *s, void *elem);
void *stack_peek(Stack *s);
isize stack_size(const Stack *s);
void stack_swap(Stack *s, Stack *t);

#endif
