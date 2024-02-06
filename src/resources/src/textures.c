#include <engine/core/logging.h>
#include <engine/resources.h>

Texture* load_texture(void* render, const Asset_TextureSpec* ts) {
  Texture* t = NULL;

  if (ts == NULL) {
    ERROR("Invalid Asset_TextureSpec\n");
    return NULL;
  }

  //t = (Texture*)malloc(sizeof(Texture));
  //t->texture = new_texture;
  ///* Assigning const value */
  //*(i32*)&t->tilesize = tw;
  //*(i32*)&t->width = ts->width;
  //*(i32*)&t->height = ts->height;

  return t;
}
