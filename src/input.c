#include <string.h>
#include <engine/input.h>
#include <engine/logging.h>


bool binding_action_cmp(binding_t *restrict a, binding_t *restrict b) {
	InputType t = a->action.type;
	if (t != b->action.type) return false;
	switch (t) {
		case InputType_action:
			return !strcmp(a->action.action.callback_str, b->action.action.callback_str);

		case InputType_state:
			return (!strcmp(a->action.state.activate_str,   b->action.state.activate_str))
			    && (!strcmp(a->action.state.deactivate_str, b->action.state.deactivate_str));

		case InputType_range: // fallthrough
		default:
			return false; // not implemented
	}
	return false;
}

bool binding_scancode_cmp(binding_t *restrict a, binding_t *restrict b) {
	return a->scancode == b->scancode
	    || a->scancode_alt == b->scancode_alt;
}

bool binding_scancode_cmp_i(binding_t *restrict a, scancode_t scancode) {
	return a->scancode == scancode
	    || a->scancode_alt == scancode;
}

// If any binding in ctx contains action, replace that entry.
bool i_update_binding(i_ctx *ctx, binding_t* binding) {
	isize idx = 0;
	if (ctx == NULL || binding == NULL) {
		ERROR("i_update_binding received nullptr!");
		return false;
	}

	while (idx < ctx->len && !binding_action_cmp(&ctx->bindings[idx], binding)) idx++;

	if (idx < ctx->len && binding_action_cmp(&ctx->bindings[idx], binding)) {
		ctx->bindings[idx].scancode = binding->scancode;
		ctx->bindings[idx].scancode_alt = binding->scancode_alt;
		return true;
	}

	return false;
}

// If any binding in ctx contains scancode, replace that action.
bool i_update_unique_binding(i_ctx *ctx, binding_t* binding) {
	isize idx = 0;
	if (ctx == NULL || binding == NULL) {
		ERROR("i_update_unique_binding received nullptr!");
		return false;
	}

	while (idx < ctx->len && !binding_scancode_cmp(&ctx->bindings[idx], binding)) idx++;

	if (idx < ctx->len && binding_scancode_cmp(&ctx->bindings[idx], binding)) {
		ctx->bindings[idx].action = binding->action;
		return true;
	}

	return false;
}

void i_flush_bindings(usize numcalls, void* state_mem, input_callback_t* c[]) {
	for (usize i = 0; i < numcalls; i++) {
		(c[i])(state_mem);
	}
}

action_t i_get_action(const i_ctx *restrict ctx, u32 time, scancode_t scancode) {
	isize idx = 0;

	if (ctx == NULL) {
		ERROR("%s received nullptr!", __func__);
		return (action_t){.type = InputType_error};
	}

	while (idx < ctx->len && !binding_scancode_cmp_i(&ctx->bindings[idx], scancode)) idx++;

	if (idx < ctx->len
		  && binding_scancode_cmp_i(&ctx->bindings[idx], scancode)) {
		ctx->bindings[idx].since_last_activation = time;
		return ctx->bindings[idx].action;
	}

	return (action_t){.type = InputType_error};
}
