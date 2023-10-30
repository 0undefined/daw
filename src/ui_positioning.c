#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdint.h>
#define ENGINE_INTERNALS

#include <engine/btree.h>
#include <engine/engine.h>

static Engine_color DEFAULT_FG = {0xFF, 0xFF, 0xFF, 0xFF};
static Engine_color DEFAULT_BG = {0x00, 0x00, 0x00, 0xFF};
static i32 DEFAULT_PADDING = 8;
static i32 DEFAULT_MARGIN = 0;

/* We want to keep track of the roots of the UI elements, s.t. we can resize and
 * reposition the elements if needed be. */
struct btree* GLOBAL_UIROOTS = NULL;

extern Platform* GLOBAL_PLATFORM;

const char* uitype_str[] = {
    [uitype_container] = "uitype_container",
    [uitype_button] = "uitype_button",
    [uitype_title] = "uitype_title",
    [uitype_text] = "uitype_text",
    [uitype_MAX] = "uitype_MAX",
    NULL,
};

const char* constraint_str[] = {
    [ui_constraint_width] = "ui_constraint_width",
    [ui_constraint_height] = "ui_constraint_height",

    [ui_constraint_horizontal_align] = "ui_constraint_horizontal_align",
    [ui_constraint_vertical_align] = "ui_constraint_vertical_align",

    [ui_constraint_left] = "ui_constraint_left",
    [ui_constraint_right] = "ui_constraint_right",
    [ui_constraint_top] = "ui_constraint_top",
    [ui_constraint_bottom] = "ui_constraint_bottom",
};

static i32 pointer_cmp(const void* a, const void* b) {
  const u64* x = a;
  const u64* y = b;
  /* We just use the address as index */
  return *x - *y;
}

static void pointer_print(const void* a) {
  const u64* t = a;
  printf("%lx\n", *t);
}

void ui_rearrange(UITree* root, v2_i32 ppos, v2_i32 psize);
i32 get_padding(UITree* t);
i32 get_margin(UITree* t);
v2_i32 get_pos(UITree* t);
v2_i32 get_size(UITree* t);

f32 ui_size_pixel_to_percent_width(i32 pixels) {
  return (f32)pixels / (f32)GLOBAL_PLATFORM->window->windowsize.x;
}

f32 ui_size_pixel_to_percent_height(i32 pixels) {
  return (f32)pixels / (f32)GLOBAL_PLATFORM->window->windowsize.y;
}

f32 ui_size_pixel_to_percent(i32 pixels) {
  return ui_size_percent_height_to_pixel(pixels);
}

i32 ui_size_percent_width_to_pixel(f32 percent) {
  return (i32)(percent * GLOBAL_PLATFORM->window->windowsize.x);
}

i32 ui_size_percent_height_to_pixel(f32 percent) {
  return (i32)(percent * GLOBAL_PLATFORM->window->windowsize.y);
}

i32 ui_size_percent_to_pixel(f32 percent) {
  return ui_size_percent_height_to_pixel(percent);
}

i32 ui_size_to_pixel_ex(ui_size s, bool vertical, v2_i32 psize) {
  if (s.type == ui_size_pixel) {
    return s.pixel.value;
  } else {
    psize.x = MAX(1, psize.x);
    psize.y = MAX(1, psize.y);
    if (vertical) {
      return psize.y * s.percent.value;
    }
    return psize.x * s.percent.value;
  }
}

i32 ui_size_to_pixel(ui_size s) {
  if (s.type == ui_size_pixel) {
    return s.pixel.value;
  } else {
    return ui_size_percent_height_to_pixel(s.percent.value);
  }
}

void ui_add(UITree* t) {

  if (GLOBAL_UIROOTS == NULL) {
    GLOBAL_UIROOTS = btree_new(sizeof(void*), 16, &pointer_cmp);
  }

  u64 tmp = (u64)t;
  btree_insert(GLOBAL_UIROOTS, &tmp);
}

void clear_ui(void) {
  UITree* t = NULL;
  while ((t = btree_first(GLOBAL_UIROOTS)) != NULL) {
    uitree_free(t);
    if (!btree_delete(GLOBAL_UIROOTS, t)) {
      WARN("Failed deletion");
      UITree* elem = (UITree*)t;
      WARN("Failed to remove %s from global ui-table", uitype_str[elem->type]);
    }
  }
}

