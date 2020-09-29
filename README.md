Daw
===

Daw is yet another amateur WIP game engine.

It is written in C99 and can be compiled and linked to easily in any CMake
project.

It currently has a very limited amount of features, but comes with some
stand-alone utility sources:
* [BTree](src/btree.c), a copy of [my own btree implementation](https://github.com/0undefined/btree).
* [stack](src/stack.c), yet another implementation of a stack.
* [list](include/engine/list.h), just a linked list.
* The `memory` module provides a simple arena allocator.
* The `vector` module just provides some simple 2d vector functions and datatype.
