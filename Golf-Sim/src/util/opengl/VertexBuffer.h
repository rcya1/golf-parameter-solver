#pragma once

namespace opengl {
class VertexBuffer {
 public:
  unsigned int id;

  VertexBuffer(unsigned int size, void* data, int stride, int type);
  void free();
  void bind();
  void setVertexAttribute(int index, int size, int type, int offset);

 private:
  int stride;
};
}  // namespace opengl