/* Unwraps the size to a v2_i32 */
v2_i32 uitree_get_size(UITree* t) {
  if (t == NULL) return (v2_i32){.x = -1, .y = -1};
  switch (t->type) {
  case uitype_container:
    return (v2_i32){.x = t->container.w, .y = t->container.h};
  case uitype_button:
    return (v2_i32){.x = t->button.w, .y = t->button.h};
  case uitype_title:
    return (v2_i32){.x = t->title.w, .y = t->title.h};
  case uitype_text:
    return (v2_i32){.x = t->text.w, .y = t->text.h};
  default:
    return (v2_i32){.x = -1, .y = -1};
  }
}

/* elem_size also takes into account the padding and other stuffs */
v2_i32 elem_size(UITree* root) {
  if (root == NULL) return (v2_i32){0, 0};

  const i32 total_padding = 2 * get_padding(root);
  return v2_i32_add_i(uitree_get_size(root), total_padding);
}

v2_i32 elem_size_spacing(UITree* root) {
  if (root == NULL) return (v2_i32){0, 0};

  const i32 total_margin = 2 * get_padding(root) + 2 * get_margin(root);
  return v2_i32_add_i(uitree_get_size(root), total_margin);
}

UITree* ui_container_new_ex(Engine_color fg, Engine_color bg,
                            Engine_color border, bool direction,
                            struct List_ui_constraint* constraints) {

  UITree* t = malloc(sizeof(UITree));

  t->container.type = uitype_container;
  t->container.visible = true;
  t->container.direction = direction;

  /* -1 == unsolved constraint */
  t->container.x = -1;
  t->container.y = -1;
  t->container.w = -1;
  t->container.h = -1;
  t->container.padding = 0;
  t->container.margin = 0;

  t->container.fg = fg;
  t->container.bg = bg;
  t->container.bordercolor = border;

  t->container.children = NULL;
  t->container.children_len = 0;
  t->container.children_size = 0;

  t->container.constraints = constraints;

  ui_rearrange(t, (v2_i32){0, 0}, (v2_i32){0, 0});

  ui_add(t);

  return t;
}

UITree* ui_container(bool direction, struct List_ui_constraint* constraints) {
  return ui_container_new_ex(
      /* We set border color to fg by default */
      DEFAULT_FG, DEFAULT_BG, DEFAULT_FG, direction, constraints);
}

UITree* ui_title(i32 font_id, const char* text,
                 struct List_ui_constraint* constraints) {
  UITree_title* t = malloc(sizeof(UITree));
  t->type = uitype_title;

  t->x = -1;
  t->y = -1;

  t->padding = DEFAULT_PADDING;
  t->margin = DEFAULT_MARGIN;

  t->fg = DEFAULT_FG;

  t->font_id = font_id;
  t->text_original = text;
  t->text_texture_index =
      engine_render_text(font_id, t->fg, (char*)text, &t->texture_size, false);

  t->w = t->texture_size.x;
  t->h = t->texture_size.y;

  t->constraints = constraints;

  ui_rearrange((UITree*)t, (v2_i32){0, 0}, (v2_i32){0, 0});

  ui_add((UITree*)t);

  return (UITree*)t;
}

UITree* ui_text(i32 font_id, const char* text,
                struct List_ui_constraint* constraints) {
  UITree* t = malloc(sizeof(UITree));
  t->text.type = uitype_text;

  t->text.x = -1;
  t->text.y = -1;

  t->text.padding = 0;
  t->text.margin = DEFAULT_MARGIN / 2;

  t->text.fg = DEFAULT_FG;

  t->text.font_id = font_id;
  t->text.text_original = text;

  /* Postpone rendering until we have a clear set of size-constraints */

  // t->text.text_texture_index =
  //	engine_render_text(font_id, DEFAULT_FG, (char*)text,
  //&t->text.texture_size, true);

  // t->text.w = t->text.texture_size.x;
  // t->text.h = t->text.texture_size.y;

  t->text.constraints = constraints;

  ui_rearrange(t, (v2_i32){0, 0}, (v2_i32){0, 0});

  ui_add(t);

  return t;
}

