#include <engine/core/logging.h>
#include <engine/resources/model.h>

Model load_model(const Asset_ModelSpec *restrict ms) {
  ModelType format = Model_error;

  isize vertices_len = 0;
  vec3 *vertices = malloc(sizeof(vec3) * 512);
  usize *vertices_idx = malloc(sizeof(usize) * 512);

  isize uvs_len = 0;
  vec2 *uvs = malloc(sizeof(vec2) * 512);
  usize *uvs_idx = malloc(sizeof(usize) * 512);

  isize normals_len = 0;
  vec3 *normals = malloc(sizeof(vec3) * 512);
  usize *normals_idx = malloc(sizeof(usize) * 512);

  // For now, just default to obj
  format = Model_obj;

  //FILE* f = fopen(ms->path, "r");
  //if (f == NULL) {
  //  ERROR("Failed to load file " TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET, ms->path);
  //  return (Model){.format = format};
  //}

  //// Start the import on the given file with some example postprocessing
  //// Usually - if speed is not the most important aspect for you - you'll t
  //// probably to request more postprocessing than we do in this example.
  //const struct aiScene* scene = aiImportFile( pFile,
  //  aiProcess_CalcTangentSpace       |
  //  aiProcess_Triangulate            |
  //  aiProcess_JoinIdenticalVertices  |
  //  aiProcess_SortByPType);

  //// If the import failed, report it
  //if( NULL == scene) {
  //  DoTheErrorLogging( aiGetErrorString());
  //  return false;
  //}

  //// Now we can access the file's contents
  //DoTheSceneProcessing( scene);

  //// We're done. Release all resources associated with this import
  //aiReleaseImport( scene);
  //return true;

  {
    Model m = {};
    m.format = format;

    // TODO add index array

    return m;
  }
}
