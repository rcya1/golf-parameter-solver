#pragma once

namespace opengl {
class IndexBuffer {
 public:
  unsigned int id;

  IndexBuffer(unsigned int size, void* data, int type);
  void free();
  void bind();
};
}  // namespace opengl
