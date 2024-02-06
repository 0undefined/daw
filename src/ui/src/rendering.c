#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <cglm/cglm.h>

//#include <engine/engine.h>
#include <engine/rendering/rendering.h>

//extern Platform* GLOBAL_PLATFORM;

extern const char* uitype_str[];

void render_container(Window w, UITree_container* t);
void render_button(Window w, UITree_button* t);
void render_title(Window w, UITree_title* t);
void render_text(Window w, UITree_text* t);
v2_i32 elem_size(UITree root);

void render_uitree(Window w, UITree* t) {
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

void render_container(Window w, UITree_container* t) {

  if (t->children != NULL && t->children_len > 0) {
    for (usize i = 0; i < t->children_len; i++) {
      render_uitree(w, t->children[i]);
    }
  }
}

void render_button(Window w, UITree_button* t) {
}

void render_title(Window w, UITree_title* t) {
}

void render_text(Window w, UITree_text* t) {
}

i64 add_texture(struct Resources* resptr, Texture* t) {
  return 0;
}

i64 engine_render_text(i32 font_id, Engine_color fg, char* text,
                       v2_i32* size_out, bool wrapped) {
  return 0;
}
