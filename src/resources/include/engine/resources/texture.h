#ifndef TEXTURE_H
#define TEXTURE_H

#include <engine/core/types.h>

typedef struct {
  const i32 tilesize;
  const i32 width;
  const i32 height;
} Texture;

Texture* load_texture(void* render, const Asset_TextureSpec* ts);

#endif
