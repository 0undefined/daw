# Input handling

_This document serves as a low-level explanation of how the input system works_

```C
typedef enum InputType InputType;
enum InputType {
  // fire once, ie. keydown. stuff like spells, semi-automatic
  InputType_action,
  // active while pressed, deactivate once released. Ignore key-repeat. stuff like running
  InputType_state,
  // ranges of motion, like mouse input or joystick, [0;1] or [-1;1]
  InputType_range, /* TBD */
};

typedef union action_t action_t;
union action_t {
  InputType type;

  struct {
    InputType type;
    input_callback_t* callback;
    const char* callback_str;
  } action;

  struct {
    InputType type;
    input_callback_t* activate;
    input_callback_t* deactivate;
    const char* activate_str;
    const char* deactivate_str;
  } state;

  /* Add range at some point */
};

typedef struct binding_t {
  action_t action;

  scancode_t scancode;
  scancode_t scancode_alt;

  u64 since_last_activation;
} binding_t;

/* Input context */
typedef struct i_ctx {
	binding_t* bindings;
	usize len;
} i_ctx;
```

The input context `i_ctx` simply holds a pointer to the start of an array of
`binding_t`. This array is traversed linearly whenever the window receives
a input event, for each event.

Since it is your (the individual states) task to clean up any allocated memory,
it would be a good idea to use a statically sized array for the bindings, inside
your states struct for the specific states keybindings, as it is cleaned up
after changing the state:
```C
typedef struct mystate_state {
  ...
	binding_t input_bindings[NUM_ACTIONS];
	i_ctx main_binding_ctx;
  ...
} mystate_state;
```

And then in your states initalize function:
```C
void mystate_init(mystate_state *s) {
  ...

  // Simply assign wasd and arrow keys to movement, space to fire
  s->input_bindings[0] = BindState(SDL_SCANCODE_W, SDL_SCANCODE_UP,    player_mv_u, player_stop_mv_u);
  s->input_bindings[1] = BindState(SDL_SCANCODE_A, SDL_SCANCODE_LEFT,  player_mv_l, player_stop_mv_l);
  s->input_bindings[2] = BindState(SDL_SCANCODE_S, SDL_SCANCODE_DOWN,  player_mv_d, player_stop_mv_d);
  s->input_bindings[3] = BindState(SDL_SCANCODE_D, SDL_SCANCODE_RIGHT, player_mv_r, player_stop_mv_r);

  s->input_bindings[4] = BindAction(SDL_SCANCODE_SPACE, 0, fire_weapon);
  /* ... or Possibly load settings from some configuration file */

  // Assign the bindings to the input context
	s->input_ctx = (i_ctx){
		.bindings = (binding_t*)&s->input_bindings,
		.len = sizeof(s->input_bindings) / sizeof(binding_t),
	};
  // Push the main binding context to the engines input stack
  input_ctx_push(s->main_binding_ctx);
}
```

If you want specific bindings for dialog windows, you can simply push a input
context to the engines stack using `input_ctx_push`, and pop it when the user
closes the dialog.
