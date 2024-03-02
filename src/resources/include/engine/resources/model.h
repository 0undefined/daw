#ifndef ENGINE_RESOURCES_MODEL_H
#define ENGINE_RESOURCES_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <engine/core/types.h>
#include <cglm/cglm.h>

typedef enum {
  Model_error,
  Model_obj,
} ModelType;

typedef struct {
  ModelType format;

  u32 m_uiVAO;
  u32 m_uiVBO;
  u32 m_uiIBO;
  unsigned m_uiNumIndices;
} Model;

#include <engine/resources.h>
Model load_model(const Asset_ModelSpec *restrict ms);

#ifdef __cplusplus
}
#endif
#endif
