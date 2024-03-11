#include <stdio.h>
#include <string.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>


#include <engine/engine.h>
#include <engine/utils.h>
#include <engine/rendering/rendering.h>


/* Extern globals */
extern Platform* GLOBAL_PLATFORM;

/* Globals */
#define drawcall_limit (64 * 1024)
#define batch_limit 64
RenderDrawCall drawcalls[drawcall_limit];
RenderBatch render_batches[batch_limit];
i32 drawcall_len = 0;

// Count is the number of models to reserve space for
RenderBatch renderbatch_new(const isize count) {
  GladGLContext* restrict gl = GLOBAL_PLATFORM->window->context;

  u32 vbo = 0;
  gl->CreateBuffers(1, &vbo);
  gl->BindBuffer(GL_ARRAY_BUFFER, vbo);
  gl->BufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
  gl->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

  return (RenderBatch){
    .msize = sizeof(RenderModel) * count,
    .mcount = 0,
    .vsize = 0,
    .vcount = 0,
    .models = (RenderModel*)malloc(sizeof(RenderModel) * count),
    .vertexbuffer = vbo,
    .vertices = NULL,
  };
}

RenderBatch renderbatch_add(RenderBatch b, const isize count, f32* vertices) {

  RenderModel m = {.count = count, .vertices = vertices};

  if (b.models == NULL) {
    b = renderbatch_new(64);
  }

  if ((b.mcount + 1) * sizeof(RenderModel) > b.msize) {
    // grow the size
    b.models = realloc(b.models, b.msize * 2);
    b.msize *= 2;
  }

  // Add the model to the tracking list
  b.models[b.mcount] = m;
  b.mcount++;

  // Update the vertex buffer
  if (b.vertices == NULL) {
    isize newsz = sizeof(f32) * count;
    b.vertices = malloc(newsz);
    b.vsize = newsz;

  } else if (sizeof(f32) * (b.vcount + count) > b.vsize) {
    // Grow the vertex buffer size
    isize newsz = b.vsize * 2;
    while (newsz < sizeof(f32) * count + b.vsize) {
      newsz *= 2;
    }

    b.vertices = realloc(&b.vertices, newsz);
    b.vsize = newsz;
  }

  // Copy the new buffer data
  memcpy(&b.vertices[b.vcount], vertices, count * sizeof(f32));
  b.vcount += count;

  const GladGLContext *restrict gl = GLOBAL_PLATFORM->window->context;
  gl->BindBuffer(GL_ARRAY_BUFFER, b.vertexbuffer);
  gl->BufferData(GL_ARRAY_BUFFER, b.vcount, b.vertices, GL_DYNAMIC_DRAW);
  gl->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

  return b;
}

RenderBatch renderbatch_add_ex(RenderBatch b, const isize num, const isize *count, f32** vertices) {

  RenderModel *mm = NULL;

  if (b.models == NULL) {
    b = renderbatch_new(num);
  }

  if ((b.mcount + num) * sizeof(RenderModel) > b.msize) {
    // grow the size
    b.models = realloc(b.models, (b.mcount + num) * sizeof(RenderModel));
    b.msize += num * sizeof(RenderModel);
  }

  // Add the models to the tracking list
  isize vcount = 0;
  for (isize i = 0; i < num; i++) {
    b.models[b.mcount + i] = (RenderModel){.count = count[i], .vertices = vertices[i]};
    vcount = count[i];
  }

  b.mcount += num;

  // Update the vertex buffer
  if (b.vertices == NULL) {
    isize newsz = sizeof(f32) * vcount;
    b.vertices = malloc(newsz);
    b.vsize = newsz;

  } else if (sizeof(f32) * (b.vcount + vcount) > b.vsize) {
    // Grow the vertex buffer size by doubling it (a couple of times)
    isize newsz = b.vsize * 2;
    while (newsz < sizeof(f32) * vcount + b.vsize) {
      newsz *= 2;
    }

    b.vertices = realloc(&b.vertices, newsz);
    b.vsize = newsz;
  }

  // Copy the new buffer data
  for (isize i = 0; i < num; i++) {
    memcpy(&b.vertices[b.vcount], vertices[i], count[i] * sizeof(f32));
    b.vcount += count[i];
  }

  const GladGLContext *restrict gl = GLOBAL_PLATFORM->window->context;
  gl->BindBuffer(GL_ARRAY_BUFFER, b.vertexbuffer);
  gl->BufferData(GL_ARRAY_BUFFER, b.vcount, b.vertices, GL_DYNAMIC_DRAW);
  gl->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

  return b;
}

void renderbatch_update(RenderBatch b, f32** vertices) {
  // TODO fixup this mess
  isize acc = 0;
  for (isize i = 0; i < b.mcount; i++) {
    const isize sz = b.models[i].count * sizeof(f32);
    memcpy(&b.vertices[acc], b.models[i].vertices, sz);
    acc += sz;
  }
}

