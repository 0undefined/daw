Daw
===

Daw is yet another hobby WIP game engine.

It is written in C99 and can be compiled and linked to easily in any CMake
project.

Currently it supports a very limited set of features:

* IO handling using input layers
* Window creation & 2D rendering (using SDL2 at the moment)
* Hot reloading
* Resource handling
* Memory handling
* UI (the input types are limited to buttons only at the moment)
* Logging

As well as a limited set of 2D grid-based game utilities:
* Pathfinding
* FOV calculations using shadowcasting

The engine is build around states. This means the developer needs to provide
initialize, update, and cleanup functions for each game state (states such as
"title screen", "game running", and such).
The whole project is made to be easily managed using CMake (yes, I too wonder
why sometimes).

Currently the only "supported" platform is linux. I do plan on adding windows
support in the future.

I plan on documenting how to use the engine as soon as I stop modifying the API,
and plan out a more solid organizational strategy for the project as a whole.

See [this CMakeLists.txt][0] to get an idea of how to use this project.


## Planned features

This is an incomplete list of features that are either under development or
planned to implement.

* Audio
* GLFW & Vulkan support
* Better cross platform support (ie. windows)

[0]: https://github.com/0undefined/rogue/blob/a947b0092d91920d56eb9af6a39bd1bd05e145fc/CMakeLists.txt