UITree* ui_button(u64 id, i32 font_id, char* text,
                  struct List_ui_constraint* constraints) {
  UITree_button* t = malloc(sizeof(UITree));
  t->type = uitype_button;

  t->enabled = true;
  t->id = id;
  t->x = -1;
  t->y = -1;
  t->padding = DEFAULT_PADDING;
  t->margin = DEFAULT_MARGIN;

  t->fg = DEFAULT_FG;
  t->bg = DEFAULT_BG;

  t->font_id = font_id;
  t->text_original = text;
  t->text_texture_index =
      engine_render_text(font_id, t->fg, (char*)text, &t->texture_size, false);

  t->w = t->texture_size.x;
  t->h = t->texture_size.y;

  t->constraints = constraints;

  ui_rearrange((UITree*)t, (v2_i32){0, 0}, (v2_i32){0, 0});

  ui_add((UITree*)t);

  return (UITree*)t;
}

void uitree_free(UITree* t) {
  switch (t->type) {
  case uitype_container:
    if (t->container.children != NULL && t->container.children_len > 0) {
      for (usize i = 0; i < t->container.children_len; i++) {
        uitree_free(t->container.children[i]);
      }
      free(t->container.children);
    }
    break;
  case uitype_button:
  case uitype_title:
  case uitype_text:
    free(t);
    break;
  default:
    break;
  }
}

struct List_ui_constraint* get_constraints(UITree* t) {
  switch (t->type) {
  case uitype_container:
    return t->container.constraints;
  case uitype_button:
    return t->button.constraints;
  case uitype_title:
    return t->title.constraints;
  case uitype_text:
    return t->text.constraints;
  default:
    return NULL;
  }
}

i32 get_padding(UITree* t) {
  switch (t->type) {
  case uitype_container:
    return t->container.padding;
  case uitype_button:
    return t->button.padding;
  case uitype_title:
    return t->title.padding;
  case uitype_text:
    return t->text.padding;
  default:
    return 0;
  }
}

i32 get_margin(UITree* t) {
  switch (t->type) {
  case uitype_container:
    return t->container.margin;
  case uitype_button:
    return t->button.margin;
  case uitype_title:
    return t->title.margin;
  case uitype_text:
    return t->text.margin;
  default:
    return 0;
  }
}

v2_i32 get_pos(UITree* t) {
  switch (t->type) {
  case uitype_container:
    return (v2_i32){t->container.x, t->container.y};
  case uitype_button:
    return (v2_i32){t->button.x, t->button.y};
  case uitype_title:
    return (v2_i32){t->title.x, t->title.y};
  case uitype_text:
    return (v2_i32){t->text.x, t->text.y};
  default:
    return (v2_i32){0, 0};
  }
}

v2_i32 get_size(UITree* t) {
  switch (t->type) {
  case uitype_container:
    return (v2_i32){t->container.w, t->container.h};
  case uitype_button:
    return (v2_i32){t->button.w, t->button.h};
  case uitype_title:
    return (v2_i32){t->title.w, t->title.h};
  case uitype_text:
    return (v2_i32){t->text.w, t->text.h};
  default:
    return (v2_i32){0, 0};
  }
}

#define SETVAL(VAL, tt)                                                        \
  {                                                                            \
    if ((VAL) > 0) t->tt = VAL;                                                \
  }
void uitree_set_pos(UITree* t, v2_i32 newpos) {
  switch (t->type) {
  case uitype_container:
    SETVAL(newpos.x, container.x);
    SETVAL(newpos.y, container.y);
    break;
  case uitype_button:
    SETVAL(newpos.x, button.x);
    SETVAL(newpos.y, button.y);
    break;
  case uitype_title:
    SETVAL(newpos.x, title.x);
    SETVAL(newpos.y, title.y);
    break;
  case uitype_text:
    SETVAL(newpos.x, text.x);
    SETVAL(newpos.y, text.y);
    break;
  default:
    return;
  }
}

void uitree_set_size(UITree* t, v2_i32 size) {
  switch (t->type) {
  case uitype_container:
    SETVAL(size.x, container.w);
    SETVAL(size.y, container.h);
    break;
  case uitype_button:
    SETVAL(size.x, button.w);
    SETVAL(size.y, button.h);
    break;
  case uitype_title:
    SETVAL(size.x, title.w);
    SETVAL(size.y, title.h);
    break;
  case uitype_text:
    SETVAL(size.x, text.w);
    SETVAL(size.y, text.h);
    break;
  default:
    return;
  }
}

