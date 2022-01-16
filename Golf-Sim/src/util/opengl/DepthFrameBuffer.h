#pragma once

namespace opengl {
class DepthFrameBuffer {
 public:
  unsigned int frameBufferId;
  unsigned int depthMapId;
  int width;
  int height;

  DepthFrameBuffer(int width, int height);
  void free();
  void bind();
  void unbind();
  void prepareForCalculate();
  void bindAsTexture();
};
}  // namespace opengl
