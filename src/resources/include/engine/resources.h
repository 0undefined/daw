#ifndef RESOURCES_H
#define RESOURCES_H

#include <engine/core/types.h>

// TODO
/* We need some "global resources", available to all states.
 * These are resources such as common fonts, GUI frames, button background
 * images.
 *
 * We need to define state-specific resources as well.
 * - Can both be defined alike?
 *   If we lazy-load all resources we can get away with a lot.
 *   Maybe use fall-back resources? like the missing source texture, and an ugly
 *   font?
 * - Then declare to the engine, in the main function for the game, that these
 *   resources are to be made available throughout?
 * - Make all resource specifications A UNION?! 🤯
 * */

enum Asset {
  Asset_error,
  Asset_font,
  Asset_texture,
  Asset_audio,
};

typedef struct {
  enum Asset type;
  const char* font_path;
  i32 ptsize;
} Asset_FontSpec;

typedef struct {
  enum Asset type;
  const char* path;
  i32 width;
  i32 height;
} Asset_TextureSpec;

typedef struct {
  enum Asset type;
  const char* path;
} Asset_AudioSpec;

typedef union {
  enum Asset type;
  Asset_FontSpec font;
  Asset_TextureSpec texture;
  Asset_AudioSpec audio;
} asset_t;

// The resource spec
struct Resources {
  // Was:
//  usize textures_len;
//  usize textures_size;
//  usize fonts_len;
//
//  usize texturepaths_len;
//  usize fontpaths_len;
//
//  /* Paths for our sources, kept in case the user wants to reload them */
//  Asset_TextureSpec** texture_paths;
//  Asset_FontSpec** font_paths;
//
//  /* Our actual sources */
//  Texture** textures;
//  //TTF_Font** fonts;

  // But with the new way:
  // usize assets_len;
  // asset_t assets*;
};

#define Resource_FontDefinition(_path, _fontsize)                              \
  (const Asset_FontSpec) {                                                     \
    .type = Asset_font, .font_path = _path, .ptsize = _fontsize                \
  }

#define Resource_TextureAtlasDefinition(_path, _subtexture_width,              \
                                        _subtexture_height)                    \
  (const Asset_TextureSpec) {                                                  \
    .type = Asset_texture, .width = _subtexture_width,                         \
    .height = _subtexture_height, .path = _path                                \
  }

#define TextureDefinition(_path, ...) unimplemented

#define Resource_AudioDefinition(_path, ...) unimplemented

/* Each of resource_load_font, resource_load_texture, and resource_load_audio
 * loads a given resource into the engines memory and returns an identifier.
 */
isize resource_load_font(Asset_FontSpec font_def);
isize resource_load_texture(Asset_TextureSpec texture_def);
isize resource_load_audio(Asset_AudioSpec audio_def);

/* Makes a resource globally available. This must be called **BEFORE** any call
 * to `engine_run` */
isize resource_make_global(isize resource_id);

#include <engine/resources/texture.h>

#endif
