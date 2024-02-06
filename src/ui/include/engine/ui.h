#ifndef ENGINE_UI_H
#define ENGINE_UI_H

#include <engine/core/types.h>
#include <engine/utils/list.h>
#include <engine/utils/vector.h>

#define DIRECTION_HORIZONTAL true
#define DIRECTION_VERTICAL false

/* Ui positioning constraints */
typedef enum {
  ui_size_pixel,
  ui_size_percent,
} ui_size_t;

typedef union {
  ui_size_t type;
  struct {
    ui_size_t type;
    i32 value;
  } pixel;
  struct {
    ui_size_t type;
    f32 value;
  } percent;
} ui_size;

typedef enum {
  ui_constraint_width,
  ui_constraint_height,

  ui_constraint_horizontal_align,
  ui_constraint_vertical_align,

  ui_constraint_left,
  ui_constraint_right,
  ui_constraint_top,
  ui_constraint_bottom,
} ui_constraint_t;

typedef enum {
  ui_constraint_vertical_align_top,
  ui_constraint_vertical_align_center,
  ui_constraint_vertical_align_bottom,
} ui_constraint_vertical_align_t;

typedef enum {
  ui_constraint_horizontal_align_left,
  ui_constraint_horizontal_align_center,
  ui_constraint_horizontal_align_right,
} ui_constraint_horizontal_align_t;

typedef union {
  struct {
    ui_size size;
  } width;
  struct {
    ui_size size;
  } height;

  /* spacing is the padding between the aligned edge and the container itself */
  struct {
    ui_constraint_horizontal_align_t align;
    ui_size spacing;
  } horizontal_align;
  struct {
    ui_constraint_vertical_align_t align;
    ui_size spacing;
  } vertical_align;

  struct {
    ui_size pos;
  } left;
  struct {
    ui_size pos;
  } top;
  struct {
    ui_size pos;
  } right;
  struct {
    ui_size pos;
  } bottom;
} constraintval_t;

#define CONSTRAINT_POSITION(axis, alignment_sub, margin, cnext)                \
  {                                                                            \
    .value =                                                                   \
        {                                                                      \
            .type = ui_constraint_##axis##_align,                              \
            .constraint.axis##_align =                                         \
                {                                                              \
                    .align = ui_constraint_##axis##_align_##alignment_sub,     \
                    .spacing = margin,                                         \
                },                                                             \
        },                                                                     \
    .next = cnext,                                                             \
  }

#define CONSTRAINT_SIZE(c_type, csize, cnext)                                  \
  {                                                                            \
    .value =                                                                   \
        {                                                                      \
            .type = ui_constraint_##c_type,                                    \
            .constraint.c_type =                                               \
                {                                                              \
                    .size = csize,                                             \
                },                                                             \
        },                                                                     \
    .next = cnext,                                                             \
  }

typedef struct {
  ui_constraint_t type;
  constraintval_t constraint;
} ui_constraint;

typedef DEFINE_LLIST(ui_constraint);

enum uitype {
  uitype_container,
  uitype_button,
  uitype_title,
  uitype_text,

  uitype_MAX,
};

typedef struct UITree_container {
  enum uitype type;
  bool visible;
  bool direction;
  i32 x, y, w, h;
  i32 padding;
  i32 margin;

  Engine_color fg;
  Engine_color bg;
  Engine_color bordercolor;

  usize children_len;  /* Number of children */
  usize children_size; /* Memory size */
  union UITree** children;
  struct List_ui_constraint* constraints;
} UITree_container;

typedef struct UITree_button {
  enum uitype type;

  bool enabled;
  u64 id;

  i32 x, y, w, h;
  i32 padding;
  i32 margin;

  Engine_color fg;
  Engine_color bg;

  i32 font_id;
  const char* text_original;
  u64 text_texture_index;
  v2_i32 texture_size;
  struct List_ui_constraint* constraints;
} UITree_button;

typedef struct UITree_title {
  enum uitype type;
  i32 x, y, w, h;

  i32 padding;
  i32 margin;

  Engine_color fg;

  i32 font_id;
  const char* text_original;
  u64 text_texture_index;
  v2_i32 texture_size;
  struct List_ui_constraint* constraints;
} UITree_title;

typedef struct UITree_text {
  enum uitype type;
  i32 x, y, w, h;

  i32 padding;
  i32 margin;

  Engine_color fg;

  i32 font_id;
  const char* text_original;
  u64 text_texture_index;
  v2_i32 texture_size;
  struct List_ui_constraint* constraints;
} UITree_text;

typedef union UITree {
  enum uitype type;

  UITree_container container;
  UITree_button button;
  UITree_title title;
  UITree_text text;
  struct List_ui_constraint constraints;
} UITree;

/* Sizes */
f32 ui_size_pixel_to_percent_width(i32 pixels);
f32 ui_size_pixel_to_percent_height(i32 pixels);
f32 ui_size_pixel_to_percent(i32 pixels);
i32 ui_size_percent_width_to_pixel(f32 percent);
i32 ui_size_percent_height_to_pixel(f32 percent);
i32 ui_size_percent_to_pixel(f32 percent);
i32 ui_size_to_pixel(ui_size s);

#define ui_size_percent_new(p)                                                 \
  {                                                                            \
    .percent = {.type = ui_size_percent, .value = p }                          \
  }
#define ui_size_pixel_new(p)                                                   \
  {                                                                            \
    .pixel = {.type = ui_size_pixel, .value = p }                              \
  }

/* Constructors */
UITree* ui_container(bool direction, struct List_ui_constraint* constraints);
UITree* ui_button(u64 id, i32 font_id, char* text,
                  struct List_ui_constraint* constraints);
UITree* ui_title(i32 font_id, const char* text,
                 struct List_ui_constraint* constraints);
UITree* ui_text(i32 font_id, const char* text,
                struct List_ui_constraint* constraints);

/* Extended Constructors */
UITree* ui_container_new_ex(Engine_color fg, Engine_color bg,
                            Engine_color border, bool direction,
                            struct List_ui_constraint* constraints);

/* Destructors */
void clear_ui(void);
void uitree_free(UITree* t);

/* Manipulation */
void ui_container_attach(UITree* container, UITree* child);

void ui_resolve_constraints(void);

void ui_button_enable(UITree_button* b);
void ui_button_disable(UITree_button* b);

/* Tweaking default colors */
void ui_set_default_colors(Engine_color fg, Engine_color bg);
void ui_set_default_padding(i32 padding);
Engine_color ui_get_default_fg(void);
Engine_color ui_get_default_bg(void);

#define ELEM_NOT_FOUND (-1)
#define NO_CLICK (-2)
/* Returns -1 if not found */
u64 ui_check_click(UITree* root);

#endif