/* Implementations */

/* Clear the screen,
 * To be used inbetween draw calls */
void render_begin(Window* w) {
  glfwMakeContextCurrent(w->window);
  ((GladGLContext*)(w->context))->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_present(Window* w) {
  /* This is GL specific, TODO: move the GL-specific code elsewhere. Maybe make
   * this whole present GL specific? assign it as a fn ptr in the Window struct? */
  GladGLContext *restrict gl = w->context;
  Camera c = *GLOBAL_PLATFORM->cam;
  const f32 ratio = (float)w->windowsize[0] / (float)w->windowsize[1];

  mat4 view; // view
  vec3 angle; // viewing angle / direction of the camera

  glm_vec3_sub(c.pos, c.dir, angle);
  glm_lookat(c.pos, angle, GLM_YUP, view);

  for (i32 i = 0; i < drawcall_len; i++) {
    RenderDrawCall dc = drawcalls[i];
    switch (dc.type) {
    case RenderDrawCallType_Sprite: {
      // TODO render a quad
    } break;

    case RenderDrawCallType_Batch: {
      //
      const f64 t = get_time();

      const RenderBatch* b = &render_batches[dc.data.batch.id];
      const RenderObject* o = dc.data.batch.model;
      //vec3 pos;
      //glm_vec3_copy(dc.data.model.pos, pos);

      gl->UseProgram(o->shader.program);
      // TODO: Use texture atlas
      gl->ActiveTexture(GL_TEXTURE0);
      gl->BindTexture(GL_TEXTURE_2D, o->texture);

      {
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        mat4 modelviewprojection;

        //model[3][0] = pos[0];
        //model[3][1] = pos[1];
        //model[3][2] = pos[2];

        {  // modelviewprojection = p * view * model
          mat4 t;
          glm_mat4_mul(view, model, t);
          glm_mat4_mul(c.per, t, modelviewprojection);
        }

        // TODO: Do this only once during initialization
        u32 matrix = o->mvp;

        gl->UniformMatrix4fv(matrix, 1, GL_FALSE, &modelviewprojection[0][0]);
      }

      // TODO the buffers need to be abstracted a bit more
      gl->BindVertexArray(o->vao);

      for (usize i = 0; i < o->buffer_len; i++) {
        gl->EnableVertexAttribArray(i);
        gl->BindBuffer(GL_ARRAY_BUFFER, o->buffer[i].buffername);
        gl->VertexAttribPointer(
            i,                  // ...
            o->buffer[i].m,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
      }

      // Draw the model !
      // TODO: Use DrawElements and an index buffer!
      gl->DrawArrays(GL_TRIANGLES, 0, 3*b->vcount); // Starting from vertex 0; 3 vertices total -> 1 triangle

      for (usize i = 0; i < o->buffer_len; i++) {
        gl->DisableVertexAttribArray(i);
      }

      //gl->DisableVertexAttribArray(1);
      gl->BindVertexArray(0);

      if (i == 8) {
        printf("\r obj: %.3f", (double)(get_time() - t) * 1000.);
      }

    } break;

    case RenderDrawCallType_Model: {

      // bind shader program
      // - set uniforms
      // bind vertex array
      // bind index buffer


      f64 t = get_time();
      RenderObject* o = dc.data.model.model;
      vec3 pos;
      glm_vec3_copy(dc.data.model.pos, pos);

      gl->UseProgram(o->shader.program);
      // TODO: Use texture atlas
      gl->ActiveTexture(GL_TEXTURE0);
      gl->BindTexture(GL_TEXTURE_2D, o->texture);

      {
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        mat4 modelviewprojection;

        model[3][0] = pos[0];
        model[3][1] = pos[1];
        model[3][2] = pos[2];

        {  // modelviewprojection = p * view * model
          mat4 t;
          glm_mat4_mul(view, model, t);
          glm_mat4_mul(c.per, t, modelviewprojection);
        }

        // TODO: Do this only once during initialization
        u32 matrix = o->mvp;

        gl->UniformMatrix4fv(matrix, 1, GL_FALSE, &modelviewprojection[0][0]);
      }

      // TODO the buffers need to be abstracted a bit more
      gl->BindVertexArray(o->vao);

      for (usize i = 0; i < o->buffer_len; i++) {
        gl->EnableVertexAttribArray(i);
        gl->BindBuffer(GL_ARRAY_BUFFER, o->buffer[i].buffername);
        gl->VertexAttribPointer(
            i,                  // ...
            o->buffer[i].m,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
      }

      // Draw the model !
      // TODO: Use DrawElements and an index buffer!
      gl->DrawArrays(GL_TRIANGLES, 0, 3*12); // Starting from vertex 0; 3 vertices total -> 1 triangle

      for (usize i = 0; i < o->buffer_len; i++) {
        gl->DisableVertexAttribArray(i);
      }

      //gl->DisableVertexAttribArray(1);
      gl->BindVertexArray(0);

      if (i == 8) {
        printf("\r obj: %.3f", (double)(get_time() - t) * 1000.);
      }














    } break;
    default:
      break;
    }
  }

  drawcall_len = 0;

  glfwSwapBuffers(w->window);
}

void drawcall_reset(void) { drawcall_len = 0; }

void r_perspective(f32 fov, Camera *c) {
  const f64 ratio = (f64)GLOBAL_PLATFORM->window->windowsize[0]
                  / (f64)GLOBAL_PLATFORM->window->windowsize[1];

  c->type = Camera_Perspective;
  c->parameters.perspective.fov = fov;

  glm_perspective(glm_rad(fov), ratio, 0.1, 100.0f, c->per);
}

void r_perspective_ortho(f32 sz, Camera *c) {
  const f64 ratio = (f64)GLOBAL_PLATFORM->window->windowsize[0]
                  / (f64)GLOBAL_PLATFORM->window->windowsize[1];

  c->type = Camera_Orthogonal;
  c->parameters.orthogonal.sz = sz;

  glm_ortho(-sz * ratio, sz * ratio, -sz, sz, -sz * 10.f, sz * 10.f, c->per);
}

void r_set_camera(Camera* c) {
  GLOBAL_PLATFORM->cam = c;
}

void engine_window_resize_pointers(i32* w, i32* h) {
  //GLOBAL_PLATFORM->window->game_w = w;
  //GLOBAL_PLATFORM->window->game_h = h;
}

void engine_window_resize_pointers_reset(void) {
  //GLOBAL_PLATFORM->window->game_w = NULL;
  //GLOBAL_PLATFORM->window->game_h = NULL;
}

void engine_draw_sprite(Sprite* s, v2_i32* pos, f32 scale) {
  if (drawcall_len + 1 >= drawcall_limit) return;
#ifdef _DEBUG
  if (s == NULL) __asm__("int3;");
#endif
  drawcalls[drawcall_len++] =
      (RenderDrawCall){.type = RenderDrawCallType_Sprite,
                       .data.sprite = {
                           .sprite = s,
                           .x = pos->x,
                           .y = pos->y,
                           .scale = scale,
                           //.mod = {0xFF, 0xFF, 0xFF, 0xFF},
                       }};
}

void engine_draw_sprite_ex(Sprite* s, v2_i32* pos, f32 scale,
                           Engine_color colormod) {
  if (drawcall_len + 1 >= drawcall_limit) return;
#ifdef _DEBUG
  if (s == NULL) __asm__("int3;");
#endif
  drawcalls[drawcall_len++] = (RenderDrawCall){
      .type = RenderDrawCallType_Sprite,
      .data.sprite = {
          .sprite = s,
          .x = pos->x,
          .y = pos->y,
          .scale = scale,
          //.mod = {colormod.r, colormod.g, colormod.b, colormod.a},
      }};
}

void engine_draw_model(RenderObject* o, vec3 pos) {
  if (drawcall_len + 1 >= drawcall_limit) return;
#ifdef _DEBUG
  if (o == NULL) __asm__("int3;");
#endif
  RenderDrawCall dc = {
      .type = RenderDrawCallType_Model,
      .data.model = {
          .model = o,
          .scale = 1.f,
      }};


  glm_vec3_copy(pos, dc.data.model.pos);

  drawcalls[drawcall_len++] = dc;
}

Sprite sprite_new(u64 tid, u8 coord) {
  const i32 ts = 16;
  // FIXME; used to be
  //((struct Resources*)GLOBAL_PLATFORM->data)->textures[tid]->tilesize;
  return (Sprite){
    .texture_id = tid,
      (v2_i32){
        .x = ts * (coord & 0x0F),
        .y = ts * ((coord & 0xF0) >> 4),
      }};
}

Texture createTextureFromImageData(unsigned char* image_data, i32 width, i32 height) {
  Window* restrict w = GLOBAL_PLATFORM->window;
  Texture t;
  t.width = width;
  t.height = height;

  if (w->renderer != WINDOW_RENDERER_OPENGL) {
    ERROR("createTextureFromImageData not implemented for chosen renderer!");
    return (Texture){.id = 0, .width = 0, .height = 0};
  }

  const GladGLContext* gl = w->context;

  gl->GenTextures(1, &t.id);
  gl->BindTexture(GL_TEXTURE_2D, t.id);

  gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image_data);

  gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  gl->BindTexture(GL_TEXTURE_2D, 0);

  return t;
}
