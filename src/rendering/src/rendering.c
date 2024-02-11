#include <stdio.h>
#include <string.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>


#define ENGINE_INTERNALS
#include <engine/engine.h>
#include <engine/rendering/rendering.h>

/* Extern globals */
extern Platform* GLOBAL_PLATFORM;

/* Globals */
#define drawcall_limit (64 * 1024)
RenderDrawCall drawcalls[drawcall_limit];
i32 drawcall_len = 0;

//struct RenderObject;

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
  GladGLContext *gl = w->context;
  const f32 ratio = (float)w->windowsize[0] / (float)w->windowsize[1];

  for (i32 i = 0; i < drawcall_len; i++) {
    RenderDrawCall dc = drawcalls[i];
    switch (dc.type) {
//    case RenderDrawCallType_Sprite: {
//#ifdef _DEBUG
//      if (dc.data.sprite.sprite == NULL) {
//        WARN("Sprite %lx in drawcall %d/%d had NULL reference",
//             dc.data.sprite.sprite, i, drawcall_len);
//        __asm__("int3;");
//
//        drawcall_len = 0;
//        exit(EXIT_FAILURE);
//      }
//#endif
//      Sprite s = *dc.data.sprite.sprite;
//      Texture* t =
//          ((struct Resources*)GLOBAL_PLATFORM->data)->textures[s.texture_id];
//      i32 ts = t->tilesize;
//    } break;


    case RenderDrawCallType_Model: {
#ifdef _DEBUG
      if (dc.data.sprite.sprite == NULL) {
        WARN("Sprite %lx in drawcall %d/%d had NULL reference",
             dc.data.sprite.sprite, i, drawcall_len);
        __asm__("int3;");

        drawcall_len = 0;
        exit(EXIT_FAILURE);
      }
#endif
      RenderObject* o = dc.data.model.model;
      vec3 pos;
      glm_vec3_copy(dc.data.model.pos, pos);
      gl->UseProgram(o->shader.program);





      {
        vec3 cam = {3., 2., 3.}; // perspective
        mat4 per; // perspective
        mat4 v; // view
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        mat4 modelviewprojection;
        const float sz = 10.f;

        //LOG("<%.1f, %.1f, %.1f>", pos[0], pos[1], pos[2]);
        model[3][0] = pos[0];
        model[3][1] = pos[1];
        model[3][2] = pos[2];

        //glm_perspective(45.f , 600.f / 400.f, 0.1, 100.0f, per);
        glm_ortho(-sz * ratio, sz * ratio, -sz, sz, -sz, sz, per);

        /* Lookat zero should be changed to whatever later */
        glm_lookat(cam, GLM_VEC3_ZERO, GLM_YUP, v);

        { mat4 t;
          //modelviewprojection = p * v * model
          glm_mat4_mul(v, model, t);
          // TODO: Remove this later
          glm_rotate_at(t, (vec3){0,0,0}, get_time() / 2.f, GLM_YUP); //, (vec3)({0,1,0}));

          glm_mat4_mul(per, t, modelviewprojection);
        }

        // TODO: Do this only once during initialization
        u32 matrix = gl->GetUniformLocation(o->shader.program, "MVP");

        gl->UniformMatrix4fv(matrix, 1, GL_FALSE, &modelviewprojection[0][0]);
      }


      gl->EnableVertexAttribArray(0);
      gl->BindBuffer(GL_ARRAY_BUFFER, o->vbo);
      gl->VertexAttribPointer(
          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
          3,                  // size
          GL_FLOAT,           // type
          GL_FALSE,           // normalized?
          0,                  // stride
          (void*)0            // array buffer offset
          );

      //// Do the color buffer (?)
      //gl->EnableVertexAttribArray(1);
      //gl->BindBuffer(GL_ARRAY_BUFFER, p->testobject->col);
      //gl->VertexAttribPointer(
      //    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      //    3,                                // size
      //    GL_FLOAT,                         // type
      //    GL_FALSE,                         // normalized?
      //    0,                                // stride
      //    (void*)0                          // array buffer offset
      //    );




      //gl->UseProgram(p->testobject->shaderprogram);
      //// Draw the triangle !
      gl->DrawArrays(GL_TRIANGLES, 0, 3*12); // Starting from vertex 0; 3 vertices total -> 1 triangle

      gl->DisableVertexAttribArray(0);
      //gl->DisableVertexAttribArray(1);













    } break;
    default:
      break;
    }
  }

  drawcall_len = 0;

  glfwSwapBuffers(w->window);
}

void drawcall_reset(void) { drawcall_len = 0; }

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
