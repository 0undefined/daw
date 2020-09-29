#ifndef ENGINE_LIST_H
#define ENGINE_LIST_H

#define DEFINE_LLIST(type)    \
struct List_##type {          \
  type value;                 \
  struct List_##type* next;   \
  /* Force the user to add `;` for style consistency */\
} List_##type

#endif
