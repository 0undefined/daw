#include <errno.h>
#include <string.h>

#include <engine/core/dltools.h>
#include <engine/core/logging.h>
#include <engine/core/platform.h>
#include <engine/ctrl/input.h>

extern input_callback_t* callbacks[128];
extern usize callbacks_len;

extern Platform* GLOBAL_PLATFORM;
/* Lazy binds, used internally. They are similar to BindAction and friends.
 * The only difference is that we set callbacks and such to NULL, but populate
 * the function name strings such that can be reloaded. */
#define BindActionLazy(key, altkey, action_str)                                \
  (binding_t) {                                                                \
    .action = (action_t){.action =                                             \
                             {                                                 \
                                 .type = InputType_action,                     \
                                 .callback = NULL,                             \
                                 .callback_str = strdup(action_str),           \
                             }},                                               \
    .scancode = key, .scancode_alt = altkey, .since_last_activation = 0        \
  }

#define BindStateLazy(key, altkey, _activate_str, _deactivate_str)             \
  (binding_t) {                                                                \
    .action = (action_t){.state =                                              \
                             {                                                 \
                                 .type = InputType_state,                      \
                                 .activate = NULL,                             \
                                 .deactivate = NULL,                           \
                                 .activate_str = strdup(_activate_str),        \
                                 .deactivate_str = strdup(_deactivate_str),    \
                             }},                                               \
    .scancode = key, .scancode_alt = altkey, .since_last_activation = 0        \
  }

void key_callback(void* window, int key, int scancode, int action, int mods) {

  const i_ctx* bindings = *GLOBAL_PLATFORM->bindings;
  const usize bindings_len = GLOBAL_PLATFORM->bindings_len;

  const f64 now = get_time();

  for (usize b = 0; b < bindings_len; b++) {
    const action_t a = i_get_action(&bindings[b], now, scancode);
    switch (action) {
    case GLFW_PRESS:

      switch (a.type) {
        case InputType_action:
          if (a.action.callback != NULL) {
            callbacks[callbacks_len++] = a.action.callback;
          }
          break;

        case InputType_state:
          if (a.state.activate != NULL) {
            callbacks[callbacks_len++] = a.state.activate;
          }
          break;
        case InputType_range:
          WARN("Range inputs not supported yet!");
          break;
        case InputType_error:
          WARN("Unhandled scancode: %lu", scancode);

        default:
          break;
        }
      break;

    case GLFW_RELEASE:
      switch (a.type) {
        case InputType_state:
          if (a.state.deactivate != NULL) {
            callbacks[callbacks_len++] = a.state.deactivate;
          }
          break;
        default:break;
      }
      break;
    case GLFW_REPEAT:
      /* unhandled so far */
      break;
    default:
      break;
  }
  }
}

void binding_t_free(binding_t* b) {
  switch (b->action.type) {
  case InputType_error:
    ERROR("Cannot free binding of type InputType_error");
    break;
  case InputType_action:
    free(b->action.action.callback_str);
    return;

  case InputType_state:
    free(b->action.state.activate_str);
    free(b->action.state.deactivate_str);
    break;

  case InputType_range:
    ERROR("Cannot free binding of type InputType_rage");
    break;

  default:
    ERROR("Unknown bindings type");
    break;
  }
  exit(EXIT_FAILURE);
}

void i_ctx_t_free(i_ctx* c) {
  for (isize i = 0; i < c->len; i++) {
    binding_t_free(&c->bindings[i]);
  }
}

bool binding_action_cmp(binding_t* restrict a, binding_t* restrict b) {
  InputType t = a->action.type;
  if (t != b->action.type) return false;
  switch (t) {
  case InputType_action:
    return !strcmp(a->action.action.callback_str,
                   b->action.action.callback_str);

  case InputType_state:
    return (!strcmp(a->action.state.activate_str,
                    b->action.state.activate_str)) &&
           (!strcmp(a->action.state.deactivate_str,
                    b->action.state.deactivate_str));

  case InputType_range:  // fallthrough
  default:
    return false;  // not implemented
  }
  return false;
}

