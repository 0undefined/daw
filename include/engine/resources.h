#ifndef RESOURCES_H
#define RESOURCES_H

#include <engine/types.h>
#include <engine/engine.h>

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
 * - Make all resource specifications A UNION?! (gone wrong) 🛩️🙈 -- 💯 real!!!
 * */

#define Resource_FontDefinition(_path, _fontsize) \
	(const FontSpec){ .font_path=_path, .ptsize=_fontsize}

#define Resource_TextureAtlasDefinition(_path, _subtexture_width, _subtexture_height) \
	(const TextureSpec){.width = _subtexture_width, .height = _subtexture_height, .path = _path}

#define TextureDefinition(_path, ...) \
	unimplemented

#define Resource_AudioDefinition(_path, ...) \
	unimplemented

isize resource_load_texture(TextureSpec font_def);
isize resource_load_font(FontSpec font_def);
//isize load_audio(void);


isize resource_make_global(isize resource_id);

#endif
