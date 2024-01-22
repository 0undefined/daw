Project & game architecture
---------------------------

The different source files in the project is divided into several source files
within the `src` directory of this repository with a (somewhat) corresponding
name.

## State Management [state.c](../src/state.c)

![gamestate illustration](gamestate.svg)
![gameloop illustration](gameloop.svg)

WIP

```
while((newstate = state_update(current_state)) != STATE_Exit) {
  if (newstate != STATE_NULL) {
    state = newstate;
    state_free(current_state);
    current_state = state_load(newstate);
  } else {
    // Input stuff
    // Update stuff
    // Render stuff
    // Time stuff
  }
}
```


## Input Handling [keypress.h](../src/keypress.h)

We use a null-terminated statically allocated list of structs containing the
following:
* `struct Key`: Takes a `uint16 modifier` and a `uint32 key`
  ([SDL Keycode](https://wiki.libsdl.org/SDLKeycodeLookup))
* `uint32 time_pressed`: Time in ms since it was pressed.
* `void (*down_function)(void*)`: The function to be run upon key activation.
* `struct Funarg down_arg`: The argument for the function to be called.  The argument is
  automatically added a pointer to the current (game) context for each call.
* `void (*up_function)(void*)`: The function to be run upon key deactivation.
* `struct Funarg up_arg`: The argument for the function to be called.

Upon keypress,
```c
for (int i = 0; i < len(keys) && keys[i] != NULL; i++) {
  if ( keys[i].function_down != NULL
    && keys[i].key.key == pressed_key
    && keys[i].key.modifier & current_mods == current_mods) {

      time_pressed = now();
      keys[i].function_down(funarg_up);

    }
}
```
And basically the same goes for keyup, except, we dont check any modifier keys,
instead we check if `time_pressed > 0`

By using `time_pressed`, we can "charge" moves if needed be, and once released
we can measure how long the player held down the key.
It can be visualized by giving a pointer to a UI-part and setting it to visible,
and toggling said visibility again once the key is released.

## Rendering

### Tile rendering

When rendering tiles, we go by drawing walls on the floor tile, such that for
each floor tile we calculate the number of adjacent walls and fetch the
corresponding texture that match that combination of neighbouring walls.
