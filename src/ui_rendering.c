#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define ENGINE_INTERNALS

#include <engine/engine.h>
#include <engine/rendering.h>


extern Platform *GLOBAL_PLATFORM;

extern const char *uitype_str[];

void render_uitree(Window *w, UITree *t) {
	switch (t->type) {
	case uitype_container:
		render_container(w, &t->container);
		break;
	case uitype_button:
		render_button(w, &t->button);
		break;
	case uitype_title:
		render_title(w, &t->title);
		break;
	case uitype_text:
		render_text(w, &t->text);
		break;
	default:
		if (t->type >= uitype_MAX) {
			ERROR("Unknown uitype: %d", t->type);
		} else {
			WARN("Rendering not implemented for %s", uitype_str[t->type]);
		}
		break;
	}
}

void render_container(Window *w, UITree_container *t) {
	SDL_Rect r = {
		.x = t->x,
		.y = t->y,
		.w = t->w,
		.h = t->h
	};

	SDL_SetRenderDrawColor(w->renderer,
			t->bg.r,
			t->bg.g,
			t->bg.b,
			t->bg.a);

	SDL_RenderFillRect(w->renderer, &r);

	SDL_SetRenderDrawColor(w->renderer,
			t->fg.r,
			t->fg.g,
			t->fg.b,
			t->fg.a);

	SDL_RenderDrawRect(w->renderer, &r);

#ifdef DAW_BUILD_DEBUG
	r.x += t->padding;
	r.y += t->padding;
	r.w -= t->padding *	2;
	r.h -= t->padding *	2;
	SDL_SetRenderDrawColor(w->renderer,
			0xFF,
			0xFF,
			0xFF,
			0x7A);
	SDL_RenderDrawRect(w->renderer, &r);
#endif

	if (t->children != NULL && t->children_len > 0) {
		for (usize i = 0; i < t->children_len; i++) {
			render_uitree(w, t->children[i]);
		}
	}
}

void render_button(Window *w, UITree_button *t) {

	SDL_Rect inner_r = {
	.x = t->x + t->padding + (( t->w - t->texture_size.x) / 2),
	.y = t->y + t->padding + (( t->h - t->texture_size.y) / 2),
	.w = t->texture_size.x,
	.h = t->texture_size.y,
	};

	SDL_Rect outer_r = {
	.x = t->x,
	.y = t->y,
	.w = t->w + t->padding * 2,
	.h = t->h + t->padding * 2,
	};

	SDL_Color fg = {
		t->fg.r,
		t->fg.g,
		t->fg.b,
		t->fg.a,
	};

	if (t->enabled == false) {
		fg.r /= 2;
		fg.g /= 2;
		fg.b /= 2;
	}

	SDL_SetRenderDrawColor(w->renderer,
			t->bg.r,
			t->bg.g,
			t->bg.b,
			t->bg.a);

	SDL_RenderFillRect(w->renderer, &outer_r);

	SDL_SetRenderDrawColor(w->renderer,
			fg.r,
			fg.g,
			fg.b,
			fg.a);

	SDL_RenderDrawRect(w->renderer, &outer_r);
	SDL_Texture *texture = ((struct Resources*)GLOBAL_PLATFORM->data)
		->textures[t->text_texture_index]->texture;
	SDL_RenderCopy(
		w->renderer,
		texture,
		NULL,
		&inner_r);

}


void render_title(Window *w, UITree_title *t) {
	SDL_Rect r = {
		.x = t->x,
		.y = t->y,
		.w = t->texture_size.x,
		.h = t->texture_size.y,
	};

	SDL_SetRenderDrawColor(w->renderer,
			t->fg.r,
			t->fg.g,
			t->fg.b,
			t->fg.a);

	SDL_Texture *texture = ((struct Resources*)GLOBAL_PLATFORM->data)
		->textures[t->text_texture_index]->texture;

	SDL_RenderCopy(
		w->renderer,
		texture,
		NULL,
		&r);

#ifdef DAW_BUILD_DEBUG
	SDL_SetRenderDrawColor(w->renderer,
			0xFF,
			0xFF,
			0xFF,
			0x7A);

	SDL_RenderDrawRect(w->renderer, &r);
#endif
}

