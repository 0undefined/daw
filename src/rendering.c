#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define ENGINE_INTERNALS
#include <engine/engine.h>
#include <engine/rendering.h>

/* Extern globals */
extern Platform *GLOBAL_PLATFORM;

/* Globals */
#define drawcall_limit (64 * 1024)
RenderDrawCall drawcalls[drawcall_limit];
i32 drawcall_len = 0;

/* Implementations */

/* Clear the screen,
 * To be used inbetween draw calls */
void render_begin(Window *w) {
#ifdef DEBUG
	SDL_SetRenderDrawColor(w->renderer, 0x10, 0x0a, 0x33, 0x00);
#else
	SDL_SetRenderDrawColor(w->renderer, 0x00, 0x00, 0x00, 0x00);
#endif
	SDL_RenderClear(w->renderer);
}

void render_present(Window *w) {
	for (i32 i = 0; i < drawcall_len; i++) {
		RenderDrawCall dc = drawcalls[i];
		switch (dc.type) {
			case RenderDrawCallType_UITree:
				render_uitree(w, dc.data.data);
				break;
			case RenderDrawCallType_Text:
				LOG("RenderDrawCallType_Text rendering not implemented!");
				break;
			case RenderDrawCallType_Sprite:
				{
#ifdef DEBUG
					if (dc.data.sprite.sprite == NULL) {
						__asm__("int3;");
						WARN("Sprite %lx in drawcall %d/%d had NULL reference", dc.data.sprite.sprite, i, drawcall_len);

						drawcall_len = 0;
						SDL_RenderPresent(w->renderer);
						exit(EXIT_FAILURE);
					}
#endif
					Sprite s = *dc.data.sprite.sprite;
					Texture *t = ((struct Resources*)GLOBAL_PLATFORM->data)->textures[s.texture_id];
					i32 ts = t->tilesize;
					SDL_Rect src = {
						s.coord.x,
						s.coord.y,
						ts,ts
					};
					SDL_Rect dst = {
						dc.data.sprite.x,
						dc.data.sprite.y,
						ts * dc.data.sprite.scale,
						ts * dc.data.sprite.scale,
					};
					SDL_SetTextureColorMod(t->texture,
							dc.data.sprite.mod.r,
							dc.data.sprite.mod.g,
							dc.data.sprite.mod.b);
					SDL_RenderCopy(
						w->renderer,
						t->texture,
						&src, &dst);
				}
				break;
			default:
				break;
		}
	}

	drawcall_len = 0;

	SDL_RenderPresent(w->renderer);
}

void drawcall_reset(void) {
	drawcall_len = 0;
}

void engine_window_resize_pointers(i32* w, i32* h) {
	GLOBAL_PLATFORM->window->game_w = w;
	GLOBAL_PLATFORM->window->game_h = h;
}

void engine_window_resize_pointers_reset(void) {
	GLOBAL_PLATFORM->window->game_w = NULL;
	GLOBAL_PLATFORM->window->game_h = NULL;
}

void engine_draw_uitree(UITree *t) {
	if (drawcall_len + 1 >= drawcall_limit) return;
	drawcalls[drawcall_len++] = (RenderDrawCall){
		.type = RenderDrawCallType_UITree,
		.data.data = (void*)t
	};
}

void engine_draw_sprite(Sprite *s, v2_i32 *pos, f32 scale) {
	if (drawcall_len + 1 >= drawcall_limit) return;
#ifdef DEBUG
	if (s == NULL) __asm__("int3;");
#endif
	drawcalls[drawcall_len++] = (RenderDrawCall){
		.type = RenderDrawCallType_Sprite,
		.data.sprite = {
			.sprite = s,
			.x = pos->x,
			.y = pos->y,
			.scale = scale,
			.mod = {0xFF, 0xFF, 0xFF, 0xFF},
		}
	};
}

void engine_draw_sprite_ex(Sprite *s, v2_i32 *pos, f32 scale, Engine_color colormod) {
	if (drawcall_len + 1 >= drawcall_limit) return;
#ifdef DEBUG
	if (s == NULL) __asm__("int3;");
#endif
	drawcalls[drawcall_len++] = (RenderDrawCall){
		.type = RenderDrawCallType_Sprite,
		.data.sprite = {
			.sprite = s,
			.x = pos->x,
			.y = pos->y,
			.scale = scale,
			.mod = {colormod.r, colormod.g, colormod.b, colormod.a},
		}
	};
}


Sprite sprite_new(u64 tid, u8 coord) {
	const i32 ts = ((struct Resources*)GLOBAL_PLATFORM->data)->textures[tid]->tilesize;
	return (Sprite){.texture_id = tid, (v2_i32){
		.x = ts *  (coord & 0x0F),
		.y = ts * ((coord & 0xF0) >> 4),
	}};
}
