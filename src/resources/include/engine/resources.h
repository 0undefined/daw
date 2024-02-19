#ifndef ENGINE_RESOURCES_H
#define ENGINE_RESOURCES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <engine/core/types.h>
#include <engine/rendering/rendering.h>

// TODO
/* We need some "global resources", available to all states.
 * These are resources used throughout the applications lifetime, such as common
 * fonts, GUI frames, button background images.
 *
 * We need to define state-specific resources as well.
 * - Can both be defined alike?
 *   If we lazy-load all resources we can get away with a lot.
 *   Maybe use fall-back resources? like the missing source texture, and an ugly
 *   font?
 * - Then declare to the engine, in the main function for the game, that these
 *   resources are to be made available throughout?
 * - Shaders, oh boy oh shaders.
 *   We need to make some meta-shader declaration, so we can declare a set of
 *   shaders, that are used to link with other shaders, s.t. we can free up the
 *   shaders after compilation.
 * - Make all resource specifications a union.
 * */

enum Asset {
  Asset_error,
  Asset_audio,
  Asset_font,
  Asset_shader,
  Asset_shaderprog,
  Asset_texture,
};

typedef struct {
  enum Asset type;
  const char* path;
} Asset_AudioSpec;

typedef struct {
  enum Asset type;
  const char* path;
} Asset_FontSpec;

// if a shader is declared GLOBALLY, we should not destroy it when done with the
// "main" shader compilations.
typedef struct {
  enum Asset type;
  // Assume shader type from filename
  const char* path;
} Asset_ShaderSpec;

// Use list of gluint shader program ids to link against. This translates to a
// call to compose_shader.
typedef struct {
  enum Asset type;
  const u32* shader;
  const usize shader_len;
} Asset_ShaderProgramSpec;

typedef struct {
  enum Asset type;
  const char* path;
  /* Bits per component, set to zero if you don't want to change the format. */
  i32 bpc;
} Asset_TextureSpec;

typedef union {
  enum Asset type;
  Asset_AudioSpec audio;
  Asset_FontSpec font;
  Asset_ShaderSpec shader;
  Asset_ShaderProgramSpec shaderprog;
  Asset_TextureSpec texture;
} asset_t;

// The resource spec
typedef struct {
  /* Assorted asset specification, makes reloading them easier. */
  usize assets_len;
  asset_t* assets;

  /* Translation from `assets`'s indices to type-specific loaded assets: */
  usize* get; // Let r=Resources, then use as: `r.shader[ r.get[ MyShader ] ]`

  /* Loaded assets */
  usize shader_len;
  Shader* shader;

  usize texture_len;
  Texture* texture;
} Resources;

#define TextureDefinition(_path, ...) unimplemented
#define Resource_AudioDefinition(_path, ...) unimplemented
#define Declare_Shader(PATH) (const asset_t){.shader = {.type = Asset_shader, .path=PATH}}
#define Declare_ShaderProgram(SHADERS, NUMSHADERS) (const asset_t){.shaderprog = {.type = Asset_shaderprog, .shader=SHADERS, .shader_len=NUMSHADERS}}
#define Declare_Texture(PATH) (const asset_t){.texture = {.type = Asset_texture, .path=PATH, .bpc=0}}

void* get_asset(Resources* r, u32 idx);

/* Each of resource_load_font, resource_load_texture, and resource_load_audio
 * loads a given resource into the engines memory and returns an identifier.
 */
//isize resource_load_font(Asset_FontSpec font_def);
//isize resource_load_texture(Asset_TextureSpec texture_def);
//isize resource_load_audio(Asset_AudioSpec audio_def);

// Loads all resources specified in `assets` and populates corresponding
// resources members.
i32 resources_load(Resources *resources);

/* Makes a resource globally available. This must be called **BEFORE** any call
 * to `engine_run` */
isize resource_make_global(isize resource_id);

#include <engine/resources/texture.h>

#ifdef __cplusplus
}
#endif
#endif
