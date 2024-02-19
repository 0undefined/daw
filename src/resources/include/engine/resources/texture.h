#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <engine/core/types.h>
#include <engine/resources.h>
#include <engine/rendering/rendering.h>

Texture load_texture(const Asset_TextureSpec *restrict ts);

#ifdef __cplusplus
}
#endif
#endif
