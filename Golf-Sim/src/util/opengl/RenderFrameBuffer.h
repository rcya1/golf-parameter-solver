#pragma once

namespace opengl {
class RenderFrameBuffer {
 public:
  unsigned int frameBufferId;
  unsigned int textureId;
  unsigned int renderBufferId;

  int width;
  int height;

  RenderFrameBuffer(int width, int height);
  void free();
  void bind();
  void unbind();
  void prepareForRender();
  void updateSize(int newWidth, int newHeight);
};
}  // namespace opengl