bool uitree_has_constraint(UITree* t, const ui_constraint_t constraint) {
  struct List_ui_constraint* c = NULL;
  struct List_ui_constraint* first_constraint;
  if (t == NULL) {
    ERROR("%s got nullptr argument!", __func__);
    exit(EXIT_FAILURE);
  }

  switch (t->type) {
  case uitype_container:
    c = t->container.constraints;
    break;
  case uitype_button:
    c = t->button.constraints;
    break;
  case uitype_title:
    c = t->title.constraints;
    break;
  case uitype_text:
    c = t->text.constraints;
    break;
  default:
    break;
  }

  first_constraint = c;

  while (c != NULL) {
    if ((void*)c->next == (void*)first_constraint) {
      ERROR("Infinite constraint loop detected!");
      exit(EXIT_FAILURE);
    }
    if (constraint == c->value.type) return true;

    c = c->next;
  }
  return false;
}

void ui_constraints_apply(UITree* t, v2_i32 ppos, v2_i32 psize) {
  struct List_ui_constraint* constraints = get_constraints(t);
  struct List_ui_constraint* first_constraint = constraints;
  v2_i32 tsize = uitree_get_size(t);

  if (constraints == NULL) return;

  while (constraints != NULL) {
    if ((void*)constraints->next == (void*)first_constraint) {
      ERROR("Infinite constraint loop detected!");
      ERROR("  Type: %s", constraint_str[constraints->value.type]);
      exit(EXIT_FAILURE);
    }

    const constraintval_t con = constraints->value.constraint;

    switch (constraints->value.type) {
    case ui_constraint_width: {
      i32 ww = ui_size_to_pixel_ex(con.width.size, false, psize);
      if (con.width.size.type == ui_size_percent) ww -= 2 * get_padding(t);
      /* FIXME: Resizing the window makes some elements retain their
       * size on window resize.
       * This is fixed by replacing MAX(...) with just `ww`,
       * This in turn makes elements that are supposed to have some
       * relative width not take up the full width. */
      uitree_set_size(t, (v2_i32){.x = MAX(tsize.x, ww), .y = 0});
      tsize = uitree_get_size(t);
    } break;

    case ui_constraint_height: {
      i32 ww = ui_size_to_pixel_ex(con.height.size, true, psize);
      if (con.height.size.type == ui_size_percent) ww -= 2 * get_padding(t);
      /* FIXME: Same as above */
      uitree_set_size(t, (v2_i32){.y = MAX(tsize.y, ww), .x = 0});
      tsize = uitree_get_size(t);
    } break;

    case ui_constraint_horizontal_align: {
      const i32 size =
          ui_size_to_pixel_ex(con.horizontal_align.spacing, false, psize);
      i32 new_x = ppos.x;
      switch (con.horizontal_align.align) {
      case ui_constraint_horizontal_align_left:
        new_x += size + get_margin(t);
        break;
      case ui_constraint_horizontal_align_center:
        new_x += (psize.x / 2) - (tsize.x / 2);
        break;
      case ui_constraint_horizontal_align_right:
        new_x += psize.x - size - tsize.x - get_margin(t);
        break;
      }
      uitree_set_pos(t, (v2_i32){.x = new_x, .y = 0});
    } break;

    case ui_constraint_vertical_align: {
      i32 size = ui_size_to_pixel_ex(con.horizontal_align.spacing, true, psize);
      i32 new_y = ppos.y;
      switch (con.vertical_align.align) {
      case ui_constraint_vertical_align_top:
        new_y += size + get_margin(t);
        break;
      case ui_constraint_vertical_align_center:
        new_y += ((psize.y) / 2) - (tsize.y / 2);
        break;
      case ui_constraint_vertical_align_bottom:
        new_y += psize.y - size - tsize.y - get_margin(t);
        break;
      }
      uitree_set_pos(t, (v2_i32){.x = 0, .y = new_y});
    } break;

    case ui_constraint_left:
      uitree_set_pos(t, (v2_i32){.x = ppos.x + ui_size_to_pixel_ex(
                                                   con.left.pos, false, psize),
                                 .y = 0});
      break;

    case ui_constraint_top:
      uitree_set_pos(t, (v2_i32){.y = ppos.y + ui_size_to_pixel_ex(con.top.pos,
                                                                   true, psize),
                                 .x = 0});
      break;

    case ui_constraint_right:
      if (tsize.x > 0)
        uitree_set_pos(
            t, (v2_i32){.x = ppos.x + psize.x -
                             ui_size_to_pixel_ex(con.right.pos, false, psize) -
                             tsize.x,
                        .y = 0});
      break;

    case ui_constraint_bottom:
      if (tsize.y > 0)
        uitree_set_pos(
            t, (v2_i32){.y = ppos.y + psize.y -
                             ui_size_to_pixel_ex(con.bottom.pos, true, psize) -
                             tsize.y,
                        .x = 0});
      break;

    default:
      ERROR("Unknown constraint: %d", constraints->value.type);
      exit(EXIT_FAILURE);
      break;
    }

    constraints = constraints->next;
  }
}

