#include <string.h>

#include <engine/core/types.h>
#include <engine/core/logging.h>
#include <engine/core/platform.h>
#include <engine/resources.h>

extern Platform* GLOBAL_PLATFORM;

extern const char* ShaderType_str[];

void* get_asset(Resources* r, u32 idx) {
  switch (r->assets[idx].type) {
  case Asset_shader:
    // The shaders used to compile shader programs are not preserved.
    WARN("We don't store pure shaders");
    break;

  case Asset_shaderprog:
    LOG("Idx: r->get[idx] = %d", r->get[idx]);
    LOG("Ptr: &r->shader[r->get[idx]] = %z", &r->shader[r->get[idx]]);
    return &r->shader[r->get[idx]];

  case Asset_texture:
  case Asset_error:
  case Asset_audio:
  case Asset_font:
  default:
    ERROR("Asset type Not implemented");
    break;
  }
  return NULL;
}

i32 resources_load(Resources *resources) {
  resources->get = calloc(resources->assets_len, sizeof(usize));
  isize audio_len = 0;
  isize font_len = 0;
  isize shader_len = 0;
  isize shaderprog_len = 0;
  isize texture_len = 0;

  isize i = 0;

  for (i = 0; i < resources->assets_len; i++) {
    isize idx = 0;

    switch (resources->assets[i].type) {
      case Asset_audio:      idx = audio_len++;      WARN("Audio resource type not implemented!"); break;
      case Asset_font:       idx = font_len++;       WARN("Font resource type not implemented!"); break;
      case Asset_shader:     idx = shader_len++;     break;
      case Asset_shaderprog: idx = shaderprog_len++; break;
      case Asset_texture:    idx = texture_len++;    WARN("Texture resource type not implemented!");break;

      case Asset_error:
      default:
        ERROR("Unknown resource type!");
        exit(EXIT_FAILURE);
        break;
    }

    resources->get[i] = idx;
  }

  //resources->audio = calloc(audio_len, sizeof());
  //resources->font = calloc(font_len, sizeof());
  resources->shader = calloc(shaderprog_len, sizeof(Shader));
  //resources->texture = calloc(texture_len, sizeof());

  Shader* imm_shader = calloc(shader_len, sizeof(Shader));

  audio_len = 0;
  font_len = 0;
  shader_len = 0;
  shaderprog_len = 0;
  texture_len = 0;

  for (i = 0; i < resources->assets_len; i++) {
    isize idx = 0;

    switch (resources->assets[i].type) {
    case Asset_audio:
        //resources->audio_len++;
        WARN("Audio resource type not implemented!");
        break;
    case Asset_font:
        //resources->font_len++;
        WARN("Font resource type not implemented!");
        break;
    case Asset_shader: {
        ShaderType t =
            guess_shadertype_from_filename(resources->assets[i].shader.path);
        const Shader s = compile_shader(resources->assets[i].shader.path, t);
        LOG("Compiled %s! (%s)", resources->assets[i].shader.path, ShaderType_str[t]);
        imm_shader[shader_len++] = s;
    } break;
    case Asset_shaderprog: {
        const isize sz = resources->assets[i].shaderprog.shader_len;
        Shader* shaders = calloc(sz, sizeof(Shader));

        for (int j = 0; j < sz; j++) {
          //DEBUG("shader[%d] = %d\n", j, imm_shader[resources->assets[i].shaderprog.shader[j]].program);
          //shaders[j] = imm_shader[resources->assets[i].shaderprog.shader[j]];
          shaders[j] = imm_shader[j];
          DEBUG("shader[%d] = %d -- %s\n", j, shaders[j].program, ShaderType_str[shaders[j].type]);
        }
        const Shader s = compose_shader(shaders, sz);
        DEBUG("shader = %d -- %s\n", s.program, ShaderType_str[s.type]);

        resources->shader[resources->shader_len++] = s;
    } break;
    case Asset_texture:
        texture_len++;
        WARN("Texture resource type not implemented!");
        break;

    case Asset_error:
    default:
        ERROR("Unknown resource type!");
        exit(EXIT_FAILURE);
        break;
    }

    resources->get[i] = idx;
  }

  free(imm_shader);

  return 0;
}

isize resource_make_global(isize resource_id) {
  ERROR("`resource_make_global` Not implemented");
  return -1;
}
