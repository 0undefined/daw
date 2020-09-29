#ifndef ENGINE_HASHMAP_H
#define ENGINE_HASHMAP_H

#include "types.h"

#include <stdlib.h>
#include "list.h"
#include "memory.h"


i32 lolhash(const usize s, i32 v);

/* Define a linked list before using this */
/* Example: DEFINE_LLIST(i32) */
#define DEFINE_HASHMAP(type, lsize, cmp, type_to_int) \
typedef DEFINE_LLIST(type);                           \
typedef struct hashmap_##type {                       \
  usize size;                                        \
  List_##type elems[64];                              \
} hashmap_##type;                                     \
                                                      \
type* hashmap_##type##_lookup(hashmap_##type* hmap, const type* val) { \
  const i32 idx = lolhash(64, type_to_int(val));                 \
  List_##type *head = &hmap->elems[idx];                         \
  while (head != NULL) {                                         \
    if (!cmp(&(head->value), val)) return &(head->value);        \
    head = head->next;                                           \
  }                                                              \
  return NULL;                                                   \
}                                                                \
                                                                 \
void hashmap_##type##_insert(memory *m, hashmap_##type *hmap, const type *val) { \
  const i32 idx = lolhash(64, type_to_int(val));                           \
  List_##type *head = &(hmap->elems[idx]);                                 \
                                                                           \
  /* This is highly dependant on whether the memory is zero-initialized */ \
  if (!type_to_int(&(head->value)))                                        \
  {                                                                        \
    memcpy(&(head->value), val, sizeof(type));                             \
    return;                                                                \
  }                                                                        \
                                                                           \
                                                                           \
  while (head->next != NULL && cmp(&head->value, val)) {                   \
      head = head->next;                                                   \
  }                                                                        \
                                                                           \
  if (!cmp(&head->value, val)) memcpy(&(head->value), val, sizeof(type));  \
  else {                                                                   \
    head->next = memory_allocate(m, sizeof(List_##type));                  \
    memcpy(&(head->next->value), val, sizeof(type));                       \
  }                                                                        \
}

#endif