void ui_rearrange_container(UITree_container* c, v2_i32 ppos, v2_i32 psize) {
  struct List_ui_constraint* constraints = c->constraints;
  struct List_ui_constraint* first_constraint = constraints;
  bool fitwidth = !uitree_has_constraint((UITree*)c, ui_constraint_width);
  bool fitheight = !uitree_has_constraint((UITree*)c, ui_constraint_height);

  if (fitwidth) c->w = 0;
  if (fitheight) c->h = 0;

  ui_constraints_apply((UITree*)c, ppos, psize);
  /* Apply constraints to children here? */
  {
    const v2_i32 pos =
        v2_i32_sub_i((v2_i32){.x = c->x, .y = c->y}, 0);  // c->margin);
    const v2_i32 size =
        v2_i32_sub_i((v2_i32){.x = c->w, .y = c->h},
                     (2 + MAX(c->children_len - 1, 0)) * c->padding);
    for (usize cc = 0; cc < c->children_len; cc++) {
      ui_rearrange(c->children[cc], pos, size);
    }
  }

  /* TODO: We want to update children regardless of whether we are doing a fit
   * on the size of children. Switch the conditions (fitwidth||fitheight) and
   * (children_len > 0); the size-fit doesn't matter if there's no children,
   * the childrens size only matters if we do a size-fit. */

  /* If we do not have a size > -1 we need to adapt the size to all children
   * and add a size constraint on the largest of one of them */
  if (fitwidth || fitheight) {

    i32 maxwidth = 2 * c->padding;
    i32 maxheight = maxwidth;

    if (c->children_len > 0) {
      if (c->children == NULL) {
        ERROR(
            "Container has children_len(%d) > 0 but children pointer is NULL!",
            c->children_len);
        exit(EXIT_FAILURE);
      }

      /* Calculate new posisitions and save the largest one */
      v2_i32 startpos = (v2_i32){.x = c->x + c->margin, .y = c->y + c->margin};
      v2_i32 startsize =
          (v2_i32){.x = c->w - (2 * c->padding), .y = c->h - (2 * c->padding)};

      for (usize cc = 0; cc < c->children_len; cc++) {
        const v2_i32 childsize = elem_size(c->children[cc]);
        i32 margin = (cc + 1 == c->children_len)
                         ? 0
                         : MAX(c->padding, get_margin(c->children[cc]));

        /* We just sum up in the direction we are going */
        if (c->direction == DIRECTION_HORIZONTAL) {
          maxwidth += childsize.x + margin;
          maxheight = MAX(childsize.y + 2 * c->padding, maxheight);

          startpos.x += childsize.x + margin;
          startsize.x -= childsize.x + margin;
        } else {
          maxwidth = MAX(childsize.x + 2 * c->padding, maxwidth);
          maxheight += childsize.y + margin;

          startpos.y += childsize.y + margin;
          startsize.y -= childsize.y + margin;
        }
      }

      struct List_ui_constraint* constraint_head = c->constraints;
      struct List_ui_constraint constraint_width;
      struct List_ui_constraint constraint_height;

      if (fitwidth) {
        /* Create new size constraint */
        constraint_width = (struct List_ui_constraint){
            .value =
                {
                    .type = ui_constraint_width,
                    .constraint.width =
                        {
                            .size = ui_size_pixel_new(maxwidth),
                        },
                },
            .next = constraint_head,
        };

        /* Prepend it to the list of constraints */
        c->constraints = &constraint_width;
        constraint_head = c->constraints;
      }
      if (fitheight) {
        constraint_height = (struct List_ui_constraint){
            .value =
                {
                    .type = ui_constraint_height,
                    .constraint.height =
                        {
                            .size = ui_size_pixel_new(maxheight),
                        },
                },
            .next = constraint_head,
        };
        /* Prepend it to the list of constraints */
        c->constraints = &constraint_height;
        // constraint_head = c->constraints;
        /* TODO: Convert to scrollable container if the height is
         * greater than some constant relative to the screen height */
      }

      /* Recurse with new constraints */
      ui_rearrange_container(c, ppos, psize);

      /* Replace the original set of constraints, as the new ones will be
       * out of scope once we return from the current call frame */
      c->constraints = first_constraint;

      /* Update child elements */
      /* This is 1:1 (almost) to the previous iteration of children */
      startpos = (v2_i32){c->x + c->padding, c->y + c->padding};
      startsize = (v2_i32){c->w - 2 * c->padding, c->h - 2 * c->padding};

      for (usize cc = 0; cc < c->children_len; cc++) {
        v2_i32 childsize = elem_size(c->children[cc]);
        i32 margin = MAX(c->padding, get_margin(c->children[cc]));

        v2_i32 startsize_2;
        if (c->direction == DIRECTION_HORIZONTAL) {
          startsize_2 = (v2_i32){childsize.x, startsize.y};
        } else {
          startsize_2 = (v2_i32){startsize.x, childsize.y};
        }
        ui_rearrange(c->children[cc], startpos, startsize_2);

        if (c->direction == DIRECTION_HORIZONTAL) {
          maxwidth += childsize.x + margin;
          maxheight = MAX(childsize.y, maxheight);

          startpos.x += childsize.x + margin;
          startsize.x -= childsize.x + margin;

        } else {
          maxwidth = MAX(childsize.x + 2 * c->padding, maxwidth);
          maxheight += childsize.y + margin;

          startpos.y += childsize.y + margin;
          startsize.y -= childsize.y + margin;
        }
      }
    }
  }
}

