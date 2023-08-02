#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#if defined (_WIN32) || defined (__WIN32__) || defined (WIN32)
	/* include winapi */
#elif defined (__APPLE__)
	/* mac includes */
#elif defined (__linux) || defined (__linux__) || defined (linux)

#include <unistd.h>

#endif


#define ENGINE_INTERNALS
#include <engine/engine.h>
#include <engine/btree.h>
#include <engine/hashmap.h>
#include <engine/list.h>

#include <engine/state.h>
//#include <states/titlescreen.h>
//#include <states/gameover.h>

#define DEFAULT_NUM_PROCS 8

#ifdef BENCHMARK
#define BENCHEXPR(timevar, expr) { \
	u32 t = SDL_GetTicks();   \
	expr                           \
	timevar += SDL_GetTicks() - t; \
}

extern i32 drawcall_len;

#else
#define BENCHEXPR(timevar, expr) expr
#endif

static u64 FPS_CAP   = 50;
Platform *GLOBAL_PLATFORM = NULL;

input_callback_t *callbacks[128];
usize callbacks_len;

i32 nproc(void) {
	return SDL_GetCPUCount();
}

i32 cmp_int(const void *a, const void *b) {
	const i32 *x = a;
	const i32 *y = b;

	return *x - *y;
}

v2_i32 get_canvas_size(SDL_Renderer *renderer) {
  v2_i32 realsize;
  SDL_GetRendererOutputSize(renderer, &(realsize.x), &(realsize.y));

  /* Set logical render size */
  return realsize;
}

Texture *load_texture(SDL_Renderer *render, const TextureSpec *ts) {
	SDL_Texture* new_texture = NULL;
	SDL_Surface* loaded_surface = NULL;
	Texture *t = NULL;

	if (ts == NULL) {
		ERROR("Invalid TextureSpec\n");
		return NULL;
	}

	loaded_surface = IMG_Load(ts->path);
	if (loaded_surface == NULL) {
		ERROR("Unable to load image \"%s\"!\n", ts->path);
		ERROR("SDL_image Error: %s\n", IMG_GetError());
		return NULL;
	}

	const i32 tw = loaded_surface->w / ts->width;

	SDL_SetColorKey(loaded_surface, SDL_TRUE,
			SDL_MapRGB(loaded_surface->format, 0xFF, 0x00, 0xFF));

	/*Create texture from surface pixels */
	new_texture = SDL_CreateTextureFromSurface(render, loaded_surface);
	if (new_texture == NULL) {
		ERROR("Unable to create texture from \"%s\"!\n", ts->path);
		ERROR("SDL Error: %s\n", SDL_GetError());
	}

	/*Get rid of old loaded surface */
	SDL_FreeSurface(loaded_surface);

	t = (Texture*)malloc(sizeof(Texture));
	t->texture = new_texture;
	/* Assigning const value */
	*(i32*)&t->tilesize = tw;
	*(i32*)&t->width    = ts->width;
	*(i32*)&t->height   = ts->height;

	return t;
}

void engine_update_window(Window *w, SDL_WindowEvent *e) {
	switch (e->event) {
		case SDL_WINDOWEVENT_NONE:
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_HIDDEN:
		case SDL_WINDOWEVENT_EXPOSED:
		case SDL_WINDOWEVENT_MOVED:
			break;
		case SDL_WINDOWEVENT_RESIZED:
			w->windowsize = get_canvas_size(w->renderer);
			LOG("Resized window to %dx%d", w->windowsize.x, w->windowsize.y);
			ui_resolve_constraints();
			if (w->game_w != NULL && w->game_h != NULL) {
				*w->game_h = w->windowsize.x;
				*w->game_w = w->windowsize.y;
			}
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_MINIMIZED:
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED:
		case SDL_WINDOWEVENT_ENTER:
		case SDL_WINDOWEVENT_LEAVE:
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_CLOSE:
		case SDL_WINDOWEVENT_TAKE_FOCUS:
		case SDL_WINDOWEVENT_HIT_TEST:
			break;
		default:
			WARN("Unhandled window event 0x%04x", (i32)e->event);
			break;
	}
	return;
}