void render_text(Window *w, UITree_text *t) {
	SDL_Rect r = {
		.x = t->x,
		.y = t->y,
		.w = t->texture_size.x,
		.h = t->texture_size.y,
	};

	SDL_SetRenderDrawColor(w->renderer,
			t->fg.r,
			t->fg.g,
			t->fg.b,
			t->fg.a);

	SDL_Texture *texture = ((struct Resources*)GLOBAL_PLATFORM->data)
		->textures[t->text_texture_index]->texture;

	SDL_RenderCopy(
		w->renderer,
		texture,
		NULL,
		&r);

#ifdef DAW_BUILD_DEBUG
	SDL_SetRenderDrawColor(w->renderer,
			0xFF,
			0xFF,
			0xFF,
			0x7A);

	SDL_RenderDrawRect(w->renderer, &r);
#endif

}

i64 add_texture(struct Resources *resptr, Texture *t) {
	if (NULL == resptr
	 || NULL == t)      return -1;

	if (resptr->textures == NULL) {
		resptr->textures = malloc(sizeof(Texture*) * TEXTURES_INCREMENT);
	} else if (resptr->textures_len + 1 >= resptr->textures_size) {
		resptr->textures_size += TEXTURES_INCREMENT;
		resptr->textures = realloc(resptr->textures, sizeof(Texture*) * resptr->textures_size);
		memset(resptr->textures + resptr->textures_size - TEXTURES_INCREMENT, 0, TEXTURES_INCREMENT);
	}

	resptr->textures[resptr->textures_len] = t;
	return resptr->textures_len++;
}

i64 engine_render_text(i32 font_id, Engine_color fg, char *text, v2_i32 *size_out, bool wrapped) {
	const SDL_Color sdl_fg = {.r = fg.r, .g = fg.g, .b = fg.b, .a = fg.a};
	Texture *t = malloc(sizeof(Texture));
	struct Resources *r;

	if (t == NULL) {
		ERROR("Failed to allocate memory for texture!\n");
		exit(EXIT_FAILURE);
	}

	if (GLOBAL_PLATFORM == NULL) {
		ERROR("Platform is uninitialized.\n");
		exit(EXIT_FAILURE);
	}

	r = (struct Resources*)GLOBAL_PLATFORM->data;

	if (r == NULL) {
		ERROR("Resources not loaded!\n");
		exit(EXIT_FAILURE);
	}

	if (r->fonts == NULL) {
		ERROR("No fonts are initialized!\n");
		exit(EXIT_FAILURE);
	}

	if ((usize)font_id >= r->fonts_len) {
		ERROR("font-id \"%d\" is out of bounds!\n", font_id);
		ERROR("Couldn't render text \"%s\"\n", text);
	}

	SDL_Surface *s = NULL;

	if (wrapped) {
		s = TTF_RenderUTF8_Solid_Wrapped(r->fonts[font_id], text, sdl_fg, size_out->x);

		if (s == NULL) {
			ERROR("Failed call to TTF_RenderUTF8_Solid_Wrapped: %s\n", TTF_GetError());
			exit(EXIT_FAILURE);
		}
	} else {
		s = TTF_RenderUTF8_Solid(r->fonts[font_id], text, sdl_fg);

		if (s == NULL) {
			ERROR("Failed call to TTF_RenderUTF8_Solid: %s\n", TTF_GetError());
			exit(EXIT_FAILURE);
		}
	}

	t->texture = SDL_CreateTextureFromSurface(GLOBAL_PLATFORM->window->renderer, s);

	if (t->texture == NULL) {
		ERROR("Failed call to SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
		INFO("FontID: %d", font_id);
		INFO("Text: \"%s\" [%lu]", text, strlen(text));
		exit(EXIT_FAILURE);
	}

	*(i32*)&t->width   = s->w;
	*(i32*)&t->height  = s->h;

	size_out->x = s->w;
	size_out->y = s->h;

	SDL_FreeSurface(s);

	return add_texture(r, t);
}