bool binding_scancode_cmp(binding_t* restrict a, binding_t* restrict b) {
  return a->scancode == b->scancode || a->scancode_alt == b->scancode_alt;
}

bool binding_scancode_cmp_i(binding_t* restrict a, scancode_t scancode) {
  return a->scancode == scancode || a->scancode_alt == scancode;
}

// If any binding in ctx contains action, replace that entry.
bool i_update_binding(i_ctx* ctx, binding_t* binding) {
  isize idx = 0;
  if (ctx == NULL || binding == NULL) {
    ERROR("i_update_binding received nullptr!");
    return false;
  }

  while (idx < ctx->len && !binding_action_cmp(&ctx->bindings[idx], binding))
    idx++;

  if (idx < ctx->len && binding_action_cmp(&ctx->bindings[idx], binding)) {
    ctx->bindings[idx].scancode = binding->scancode;
    ctx->bindings[idx].scancode_alt = binding->scancode_alt;
    return true;
  }

  return false;
}

// If any binding in ctx contains scancode, replace that action.
bool i_update_unique_binding(i_ctx* ctx, binding_t* binding) {
  isize idx = 0;
  if (ctx == NULL || binding == NULL) {
    ERROR("i_update_unique_binding received nullptr!");
    return false;
  }

  while (idx < ctx->len && !binding_scancode_cmp(&ctx->bindings[idx], binding))
    idx++;

  if (idx < ctx->len && binding_scancode_cmp(&ctx->bindings[idx], binding)) {
    ctx->bindings[idx].action = binding->action;
    return true;
  }

  return false;
}

void i_flush_bindings(f64 dt, usize numcalls, input_callback_t* c[], void* state_mem) {
  for (usize i = 0; i < numcalls; i++) {
    (c[i])(state_mem);
  }
  // reset callback len and be ready for more
  callbacks_len = 0;
}

action_t i_get_action(const i_ctx* restrict ctx, u32 time,
                      scancode_t scancode) {
  isize idx = 0;

  if (ctx == NULL) {
    ERROR("%s received nullptr!", __func__);
    return (action_t){.type = InputType_error};
  }

  while (idx < ctx->len &&
         !binding_scancode_cmp_i(&ctx->bindings[idx], scancode))
    idx++;

  if (idx < ctx->len && binding_scancode_cmp_i(&ctx->bindings[idx], scancode)) {
    ctx->bindings[idx].since_last_activation = time;
    return ctx->bindings[idx].action;
  }

  return (action_t){.type = InputType_error};
}

/* Make a lazy duplication of a binding. See comments on BindActionLazy and
 * friends above. */
i_ctx* i_ctx_dup(i_ctx** ctx, usize ctx_len) {
  usize num_binds = 0;
  for (usize c = 0; c < ctx_len; c++) {
    num_binds += ctx[c]->len;
  }

  binding_t* bb = calloc(num_binds, sizeof(binding_t));
  i_ctx* ret = calloc(ctx_len, sizeof(i_ctx));

  usize cumsum = 0;
  for (usize c = 0; c < ctx_len; c++) {
    binding_t* b = ctx[c]->bindings;
    ret[c].len = ctx[c]->len;
    ret[c].bindings = &bb[cumsum];

    for (isize i = 0; i < ctx[c]->len; i++) {
      switch (b[i].action.type) {
      case InputType_error:
        break;
      case InputType_action:
        bb[cumsum] = BindActionLazy(b[i].scancode, b[i].scancode_alt,
                                    strdup(b[i].action.action.callback_str));
        break;
      case InputType_state:
        bb[cumsum] = BindStateLazy(b[i].scancode, b[i].scancode_alt,
                                   strdup(b[i].action.state.activate_str),
                                   strdup(b[i].action.state.deactivate_str));
        break;
      case InputType_range:
      default:
        break;
      }
      cumsum++;
    }
  }
  return ret;
}

