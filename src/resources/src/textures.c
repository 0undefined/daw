#include <engine/core/logging.h>
#include <engine/resources.h>
#include <stb/stb_image.h>

/* Uses stb_image to load an image, and passes it to the renderer for
 * backend-specific texture creation. */
Texture load_texture(const Asset_TextureSpec *restrict ts) {
  int width;
  int height;
  int components_per_pixel;
  unsigned char* img;
  Texture t;
  const Texture err = (Texture){.id = 0, .width = 0, .height = 0};

  if (ts == NULL) {
    ERROR("Invalid Asset_TextureSpec\n");
    return err;
  }

  if (ts->path == NULL) {
    ERROR("Missing path in Asset_TextureSpec\n");
    return err;
  }

  img = stbi_load(ts->path, &width, &height, &components_per_pixel, 0);

  if (img == NULL) {
    ERROR("Failed to load image %s", ts->path);
    return err;
  } else {
    t = createTextureFromImageData(img, width, height);
    stbi_image_free(img);
  }

  if (t.id == 0) {
    ERROR("Failed to create texture %s!", ts->path);
    return err;
  }

  return t;
}
