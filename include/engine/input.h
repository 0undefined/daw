#ifndef INPUT_H
#define INPUT_H

#include <engine/types.h>

typedef void input_callback_t(void*);
typedef i32 scancode_t;

typedef enum InputType {
  InputType_error = 0,
  InputType_action,
  InputType_state,
  InputType_range, /* TBD */
} InputType;

typedef union action_t {
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
} action_t;

typedef struct binding_t {
  action_t action;

  // Change type depending on input handling back-end. like u16 for GLFW_KEY
  //enum SDL_Scancode keycode;
  scancode_t scancode;
  scancode_t scancode_alt;

  u64 since_last_activation;
} binding_t;

typedef struct i_ctx {
	binding_t* bindings;
	isize len;
} i_ctx;

void i_flush_bindings(usize numcalls, void* state_mem, input_callback_t* c[]);
action_t i_get_action(const i_ctx *ctx, u32 time, scancode_t scancode);

#define BindAction(key, altkey, f_action) \
  (binding_t){\
  .action = (action_t){.action = {\
		.type = InputType_action,\
		.callback     = (input_callback_t*)&f_action,\
		.callback_str = #f_action,\
	}},\
	.scancode = key,\
	.scancode_alt = altkey,\
  .since_last_activation = 0\
}

#define BindState(key, altkey, f_activate, f_deactivate) \
  (binding_t){\
  .action = (action_t){.state = {\
		.type = InputType_state,\
    .activate   = (input_callback_t*)&f_activate,\
    .deactivate = (input_callback_t*)&f_deactivate,\
    .activate_str   = #f_activate,\
    .deactivate_str = #f_deactivate,\
	}},\
	.scancode = key,\
	.scancode_alt = altkey,\
  .since_last_activation = 0\
}

#endif