Platform *engine_init(
    const char        *windowtitle,
    v2_i32             windowsize,
    const f32          render_scale,
    const u32          flags,
    const usize       initial_memory,
    const FontSpec    *fonts[],
    const TextureSpec *textures[]) {

#ifdef BENCHMARK
	u32 init_start = SDL_GetTicks();
#endif

#if defined (__linux) || defined (__linux__) || defined (linux)
	{
		pid_t pid = getpid();
		INFO("Starting with pid %lu", pid);
	}
#endif

	Platform     *p = (Platform*)malloc(sizeof(Platform));
	Window       *w = (Window*)malloc(sizeof(Window));
	SDL_Window   *window = NULL;
	SDL_Renderer *renderer = NULL;

	/* initialize resources */
	struct Resources *resources = (struct Resources*)malloc(sizeof(struct Resources));
	resources->textures_len     = 0;
	resources->fonts_len        = 0;
	resources->texturepaths_len = 0;
	resources->fontpaths_len    = 0;
	resources->texture_paths    = NULL;
	resources->font_paths       = NULL;
	resources->textures         = NULL;
	resources->fonts            = NULL;


	{ /* Init subsystems */
		INFO_("initializing sdl...");
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			ERROR("failed to initialize sdl: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		} else printf("ok\n");

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		INFO_("creating window...");
		window = SDL_CreateWindow(windowtitle,
				SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				windowsize.x, windowsize.y,
				SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | flags);

		if (window == NULL) {
			ERROR("failed to create window: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		} else printf("ok\n");

		INFO_("creating renderer...");
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == NULL) {
			ERROR("failed to create renderer: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		} else printf("ok\n");

		INFO_("initializing sdl_image...");
		if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
			ERROR("failed to initialize sdl_image png support\n");
			exit(EXIT_FAILURE);
		} else printf("ok\n");

		INFO_("initializing sdl_ttf...");
		if (TTF_Init() == -1) {
			ERROR("failed to initialize sdl_ttf: %s\n", TTF_GetError());
			exit(EXIT_FAILURE);
		} else printf("ok\n");
	}


	{ /* Resource loading */

		/* Count resources */
		usize n_textures = 0;
		usize n_fonts    = 0;

		if (textures != NULL) while (textures[n_textures] != NULL) n_textures++;
		if (fonts    != NULL) while (fonts[n_fonts]       != NULL) n_fonts++;

		INFO("Number of textures: "TERM_COLOR_YELLOW"%d"TERM_COLOR_RESET, n_textures);
		INFO("Number of fonts: "TERM_COLOR_YELLOW"%d"TERM_COLOR_RESET, n_fonts);


		/* Save the textures and fonts, if we should need to reload them later */
		resources->texture_paths = (TextureSpec**)textures;
		resources->font_paths    = (FontSpec**)fonts;

		/* Allocate memory for textures and fonts */
		resources->textures = (Texture**)malloc(sizeof(Texture*) * n_textures);
		resources->fonts    = (TTF_Font**)malloc(sizeof(TTF_Font*) * n_fonts);
		resources->textures_size = n_textures;

		for (usize i = 0; i < n_textures; i++) resources->textures[i] = NULL;
		for (usize i = 0; i < n_fonts; i++)    resources->fonts[i]    = NULL;

		/* Load textures */
		for (usize i = 0; i < n_textures; i++) {
			Texture *t = NULL;
			INFO_("loading texture \""
			      TERM_COLOR_YELLOW"%s"TERM_COLOR_RESET
			      "\"...", textures[i]->path);

			t = load_texture(renderer, textures[i]);
			if (t == NULL) {
				puts("");
				ERROR("failed to load texture\n");
				exit(EXIT_FAILURE);
			}

			if (t->tilesize < 8) {
				puts("");
				ERROR("texture too small!\n");
				exit(EXIT_FAILURE);
			}

			if (t->texture == NULL) {
				puts("");
				ERROR("failed to load texture\n");
			} else {
				printf("ok\n");
				resources->textures[i] = t;
				resources->textures_len++;
			}
		}

		/* Load fonts */
		for (usize i = 0; i < n_fonts; i++) {
			INFO_("loading font \""
			      TERM_COLOR_YELLOW"%s"TERM_COLOR_RESET
			      "\"...", fonts[i]->font_path);

			TTF_Font *font = TTF_OpenFont(fonts[i]->font_path
			        , fonts[i]->ptsize);
			if (!font) {
				ERROR("failed to load font: %s\n", TTF_GetError());
			} else {
				printf("ok\n");
				resources->fonts[i] = font;
				resources->fonts_len++;
			}
		}

		if (resources->textures_len != n_textures) {
			WARN("Done. %d/%d textures loaded.", resources->textures_len, n_textures);
		} else {
			INFO("Done. All %d textures loaded.", n_textures);
		}

		if (resources->fonts_len != n_fonts) {
			WARN("Done. %d/%d fonts loaded.", resources->fonts_len, n_fonts);
		} else {
			INFO("Done. All %d fonts loaded.", n_fonts);
		}

		resources->texturepaths_len  = resources->textures_len;
		resources->fontpaths_len     = resources->fonts_len;

	}


	{ /* Adjust window and such */
		/* Set actual windowsize, which might be forced by OS */
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
		INFO("Adjusting window size...");
		windowsize = get_canvas_size(renderer);

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

		INFO("Windowsize: <%d,%d>", windowsize.x, windowsize.y);
	}

	w->renderer     = renderer;
	w->window       = window;
	w->render_scale = render_scale;
	w->windowsize   = windowsize;
	w->game_w = NULL;
	w->game_h = NULL;

	p->data         = (void*)resources;
	p->data_len     = sizeof(struct Resources);
	p->window       = w;
	p->quit         = false;

	p->frame        = 0;
	p->fps_target   = 60;

	p->mem          = memory_new(initial_memory);

	/* Getting the mouse coords now resolves the issue where a click "isn't
	 * registered" when the mouse isn't moved before the user clicks */
	SDL_GetMouseState(&p->mouse_pos.x, &p->mouse_pos.y);

  p->mousedown  = (v2_i32){-1, -1};
  p->mouseup    = (v2_i32){-1, -1};

	p->mouse_lclick = false;
	p->mouse_rclick = false;

	p->camera_x     = 0;
	p->camera_y     = 0;

	p->edit_text    = NULL;
	p->edit_pos     = 0;

	p->bindings     = NULL;
	p->bindings_sz  = 0;
	p->bindings_len = 0;

	// TODO: Add global bindings

#ifdef BENCHMARK
	u32 init_stop = SDL_GetTicks();
	INFO("Initialization took %dms", init_stop - init_start);
#endif

	INFO("Available cores: %d", nproc());

	GLOBAL_PLATFORM = p;


#ifdef DAW_BUILD_HOTRELOAD

#define State(name)                                                             \
if (!State_reload(STATE_##name)) {                                              \
    ERROR("Failed to reload shared object file for state %s", #name );          \
};

#include <states/list_of_states.h>
#undef State

#endif

	return p;
}

i32 engine_run(Platform *p, StateType initial_state) {
	if (p == NULL) {
		ERROR("Platform is uninitialized.\n");
		INFO("initialize with `engine_init`");
		return -1;
	}

	memory* mem = p->mem;

	StateType state = initial_state;

	{
		u32 state_init_time = SDL_GetTicks();
		State_init(state, mem);
		INFO("Initializing state \"%s\" took %ldms", StateTypeStr[state], SDL_GetTicks() - state_init_time);
	}

	u32 time = SDL_GetTicks();

	// Update ticks
	u64 ticks = 0;

	/* Profiling values */
#ifdef BENCHMARK
	u64 profile_tick_counter   = 0;
	//u64 profile_slack          = 0;
	u64 profile_rendering      = 0;
	u64 profile_gameloop       = 0;
	u64 profile_input          = 0;
	u64 profile_input_handling = 0;
	u64 profile_num_drawcalls  = 0;
	u32 profile_interval_timer = time;
	const u32 profile_interval_ms = 5000;
	const f32 profile_interval_scale = (f32)(profile_interval_ms) / 100.0f;
#endif

	const f64 frame_interval = 1000.0 / FPS_CAP;

	StateType (*update_func)(void*) = State_updateFunc(state);

	/* Main loop */
	do {
		const u32 now = SDL_GetTicks();
		const u64 dt = now - time;
		time = now;
		/* Wait frame_interval */
		if (dt < frame_interval) {
#ifndef BENCHMARK
			SDL_Delay(frame_interval - dt);

#else
			/* We want to know how much time is spend sleeping */
			//profile_slack += frame_interval - dt;
#endif
		}

#ifdef BENCHMARK
		if (time - profile_interval_timer > profile_interval_ms) {
			/* Ticks/frames since last measurement */
			u32 fps = (ticks - profile_tick_counter) / profile_interval_scale;
			u64 drawcalls = profile_num_drawcalls / profile_interval_scale / fps;

			u32 sum =
				+ profile_rendering
				//+ profile_slack
				+ profile_input
				+ profile_input_handling
				+ profile_gameloop
				;


			/* Log fps and slack percentage */
			LOG("fps:%d\t"
			    "rendering:%.2f%%\t"
			    //"slack:%.2f%%\t"
			    "input:%.2f%% (%.2f%%)\t"
			    "gameloop:%.2f%%\t"
			    "unaccounted:%llu / %llu ms\t"
			    "avg drawcalls:%llu",
			    fps,
			    100.0f * (f32)profile_rendering      / (f32)sum,
			    //100.0f * (f32)profile_slack          / (f32)sum,
			    100.0f * (f32)profile_input          / (f32)sum,
			    100.0f * (f32)profile_input_handling / (f32)sum,
			    100.0f * (f32)profile_gameloop       / (f32)sum,
			    time - profile_interval_timer - sum, sum,
			    drawcalls);
			/* Reset values */
			profile_tick_counter   = ticks;
			profile_interval_timer = time;
			//profile_slack          = 0;
			profile_rendering      = 0;
			profile_gameloop       = 0;
			profile_input          = 0;
			profile_input_handling = 0;
			profile_num_drawcalls  = 0;
		}
#endif

		/* Events */
		BENCHEXPR(profile_input, {

			if (p->mouse_lclick) {
				p->mouseup.x    = -1;
				p->mouseup.y    = -1;
				p->mousedown.x  = -1;
				p->mousedown.y  = -1;
				p->mouse_lclick = false;
			}
			if (p->mouse_rclick) {
				p->mouse_rclick = false;
			}

			/* Window events */
			SDL_Event e[8];
			i32 num_events;
			SDL_PumpEvents();
			while ((num_events = SDL_PeepEvents(e, 8, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_SYSWMEVENT)) > 0) {
				for (i32 i = 0; i < num_events; i++) {
					switch (e[i].type) {
						case SDL_QUIT:
							state = STATE_quit;
							break;
						case SDL_WINDOWEVENT:
							engine_update_window(p->window, &e[i].window);
							break;
						default:
							WARN("Unhandled event 0x%04x", (i32)e[i].type);
					}
				}
			}

			/* Mouse events */
			while ((num_events = SDL_PeepEvents(e, 8, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEWHEEL)) > 0) {
				for (i32 i = 0; i < num_events; i++) {
					switch (e[i].type) {
					case SDL_MOUSEWHEEL:    break;
					case SDL_MOUSEMOTION:
						{
							SDL_MouseMotionEvent m = e[i].motion;
							/* In case of a first-person game, use xrel and yrel */
							p->mouse_pos.x = m.x;
							p->mouse_pos.y = m.y;
						}
						break;
					case SDL_MOUSEBUTTONUP:
						{
							switch (e[i].button.button) {
								case SDL_BUTTON_LEFT:
									p->mouseup = p->mouse_pos;

									p->mouse_lclick = true;
								case SDL_BUTTON_RIGHT:
									break;
								default:
									break;
							}
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
						switch (e[i].button.button) {
							case SDL_BUTTON_LEFT:
								p->mousedown = p->mouse_pos;
								break;
							case SDL_BUTTON_RIGHT:
								break;
							default:
								break;
						}
						break;
					default:
						WARN("Unhandled mouse event 0x%04x", (i32)e[i].type);
						break;
					}
				}
			}

			BENCHEXPR(profile_input_handling, {
			if (p->bindings != NULL) {
			const i_ctx *bindings = *p->bindings;
			const usize bindings_len = p->bindings_len;

			while ((num_events = SDL_PeepEvents(e, 8, SDL_GETEVENT, SDL_KEYDOWN, SDL_KEYUP)) > 0) {
				for (i32 i = 0; i < num_events; i++) {
					switch (e[i].type) {
					case SDL_KEYDOWN:
					  if (e[i].key.keysym.sym == SDLK_F7) {
							INFO("Reloading %s", StateTypeStr[state]);
							if (!State_reload(state)) {
								ERROR("Failed to reload state library!");
							} else {
								update_func = State_updateFunc(state);
							}
							break;
						}
						for (usize b = 0; b < bindings_len; b++) {
							const action_t a = i_get_action(&bindings[b], e[i].key.timestamp, e[i].key.keysym.scancode);

							switch (a.type) {
								case InputType_action:
									if (a.action.callback != NULL) {
										callbacks[callbacks_len++] = a.action.callback;
									}
									break;

								case InputType_state:
									if (!e[i].key.repeat && a.state.activate != NULL) {
										callbacks[callbacks_len++] = a.state.activate;
									}
									break;

								case InputType_range:
									WARN("Range inputs not supported yet!");
									break;
								case InputType_error:
									WARN("Unhandled scancode: %lu", e[i].key.keysym.scancode);

								default:
									break;
							}
						}
						break;

					case SDL_KEYUP:
						for (usize b = 0; b < bindings_len; b++) {
							const action_t a = i_get_action(&bindings[b], e[i].key.timestamp, e[i].key.keysym.scancode);
									if (a.type == InputType_state && a.state.deactivate != NULL && !e[i].key.repeat) {
											callbacks[callbacks_len++] = a.state.deactivate;
									}
						}
						break;
					default:
						WARN("Unhandled mouse event 0x%04x", (i32)e[i].type);
						break;
					}
				}
			}
			}
			});
		});

		i_flush_bindings(callbacks_len, mem->data, callbacks);
		callbacks_len = 0;

		/* update */
		StateType next_state;
		BENCHEXPR(profile_gameloop, {next_state = update_func((void*)(mem->data));} );//State_update(state, mem);});

		if (next_state != STATE_null) {
			if (next_state == STATE_quit) break;

			drawcall_reset();

			engine_window_resize_pointers_reset();
			State_free(state, mem);
			memory_clear(mem);

			p->bindings_len = 0;

			state = next_state;
			update_func = State_updateFunc(state);
#ifdef BENCHMARK
			{
				u32 t = SDL_GetTicks();
				State_init(state, mem);
				LOG("Initializing %s took %dms", StateTypeStr[state], SDL_GetTicks() - t);
			}
#else
			State_init(state, mem);
#endif
		} else {
#ifdef BENCHMARK
			profile_num_drawcalls += drawcall_len;
#endif
			render_begin(p->window);
			BENCHEXPR(profile_rendering, {render_present(p->window);})
		}


		ticks++;
	} while (state != STATE_quit);

	return 0;
}


void stop(Platform *p) {
	if (p == NULL) return;

	{ /* Deallocate resources */
		struct Resources *r = (struct Resources*)p->data;
		if (r != NULL) {
			/* Destroy textures */
			for (usize i = 0; i < r->textures_len; i++) {
				if (r->textures[i] != NULL) {
					SDL_DestroyTexture(r->textures[i]->texture);
					r->textures[i] = NULL;
				}
			}
			free(r->textures);

			/* Destroy Fonts */
			for (usize i = 0; i < r->fonts_len; i++) {
				if (r->fonts[i] != NULL) {
					TTF_CloseFont(r->fonts[i]);
					r->fonts[i] = NULL;
				}
			}
			free(r->fonts);
		}
	}

	{ /* Deallocate window */
		Window *w = p->window;
		if (w != NULL) {
			if (w->window   != NULL) { SDL_DestroyWindow(w->window);     w->window   = NULL; }
			if (w->renderer != NULL) { SDL_DestroyRenderer(w->renderer); w->renderer = NULL; }
		}
	}

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void engine_fps_max(u64 cap) { FPS_CAP = cap; }

void engine_input_ctx_push(i_ctx *ctx) {
	if (GLOBAL_PLATFORM->bindings == NULL) {
		GLOBAL_PLATFORM->bindings = calloc(8, sizeof(i_ctx*));
		GLOBAL_PLATFORM->bindings_sz = 8;
	}
	if (GLOBAL_PLATFORM->bindings_len + 1 >= GLOBAL_PLATFORM->bindings_sz) {
		 void* m = realloc(GLOBAL_PLATFORM->bindings, GLOBAL_PLATFORM->bindings_sz + 8);
		 if (m == NULL) {
			 ERROR("Failed to allocate 8 bytes (%d): %s", errno, strerror(errno));
			 exit(EXIT_FAILURE);
		 }
		 GLOBAL_PLATFORM->bindings_sz += 8;
	}

	/*
	LOG("Bindings in ctx:");
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
	*/
	GLOBAL_PLATFORM->bindings[GLOBAL_PLATFORM->bindings_len++] = ctx;
}

void engine_input_ctx_pop(void) {
	if (GLOBAL_PLATFORM->bindings == NULL || GLOBAL_PLATFORM->bindings_sz == 0) return;
}

void engine_input_ctx_reset(void) {
	GLOBAL_PLATFORM->bindings_len = 0;
}

u32 get_time(void) {return SDL_GetTicks();}
v2_i32   get_windowsize(void) {return GLOBAL_PLATFORM->window->windowsize;}
v2_i32  *get_mousepos(void) { return &GLOBAL_PLATFORM->mouse_pos; }