void ui_rearrange_button(UITree_button* t, v2_i32 ppos, v2_i32 psize) {
  t->x = ppos.x;
  t->y = ppos.y;

  ui_constraints_apply((UITree*)t, ppos, psize);
}

void ui_rearrange_title(UITree_title* t, v2_i32 ppos, v2_i32 psize) {
  /*Defaults to just sit in the top-left corner of the parent*/
  t->x = ppos.x;
  t->y = ppos.y;

  ui_constraints_apply((UITree*)t, ppos, psize);
}

void ui_rearrange_text(UITree_text* t, v2_i32 ppos, v2_i32 psize) {
  t->x = ppos.x;
  t->y = ppos.y;
  t->w = psize.x;
  t->h = psize.y;

  t->texture_size.x = psize.x;
  t->texture_size.y = psize.y;

  /* Free the old texture */
  /* Re-render the text */
  t->text_texture_index = engine_render_text(
      t->font_id, t->fg, (char*)t->text_original, &t->texture_size, true);

  t->w = t->texture_size.x;
  t->h = t->texture_size.y;

  ui_constraints_apply((UITree*)t, ppos, psize);
}

void ui_rearrange(UITree* root, v2_i32 ppos, v2_i32 psize) {
  if (root == NULL) {
    WARN("%s received a null pointer, ignoring", __func__);
    return;
  }

  switch (root->type) {
  case uitype_container:
    ui_rearrange_container(&root->container, ppos, psize);
    break;
  case uitype_button:
    ui_rearrange_button(&root->button, ppos, psize);
    break;
  case uitype_title:
    ui_rearrange_title(&root->title, ppos, psize);
    break;
  case uitype_text:
    ui_rearrange_text(&root->text, ppos, psize);
    break;
  default:
    if (root->type >= uitype_MAX) {
      ERROR("Unknown uitype: %d", root->type);
      exit(EXIT_FAILURE);
    } else {
      WARN("Rendering not implemented for %s", uitype_str[root->type]);
    }
    break;
  }
}

void ui_resolve_constraints(void) {
  u64* i = NULL;
  struct btree_iter_t* it = NULL;

  /*absolutely bonkers way to do this*/
  if (GLOBAL_UIROOTS == NULL) {
    WARN("UIROOTS not initialized");
  }

  it = btree_iter_t_new(GLOBAL_UIROOTS);

  while ((i = btree_iter(GLOBAL_UIROOTS, it)) != NULL) {
    ui_rearrange((UITree*)*i, (v2_i32){0, 0},
                 GLOBAL_PLATFORM->window->windowsize);
  }
  free(it);
}

