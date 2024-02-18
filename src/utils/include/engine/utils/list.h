#ifndef ENGINE_LIST_H
#define ENGINE_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEFINE_LLIST(type)                                                     \
  struct List_##type {                                                         \
    type value;                                                                \
    struct List_##type* next;                                                  \
    /* Force the user to add `;` for style consistency */                      \
  } List_##type

#ifdef __cplusplus
}
#endif
#endif