/* Returns a pointer to a binding in c with .action == a.
 * Returns NULL if not found. */
binding_t* get_action(i_ctx* c, action_t* a) {
  for (isize i = 0; i < c->len; i++) {
    if (c->bindings[i].action.type != a->type) continue;

    switch (c->bindings[i].action.type) {
    case InputType_error:
      return NULL;

    case InputType_action:
      if (!strcmp(c->bindings[i].action.action.callback_str,
                  a->action.callback_str)) {
        return &c->bindings[i];
      }
      break;

    case InputType_state:
      if (!strcmp(c->bindings[i].action.state.activate_str,
                  a->state.activate_str) &&
          !strcmp(c->bindings[i].action.state.deactivate_str,
                  a->state.deactivate_str)) {
        return &c->bindings[i];
      }
      break;

    case InputType_range:
    default:
      return NULL;
    }
  }
  return NULL;
}

void i_bind_ctx(i_ctx* c, scancode_t s, action_t* a) {
  binding_t* b = get_action(c, a);

  if (b == NULL) {
    WARN("Failed to update keybinding");
    return;
  }

  b->scancode = s;
}

void i_bind_ctx_alt(i_ctx* c, scancode_t s, action_t* a) {
  binding_t* b = get_action(c, a);

  if (b == NULL) {
    WARN("Failed to update keybinding");
    return;
  }

  b->scancode_alt = s;
}

void i_bind(binding_t* b, scancode_t s) {
  if (b == NULL) {
    WARN("Failed to update keybinding");
    return;
  }

  b->scancode = s;
}

void i_bind_alt(binding_t* b, scancode_t s) {
  if (b == NULL) {
    WARN("Failed to update keybinding");
    return;
  }

  b->scancode_alt = s;
}

/* Pushes an input context onto the input handling stack */
void i_ctx_push(i_ctx* ctx) {
  if (GLOBAL_PLATFORM->bindings == NULL) {
    GLOBAL_PLATFORM->bindings = calloc(8, sizeof(i_ctx*));
    GLOBAL_PLATFORM->bindings_sz = 8;
  }

  if (GLOBAL_PLATFORM->bindings_len + 1 >= GLOBAL_PLATFORM->bindings_sz) {
    void* m =
        realloc(GLOBAL_PLATFORM->bindings, GLOBAL_PLATFORM->bindings_sz + 8);
    if (m == NULL) {
      ERROR("Failed to allocate 8 bytes (%d): %s", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    GLOBAL_PLATFORM->bindings_sz += 8;
  }

  LOG("Bindings in ctx[%d]:", GLOBAL_PLATFORM->bindings_len);
  for (isize i = 0; i < ctx->len; i++) {
    switch (ctx->bindings[i].action.type) {
    case InputType_error:
      LOG("(error)");
      break;

    case InputType_action:
      LOG("(action) %s", ctx->bindings[i].action.action.callback_str);
      break;

    case InputType_state:
      LOG("(+state) %s", ctx->bindings[i].action.state.activate_str);
      LOG("(-state) %s", ctx->bindings[i].action.state.deactivate_str);
      break;

    case InputType_range:
      LOG("(range) --unhandled--");
      break;
    }
  }

  GLOBAL_PLATFORM->bindings[GLOBAL_PLATFORM->bindings_len++] = ctx;
}

/* Pops an input context from the input stack */
void i_ctx_pop(void) {
  if (GLOBAL_PLATFORM->bindings == NULL || GLOBAL_PLATFORM->bindings_sz == 0)
    return;
  i_ctx_t_free(GLOBAL_PLATFORM->bindings[--GLOBAL_PLATFORM->bindings_len]);
}

/* Removes all input contexts from the input stack */
void i_ctx_reset(void) {
  while (GLOBAL_PLATFORM->bindings_len > 0) {
    i_ctx_t_free(GLOBAL_PLATFORM->bindings[--GLOBAL_PLATFORM->bindings_len]);
  }
}