void ui_container_attach(UITree* root, UITree* child) {
  if (root == NULL) {
    WARN("%s received a null pointer", __func__);
    return;
  }
  if (root->type != uitype_container) {
    WARN("Trying to attach a child to a non-container UI element");
    return;
  }

  UITree_container* c = &(root->container);

  static const usize size_increment = 32;
  static const usize uitree_sz = sizeof(UITree*);

  if (c->children == NULL) {
    /* Allocate space for children */
    c->children = malloc(uitree_sz * size_increment);

  } else if ((c->children_len + 1) * uitree_sz >= c->children_size) {
    /* If there's not enough room for more children we will need to
     * reallocate some more memory */
    c->children =
        realloc(c->children, (c->children_len + size_increment) * uitree_sz);
    c->children_size += size_increment * uitree_sz;
  }

  /* Finally: attach the new element */
  c->children[c->children_len++] = child;

  /* Since the child is no longer a part of the root-ui we can delete it from
   * the btree-mapping */
  {
    u64 tmp = (u64)child;
    if (!btree_delete(GLOBAL_UIROOTS, &tmp)) {
      WARN("Could not find child %p in global lookup table", child);
      btree_print(GLOBAL_UIROOTS, &pointer_print);
    }
  }
}

void ui_button_enable(UITree_button* b) {
  b->enabled = true;
  SDL_Texture* t = ((struct Resources*)GLOBAL_PLATFORM->data)
                       ->textures[b->text_texture_index]
                       ->texture;
  SDL_SetTextureColorMod(t, 0xff, 0xff, 0xff);
}

void ui_button_disable(UITree_button* b) {
  b->enabled = false;

  SDL_Texture* t = ((struct Resources*)GLOBAL_PLATFORM->data)
                       ->textures[b->text_texture_index]
                       ->texture;
  SDL_SetTextureColorMod(t, 0x7f, 0x7f, 0x7f);
}

void ui_set_default_colors(Engine_color fg, Engine_color bg) {
  DEFAULT_FG = fg;
  DEFAULT_BG = bg;
}

void ui_set_default_padding(i32 padding) { DEFAULT_PADDING = padding; }

Engine_color ui_get_default_fg(void) { return DEFAULT_FG; }
Engine_color ui_get_default_bg(void) { return DEFAULT_BG; }

bool check_point_in_rect(const v2_i32* p, const SDL_Rect* r) {
  if (p->x > r->x && p->x < r->x + r->w && p->y > r->y && p->y < r->y + r->h)
    return true;
  return false;
}

u64 ui_check_click_internal(UITree* root, v2_i32 pos) {
  v2_i32 ps = get_pos(root);
  v2_i32 sz = get_size(root);
  SDL_Rect elem_rect = {.x = ps.x, .y = ps.y, .w = sz.x, .h = sz.y};
  /* */
  switch (root->type) {
  case uitype_container:
    if (root->container.children != NULL && root->container.children_len > 0) {
      const UITree_container* container = &root->container;

      if (check_point_in_rect(&pos, &elem_rect)) {

        for (usize i = 0; i < container->children_len; i++) {
          u64 res = ui_check_click_internal(container->children[i], pos);
          if (res != (u64)ELEM_NOT_FOUND) {
            return res;
          }
        }
      }
    }
    break;
  case uitype_button:
    if (root->button.enabled) {
      WARN("Checking button");
      elem_rect.h += root->button.padding * 2;
      elem_rect.w += root->button.padding * 2;
      WARN("Button specs:	<%d,%d>+%d,%d", elem_rect.w, elem_rect.h,
           elem_rect.x, elem_rect.y);
      if (check_point_in_rect(&pos, &elem_rect)) {
        return root->button.id;
      }
    }
    break;
  case uitype_title:
  case uitype_text:
  default:
    break;
  }
  return ELEM_NOT_FOUND;
}

u64 ui_check_click(UITree* root) {
  if (!GLOBAL_PLATFORM->mouse_lclick) return NO_CLICK;

  v2_i32 pos = GLOBAL_PLATFORM->mouse_pos;

  INFO("Mousepos:	<%d,%d>", pos.x, pos.y);

  return ui_check_click_internal(root, pos);
}
